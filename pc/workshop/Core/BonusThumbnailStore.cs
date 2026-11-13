using System.Buffers.Binary;
using System.IO.Compression;
using System.IO.Hashing;

namespace YoshiWorkshop;

public readonly record struct BonusThumbnailInfo(int Width,int Height,int Bytes);

/// <summary>Optional, user-selected Bonus images. This library never changes game progress.</summary>
public sealed class BonusThumbnailStore {
    public const int MaxBytes=1_048_576;
    public const string RelativeDirectory="content/bonus-thumbnails0358";
    readonly string pcRoot;
    public string DirectoryPath {get;}
    public BonusThumbnailStore(string pcRoot) {
        ArgumentException.ThrowIfNullOrWhiteSpace(pcRoot);
        this.pcRoot=Path.TrimEndingDirectorySeparator(Path.GetFullPath(pcRoot));
        DirectoryPath=Path.Combine(this.pcRoot,"content","bonus-thumbnails0358");
    }
    static void Slot(int bonus){if(bonus is <1 or >6)throw new ArgumentOutOfRangeException(nameof(bonus),"Bonus must be in1..6.");}
    public string ThumbnailPath(int bonus){Slot(bonus);return Path.Combine(DirectoryPath,$"7-{bonus}.png");}
    static void NoLink(string path) {
        var info=new FileInfo(path);
        if(info.LinkTarget!=null || ((File.Exists(path)||Directory.Exists(path))&&(File.GetAttributes(path)&FileAttributes.ReparsePoint)!=0))
            throw new InvalidDataException("A linked thumbnail path is not supported.");
    }
    bool SafeDirectory(bool create) {
        if(!Directory.Exists(pcRoot))throw new DirectoryNotFoundException("The Workshop PC folder does not exist.");
        NoLink(pcRoot);
        string current=pcRoot;
        foreach(string part in new[]{"content","bonus-thumbnails0358"}) {
            current=Path.Combine(current,part);NoLink(current);
            if(File.Exists(current)&&!Directory.Exists(current))throw new InvalidDataException("A thumbnail directory is occupied by a file.");
            if(!Directory.Exists(current)) {if(!create)return false;Directory.CreateDirectory(current);NoLink(current);}
        }
        return true;
    }
    public byte[]? Read(int bonus) {
        string path=ThumbnailPath(bonus);
        if(!SafeDirectory(false))return null;
        NoLink(path);
        if(Directory.Exists(path))throw new InvalidDataException("The thumbnail path is a directory.");
        if(!File.Exists(path))return null;
        using var stream=new FileStream(path,FileMode.Open,FileAccess.Read,FileShare.Read);
        if(stream.Length is <57 or >MaxBytes)throw new InvalidDataException("Bonus PNG exceeds the supported size.");
        byte[] bytes=new byte[(int)stream.Length];stream.ReadExactly(bytes);
        if(stream.ReadByte()!=-1)throw new InvalidDataException("Thumbnail changed while reading.");
        Inspect(bytes);return bytes;
    }
    public void Import(int bonus,ReadOnlySpan<byte> png) {
        string destination=ThumbnailPath(bonus);Inspect(png);
        SafeDirectory(true);NoLink(destination);
        if(Directory.Exists(destination))throw new InvalidDataException("The thumbnail path is a directory.");
        string temporary=Path.Combine(DirectoryPath,$".7-{bonus}-{Guid.NewGuid():N}.tmp");
        try {
            using(var output=new FileStream(temporary,FileMode.CreateNew,FileAccess.Write,FileShare.None,65536,FileOptions.WriteThrough)) {
                output.Write(png);output.Flush(true);
            }
            SafeDirectory(false);NoLink(destination);
            File.Move(temporary,destination,true);
        }
        finally {if(File.Exists(temporary))File.Delete(temporary);}
    }
    static InvalidDataException Invalid()=>new("Use a complete RGB/RGBA8 PNG in320×180 or640×360, at most1MiB.");
    public static BonusThumbnailInfo Inspect(ReadOnlySpan<byte> png) {
        if(png.Length is <57 or >MaxBytes || !png[..8].SequenceEqual(new byte[]{137,80,78,71,13,10,26,10}))throw Invalid();
        int offset=8,width=0,height=0,channels=0,chunks=0;
        bool ihdr=false,idat=false,idatEnded=false,iend=false;
        var ancillary=new HashSet<string>(StringComparer.Ordinal);
        using var compressed=new MemoryStream();
        while(offset<png.Length) {
            if(++chunks>128 || offset>png.Length-12)throw Invalid();
            uint length=BinaryPrimitives.ReadUInt32BigEndian(png.Slice(offset,4));
            if(length>MaxBytes || length>(uint)(png.Length-offset-12))throw Invalid();
            int size=(int)length;var kind=png.Slice(offset+4,4);var data=png.Slice(offset+8,size);
            if(kind.ToArray().Any(v=>v is not(>=65 and <=90)and not(>=97 and <=122)) || kind[2] is >=97 and <=122)throw Invalid();
            uint crc=BinaryPrimitives.ReadUInt32BigEndian(png.Slice(offset+8+size,4));
            if(Crc32.HashToUInt32(png.Slice(offset+4,size+4))!=crc)throw Invalid();
            string type=System.Text.Encoding.ASCII.GetString(kind);
            if(!ihdr&&type!="IHDR")throw Invalid();
            switch(type) {
                case "IHDR":
                    if(ihdr||chunks!=1||size!=13)throw Invalid();
                    uint w=BinaryPrimitives.ReadUInt32BigEndian(data[..4]),h=BinaryPrimitives.ReadUInt32BigEndian(data.Slice(4,4));
                    if(!((w==320&&h==180)||(w==640&&h==360)) || data[8]!=8 || data[9] is not(2 or 6) || data[10]!=0 || data[11]!=0 || data[12]!=0)throw Invalid();
                    width=(int)w;height=(int)h;channels=data[9]==2?3:4;ihdr=true;break;
                case "IDAT":
                    if(idatEnded||iend)throw Invalid();
                    idat=true;compressed.Write(data);break;
                case "IEND":
                    if(!idat||iend||size!=0||offset+12!=png.Length)throw Invalid();
                    iend=true;idatEnded=true;break;
                case "gAMA":case "cHRM":case "sRGB":case "pHYs":
                    if(idat||!ancillary.Add(type))throw Invalid();
                    if(size!=(type=="gAMA"?4:type=="cHRM"?32:type=="sRGB"?1:9))throw Invalid();
                    if(type=="sRGB"&&data[0]>3 || type=="pHYs"&&data[8]>1)throw Invalid();
                    break;
                default:throw Invalid();
            }
            if(idat&&type!="IDAT")idatEnded=true;
            offset+=size+12;
        }
        if(!ihdr||!idat||!iend||compressed.Length==0)throw Invalid();
        int stride=checked(width*channels+1),expected=checked(stride*height);
        byte[] packed=compressed.ToArray();
        if(packed.Length<6 || (packed[0]&15)!=8 || (packed[0]>>4)>7 || ((packed[0]<<8)|packed[1])%31!=0 || (packed[1]&32)!=0)throw Invalid();
        compressed.Position=0;byte[] scanlines=new byte[expected+1];int read=0;
        try {
            using var inflate=new ZLibStream(compressed,CompressionMode.Decompress,true);
            while(read<scanlines.Length) {int n=inflate.Read(scanlines,read,scanlines.Length-read);if(n==0)break;read+=n;}
            if(read!=expected)throw Invalid();
        }
        catch(InvalidDataException){throw Invalid();}
        uint adlerA=1,adlerB=0;
        for(int i=0;i<expected;i++){adlerA=(adlerA+scanlines[i])%65521;adlerB=(adlerB+adlerA)%65521;}
        if(BinaryPrimitives.ReadUInt32BigEndian(packed.AsSpan(packed.Length-4))!=((adlerB<<16)|adlerA))throw Invalid();
        for(int row=0;row<height;row++)if(scanlines[row*stride]>4)throw Invalid();
        return new(width,height,png.Length);
    }
}
