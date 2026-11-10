using System.Buffers.Binary;
namespace YoshiWorkshop;

public sealed class RoomProject {
    public const int MaxObjects=149;
    public int SourceZone {get;set;}
    public int? MusicId {get;set;}
    public RoomEntry? Entry {get;set;}
    public List<Placement> Objects {get;set;}=[];
    public List<TerrainCellChange> Terrain {get;set;}=[];
}
public static class RoomProjects {
    public static IEnumerable<RoomProject> AllRooms(this LevelProject level){yield return new(){SourceZone=level.SourceZone,Objects=level.Objects,Terrain=level.Terrain,MusicId=level.MusicId,Entry=level.Entry};foreach(var room in level.Rooms)yield return room;}
    public static List<Placement> ObjectsIn(this LevelProject level,int room)=>room==level.SourceZone?level.Objects:level.Rooms.Single(r=>r.SourceZone==room).Objects;
}
public sealed partial class VanillaRom {
    public RoomProject ReadRoom(int zone){
        if(!NativeRooms.Known(zone))throw new InvalidDataException("Cette salle ne fait pas partie des niveaux éditables.");
        int header=Bank(U32(0xA84A0+zone*8+4)),meta=Bank(U32(Bank(U32(header+0x5C)))),start=Bank(U32(meta+4)),count=U16(meta);
        if(count is <1 or >2000 || start+count*12>data.Length)throw new InvalidDataException("Placements de salle invalides.");
        var room=new RoomProject{SourceZone=zone};
        for(int i=0;i<count;i++){int p=start+i*12;room.Objects.Add(new(){ObjectId=U16(p),Layer=U16(p+2),X=BitConverter.Int32BitsToSingle((int)U32(p+4)),Y=BitConverter.Int32BitsToSingle((int)U32(p+8))});}
        return room;
    }
    public void IncludeOriginalRooms(LevelProject level){
        ProjectStore.Validate(level);
        foreach(int zone in NativeRooms.For(level.SourceZone))if(zone!=level.SourceZone && !level.Rooms.Any(r=>r.SourceZone==zone))level.Rooms.Add(ReadRoom(zone));
        level.Schema=Math.Max(2,level.Schema);ProjectStore.Validate(level);
    }
    // func_8006D85C copies exactly 0x118 bytes, including the preload list.
    int DescriptorLength(int zone){int header=Bank(U32(0xA84A0+zone*8+4));if(header+0x118>data.Length)throw new InvalidDataException("Descripteur de salle invalide.");return 0x118;}
    void CheckCapacity(LevelProject level){int cursor=0;foreach(var r in level.AllRooms()){cursor+=r.Objects.Count*12;cursor=(cursor+15)&~15;cursor+=12;cursor=(cursor+15)&~15;cursor+=DescriptorLength(r.SourceZone);}if(cursor>65536)throw new InvalidDataException("Ce niveau contient trop d’objets pour son chargement. Retire quelques objets avant de le tester.");}
    byte[] ExportRooms(LevelProject level){
        CheckCapacity(level);if(NavigationEditing.HasEdits(level))return ExportNavigationRooms(level);if(level.AllRooms().Any(r=>r.MusicId.HasValue))return ExportMusicRooms(level);if(level.AllRooms().Any(r=>r.Terrain.Count>0))return ExportTerrainRooms(level);var rooms=level.AllRooms().ToList();bool v3=rooms.Count>1,v2=level.Objects.Any(p=>p.HybridVariant.HasValue);int stride=v3||v2?16:12;
        byte[] bytes=new byte[16+(v3?rooms.Count*8:0)+rooms.Sum(r=>r.Objects.Count)*stride];(v3?"YSLEVEL3"u8:v2?"YSLEVEL2"u8:"YSLEVEL1"u8).CopyTo(bytes);BinaryPrimitives.WriteUInt32BigEndian(bytes.AsSpan(8),(uint)level.SourceZone);BinaryPrimitives.WriteUInt32BigEndian(bytes.AsSpan(12),(uint)(v3?rooms.Count:level.Objects.Count));int cursor=16;
        foreach(var room in rooms){if(v3){BinaryPrimitives.WriteUInt32BigEndian(bytes.AsSpan(cursor),(uint)room.SourceZone);BinaryPrimitives.WriteUInt32BigEndian(bytes.AsSpan(cursor+4),(uint)room.Objects.Count);cursor+=8;}
            foreach(var p in room.Objects){var b=bytes.AsSpan(cursor,stride);BinaryPrimitives.WriteUInt16BigEndian(b,p.ObjectId);BinaryPrimitives.WriteUInt16BigEndian(b[2..],p.Layer);BinaryPrimitives.WriteInt32BigEndian(b[4..],BitConverter.SingleToInt32Bits(p.X));BinaryPrimitives.WriteInt32BigEndian(b[8..],BitConverter.SingleToInt32Bits(p.Y));if(stride==16)b[12]=(byte)(p.HybridVariant??255);cursor+=stride;}
        }return bytes;
    }
    byte[] BuildRooms(LevelProject level){
        CheckCapacity(level);byte[] result=(byte[])data.Clone();int cursor=0xF00000;
        void Append(ReadOnlySpan<byte> bytes){if(cursor+bytes.Length>result.Length || result.AsSpan(cursor,bytes.Length).IndexOfAnyExcept((byte)255)>=0)throw new InvalidDataException("Espace de données custom insuffisant.");bytes.CopyTo(result.AsSpan(cursor));cursor+=bytes.Length;}
        uint Ptr(int p)=>0x04000000u|(uint)(p-banks[4]);
        foreach(var room in level.AllRooms()){
            int objects=cursor;byte[] item=new byte[12];foreach(var p in room.Objects){Span<byte> b=item;BinaryPrimitives.WriteUInt16BigEndian(b,p.ObjectId==0x45E3?(ushort)0x45E5:p.ObjectId);BinaryPrimitives.WriteUInt16BigEndian(b[2..],p.Layer);BinaryPrimitives.WriteInt32BigEndian(b[4..],BitConverter.SingleToInt32Bits(p.X));BinaryPrimitives.WriteInt32BigEndian(b[8..],BitConverter.SingleToInt32Bits(p.Y));Append(b);}
            cursor=(cursor+15)&~15;int meta=cursor;byte[] m=new byte[12];BinaryPrimitives.WriteUInt16BigEndian(m,(ushort)room.Objects.Count);BinaryPrimitives.WriteUInt32BigEndian(m.AsSpan(4),Ptr(objects));BinaryPrimitives.WriteUInt32BigEndian(m.AsSpan(8),Ptr(meta));Append(m);
            cursor=(cursor+15)&~15;int entry=0xA84A0+room.SourceZone*8,original=Bank(U32(entry+4)),header=cursor;byte[] descriptor=data.AsSpan(original,DescriptorLength(room.SourceZone)).ToArray();BinaryPrimitives.WriteUInt32BigEndian(descriptor.AsSpan(0x5C),Ptr(meta+8));if(room.MusicId is int music)BinaryPrimitives.WriteInt32BigEndian(descriptor.AsSpan(RoomMusic.DescriptorOffset),music);if(room.Entry is not null){var origin=CalcCameraOrigin(room.Entry.X,room.Entry.Y);BinaryPrimitives.WriteInt32BigEndian(descriptor.AsSpan(0x28),BitConverter.SingleToInt32Bits(origin.X));BinaryPrimitives.WriteInt32BigEndian(descriptor.AsSpan(0x2C),BitConverter.SingleToInt32Bits(origin.Y));}Append(descriptor);BinaryPrimitives.WriteUInt32BigEndian(result.AsSpan(entry+4),Ptr(header));
        }
        if(!result.AsSpan(0x1000,0xA1C90-0x1000).SequenceEqual(data.AsSpan(0x1000,0xA1C90-0x1000)) || !result.AsSpan(0xB6BCC0,0xEBDA70-0xB6BCC0).SequenceEqual(data.AsSpan(0xB6BCC0,0xEBDA70-0xB6BCC0)))throw new InvalidDataException("Code natif modifié.");return BuildTerrainData(level,result);
    }
}
