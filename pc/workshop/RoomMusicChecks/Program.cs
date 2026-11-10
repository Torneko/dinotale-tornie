using System.Buffers.Binary;
using System.Security.Cryptography;
using System.Text.Json;
using YoshiWorkshop;
string pc=Path.GetFullPath(args[0]);string output=Path.Combine(pc,"build/editor-0200");Directory.CreateDirectory(output);
var rom=new VanillaRom(new SpecialContentRunner(pc).OriginalRom);byte[] original=File.ReadAllBytes(new SpecialContentRunner(pc).OriginalRom);
void Need(bool value,string message){if(!value)throw new Exception(message);}
string State(LevelProject p)=>JsonSerializer.Serialize(p,ProjectStore.Json);
string Hash(byte[] data)=>Convert.ToHexStringLower(SHA256.HashData(data));
int rejected=0;void Reject(Action action){try{action();}catch(InvalidDataException){rejected++;return;}throw new Exception("Invalid music accepted");}
uint U32(byte[] b,int p)=>BinaryPrimitives.ReadUInt32BigEndian(b.AsSpan(p));
int Bank(byte[] b,uint pointer)=>checked((int)(U32(b,0xA79D4+(int)((pointer>>24)&15)*4)+(pointer&0xffffff)));
int Header(byte[] b,int room)=>Bank(b,U32(b,0xA84A0+room*8+4));
var defaults=new Dictionary<int,int>();
foreach(int root in VanillaRom.SourceZones)foreach(int room in NativeRooms.For(root)){int id=rom.ReadRoomMusic(room);Need(RoomMusic.Valid(id),"Unknown music in editable room");Need(U32(original,Header(original,room)+0x100)==id,"Music does not match native descriptor");defaults[room]=id;}
Need(defaults.Count==113 && RoomMusic.Tracks.Count==17,"Music source coverage changed");
var plain=rom.CloneLevel(43);byte[] baseline=rom.ExportPlacements(plain);Need(baseline[7]=='3',"Legacy V3 changed");
var saved019=Directory.GetFiles(Path.Combine(pc,"build/editor-0190/validation-core/content/levels"),"*.json").Select(p=>JsonSerializer.Deserialize<LevelProject>(File.ReadAllText(p),ProjectStore.Json)!).First(p=>p.SourceZone==43 && p.Terrain.Count>0);
Need(rom.ExportPlacements(saved019).AsSpan().SequenceEqual(File.ReadAllBytes(Path.Combine(pc,"build/editor-0190/section-runtime.yslevel"))),"Legacy V4 bytes changed");
Need(rom.BuildDataOnly(saved019).AsSpan().SequenceEqual(File.ReadAllBytes(Path.Combine(pc,"build/editor-0190/section-runtime.z64"))),"Legacy V4 data changed");
string before=State(plain);Need(!rom.SetRoomMusic(plain,43,null) && !rom.SetRoomMusic(plain,43,rom.ReadRoomMusic(43)) && State(plain)==before,"Inherited/no-op music mutated level");
Need(rom.SetRoomMusic(plain,43,2) && plain.Schema==4 && plain.MusicId==2 && RoomMusic.Selected(plain,81)==null,"Root override failed");
Need(rom.SetRoomMusic(plain,81,8) && RoomMusic.Selected(plain,81)==8,"Secondary override failed");
byte[] data=rom.BuildDataOnly(plain);Need(U32(data,Header(data,43)+0x100)==2 && U32(data,Header(data,81)+0x100)==8,"Music descriptor overrides missing");
foreach(int room in NativeRooms.For(43))for(int p=0;p<0x118;p+=4)if(p!=0x100 && p!=0x5C)Need(U32(data,Header(data,room)+p)==U32(original,Header(original,room)+p),"Unrelated descriptor field changed");
Need(data.AsSpan(0x1000,0xA1C90-0x1000).SequenceEqual(original.AsSpan(0x1000,0xA1C90-0x1000)) && data.AsSpan(0xB6BCC0,0xEBDA70-0xB6BCC0).SequenceEqual(original.AsSpan(0xB6BCC0,0xEBDA70-0xB6BCC0)),"Music modified executable game code");
var store=new ProjectStore(Path.Combine(output,"validation-music/content"));store.Save(plain);var restored=store.Levels().Single(p=>p.Id==plain.Id);Need(State(restored)==State(plain),"Music not persisted");var clone=store.Duplicate(restored);rom.SetRoomMusic(clone,81,9);Need(RoomMusic.Selected(restored,81)==8,"Duplicate shares music state");
foreach(int id in new[]{-1,0,17,36,53,55,69,70,65535}){before=State(plain);Reject(()=>rom.SetRoomMusic(plain,43,id));Need(State(plain)==before,"Invalid music partially changed level");}
Reject(()=>rom.SetRoomMusic(plain,90,1));var invalid=JsonSerializer.Deserialize<LevelProject>(State(plain),ProjectStore.Json)!;invalid.Schema=3;Reject(()=>ProjectStore.Validate(invalid));invalid.Schema=4;invalid.MusicId=17;Reject(()=>ProjectStore.Validate(invalid));
var legacy=rom.CloneFirstLevel();Reject(()=>rom.SetRoomMusic(legacy,81,2));byte[] legacyV1=rom.ExportPlacements(legacy);rom.SetRoomMusic(legacy,43,2);Need(rom.ExportPlacements(legacy)[7]=='5',"V1 music export failed");rom.SetRoomMusic(legacy,43,null);Need(rom.ExportPlacements(legacy).AsSpan().SequenceEqual(legacyV1),"Legacy V1 not recovered");
legacy.Objects.First(p=>ObjectCatalog.IsMaskass(p.ObjectId)).HybridVariant=2;byte[] legacyV2=rom.ExportPlacements(legacy,true);Need(legacyV2[7]=='2',"Expected V2 hybrid fixture");rom.SetRoomMusic(legacy,43,2);Need(rom.ExportPlacements(legacy,true)[7]=='5',"V2 music export failed");rom.SetRoomMusic(legacy,43,null);Need(rom.ExportPlacements(legacy,true).AsSpan().SequenceEqual(legacyV2),"Legacy V2 not recovered");
rom.SetRoomMusic(plain,43,null);Need(plain.Schema==4,"Clearing one override lost other music");rom.SetRoomMusic(plain,81,null);Need(plain.Schema==2 && rom.ExportPlacements(plain).AsSpan().SequenceEqual(baseline),"Clearing all music did not recover legacy bytes");
// The same terrain API must preserve Schema4, and clearing music preserves terrain.
var runtime=JsonSerializer.Deserialize<LevelProject>(State(saved019),ProjectStore.Json)!;rom.SetRoomMusic(runtime,43,2);rom.SetRoomMusic(runtime,81,8);var native=rom.ReadTerrain(43).Single();int cell=native.CellIndex(8,12);rom.SetTerrainCells(runtime,43,native.ResourceId,new Dictionary<int,(ushort,ushort)>{{cell,((ushort)65535,(ushort)0)}});Need(runtime.Schema==4,"Painting downgraded music schema");
byte[] payload=rom.ExportPlacements(runtime),nativeData=rom.BuildDataOnly(runtime);Need(payload[7]=='5' && U32(payload,16+12)==2,"Music export is not V5");
File.WriteAllBytes(Path.Combine(output,"music-runtime.yslevel"),payload);File.WriteAllBytes(Path.Combine(output,"music-runtime.z64"),nativeData);
rom.SetRoomMusic(runtime,43,null);rom.SetRoomMusic(runtime,81,null);Need(runtime.Schema==3 && rom.ExportPlacements(runtime)[7]=='4',"Clearing music lost terrain schema");
int matrix=0;foreach(var track in RoomMusic.Tracks){var level=rom.CloneLevel(43);rom.SetRoomMusic(level,43,track.Id);var bytes=rom.BuildDataOnly(level);Need(U32(bytes,Header(bytes,43)+0x100)==track.Id,"Track matrix descriptor incorrect");Need(U32(bytes,Header(bytes,81)+0x100)==1,"Track matrix leaked to secondary room");matrix++;}
var result=new{passed=true,rooms=defaults.Count,tracks=RoomMusic.Tracks.Count,trackMatrix=matrix,rejected,descriptorOffset=RoomMusic.DescriptorOffset,rootAndSecondaryRoomIndependent=true,legacyV1V2V3V4Exact=true,sourceCodeUnchanged=true,silenceNotExposed=true,saveReload=true,duplicateIsolation=true,noOpAndNativeThemeNormalize=true,invalidCasesAtomic=true,schemaTransitions=true,paintingPreservesMusic=true,clearingMusicPreservesTerrain=true,payloadVersion=5,defaults,fixtureRootMusic=2,fixtureSecondaryMusic=8,payloadSha256=Hash(payload),dataSha256=Hash(nativeData)};
File.WriteAllText(Path.Combine(output,"music-core.json"),JsonSerializer.Serialize(result,ProjectStore.Json));Console.WriteLine($"Room music checks passed: {defaults.Count} rooms, {matrix} tracks, {rejected} rejected invalid cases.");
