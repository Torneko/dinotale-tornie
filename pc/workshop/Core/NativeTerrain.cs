using System.Buffers.Binary;
namespace YoshiWorkshop;

public static class SmsrBlock {
    public static byte[] Decode(ReadOnlySpan<byte> source,int expected){
        if(expected is <1 or >8_388_608 || source.Length<32 || !source[..4].SequenceEqual("CMPR"u8) || !source.Slice(16,6).SequenceEqual("SMSR00"u8) || BinaryPrimitives.ReadUInt32BigEndian(source[24..])!=expected)throw new InvalidDataException("Bloc de décor compressé invalide.");
        int control=32,literal=checked(32+(int)BinaryPrimitives.ReadUInt32BigEndian(source[28..])),written=0,bits=0,flags=0;byte[] output=new byte[expected];
        while(written<expected){
            if(bits==0){if(control+2>source.Length)throw new InvalidDataException("Commandes du décor tronquées.");flags=BinaryPrimitives.ReadUInt16BigEndian(source[control..]);control+=2;bits=16;}
            if((flags&0x8000)!=0){if(literal>=source.Length)throw new InvalidDataException("Pixels du décor tronqués.");output[written++]=source[literal++];}
            else{if(control+2>source.Length)throw new InvalidDataException("Copie du décor tronquée.");int pair=BinaryPrimitives.ReadUInt16BigEndian(source[control..]);control+=2;int count=(pair>>12)+3,distance=(pair&4095)+1;if(distance>written || count>expected-written)throw new InvalidDataException("Copie du décor hors limites.");for(int n=0;n<count;n++){output[written]=output[written-distance];written++;}}
            flags=(flags<<1)&65535;bits--;
        }return output;
    }
}

public sealed class NativeTerrainMap {
    public ushort ResourceId {get;init;}
    public int Width {get;init;}public int Height {get;init;}public int TileWidth {get;init;}public int TileHeight {get;init;}public int PageHeight {get;init;}
    public float WorldX {get;init;}public float WorldY {get;init;}
    public byte[] Pages {get;init;}=[];public int LoadedPages {get;init;}
    public byte[] Pixels {get;init;}=[];public uint[] Palette {get;init;}=[];public ushort[] Tiles {get;init;}=[];public ushort[] Collisions {get;init;}=[];
    // Native tile rows alternate between the vertical 256-pixel pages.
    public int CellIndex(int x,int y){int columns=Width/PageHeight,rowsPerPage=PageHeight/TileHeight,page=Pages[(y/rowsPerPage)*columns+x/rowsPerPage];return page==255?-1:(y%rowsPerPage)*LoadedPages*rowsPerPage+page*rowsPerPage+x%rowsPerPage;}
    public ushort CollisionAt(float x,float y){int tx=(int)MathF.Floor((x-WorldX)/TileWidth),ty=(int)MathF.Floor((y-WorldY)/TileHeight);return tx<0||ty<0||tx>=Width/TileWidth||ty>=Height/TileHeight?(ushort)0:CellIndex(tx,ty) is int cell && cell>=0?Collisions[cell]:(ushort)0;}
    public int[] Rasterize(){
        var result=new int[checked(Width*Height)];int area=TileWidth*TileHeight;var atlas=new int[Pixels.Length];for(int p=0;p<atlas.Length;p++)atlas[p]=unchecked((int)Palette[Pixels[p]]);
        for(int y=0;y<Height/TileHeight;y++)for(int x=0;x<Width/TileWidth;x++){int cell=CellIndex(x,y);if(cell<0)continue;int tile=Tiles[cell];if(tile==65535)continue;for(int row=0;row<TileHeight;row++)Array.Copy(atlas,tile*area+row*TileWidth,result,(y*TileHeight+row)*Width+x*TileWidth,TileWidth);}
        return result;
    }
}
public sealed record NativePassage(ushort ObjectId,int Destination,int NativeStage,float ArrivalX,float ArrivalY);

public sealed partial class VanillaRom {
    Dictionary<ushort,(int A,int B)>? resourceEntries;
    readonly Dictionary<int,byte[]> terrainBlocks=[];
    Dictionary<ushort,(int A,int B)> Resources {
        get{if(resourceEntries!=null)return resourceEntries;var list=new Dictionary<ushort,(int,int)>();for(int i=0;i<1582;i++){int p=0xA1C90+i*12;list.TryAdd(U16(p),(Bank(U32(p+4)),Bank(U32(p+8))));}return resourceEntries=list;}
    }
    byte[] TerrainBlock(uint pointer){
        int info=Bank(pointer);if(terrainBlocks.TryGetValue(info,out var old))return old;int count=checked((int)U32(info)),encoded=checked((int)U32(info+4)),at=Bank(U32(info+8));
        if(count is <1 or >8_388_608 || encoded is <1 or >8_388_608 || at+encoded>data.Length)throw new InvalidDataException("Taille du décor invalide.");
        bool compressed=data.AsSpan(at,4).SequenceEqual("CMPR"u8);if(!compressed && count>encoded)throw new InvalidDataException("Bloc de terrain incomplet.");byte[] block=compressed?SmsrBlock.Decode(data.AsSpan(at,encoded),count):data.AsSpan(at,count).ToArray();terrainBlocks.Add(info,block);return block;
    }
    public IReadOnlyList<NativeTerrainMap> ReadTerrain(int room){
        var result=new List<NativeTerrainMap>();
        foreach(var placement in ReadRoom(room).Objects.Where(p=>p.ObjectId>=0x8000 && p.Layer==1)){
            var entry=Resources[placement.ObjectId];int a=entry.A;if(U16(a)!=0 || U32(a+8)!=0x8006E548 || U32(a+0x4C)==0)continue;
            int d=Bank(U32(a+0x20)),tw=U16(d+4),th=U16(d+6),width=U16(d+8),height=U16(d+10),page=U16(d+2);
            if(tw!=16 || th!=16 || width%tw!=0 || height%th!=0 || page<16 || page%th!=0 || height%page!=0 || width<16 || height<16 || (long)width*height>16_777_216)throw new InvalidDataException("Dimensions du terrain non prises en charge.");
            byte[] pages=TerrainBlock(U32(a+0x40)),animation=U32(a+0x44)!=0?TerrainBlock(U32(a+0x48)):[];
            byte[] pixels=TerrainBlock(U32(a+0x28)),palette=TerrainBlock(U32(a+0x30)),tiles=TerrainBlock(U32(a+0x38)),collision=TerrainBlock(U32(a+0x50));int cells=width/tw*(height/th),tileCount=pixels.Length/(tw*th);
            int loaded=tiles.Length/(2*256);
            if(page!=256 || width%256!=0 || pixels.Length%(tw*th)!=0 || palette.Length!=512 || tiles.Length%512!=0 || loaded<1 || collision.Length!=tiles.Length || pages.Take(width/256*(height/256)).Any(v=>v!=255 && v>=loaded) || animation.Length%10!=0)throw new InvalidDataException("Cartes du terrain incompatibles.");
            int pageCells=width/256*(height/256);var pageGrid=Enumerable.Repeat((byte)255,pageCells).ToArray();Array.Copy(pages,pageGrid,Math.Min(pageCells,pages.Length));
            var colors=new uint[256];for(int i=0;i<256;i++){uint c=BinaryPrimitives.ReadUInt16BigEndian(palette.AsSpan(i*2));colors[i]=((c&1)!=0?0xff000000u:0u)|((c>>11)*255/31<<16)|(((c>>6)&31)*255/31<<8)|((c>>1)&31)*255/31;}
            cells=tiles.Length/2;var map=new ushort[cells];var mask=new ushort[cells];for(int i=0;i<cells;i++){map[i]=BinaryPrimitives.ReadUInt16BigEndian(tiles.AsSpan(i*2));mask[i]=BinaryPrimitives.ReadUInt16BigEndian(collision.AsSpan(i*2));if(map[i]!=65535 && map[i]>=32768){int animated=(map[i]&32767)*10;if(animated+2>animation.Length)throw new InvalidDataException("Animation de terrain invalide.");map[i]=BinaryPrimitives.ReadUInt16BigEndian(animation.AsSpan(animated));if(map[i]==32768)map[i]=65535;}
                if(map[i]!=65535 && map[i]>=tileCount)throw new InvalidDataException("Indice de tuile du terrain invalide.");}
            result.Add(new(){ResourceId=placement.ObjectId,Width=width,Height=height,TileWidth=tw,TileHeight=th,PageHeight=page,WorldX=placement.X-160,WorldY=placement.Y-120,Pages=pageGrid,LoadedPages=loaded,Pixels=pixels,Palette=colors,Tiles=map,Collisions=mask});
        }return result;
    }
    public NativePassage? Passage(ushort id){
        if(!Resources.TryGetValue(id,out var entry))return null;uint pointer=U32(entry.B+0x34);if(pointer==0 || pointer>>28!=0)return null;int record=Bank(pointer);
        if(record+32>data.Length || U32(record)!=32 || data[record+28] is not (7 or 8) || data[record+12]>=177)return null;
        float x=BitConverter.Int32BitsToSingle((int)U32(record+16)),y=BitConverter.Int32BitsToSingle((int)U32(record+20));if(!float.IsFinite(x) || !float.IsFinite(y) || Math.Abs(x)>100000 || Math.Abs(y)>100000)return null;return new(id,data[record+12],data[record+28],x,y);
    }
}
