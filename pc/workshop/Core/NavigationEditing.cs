namespace YoshiWorkshop;

public sealed record RoomEntry(float X,float Y);
public sealed record PassageOverride(int DestinationRoom,float? ArrivalX=null,float? ArrivalY=null);

public static class NavigationEditing {
    public static bool HasEdits(LevelProject level)=>level.AllRooms().Any(r=>r.Entry!=null || r.Objects.Any(p=>p.Passage!=null));
    public static void UpdateSchema(LevelProject level)=>level.Schema=HasEdits(level)?5:level.AllRooms().Any(r=>r.MusicId.HasValue)?4:level.AllRooms().Any(r=>r.Terrain.Count>0)?3:level.Rooms.Count>0?2:1;
    public static bool ValidCoordinate(float value)=>float.IsFinite(value) && Math.Abs(value)<=100000;
    public static void ValidateShape(LevelProject level){
        if(HasEdits(level) && level.Schema!=5)throw new InvalidDataException("Les entrées et passages modifiés nécessitent le format 5.");
        foreach(var room in level.AllRooms()){
            if(room.Entry is {} entry && (!ValidCoordinate(entry.X) || !ValidCoordinate(entry.Y)))throw new InvalidDataException("Coordonnées du point d’entrée invalides.");
            foreach(var p in room.Objects.Where(p=>p.Passage!=null)){
                var route=p.Passage!;
                if(route.DestinationRoom is <0 or >=177 || route.ArrivalX.HasValue!=route.ArrivalY.HasValue || route.ArrivalX is float x && !ValidCoordinate(x) || route.ArrivalY is float y && !ValidCoordinate(y))throw new InvalidDataException("Destination ou coordonnées d’arrivée invalides.");
            }
        }
    }
    // Match the native page calculation in 8006DC38. Use this only for an
    // explicit edited entry: ten source roots have deliberately different
    // default framing, which must remain byte-identical without an override.
    public static RoomEntry CalcCameraOrigin(float x,float y){
        if(!ValidCoordinate(x) || !ValidCoordinate(y))throw new InvalidDataException("Point d’entrée hors limites.");
        float px=MathF.Truncate(x/256)*256,py=MathF.Truncate(y/256)*256;
        return new(px==0?0:px-64,py==0?0:py+16);
    }
}

public sealed partial class VanillaRom {
    public static RoomEntry CalcCameraOrigin(float x,float y)=>NavigationEditing.CalcCameraOrigin(x,y);
    static uint NavigationKey(Placement p)=>((uint)p.ObjectId<<16|p.Layer)&0xFFFF03FFu;
    RoomProject NavigationRoom(LevelProject level,int room)=>NativeRooms.Contains(level.SourceZone,room)?level.AllRooms().SingleOrDefault(r=>r.SourceZone==room)??throw new InvalidDataException("Cette salle n’est pas incluse dans la création."):throw new InvalidDataException("Cette salle n’appartient pas au niveau.");
    static bool SamePoint(RoomEntry a,RoomEntry b)=>a.X==b.X && a.Y==b.Y;
    public bool CanEditPassage(Placement p)=>Resources.TryGetValue(p.ObjectId,out var resource) && Resources.TryGetValue(0x4208,out var pot) && resource.A==pot.A && Passage(p.ObjectId) is {NativeStage:7};
    public bool EntryWithinTerrain(int room,float x,float y){
        if(!NavigationEditing.ValidCoordinate(x) || !NavigationEditing.ValidCoordinate(y))return false;
        foreach(var map in ReadTerrain(room)){
            int tx=(int)MathF.Floor((x-map.WorldX)/map.TileWidth),ty=(int)MathF.Floor((y-map.WorldY)/map.TileHeight);
            if(map.TryCell(tx,ty,out _))return true;
        }
        return false;
    }
    void RequireEntryBounds(int room,RoomEntry entry){if(!EntryWithinTerrain(room,entry.X,entry.Y))throw new InvalidDataException("Choisis une arrivée dans une page de terrain existante de la salle de destination.");}
    // The old NativePassage.ArrivalX/Y fields describe the transition record.
    // With a target actor they are not necessarily the player's arrival point.
    // Resolve the actual native target placement instead of inventing (0,0).
    RoomEntry? NativeArrival(Placement source,int destination){
        if(!Resources.TryGetValue(source.ObjectId,out var resource))return null;
        uint pointer=U32(resource.B+0x34);if(pointer==0 || pointer>>28!=0)return null;
        int record=Bank(pointer);if(record+32>data.Length || U32(record)!=32)return null;
        uint target=U32(record+8);
        // A -1 target supplies a camera origin, not a proven player anchor.
        if(target==uint.MaxValue)return null;
        var matches=ReadRoom(destination).Objects.Where(p=>NavigationKey(p)==(target&0xFFFF03FFu)).ToList();
        return matches.Count==1?new(matches[0].X,matches[0].Y):null;
    }
    public RoomEntry EffectiveEntry(LevelProject level,int room){
        var selected=NavigationRoom(level,room);if(selected.Entry is {} custom)return custom;
        var spawn=ReadRoom(room).Objects.Single(p=>p.ObjectId==0x4001);
        if(spawn.X!=0 || spawn.Y!=0)return new(spawn.X,spawn.Y);
        // Stable order from the unmodified complete source, including rooms
        // not present in an older project. Edits never change this fallback.
        foreach(int source in NativeRooms.For(level.SourceZone))foreach(var p in ReadRoom(source).Objects){
            if(Passage(p.ObjectId) is not {NativeStage:7} native || native.Destination!=room)continue;
            if(NativeArrival(p,room) is {} arrival && NavigationEditing.ValidCoordinate(arrival.X) && NavigationEditing.ValidCoordinate(arrival.Y))return arrival;
        }
        throw new InvalidDataException("Aucun point d’arrivée natif fiable n’a été trouvé. Définis d’abord une entrée dans cette salle.");
    }
    RoomEntry? DefaultPassagePoint(LevelProject level,Placement placement,NativePassage native){
        var destination=level.AllRooms().SingleOrDefault(r=>r.SourceZone==native.Destination);
        if(CanEditPassage(placement) && destination?.Entry is {} edited)return edited;
        return NativeRooms.Known(native.Destination)?NativeArrival(placement,native.Destination):null;
    }
    public NativePassage? EffectivePassage(LevelProject level,int room,Placement placement){
        NavigationRoom(level,room);var native=Passage(placement.ObjectId);if(native==null)return null;
        if(placement.Passage is not {} custom){
            var original=DefaultPassagePoint(level,placement,native);
            return original==null?native:native with{ArrivalX=original.X,ArrivalY=original.Y};
        }
        var point=custom.ArrivalX is float x && custom.ArrivalY is float y?new RoomEntry(x,y):EffectiveEntry(level,custom.DestinationRoom);
        return native with{Destination=custom.DestinationRoom,ArrivalX=point.X,ArrivalY=point.Y};
    }
    public void ValidateNavigation(LevelProject level){
        NavigationEditing.ValidateShape(level);var rooms=level.AllRooms().ToList();var present=rooms.Select(r=>r.SourceZone).ToHashSet();
        foreach(var room in rooms){
            NavigationRoom(level,room.SourceZone);var templates=ReadRoom(room.SourceZone).Objects;var original=templates.Single(p=>p.ObjectId==0x4001);var spawn=room.Objects.Single(p=>p.ObjectId==0x4001);
            if(spawn.Layer!=original.Layer)throw new InvalidDataException("Les paramètres natifs du départ de Yoshi doivent être conservés.");
            if(room.Entry is {} entry){if(spawn.X!=entry.X || spawn.Y!=entry.Y)throw new InvalidDataException("Le départ de Yoshi ne correspond pas à l’entrée de la salle.");RequireEntryBounds(room.SourceZone,entry);}
            else if(spawn.X!=original.X || spawn.Y!=original.Y)throw new InvalidDataException("Le départ a été déplacé sans définir une entrée personnalisée. Rétablis son point d’origine.");
            foreach(var p in room.Objects.Where(CanEditPassage)){
                var native=Passage(p.ObjectId)!;
                bool planned=p.Passage!=null || rooms.Any(destination=>destination.SourceZone==native.Destination && destination.Entry!=null);
                if(planned && !templates.Any(source=>source.ObjectId==p.ObjectId && source.Layer==p.Layer))throw new InvalidDataException("Les paramètres natifs du pot de transport doivent être conservés.");
            }
            foreach(var p in room.Objects.Where(p=>p.Passage!=null)){
                if(!CanEditPassage(p) || !SourceObjects(room.SourceZone).Contains(p.ObjectId))throw new InvalidDataException("Seuls les pots de transport compatibles de cette salle sont modifiables.");
                var route=p.Passage!;if(!present.Contains(route.DestinationRoom) || !NativeRooms.Contains(level.SourceZone,route.DestinationRoom))throw new InvalidDataException("La destination doit être une salle de cette création ; les scènes externes et les boss restent natifs.");
                var point=route.ArrivalX is float x && route.ArrivalY is float y?new RoomEntry(x,y):EffectiveEntry(level,route.DestinationRoom);RequireEntryBounds(route.DestinationRoom,point);
            }
            foreach(var group in room.Objects.GroupBy(p=>(p.ObjectId,p.Layer,BitConverter.SingleToInt32Bits(p.X),BitConverter.SingleToInt32Bits(p.Y))))
                if(group.Count()>1 && group.Any(p=>p.Passage!=null))throw new InvalidDataException("Deux passages superposés ne peuvent pas partager des réglages d’arrivée ambigus.");
        }
    }
    public bool SetRoomEntry(LevelProject level,int room,float? x,float? y){
        ProjectStore.Validate(level);var selected=NavigationRoom(level,room);if(x.HasValue!=y.HasValue)throw new InvalidDataException("Renseigne les deux coordonnées de l’entrée.");
        var original=ReadRoom(room).Objects.Single(p=>p.ObjectId==0x4001);var spawn=selected.Objects.Single(p=>p.ObjectId==0x4001);
        RoomEntry? entry=x is float xx && y is float yy?new(xx,yy):null;
        if(entry!=null)RequireEntryBounds(room,entry);
        float sx=entry?.X??original.X,sy=entry?.Y??original.Y;
        if(selected.Entry==entry && spawn.X==sx && spawn.Y==sy && spawn.Layer==original.Layer)return false;
        var old=selected.Entry;float oldX=spawn.X,oldY=spawn.Y;ushort oldLayer=spawn.Layer;int schema=level.Schema;
        void Write(RoomEntry? value){if(room==level.SourceZone)level.Entry=value;else selected.Entry=value;}
        Write(entry);spawn.X=sx;spawn.Y=sy;spawn.Layer=original.Layer;NavigationEditing.UpdateSchema(level);
        try{ProjectStore.Validate(level);ValidateNavigation(level);}catch{Write(old);spawn.X=oldX;spawn.Y=oldY;spawn.Layer=oldLayer;level.Schema=schema;throw;}
        return true;
    }
    public bool SetPassage(LevelProject level,int room,string placementId,int? target,float? x,float? y){
        ProjectStore.Validate(level);var selected=NavigationRoom(level,room);var p=selected.Objects.SingleOrDefault(p=>p.Id==placementId)??throw new InvalidDataException("Passage introuvable dans la salle.");
        if(!CanEditPassage(p))throw new InvalidDataException("Ce passage conserve son comportement natif et n’est pas modifiable.");
        if(x.HasValue!=y.HasValue || !target.HasValue && (x.HasValue || y.HasValue))throw new InvalidDataException("Renseigne une destination et les deux coordonnées d’arrivée, ou laisse les deux coordonnées vides pour suivre l’entrée.");
        PassageOverride? route=target is int destination?new(destination,x,y):null;
        if(route!=null){
            NavigationRoom(level,route.DestinationRoom);var native=Passage(p.ObjectId)!;
            if(route.ArrivalX is float xx && route.ArrivalY is float yy && route.DestinationRoom==native.Destination && DefaultPassagePoint(level,p,native) is {} original && SamePoint(original,new(xx,yy)))route=null;
        }
        if(p.Passage==route)return false;var old=p.Passage;int schema=level.Schema;p.Passage=route;NavigationEditing.UpdateSchema(level);
        try{ProjectStore.Validate(level);ValidateNavigation(level);}catch{p.Passage=old;level.Schema=schema;throw;}
        return true;
    }
}
