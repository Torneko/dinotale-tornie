using System.Buffers.Binary;
using System.IO.Compression;
using System.IO.Hashing;
using System.Security.Cryptography;
using System.Text;
using System.Text.Json;
using YoshiWorkshop;

if(args.Length!=1)throw new ArgumentException("Supply a new isolated fixture directory.");
string output=Path.GetFullPath(args[0]);if(Directory.Exists(output))throw new IOException("Existing output refused.");Directory.CreateDirectory(output);
string pc=Path.Combine(output,"pc");Directory.CreateDirectory(pc);var store=new BonusThumbnailStore(pc);
int checks=0,rejected=0,symlinkCases=0,symlinkUnavailable=0,junctionCases=0;
void Need(bool value,string message){checks++;if(!value)throw new Exception(message);}
void Reject(Action action,string message){bool refused=false;try{action();}catch(Exception e)when(e is InvalidDataException or ArgumentOutOfRangeException or IOException or UnauthorizedAccessException){refused=true;rejected++;}Need(refused,message);}
string Hash(byte[] bytes)=>Convert.ToHexStringLower(SHA256.HashData(bytes));
byte[] Chunk(string type,byte[] data){byte[] b=new byte[data.Length+12];BinaryPrimitives.WriteUInt32BigEndian(b,(uint)data.Length);Encoding.ASCII.GetBytes(type).CopyTo(b,4);data.CopyTo(b,8);BinaryPrimitives.WriteUInt32BigEndian(b.AsSpan(8+data.Length),Crc32.HashToUInt32(b.AsSpan(4,data.Length+4)));return b;}
byte[] Header(int width,int height,int type=6){byte[] h=new byte[13];BinaryPrimitives.WriteUInt32BigEndian(h,(uint)width);BinaryPrimitives.WriteUInt32BigEndian(h.AsSpan(4),(uint)height);h[8]=8;h[9]=(byte)type;return h;}
byte[] Packed(int width,int height,int type=6,int filter=0,int rowsDelta=0){int channels=type==2?3:4,stride=width*channels+1;byte[] pixels=new byte[stride*(height+rowsDelta)];for(int row=0;row<height+rowsDelta;row++){pixels[row*stride]=(byte)filter;for(int x=1;x<stride;x++)pixels[row*stride+x]=(byte)(x*17+row*3);}using var m=new MemoryStream();using(var z=new ZLibStream(m,CompressionLevel.SmallestSize,true))z.Write(pixels);return m.ToArray();}
byte[] PngParts(params byte[][] chunks){using var m=new MemoryStream();m.Write(new byte[]{137,80,78,71,13,10,26,10});foreach(byte[] b in chunks)m.Write(b);return m.ToArray();}
byte[] Png(int width=320,int height=180,int type=6,int filter=0,int delta=0)=>PngParts(Chunk("IHDR",Header(width,height,type)),Chunk("IDAT",Packed(width,height,type,filter,delta)),Chunk("IEND",[]));
byte[] a=Png(),b=Png(640,360,2);
Need(store.Read(1)==null&&!Directory.Exists(store.DirectoryPath),"Read created an image library");
string sentinel=Path.Combine(pc,"progress-never-written.json");File.WriteAllText(sentinel,"user progress");string sentinelHash=Hash(File.ReadAllBytes(sentinel));
foreach(int slot in Enumerable.Range(1,6)){store.Import(slot,a);Need(store.Read(slot)!.AsSpan().SequenceEqual(a),"Per-slot image changed");Need(Path.GetFileName(store.ThumbnailPath(slot))==$"7-{slot}.png","Wrong fixed slot filename");}
Need(BonusThumbnailStore.Inspect(a)==new BonusThumbnailInfo(320,180,a.Length),"RGBA PNG not decoded");Need(BonusThumbnailStore.Inspect(b)==new BonusThumbnailInfo(640,360,b.Length),"RGB640 PNG not decoded");
store.Import(1,b);Need(store.Read(1)!.AsSpan().SequenceEqual(b),"Image replacement failed");Need(store.Read(2)!.AsSpan().SequenceEqual(a),"Replacement touched a different slot");
foreach(int slot in new[]{int.MinValue,-1,0,7,100,int.MaxValue}){Reject(()=>store.Import(slot,a),"Invalid slot accepted");Reject(()=>store.Read(slot),"Invalid read slot accepted");}
string beforeHash=Hash(store.Read(1)!);
void Invalid(byte[] raw,string message){Reject(()=>store.Import(1,raw),message);Need(Hash(store.Read(1)!)==beforeHash,"Rejected image replaced valid thumbnail");Need(!Directory.GetFiles(store.DirectoryPath,"*.tmp").Any(),"Rejected image left a partial file");}
Invalid([],"Empty file accepted");Invalid(a[..20],"Truncated file accepted");Invalid(new byte[BonusThumbnailStore.MaxBytes+1],"Oversize file accepted");
byte[] bad=a.ToArray();bad[0]=0;Invalid(bad,"Bad signature accepted");bad=a.ToArray();bad[29]^=1;Invalid(bad,"CRC corruption accepted");
Invalid(a.Concat(new byte[]{1}).ToArray(),"Trailing data accepted");
Invalid(PngParts(Chunk("IHDR",Header(321,180)),Chunk("IDAT",Packed(320,180)),Chunk("IEND",[])),"Wrong dimensions accepted");
Invalid(PngParts(Chunk("IHDR",Header(int.MaxValue,180)),Chunk("IDAT",Packed(320,180)),Chunk("IEND",[])),"Huge dimensions accepted");
byte[] header=Header(320,180);header[8]=16;Invalid(PngParts(Chunk("IHDR",header),Chunk("IDAT",Packed(320,180)),Chunk("IEND",[])),"16-bit PNG accepted");header=Header(320,180);header[12]=1;Invalid(PngParts(Chunk("IHDR",header),Chunk("IDAT",Packed(320,180)),Chunk("IEND",[])),"Interlaced PNG accepted");
header=Header(320,180,3);Invalid(PngParts(Chunk("IHDR",header),Chunk("IDAT",Packed(320,180)),Chunk("IEND",[])),"Palette PNG accepted");
Invalid(Png(filter:5),"Unknown PNG filter accepted");Invalid(Png(delta:-1),"Missing scanline accepted");Invalid(Png(delta:1),"Extra scanline accepted");
byte[] packed=Packed(320,180);packed[^1]^=1;Invalid(PngParts(Chunk("IHDR",Header(320,180)),Chunk("IDAT",packed),Chunk("IEND",[])),"Broken zlib trailer accepted");
packed=Packed(320,180);Invalid(PngParts(Chunk("IHDR",Header(320,180)),Chunk("IDAT",packed[..^4]),Chunk("IEND",[])),"Missing zlib trailer accepted");
Invalid(PngParts(Chunk("IHDR",Header(320,180)),Chunk("IHDR",Header(320,180)),Chunk("IDAT",Packed(320,180)),Chunk("IEND",[])),"Duplicate header accepted");
Invalid(PngParts(Chunk("IHDR",Header(320,180)),Chunk("tEXt",Encoding.UTF8.GetBytes("private metadata")),Chunk("IDAT",Packed(320,180)),Chunk("IEND",[])),"Metadata from unnormalized input accepted");
Invalid(PngParts(Chunk("IHDR",Header(320,180)),Chunk("IDAT",packed),Chunk("gAMA",new byte[4]),Chunk("IDAT",[]),Chunk("IEND",[])),"Interrupted IDAT accepted");
Invalid(PngParts(Chunk("IHDR",Header(320,180)),Chunk("IDAT",packed)),"Missing end chunk accepted");
byte[] normalized=PngParts(Chunk("IHDR",Header(320,180)),Chunk("gAMA",new byte[]{0,0,177,143}),Chunk("sRGB",[0]),Chunk("pHYs",new byte[9]),Chunk("IDAT",Packed(320,180)),Chunk("IEND",[]));store.Import(3,normalized);Need(store.Read(3)!.AsSpan().SequenceEqual(normalized),"Normal encoder ancillary data rejected");
string corruptPath=store.ThumbnailPath(4);File.WriteAllBytes(corruptPath,a[..20]);Reject(()=>store.Read(4),"Corrupt stored image accepted");Need(File.ReadAllBytes(corruptPath).Length==20,"Read modified corrupt image");store.Import(4,a);
string occupiedPc=Path.Combine(output,"occupied-pc");Directory.CreateDirectory(occupiedPc);File.WriteAllText(Path.Combine(occupiedPc,"content"),"keep me");var occupied=new BonusThumbnailStore(occupiedPc);Reject(()=>occupied.Import(1,a),"Directory occupied by a file was overwritten");Need(File.ReadAllText(Path.Combine(occupiedPc,"content"))=="keep me","Unrelated file replaced");
string directoryPath=store.ThumbnailPath(5);File.Delete(directoryPath);Directory.CreateDirectory(directoryPath);Reject(()=>store.Import(5,a),"Destination directory accepted");Need(Directory.Exists(directoryPath),"Destination directory removed");Directory.Delete(directoryPath);store.Import(5,a);
if(OperatingSystem.IsWindows()){using(var held=new FileStream(store.ThumbnailPath(1),FileMode.Open,FileAccess.Read,FileShare.None)){Reject(()=>store.Import(1,a),"Locked replacement unexpectedly committed");}Need(Hash(store.Read(1)!)==beforeHash,"Failed replacement damaged old image");Need(!Directory.GetFiles(store.DirectoryPath,"*.tmp").Any(),"Failed replacement left temporary data");}
void LinkCase(bool broken){
 string root=Path.Combine(output,"link-"+Guid.NewGuid().ToString("N"));Directory.CreateDirectory(root);
 string outside=Path.Combine(output,"outside-"+Guid.NewGuid().ToString("N"));if(!broken)Directory.CreateDirectory(outside);
 bool symbolic=false;
 try{Directory.CreateSymbolicLink(Path.Combine(root,"content"),outside);symbolic=true;}
 catch(Exception e)when(e is UnauthorizedAccessException or IOException or PlatformNotSupportedException){
  if(!OperatingSystem.IsWindows()){symlinkUnavailable++;return;}
  Directory.CreateDirectory(outside);
  var start=new System.Diagnostics.ProcessStartInfo("powershell.exe"){UseShellExecute=false,CreateNoWindow=true,RedirectStandardError=true,RedirectStandardOutput=true};
  start.ArgumentList.Add("-NoProfile");start.ArgumentList.Add("-NonInteractive");start.ArgumentList.Add("-Command");
  start.ArgumentList.Add("$ErrorActionPreference='Stop'; New-Item -ItemType Junction -Path $env:DINO_BONUS_JUNCTION_LINK0358 -Target $env:DINO_BONUS_JUNCTION_TARGET0358 | Out-Null");
  start.Environment["DINO_BONUS_JUNCTION_LINK0358"]=Path.Combine(root,"content");start.Environment["DINO_BONUS_JUNCTION_TARGET0358"]=outside;
  using var process=System.Diagnostics.Process.Start(start)??throw new IOException("Cannot create isolated junction fixture.");
  string error=process.StandardError.ReadToEnd();process.StandardOutput.ReadToEnd();process.WaitForExit();Need(process.ExitCode==0,"Junction fixture failed: "+error);
  if(broken)Directory.Delete(outside);
 }
 var linked=new BonusThumbnailStore(root);Reject(()=>linked.Import(1,a),"Linked content path accepted");Reject(()=>linked.Read(1),"Linked content read accepted");
 Need(!Directory.Exists(Path.Combine(outside,"bonus-thumbnails0358")),"Import wrote through directory link");
 if(symbolic)symlinkCases++;else junctionCases++;
}
LinkCase(false);LinkCase(true);
Need(Hash(File.ReadAllBytes(sentinel))==sentinelHash,"Progress sentinel modified");Need(Directory.GetFiles(pc,"*",SearchOption.TopDirectoryOnly).Length==1,"Unexpected PC-root files written");Need(!Directory.Exists(Path.Combine(pc,"profiles")),"Game profile created");
var result=new{passed=true,checks,rejected,symlinkCases,junctionCases,symlinkUnavailable,slots=6,realPngValidation=true,sizes=new[]{"320x180","640x360"},nativeGameOrUiStarted=false,personalProfileUsed=false,originalProgressSentinelUnchanged=true,invalidImportPreservesOld=true,atomicReplacementFailureCovered=OperatingSystem.IsWindows(),screenshotAuthenticityNotInferred=true};File.WriteAllText(Path.Combine(output,"verification.json"),JsonSerializer.Serialize(result,new JsonSerializerOptions{WriteIndented=true}));Console.WriteLine(JsonSerializer.Serialize(result));
