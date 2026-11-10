using System.Text.Json;
namespace YoshiWorkshop;

public sealed class ObjectClipboard {
    internal readonly Placement[] Items;
    public int SourceRoot {get;}
    public int SourceRoom {get;}
    public int Count=>Items.Length;
    internal ObjectClipboard(int root,int room,IEnumerable<Placement> objects){SourceRoot=root;SourceRoom=room;Items=objects.Select(p=>p.Copy()).ToArray();}
}

public static class ObjectGroups {
    public const float CoordinateLimit=100000;
    public static bool Editable(Placement p)=>p.ObjectId<0x8000 && p.ObjectId!=0x4001;
    static void Require(IReadOnlyList<Placement> objects){
        if(objects.Count==0 || objects.Count>RoomProject.MaxObjects || objects.Select(p=>p.Id).Distinct().Count()!=objects.Count)throw new InvalidDataException("Sélection d’objets invalide.");
        if(objects.Any(p=>!Editable(p)))throw new InvalidDataException("Le départ de Yoshi et les couches de décor sont protégés.");
        if(objects.Any(p=>!float.IsFinite(p.X) || !float.IsFinite(p.Y) || Math.Abs(p.X)>CoordinateLimit || Math.Abs(p.Y)>CoordinateLimit))throw new InvalidDataException("Coordonnées de sélection invalides.");
    }
    public static (float X,float Y) Center(IReadOnlyList<Placement> objects){Require(objects);return ((objects.Min(p=>p.X)+objects.Max(p=>p.X))/2,(objects.Min(p=>p.Y)+objects.Max(p=>p.Y))/2);}
    public static (float X,float Y) ClampOffset(IReadOnlyList<Placement> objects,float dx,float dy){
        Require(objects);if(!float.IsFinite(dx) || !float.IsFinite(dy))throw new InvalidDataException("Déplacement invalide.");
        return (Math.Clamp(dx,-CoordinateLimit-objects.Min(p=>p.X),CoordinateLimit-objects.Max(p=>p.X)),Math.Clamp(dy,-CoordinateLimit-objects.Min(p=>p.Y),CoordinateLimit-objects.Max(p=>p.Y)));
    }
    public static bool Translate(IReadOnlyList<Placement> objects,float dx,float dy){var delta=ClampOffset(objects,dx,dy);if(delta==(0f,0f))return false;foreach(var p in objects){p.X+=delta.X;p.Y+=delta.Y;}return true;}
    public static ObjectClipboard Copy(IReadOnlyList<Placement> objects,int root,int room){Require(objects);if(!NativeRooms.Contains(root,room))throw new InvalidDataException("Salle d’origine invalide.");return new(root,room,objects);}
    public static List<Placement> PreparePaste(VanillaRom rom,LevelProject level,int room,ObjectClipboard clipboard,float centerX,float centerY){
        ProjectStore.Validate(level);var target=level.AllRooms().SingleOrDefault(r=>r.SourceZone==room)??throw new InvalidDataException("Salle de destination absente.");
        if(target.Objects.Count+clipboard.Count>RoomProject.MaxObjects)throw new InvalidDataException($"Collage impossible : {target.Objects.Count} + {clipboard.Count} objets dépassent la limite de {RoomProject.MaxObjects} dans cette salle.");
        if(level.AllRooms().Sum(r=>r.Objects.Count)+clipboard.Count>6000)throw new InvalidDataException("Le niveau dépasserait sa limite d’objets.");
        var allowed=rom.SourceObjects(room);var incompatible=clipboard.Items.Where(p=>!allowed.Contains(p.ObjectId) && !ObjectCatalog.IsSpecialFruit(p.ObjectId) && !(SpecialEditorObjects.IsEgg(p.ObjectId) && SpecialEditorObjects.Compatible(allowed)) && !(ObjectCatalog.IsSpecialBubble(p.ObjectId) && allowed.Any(ObjectCatalog.IsFruitBubble))).ToList();
        if(incompatible.Count>0)throw new InvalidDataException("Cette salle ne possède pas les ressources de : "+string.Join(", ",incompatible.Select(p=>ObjectCatalog.Get(p.ObjectId).Name).Distinct().Take(4))+". Aucun objet n’a été collé.");
        if((room!=clipboard.SourceRoom || level.SourceZone!=clipboard.SourceRoot) && clipboard.Items.Any(p=>ObjectCatalog.Get(p.ObjectId).Category is "Passages" or "Technique" or "Repères"))throw new InvalidDataException("Les passages et repères techniques se copient uniquement dans leur salle d’origine pour conserver leurs paramètres.");
        var center=Center(clipboard.Items);var copies=clipboard.Items.Select(p=>p.Copy()).ToList();Translate(copies,centerX-center.X,centerY-center.Y);
        // Validate the complete candidate before changing the user's level.
        var candidate=JsonSerializer.Deserialize<LevelProject>(JsonSerializer.Serialize(level,ProjectStore.Json),ProjectStore.Json)!;
        candidate.ObjectsIn(room).AddRange(copies);rom.ExportPlacements(candidate,true);return copies;
    }
}
