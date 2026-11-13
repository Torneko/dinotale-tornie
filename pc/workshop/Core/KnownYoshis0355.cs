using System.Buffers.Binary;
using System.Text.Json;
namespace YoshiWorkshop;

// Presentation rights only. Discovery survives deaths and differs from current
// eligibility, a saved team, currency purchases and an unfinished egg pickup.
public static class KnownYoshis0355 {
    public const uint OrdinaryMask=0x3F,AllMask=0x3FFFF;
    public readonly record struct Snapshot(uint KnownMask18) {
        public bool Known(int color)=>color is >=0 and <18&&(KnownMask18&(1u<<color))!=0;
        public string Label(int color,string name)=>Known(color)?name:"???";
        public int Initial(int color)=>Known(color)?color:0;
        public int Next(int current,int direction){
            if(direction==0)return Initial(current);
            for(int i=current+Math.Sign(direction);i>=0&&i<18;i+=Math.Sign(direction))if(Known(i))return i;
            return Initial(current);
        }
        public int Last(){for(int i=17;i>=0;i--)if(Known(i))return i;return 0;}
    }
    public static Snapshot ReadProfile(string profile){
        uint known=OrdinaryMask;
        byte[]? eeprom=ReadFile(Path.Combine(profile,"saves","yoshisstory.n64.us.bin"),2048);
        if(eeprom is {Length:2048}){
            for(int copy=0;copy<2;copy++)if(TryEeprom(eeprom.AsSpan(copy*1024,1024),out uint eggs)){known|=eggs<<6;break;}
        }
        known|=Found(profile,"pc-hybrid-finds.json",6)<<8;
        known|=Found(profile,"pc-metal-yoshis.json",2)<<14;
        known|=Found(profile,"pc-elemental-yoshis.json",2)<<16;
        return new(known&AllMask);
    }
    static byte[]? ReadFile(string path,int maximum){
        try{
            if(!File.Exists(path))return null;
            var attributes=File.GetAttributes(path);
            if((attributes&(FileAttributes.Directory|FileAttributes.ReparsePoint))!=0)return null;
            using var input=new FileStream(path,FileMode.Open,FileAccess.Read,FileShare.Read|FileShare.Delete);
            byte[] buffer=new byte[maximum+1];int count=0,read;
            while(count<buffer.Length&&(read=input.Read(buffer,count,buffer.Length-count))>0)count+=read;
            return count>maximum?null:buffer[..count];
        }catch(Exception error)when(error is IOException or UnauthorizedAccessException or ArgumentException or NotSupportedException){return null;}
    }
    static bool Natural(JsonElement root,string name,uint maximum,out uint value){
        value=0;return root.TryGetProperty(name,out var p)&&p.ValueKind==JsonValueKind.Number&&p.TryGetUInt32(out value)&&value<=maximum;
    }
    static uint Found(string profile,string name,int bits){
        var bytes=ReadFile(Path.Combine(profile,name),4096);if(bytes==null)return 0;
        try{
            using var document=JsonDocument.Parse(bytes,new JsonDocumentOptions{MaxDepth=8});var root=document.RootElement;
            if(root.ValueKind!=JsonValueKind.Object)return 0;var keys=root.EnumerateObject().Select(p=>p.Name).ToArray();
            if(keys.Distinct(StringComparer.Ordinal).Count()!=keys.Length||!Natural(root,"foundMask",(1u<<bits)-1,out uint found))return 0;
            if(name=="pc-hybrid-finds.json")return keys.Length==3&&Natural(root,"version",1,out uint version)&&version==1&&root.TryGetProperty("legacyMigrated",out var migrated)&&migrated.ValueKind==JsonValueKind.True?found:0;
            if(!Natural(root,"schema",name=="pc-elemental-yoshis.json"?2u:1u,out uint schema)||schema==0||!Natural(root,"storyAliveMask",3,out uint alive)||(alive&~found)!=0)return 0;
            if(name=="pc-metal-yoshis.json")return keys.Length==3?found:0;
            if(keys.Length!=(schema==1?3:4))return 0;
            if(schema==2&&(!Natural(root,"hybridAliveMask",3,out uint hybridAlive)||(hybridAlive&~found)!=0))return 0;
            return found;
        }catch(Exception error)when(error is JsonException or InvalidOperationException){return 0;}
    }
    static bool TryEeprom(ReadOnlySpan<byte> p,out uint eggs){
        eggs=0;uint magic=BinaryPrimitives.ReadUInt32BigEndian(p[1020..]);
        if(magic is not (0x81317539u or 0x81317538u or 0x81317531u))return false;
        ushort checksum=0;
        for(int i=0;i<1018;i++){checksum^=p[i];checksum=(ushort)((checksum<<1)|(checksum>>15));}
        if(BinaryPrimitives.ReadUInt16BigEndian(p[1018..])!=checksum)return false;
        if(magic==0x81317531)return true;
        Span<byte> meta=stackalloc byte[6];meta.Clear();
        for(int record=0;record<125;record++){
            int offset=6+record*8;ulong encoded=(ulong)(p[offset]&0x7F);
            for(int i=1;i<6;i++)encoded=(encoded<<8)|p[offset+i];
            if(encoded>=0x7FC3A4111000UL)return false;
            if(record<48)meta[record/8]|=(byte)((p[offset]>>7)<<(7-record%8));
        }
        if((meta[3]&0x80)==0)return false;
        if(magic==0x81317538)meta[5]=0xFC;
        uint trial=(uint)(meta[0]<<16|meta[1]<<8|meta[2]);
        uint story=(uint)(p[1]<<16|p[2]<<8|p[3]);
        uint found=(uint)(meta[5]&3);if(found!=0&&trial!=0xFFFFFF)return false;
        uint bits=(uint)(meta[3]<<8|meta[4]);int first=(int)(bits&7);uint used=0;
        if(first!=7){if(first>=6)return false;used=1u<<first;}
        for(int column=0;column<4;column++){
            int color=(int)((bits>>(3+3*column))&7);uint mask=0x111111u<<column;
            bool trialComplete=(trial&mask)==mask,earned=((trial|story)&mask)==mask;
            if(first!=7&&earned){
                if(color==7&&!trialComplete)continue;
                if(color>=6||(used&(1u<<color))!=0)return false;used|=1u<<color;
            }else if(color!=7)return false;
        }
        eggs=found;return true;
    }
}
