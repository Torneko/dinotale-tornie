using System.Buffers.Binary;
namespace YoshiWorkshop;

public sealed record RoomMusicTrack(int Id,string Label) {
    public override string ToString()=>Label;
}
public static class RoomMusic {
    // func_800688CC reads room-state+0x104; the LevelInfo descriptor starts at +4.
    // func_800679B0 forwards that scene parameter to func_80010730 (native BGM).
    public const int DescriptorOffset=0x100;
    public static readonly IReadOnlyList<RoomMusicTrack> Tracks=Array.AsReadOnly(new RoomMusicTrack[]{
        new(1,"Thème du 1-1 / 1-2"),new(2,"Thème du 1-4"),
        new(3,"Thème du 1-3 / 3-1"),new(4,"Thème du 2-1 / 2-2"),new(5,"Thème du 3-2"),
        new(6,"Thème du 4-2 / 4-4"),new(7,"Thème du 4-1"),new(8,"Thème du 2-3 / 2-4"),
        new(9,"Thème du 5-3"),new(10,"Thème du monde 6"),new(11,"Thème du 3-3 / 3-4"),
        new(12,"Thème du 5-2"),new(13,"Thème du 4-3"),new(14,"Salles bonus du 2-1 / 2-3"),
        new(15,"Salle bonus du 4-1"),new(16,"Salles bonus du 1-4"),new(54,"Salles intérieures du 6-2 / 6-3")
    });
    public static bool Valid(int id)=>id is >=1 and <=16 or 54;
    public static int? Selected(LevelProject level,int room)=>room==level.SourceZone?level.MusicId:level.Rooms.Single(r=>r.SourceZone==room).MusicId;
    public static void Validate(LevelProject level){
        var rooms=level.AllRooms().ToList();
        if(rooms.Any(r=>r.MusicId is int id && !Valid(id)) || rooms.Any(r=>r.MusicId.HasValue) && level.Schema is not (4 or 5))
            throw new InvalidDataException("Musique de salle invalide ou format incompatible.");
    }
    public static void UpdateSchema(LevelProject level){
        NavigationEditing.UpdateSchema(level);
    }
}
public sealed partial class VanillaRom {
    public int ReadRoomMusic(int room){if(!NativeRooms.Known(room))throw new InvalidDataException("Salle source inconnue.");int header=Bank(U32(0xA84A0+room*8+4));return checked((int)U32(header+RoomMusic.DescriptorOffset));}
    public bool SetRoomMusic(LevelProject level,int room,int? musicId){
        ProjectStore.Validate(level);if(!NativeRooms.Contains(level.SourceZone,room) || !level.AllRooms().Any(r=>r.SourceZone==room))throw new InvalidDataException("Salle musicale absente du niveau.");
        if(musicId is int value && !RoomMusic.Valid(value))throw new InvalidDataException("Choisir une musique disponible dans la liste.");
        // Choosing the native theme is the same as inheriting its unchanged descriptor.
        if(musicId==ReadRoomMusic(room))musicId=null;
        if(RoomMusic.Selected(level,room)==musicId)return false;
        if(room==level.SourceZone)level.MusicId=musicId;else level.Rooms.Single(r=>r.SourceZone==room).MusicId=musicId;
        RoomMusic.UpdateSchema(level);return true;
    }
    byte[] ExportMusicRooms(LevelProject level){
        var rooms=level.AllRooms().ToList();byte[] bytes=new byte[16+rooms.Count*16+rooms.Sum(r=>r.Objects.Count*16+r.Terrain.Count*8)];"YSLEVEL5"u8.CopyTo(bytes);
        BinaryPrimitives.WriteInt32BigEndian(bytes.AsSpan(8),level.SourceZone);BinaryPrimitives.WriteInt32BigEndian(bytes.AsSpan(12),rooms.Count);int at=16;
        foreach(var room in rooms){
            BinaryPrimitives.WriteInt32BigEndian(bytes.AsSpan(at),room.SourceZone);BinaryPrimitives.WriteInt32BigEndian(bytes.AsSpan(at+4),room.Objects.Count);BinaryPrimitives.WriteInt32BigEndian(bytes.AsSpan(at+8),room.Terrain.Count);BinaryPrimitives.WriteInt32BigEndian(bytes.AsSpan(at+12),room.MusicId??-1);at+=16;
            foreach(var p in room.Objects){var b=bytes.AsSpan(at,16);BinaryPrimitives.WriteUInt16BigEndian(b,p.ObjectId);BinaryPrimitives.WriteUInt16BigEndian(b[2..],p.Layer);BinaryPrimitives.WriteInt32BigEndian(b[4..],BitConverter.SingleToInt32Bits(p.X));BinaryPrimitives.WriteInt32BigEndian(b[8..],BitConverter.SingleToInt32Bits(p.Y));b[12]=(byte)(p.HybridVariant??255);at+=16;}
            foreach(var c in room.Terrain.OrderBy(c=>c.ResourceId).ThenBy(c=>c.Cell)){var b=bytes.AsSpan(at,8);BinaryPrimitives.WriteUInt16BigEndian(b,c.ResourceId);BinaryPrimitives.WriteUInt16BigEndian(b[2..],(ushort)c.Cell);BinaryPrimitives.WriteUInt16BigEndian(b[4..],c.Tile);BinaryPrimitives.WriteUInt16BigEndian(b[6..],c.Collision);at+=8;}
        }return bytes;
    }
}
