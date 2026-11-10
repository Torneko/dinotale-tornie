using System.Buffers.Binary;
using System.Security.Cryptography;
using System.Text.Json;
using YoshiWorkshop;
string pc=Path.GetFullPath(args[0]),output=Path.Combine(pc,"build/editor-0200");Directory.CreateDirectory(output);
var runner=new SpecialContentRunner(pc);var rom=new VanillaRom(runner.OriginalRom);byte[] original=File.ReadAllBytes(runner.OriginalRom);
void Need(bool ok,string message){if(!ok)throw new Exception(message);}
uint U32(byte[] b,int p)=>BinaryPrimitives.ReadUInt32BigEndian(b.AsSpan(p));
ushort U16(byte[] b,int p)=>BinaryPrimitives.ReadUInt16BigEndian(b.AsSpan(p));
int Bank(byte[] b,uint p)=>checked((int)(U32(b,0xA79D4+(int)((p>>24)&15)*4)+(p&0xffffff)));
int Header(byte[] b,int room)=>Bank(b,U32(b,0xA84A0+room*8+4));
string Hash(byte[] b)=>Convert.ToHexStringLower(SHA256.HashData(b));
var editable=VanillaRom.SourceZones.SelectMany(NativeRooms.For).Distinct().ToList();var compatible=editable.Where(rom.CanPlaceBlackFlyGuy).ToList();
Need(compatible.Contains(35) && !rom.CanPlaceBlackFlyGuy(43) && !rom.CanPlaceBlackFlyGuy(-1) && !rom.CanPlaceBlackFlyGuy(10000),"Source gates incorrect");
foreach(int room in compatible){Need(rom.ReadRoom(room).Objects.Any(p=>p.ObjectId==0x45E5),"Missing native assembly");Need(rom.SourceObjects(room).Contains((ushort)0x45E3) && rom.Palette(room,true).Contains((ushort)0x45E3),"Standalone actor absent from compatible palette");var placed=rom.CreatePlacement(0x45E3,room);var native=rom.ReadRoom(room).Objects.First(p=>p.ObjectId==0x45E5);Need(placed.ObjectId==0x45E3 && placed.Layer==native.Layer && placed.X==native.X && placed.Y==native.Y,"Native template parameters lost");}
int rejected=0;try{rom.CreatePlacement(0x45E3,43);}catch(InvalidDataException){rejected++;}Need(rejected==1,"Unsupported room accepted standalone actor");
var level=rom.CloneLevel(35);var assembly=level.Objects.First(p=>p.ObjectId==0x45E5).Copy();int removed=level.Objects.RemoveAll(p=>p.ObjectId==0x45E5);var fly=rom.CreatePlacement(0x45E3,35);level.Objects.Add(fly);
Need(removed>0 && !level.Objects.Any(p=>p.ObjectId==0x45E5) && level.Objects.Count(p=>p.ObjectId==0x45E3)==1,"Fixture does not isolate standalone actor");
foreach(Action invalid in new Action[]{()=>rom.ExportPlacements(level),()=>rom.BuildDataOnly(level)}){bool failed=false;try{invalid();}catch(InvalidDataException){failed=true;}Need(failed,"Standalone accepted without custom runtime");}
var duplicate=rom.CloneLevel(35);duplicate.Objects.Add(rom.CreatePlacement(0x45E3,35));bool overlap=false;try{rom.ExportPlacements(duplicate,true);}catch(InvalidDataException){overlap=true;}Need(overlap,"Ambiguous native/standalone overlap accepted");
byte[] payload=rom.ExportPlacements(level,true),data=rom.BuildDataOnly(level,true);int nativeHeader=Header(original,35),newHeader=Header(data,35);
Need(data.AsSpan(newHeader+0x60,0xA0).SequenceEqual(original.AsSpan(nativeHeader+0x60,0xA0)),"Native dependency list changed");Need(U16(data,newHeader+0xC8)==0x45E3 && U32(data,newHeader+0x100)==10,"Preload or music lost");
Need(data.AsSpan(0x1000,0xA1C90-0x1000).SequenceEqual(original.AsSpan(0x1000,0xA1C90-0x1000)) && data.AsSpan(0xB6BCC0,0xEBDA70-0xB6BCC0).SequenceEqual(original.AsSpan(0xB6BCC0,0xEBDA70-0xB6BCC0)),"Native executable code changed");
foreach(int room in NativeRooms.For(35).Where(r=>r!=35)){int nh=Header(data,room),oh=Header(original,room);Need(data.AsSpan(nh+0x60,0xB8).SequenceEqual(original.AsSpan(oh+0x60,0xB8)),"Secondary dependency or music changed");}
var old=Directory.GetFiles(Path.Combine(pc,"build/editor-0190/validation-core/content/levels"),"*.json").Select(p=>JsonSerializer.Deserialize<LevelProject>(File.ReadAllText(p),ProjectStore.Json)!).First(p=>p.SourceZone==43 && p.Terrain.Count>0);Need(rom.ExportPlacements(old).AsSpan().SequenceEqual(File.ReadAllBytes(Path.Combine(pc,"build/editor-0190/section-runtime.yslevel"))) && rom.BuildDataOnly(old).AsSpan().SequenceEqual(File.ReadAllBytes(Path.Combine(pc,"build/editor-0190/section-runtime.z64"))),"Legacy019 fixture changed");
File.WriteAllBytes(Path.Combine(output,"black-flyguy-runtime.yslevel"),payload);File.WriteAllBytes(Path.Combine(output,"black-flyguy-runtime.z64"),data);
var report=new{passed=true,version="0.20.0",editableRooms=editable.Count,compatibleRooms=compatible,rootZone=35,stage=21,actor="0x45E3",removedAssemblyCount=removed,rootAssemblyRemaining=0,position=new[]{fly.X,fly.Y},layer=fly.Layer,diagnosticSpawn=new[]{fly.X-96,fly.Y},nativeMusic=10,preloadPreserved=true,otherRoomsPreserved=true,legacy019Exact=true,nativeCodeUnchanged=true,payloadSha256=Hash(payload),expectedDataSha256=Hash(data),personalProgressUsed=false};
File.WriteAllText(Path.Combine(output,"black-flyguy-runtime.json"),JsonSerializer.Serialize(report,ProjectStore.Json));Console.WriteLine($"Standalone black Fly Guy checks passed: {compatible.Count}/{editable.Count} source rooms; no root45E5 assembly remains.");
