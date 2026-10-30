using System.Buffers.Binary;
namespace YoshiWorkshop;
// BPS1: byuu's public-domain specification, https://github.com/blakesmith/rombp/blob/master/docs/bps_spec.md
public static class BpsPatch {
    static readonly uint[] table=Enumerable.Range(0,256).Select(i=>{uint c=(uint)i;for(int n=0;n<8;n++)c=(c&1)!=0?0xedb88320u^(c>>1):c>>1;return c;}).ToArray();
    public static uint Crc(ReadOnlySpan<byte> data){uint c=0xffffffff;foreach(byte b in data)c=table[(c^b)&255]^(c>>8);return ~c;}
    public static byte[] Apply(byte[] source,byte[] patch){try{return ApplyChecked(source,patch);}catch(OverflowException e){throw new InvalidDataException("Le patch contient une taille invalide.",e);}}
    static byte[] ApplyChecked(byte[] source,byte[] patch){
        void Require(bool ok){if(!ok)throw new InvalidDataException("Le patch est incomplet ou ne correspond pas à cette ROM.");}
        Require(patch.Length is >=19 and <=4_194_304 && patch.AsSpan(0,4).SequenceEqual("BPS1"u8));int end=patch.Length-12,pos=4;
        uint Footer(int n)=>BinaryPrimitives.ReadUInt32LittleEndian(patch.AsSpan(end+n,4));Require(Crc(patch.AsSpan(0,patch.Length-4))==Footer(8) && Crc(source)==Footer(0));
        ulong Number(){ulong value=0,shift=1;for(int i=0;i<10;i++){Require(pos<end);byte b=patch[pos++];value=checked(value+(ulong)(b&127)*shift);if((b&128)!=0)return value;shift=checked(shift*128);value=checked(value+shift);}throw new InvalidDataException("Nombre BPS invalide.");}
        ulong inputSize=Number(),outputSize=Number(),metadata=Number();Require(inputSize==(ulong)source.Length && outputSize<=33_554_432 && metadata<=(ulong)(end-pos));pos+=checked((int)metadata);byte[] output=new byte[checked((int)outputSize)];int written=0;long sourceRelative=0,targetRelative=0;
        while(pos<end){ulong command=Number();int count=checked((int)(command>>2)+1);Require(count<=output.Length-written);
            switch(command&3){
                case 0:Require(written<=source.Length && count<=source.Length-written);source.AsSpan(written,count).CopyTo(output.AsSpan(written));written+=count;break;
                case 1:Require(count<=end-pos);patch.AsSpan(pos,count).CopyTo(output.AsSpan(written));pos+=count;written+=count;break;
                case 2:{ulong move=Number();long distance=checked((long)(move>>1));sourceRelative=checked(sourceRelative+((move&1)!=0?-distance:distance));Require(sourceRelative>=0 && sourceRelative<=source.Length && count<=source.Length-sourceRelative);source.AsSpan((int)sourceRelative,count).CopyTo(output.AsSpan(written));sourceRelative+=count;written+=count;break;}
                case 3:{ulong move=Number();long distance=checked((long)(move>>1));targetRelative=checked(targetRelative+((move&1)!=0?-distance:distance));for(int n=0;n<count;n++){Require(targetRelative>=0 && targetRelative<written);output[written++]=output[(int)targetRelative++];}break;}
            }
        }Require(pos==end && written==output.Length && Crc(output)==Footer(4));return output;
    }
}
