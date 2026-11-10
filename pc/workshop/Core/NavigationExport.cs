using System.Buffers.Binary;
namespace YoshiWorkshop;
public sealed partial class VanillaRom {
    byte[] ExportNavigationRooms(LevelProject level){
        var rooms=level.AllRooms().ToList();
        byte[] bytes=new byte[16+rooms.Sum(r=>32+r.Objects.Count*16+r.Terrain.Count*8+r.Objects.Count(p=>p.Passage!=null)*20)];
        "YSLEVEL6"u8.CopyTo(bytes);BinaryPrimitives.WriteInt32BigEndian(bytes.AsSpan(8),level.SourceZone);BinaryPrimitives.WriteInt32BigEndian(bytes.AsSpan(12),rooms.Count);int at=16;
        void Word(int p,int n)=>BinaryPrimitives.WriteInt32BigEndian(bytes.AsSpan(p),n);
        foreach(var room in rooms){
            Word(at,room.SourceZone);Word(at+4,room.Objects.Count);Word(at+8,room.Terrain.Count);Word(at+12,room.MusicId??-1);Word(at+16,room.Entry==null?0:1);
            Word(at+20,room.Entry==null?0:BitConverter.SingleToInt32Bits(room.Entry.X));Word(at+24,room.Entry==null?0:BitConverter.SingleToInt32Bits(room.Entry.Y));Word(at+28,room.Objects.Count(p=>p.Passage!=null));at+=32;
            foreach(var p in room.Objects){var b=bytes.AsSpan(at,16);BinaryPrimitives.WriteUInt16BigEndian(b,p.ObjectId);BinaryPrimitives.WriteUInt16BigEndian(b[2..],p.Layer);Word(at+4,BitConverter.SingleToInt32Bits(p.X));Word(at+8,BitConverter.SingleToInt32Bits(p.Y));b[12]=(byte)(p.HybridVariant??255);at+=16;}
            foreach(var c in room.Terrain.OrderBy(c=>c.ResourceId).ThenBy(c=>c.Cell)){var b=bytes.AsSpan(at,8);BinaryPrimitives.WriteUInt16BigEndian(b,c.ResourceId);BinaryPrimitives.WriteUInt16BigEndian(b[2..],(ushort)c.Cell);BinaryPrimitives.WriteUInt16BigEndian(b[4..],c.Tile);BinaryPrimitives.WriteUInt16BigEndian(b[6..],c.Collision);at+=8;}
            for(int i=0;i<room.Objects.Count;i++)if(room.Objects[i].Passage is PassageOverride route){Word(at,i);Word(at+4,route.DestinationRoom);Word(at+8,route.ArrivalX.HasValue?1:0);Word(at+12,route.ArrivalX.HasValue?BitConverter.SingleToInt32Bits(route.ArrivalX.Value):0);Word(at+16,route.ArrivalY.HasValue?BitConverter.SingleToInt32Bits(route.ArrivalY.Value):0);at+=20;}
        }return bytes;
    }
}
