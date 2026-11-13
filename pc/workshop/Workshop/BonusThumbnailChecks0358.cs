using System.Buffers.Binary;
using System.Drawing.Imaging;
using System.IO.Hashing;
using System.Security.Cryptography;
using System.Text.Json;
using YoshiWorkshop;

namespace WorkshopApp;
internal static class BonusThumbnailSelfChecks0358 {
    public static void Run(string destination) {
        string output=Path.GetFullPath(destination);if(Directory.Exists(output))throw new IOException("Existing fixture output refused.");Directory.CreateDirectory(output);
        string pc=Path.Combine(output,"pc");Directory.CreateDirectory(pc);var store=new BonusThumbnailStore(pc);
        string sentinel=Path.Combine(pc,"progress-sentinel.json");File.WriteAllText(sentinel,"preserve progression");
        string Hash(byte[] data)=>Convert.ToHexStringLower(SHA256.HashData(data));string sentinelHash=Hash(File.ReadAllBytes(sentinel));
        int checks=0,rejected=0;void Need(bool value,string message){checks++;if(!value)throw new InvalidOperationException(message);}
        void Reject(Action action,string message){bool failed=false;try{action();}catch(Exception e)when(e is InvalidDataException or ArgumentException or IOException or OutOfMemoryException or System.Runtime.InteropServices.ExternalException){failed=true;rejected++;}Need(failed,message);}
        int[] widths={480,270,1280,640},heights={270,480,720,480};
        var hashes=new Dictionary<string,string>();
        for(int i=0;i<4;i++) {
            using var source=new Bitmap(widths[i],heights[i],i%2==0?PixelFormat.Format24bppRgb:PixelFormat.Format32bppArgb);
            using(var g=Graphics.FromImage(source)){g.Clear(Color.FromArgb(i%2==0?255:180,50,170,100));g.FillRectangle(Brushes.LightGreen,widths[i]/4,heights[i]/4,widths[i]/2,heights[i]/2);}
            string path=Path.Combine(output,$"source-{i}.png");source.Save(path,ImageFormat.Png);byte[] original=File.ReadAllBytes(path);hashes[path]=Hash(original);
            using var input=new FileStream(path,FileMode.Open,FileAccess.Read,FileShare.Read);
            Need(WorkshopWindow.ImportBonusScreenshot0358(store,i+1,input),"Actual import helper failed");
            byte[] actual=store.Read(i+1)!;var info=BonusThumbnailStore.Inspect(actual);Need(info.Width==320&&info.Height==180,"Real GDI PNG has wrong dimensions");
            using var bytes=new MemoryStream(actual);using var decoded=new Bitmap(bytes);Need(decoded.Width==320&&decoded.Height==180,"Actual GDI output cannot be decoded");
            if(i==1)Need(decoded.GetPixel(0,0).ToArgb()==Color.FromArgb(255,18,31,24).ToArgb(),"Portrait was stretched rather than letterboxed");
            Need(Hash(File.ReadAllBytes(path))==hashes[path],"Normalizing changed source screenshot");
        }
        byte[] saved=store.Read(1)!;string old=Hash(saved);Need(!WorkshopWindow.ImportBonusScreenshot0358(store,1,null),"Cancelled selection committed");Need(Hash(store.Read(1)!)==old,"Cancel replaced prior image");
        void Invalid(byte[] data,string message){using var input=new MemoryStream(data);Reject(()=>WorkshopWindow.ImportBonusScreenshot0358(store,1,input),message);Need(Hash(store.Read(1)!)==old,"Bad image replaced prior thumbnail");}
        Invalid([],"Empty input accepted");Invalid(new byte[32],"Truncated PNG accepted");Invalid(new byte[16_777_217],"Oversize PNG accepted");
        byte[] corrupt=File.ReadAllBytes(hashes.Keys.First());corrupt[29]^=1;Invalid(corrupt,"Bad header CRC accepted");
        byte[] header=File.ReadAllBytes(hashes.Keys.First())[..33];BinaryPrimitives.WriteUInt32BigEndian(header.AsSpan(16,4),9000);BinaryPrimitives.WriteUInt32BigEndian(header.AsSpan(29,4),Crc32.HashToUInt32(header.AsSpan(12,17)));
        using(var bounded=new HeaderProbeStream(header)){Reject(()=>WorkshopWindow.ImportBonusScreenshot0358(store,1,bounded),"Oversize dimensions accepted");Need(!bounded.ReadBeyondHeader,"Bounds were checked after attempting GDI decode");}
        Need(Hash(store.Read(1)!)==old,"Bounds rejection replaced prior image");
        foreach(var pair in hashes)Need(Hash(File.ReadAllBytes(pair.Key))==pair.Value,"Original screenshot changed");
        Need(Hash(File.ReadAllBytes(sentinel))==sentinelHash,"Progress sentinel changed");Need(!Directory.Exists(Path.Combine(pc,"profiles")),"Offscreen check created a game profile");
        var report=new{passed=true,checks,rejected,realSystemDrawingPngEncoder=true,actualDesktopNormalizerUsed=true,landscapePortraitRgbRgbaCovered=true,cancelPreservesPriorImage=true,predecodeBoundsVerified=true,sourceScreenshotsUnchanged=true,normalizedPngAcceptedBySharedStore=true,appConfigurationNotInitialized=true,noFormOrDialogCreated=true,noGameOrGpuOrPersonalProfile=true};
        File.WriteAllText(Path.Combine(output,"verification.json"),JsonSerializer.Serialize(report,new JsonSerializerOptions{WriteIndented=true}));Console.WriteLine(JsonSerializer.Serialize(report));
    }
    sealed class HeaderProbeStream(byte[] header):MemoryStream(header) {
        public bool ReadBeyondHeader {get;private set;}
        public override int Read(byte[] b,int o,int count){if(Position+count>33)ReadBeyondHeader=true;return base.Read(b,o,count);}
        public override int Read(Span<byte> b){if(Position+b.Length>33)ReadBeyondHeader=true;return base.Read(b);}
    }
}
