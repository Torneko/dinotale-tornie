using System.Buffers.Binary;
using System.Security.Cryptography;
using System.Text.Json;
using YoshiWorkshop;
string pc=Path.GetFullPath(args[0]),outdir=Path.Combine(pc,"build/publication-0150"),validation=Path.Combine(outdir,"validation-import-"+Guid.NewGuid().ToString("N"));Directory.CreateDirectory(validation);
string Hash(byte[] b)=>Convert.ToHexStringLower(SHA256.HashData(b));
byte[] source=File.ReadAllBytes(new SpecialContentRunner(pc).OriginalRom),patch=File.ReadAllBytes(Path.Combine(outdir,"distribution/content.bps")),target=File.ReadAllBytes(Path.Combine(pc,"build/native-special-0.15.0-verified/workshop-content.bin"));
if(!BpsPatch.Apply(source,patch).SequenceEqual(target))throw new Exception("Full content reconstruction failed");int rejected=0;
void Reject(Action action){try{action();}catch(InvalidDataException){rejected++;return;}throw new Exception("Invalid patch/import accepted");}
void Number(List<byte> bytes,ulong n){while(true){byte b=(byte)(n&127);n>>=7;if(n==0){bytes.Add((byte)(b|128));return;}bytes.Add(b);n--;}}
byte[] Fixture(byte[] input,byte[] expected,Action<List<byte>> commands){List<byte> result=[.."BPS1"u8];Number(result,(ulong)input.Length);Number(result,(ulong)expected.Length);Number(result,0);commands(result);void Crc(uint value){byte[] b=new byte[4];BinaryPrimitives.WriteUInt32LittleEndian(b,value);result.AddRange(b);}Crc(BpsPatch.Crc(input));Crc(BpsPatch.Crc(expected));Crc(BpsPatch.Crc(result.ToArray()));return result.ToArray();}
byte[] a="abcdef"u8.ToArray();
var linear=Fixture(a,"abXYZf"u8.ToArray(),b=>{Number(b,4);Number(b,9);b.AddRange("XYZ"u8.ToArray());Number(b,0);});if(!BpsPatch.Apply(a,linear).AsSpan().SequenceEqual("abXYZf"u8))throw new Exception("SourceRead/TargetRead failed");
var copy=Fixture(a,"defabc"u8.ToArray(),b=>{Number(b,10);Number(b,6);Number(b,10);Number(b,13);});if(!BpsPatch.Apply(a,copy).AsSpan().SequenceEqual("defabc"u8))throw new Exception("Relative SourceCopy failed");
var repeated=Fixture([],"aaaaaa"u8.ToArray(),b=>{Number(b,1);b.Add((byte)'a');Number(b,19);Number(b,0);});if(!BpsPatch.Apply([],repeated).AsSpan().SequenceEqual("aaaaaa"u8))throw new Exception("Overlapping TargetCopy failed");
var future=Fixture([],"abc"u8.ToArray(),b=>{Number(b,11);Number(b,0);});Reject(()=>BpsPatch.Apply([],future));Reject(()=>BpsPatch.Apply([],Fixture([],"abc"u8.ToArray(),b=>Number(b,8))));Reject(()=>BpsPatch.Apply(source,patch[..^1]));byte[] bad=(byte[])patch.Clone();bad[100]^=1;Reject(()=>BpsPatch.Apply(source,bad));Reject(()=>PublicRomInstall.Normalize(new byte[16_777_216]));
string engine=Path.Combine(pc,"build/native-special-0.15.0-verified/DinoTale.exe");var roots=new List<string>();
for(int order=0;order<3;order++){
 string root=Path.Combine(validation,"order-"+order);roots.Add(root);Directory.CreateDirectory(Path.Combine(root,"distribution"));Directory.CreateDirectory(Path.Combine(root,"build/native-special"));Directory.CreateDirectory(Path.Combine(root,"profiles/content-personal/saves"));
 foreach(string name in new[]{"content.bps","content-patch.json"})File.Copy(Path.Combine(outdir,"distribution",name),Path.Combine(root,"distribution",name));File.Copy(engine,Path.Combine(root,"build/native-special/DinoTale.exe"));
 string save=Path.Combine(root,"profiles/content-personal/saves/yoshisstory.n64.us.bin");byte[] existingSave="Existing progress must survive."u8.ToArray();File.WriteAllBytes(save,existingSave);
 byte[] rom=(byte[])source.Clone();if(order==1)for(int i=0;i<rom.Length;i+=2)(rom[i],rom[i+1])=(rom[i+1],rom[i]);if(order==2)for(int i=0;i<rom.Length;i+=4){(rom[i],rom[i+3])=(rom[i+3],rom[i]);(rom[i+1],rom[i+2])=(rom[i+2],rom[i+1]);}
 string input=Path.Combine(validation,"source-"+order+".rom");File.WriteAllBytes(input,rom);PublicRomInstall.Install(root+Path.DirectorySeparatorChar,input);PublicRomInstall.Install(root,input);
 if(!File.ReadAllBytes(save).SequenceEqual(existingSave) || !File.ReadAllBytes(input).SequenceEqual(rom) || !File.ReadAllBytes(Path.Combine(root,"build/native-special/workshop-content.bin")).SequenceEqual(target) || PublicRomInstall.Needed(root))throw new Exception("Import changed input/progress or produced wrong content");
}
string first=roots[0],destination=Path.Combine(first,"profiles/content-personal/yoshisstory.n64.us.z64");File.WriteAllBytes(destination,"Keep existing unrelated file"u8.ToArray());Reject(()=>PublicRomInstall.Install(first,Path.Combine(validation,"source-0.rom")));if(!File.ReadAllBytes(destination).AsSpan().SequenceEqual("Keep existing unrelated file"u8))throw new Exception("Existing content overwritten");
var report=new{fullTargetExact=true,sourceReadTargetRead=true,relativeSourceCopy=true,overlappingTargetCopy=true,rejectedCases=rejected,threeRomOrdersSupported=true,reimportIdempotent=true,inputFilesUnchanged=true,existingProgressPreserved=true,differentContentRefused=true,validationRoot=validation,targetSha256=Hash(target),patchSha256=Hash(patch),privatePersonalProfileTouched=false};File.WriteAllText(Path.Combine(outdir,"importer-checks.json"),JsonSerializer.Serialize(report,ProjectStore.Json));Console.WriteLine("ROM import, four BPS actions, corrupt files, three byte orders and save preservation passed.");
