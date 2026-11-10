using System.Buffers.Binary;
namespace YoshiWorkshop;

public sealed class TerrainCellChange {
    public ushort ResourceId {get;set;}
    public int Cell {get;set;}
    public ushort Tile {get;set;}
    public ushort Collision {get;set;}
}
public static class TerrainEditing {
    public const int MaxChanges=196608;
    public static void ValidateStructure(LevelProject level){
        var rooms=level.AllRooms().ToList();int count=rooms.Sum(r=>r.Terrain.Count);
        if(count>MaxChanges || count>0 && level.Schema is not (3 or 4 or 5))throw new InvalidDataException("Format de terrain invalide ou trop de modifications.");
        foreach(var room in rooms){
            if(room.Terrain.Count>65536 || room.Terrain.Any(t=>t.ResourceId<0x8000 || t.Cell<0 || t.Cell>65535) || room.Terrain.Select(t=>(t.ResourceId,t.Cell)).Distinct().Count()!=room.Terrain.Count)throw new InvalidDataException("Tuile dupliquée ou modification de terrain hors limites.");
        }
    }
    public static HashSet<int> ReferencedCells(NativeTerrainMap map){var cells=new HashSet<int>();for(int y=0;y<map.Height/16;y++)for(int x=0;x<map.Width/16;x++)if(map.TryCell(x,y,out int index))cells.Add(index);return cells;}
    public static void ValidateChanges(NativeTerrainMap original,IEnumerable<TerrainCellChange> changes){
        var cells=ReferencedCells(original);var masks=original.Collisions.Append((ushort)0).ToHashSet();
        foreach(var c in changes)if(c.ResourceId!=original.ResourceId || !cells.Contains(c.Cell) || !original.ValidTile(c.Tile) || !masks.Contains(c.Collision))throw new InvalidDataException("La tuile ou sa collision ne correspond pas aux ressources de cette salle.");
    }
}
public sealed partial class VanillaRom {
    public void ValidateTerrain(LevelProject level){
        TerrainEditing.ValidateStructure(level);
        foreach(var room in level.AllRooms().Where(r=>r.Terrain.Count>0)){
            var maps=ReadTerrain(room.SourceZone).ToDictionary(m=>m.ResourceId);
            foreach(var changes in room.Terrain.GroupBy(c=>c.ResourceId)){
                if(!maps.TryGetValue(changes.Key,out var map) || room.Objects.Count(p=>p.ObjectId==changes.Key && p.Layer==1)!=1)throw new InvalidDataException("Le terrain d’origine doit rester présent dans sa salle.");
                var placed=room.Objects.Single(p=>p.ObjectId==changes.Key && p.Layer==1);var source=ReadRoom(room.SourceZone).Objects.Single(p=>p.ObjectId==changes.Key && p.Layer==1);if(placed.X!=source.X || placed.Y!=source.Y)throw new InvalidDataException("Conserver la position d’origine de la couche de terrain.");
                TerrainEditing.ValidateChanges(map,changes);
            }
        }
    }
    public bool SetTerrainCells(LevelProject level,int roomId,ushort resource,IReadOnlyDictionary<int,(ushort Tile,ushort Collision)> values){
        var room=level.AllRooms().Single(r=>r.SourceZone==roomId);var map=ReadTerrain(roomId).Single(m=>m.ResourceId==resource);
        var proposed=values.Select(v=>new TerrainCellChange{ResourceId=resource,Cell=v.Key,Tile=v.Value.Tile,Collision=v.Value.Collision}).ToList();TerrainEditing.ValidateChanges(map,proposed);
        var next=room.Terrain.ToDictionary(c=>(c.ResourceId,c.Cell));bool changed=false;
        foreach(var c in proposed){bool baseline=c.Tile==map.RawTiles[c.Cell] && c.Collision==map.Collisions[c.Cell];
            if(baseline){changed|=next.Remove((resource,c.Cell));continue;}
            if(next.TryGetValue((resource,c.Cell),out var old) && old.Tile==c.Tile && old.Collision==c.Collision)continue;
            next[(resource,c.Cell)]=c;changed=true;
        }
        if(!changed)return false;if(next.Count>65536 || level.AllRooms().Sum(r=>r.Terrain.Count)-room.Terrain.Count+next.Count>TerrainEditing.MaxChanges)throw new InvalidDataException("Ce niveau a atteint sa limite de tuiles modifiées.");
        room.Terrain.Clear();room.Terrain.AddRange(next.Values.OrderBy(c=>c.ResourceId).ThenBy(c=>c.Cell));NavigationEditing.UpdateSchema(level);return true;
    }
    byte[] ExportTerrainRooms(LevelProject level){
        var rooms=level.AllRooms().ToList();byte[] bytes=new byte[16+rooms.Count*12+rooms.Sum(r=>r.Objects.Count*16+r.Terrain.Count*8)];"YSLEVEL4"u8.CopyTo(bytes);BinaryPrimitives.WriteInt32BigEndian(bytes.AsSpan(8),level.SourceZone);BinaryPrimitives.WriteInt32BigEndian(bytes.AsSpan(12),rooms.Count);int at=16;
        foreach(var room in rooms){BinaryPrimitives.WriteInt32BigEndian(bytes.AsSpan(at),room.SourceZone);BinaryPrimitives.WriteInt32BigEndian(bytes.AsSpan(at+4),room.Objects.Count);BinaryPrimitives.WriteInt32BigEndian(bytes.AsSpan(at+8),room.Terrain.Count);at+=12;
            foreach(var p in room.Objects){var b=bytes.AsSpan(at,16);BinaryPrimitives.WriteUInt16BigEndian(b,p.ObjectId);BinaryPrimitives.WriteUInt16BigEndian(b[2..],p.Layer);BinaryPrimitives.WriteInt32BigEndian(b[4..],BitConverter.SingleToInt32Bits(p.X));BinaryPrimitives.WriteInt32BigEndian(b[8..],BitConverter.SingleToInt32Bits(p.Y));b[12]=(byte)(p.HybridVariant??255);at+=16;}
            foreach(var c in room.Terrain.OrderBy(c=>c.ResourceId).ThenBy(c=>c.Cell)){var b=bytes.AsSpan(at,8);BinaryPrimitives.WriteUInt16BigEndian(b,c.ResourceId);BinaryPrimitives.WriteUInt16BigEndian(b[2..],(ushort)c.Cell);BinaryPrimitives.WriteUInt16BigEndian(b[4..],c.Tile);BinaryPrimitives.WriteUInt16BigEndian(b[6..],c.Collision);at+=8;}
        }return bytes;
    }
    byte[] BuildTerrainData(LevelProject level,byte[] placements){
        if(!level.AllRooms().Any(r=>r.Terrain.Count>0))return placements;
        if(U32(0xA79D4+15*4)!=0)throw new InvalidDataException("Banque de terrain personnalisée indisponible.");
        using var stream=new MemoryStream(placements.Length+4*1024*1024);stream.Write(placements);
        uint Ptr(int offset)=>0x0F000000u|(uint)(offset-0x1000000);
        int Append(byte[] bytes){stream.Position=(stream.Length+15)&~15L;int at=checked((int)stream.Position);if(at+bytes.Length>0x1400000)throw new InvalidDataException("Le terrain dépasse l’espace de cette version.");stream.Write(bytes);return at;}
        int Block(ushort[] words){byte[] bytes=new byte[words.Length*2];for(int i=0;i<words.Length;i++)BinaryPrimitives.WriteUInt16BigEndian(bytes.AsSpan(i*2),words[i]);int raw=Append(bytes);byte[] info=new byte[12];BinaryPrimitives.WriteInt32BigEndian(info,bytes.Length);BinaryPrimitives.WriteInt32BigEndian(info.AsSpan(4),bytes.Length);BinaryPrimitives.WriteUInt32BigEndian(info.AsSpan(8),Ptr(raw));return Append(info);}
        var updates=new Dictionary<ushort,uint>();
        foreach(var room in level.AllRooms().Where(r=>r.Terrain.Count>0))foreach(var original in ReadTerrain(room.SourceZone)){
            var changes=room.Terrain.Where(c=>c.ResourceId==original.ResourceId).ToList();if(changes.Count==0)continue;var preview=original.Preview(changes);int tiles=Block(preview.RawTiles),collision=Block(preview.Collisions);byte[] a=data.AsSpan(Resources[original.ResourceId].A,0x64).ToArray();BinaryPrimitives.WriteUInt32BigEndian(a.AsSpan(0x38),Ptr(tiles));BinaryPrimitives.WriteUInt32BigEndian(a.AsSpan(0x50),Ptr(collision));if(!updates.TryAdd(original.ResourceId,Ptr(Append(a))))throw new InvalidDataException("Ressource de terrain partagée entre deux salles.");
        }
        byte[] result=stream.ToArray();BinaryPrimitives.WriteInt32BigEndian(result.AsSpan(0xA79D4+60),0x1000000);
        for(int i=0;i<1582;i++){int entry=0xA1C90+i*12;if(updates.TryGetValue(U16(entry),out uint pointer))BinaryPrimitives.WriteUInt32BigEndian(result.AsSpan(entry+4),pointer);}return result;
    }
}
