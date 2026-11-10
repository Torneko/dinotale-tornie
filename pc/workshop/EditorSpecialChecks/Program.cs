using System.Buffers.Binary;
using System.Security.Cryptography;
using System.Text.Json;
using YoshiWorkshop;

string pc=Path.GetFullPath(args[0]);var output=Path.Combine(pc,"build/editor-0200");Directory.CreateDirectory(output);
var romPath=new SpecialContentRunner(pc).OriginalRom;var originalHash=Hash(File.ReadAllBytes(romPath));var rom=new VanillaRom(romPath);
void Need(bool condition,string message){if(!condition)throw new Exception(message);}
string State(LevelProject level)=>JsonSerializer.Serialize(level,ProjectStore.Json);
LevelProject Clone(LevelProject level)=>JsonSerializer.Deserialize<LevelProject>(State(level),ProjectStore.Json)!;
int rejected=0;
void Reject(Action action){try{action();}catch(InvalidDataException){rejected++;return;}catch(InvalidOperationException){rejected++;return;}throw new Exception("Invalid special object operation accepted");}
int Root(int room)=>VanillaRom.SourceZones.Single(root=>NativeRooms.Contains(root,room));
var rooms=VanillaRom.SourceZones.SelectMany(NativeRooms.For).Distinct().Where(room=>SpecialEditorObjects.Compatible(rom.SourceObjects(room))).Order().ToArray();
Need(rooms.SequenceEqual(new[]{102,108,120,170}),"Unexpected compatible rooms");
Need(SpecialEditorObjects.Eggs.Count()==8 && SpecialEditorObjects.Eggs.Select(SpecialEditorObjects.Color).SequenceEqual(Enumerable.Range(6,8)),"Egg color order incorrect");
int compatiblePaletteCases=0,unsupportedPaletteCases=0;
foreach(int room in VanillaRom.SourceZones.SelectMany(NativeRooms.For).Distinct()){
 bool compatible=rooms.Contains(room);var palette=rom.Palette(room,true).ToHashSet();
 Need(SpecialEditorObjects.Eggs.All(id=>palette.Contains(id)==compatible),"Egg palette compatibility incorrect");
 Need(!rom.Palette(room,false).Any(SpecialEditorObjects.IsEgg),"Vanilla palette exposes custom eggs");
 if(compatible)compatiblePaletteCases++;else unsupportedPaletteCases++;
}
foreach(int room in rooms){
 var level=rom.CloneLevel(Root(room));var list=level.ObjectsIn(room);var native=rom.ReadRoom(room).Objects.First(p=>p.ObjectId==SpecialEditorObjects.Alias(rom.SourceObjects(room)));
 for(int i=0;i<8;i++){
  var item=rom.CreatePlacement((ushort)(0x4F30+i),room);Need(item.ObjectId==0x4F30+i && item.Layer==native.Layer && item.HybridVariant==null,"Egg placement did not retain native template");
  item.X=10000+i*64;item.Y=5000;list.Add(item);
 }
 string before=State(level);var payload=rom.ExportPlacements(level,true);Need(payload[7]=='3' && State(level)==before,"Export changed level or unexpected legacy format");
 Reject(()=>rom.ExportPlacements(level,false));
}
Reject(()=>rom.CreatePlacement(0x4F30,43));
var incompatible=rom.CloneLevel(43);var sourceEgg=rom.CreatePlacement(0x4F30,120);sourceEgg.X=10000;sourceEgg.Y=5000;incompatible.Objects.Add(sourceEgg);
string invalidBefore=State(incompatible);Reject(()=>rom.ExportPlacements(incompatible,true));Need(State(incompatible)==invalidBefore,"Failed export mutated incompatible level");
var overlap=rom.CloneLevel(46);var originalEgg=overlap.ObjectsIn(120).First(p=>SpecialEditorObjects.NativeEgg(p.ObjectId));var egg=rom.CreatePlacement(0x4F30,120);egg.X=originalEgg.X;egg.Y=originalEgg.Y;egg.Layer=originalEgg.Layer;overlap.ObjectsIn(120).Add(egg);
Reject(()=>rom.ExportPlacements(overlap,true));
var overlapTwo=rom.CloneLevel(46);var one=rom.CreatePlacement(0x4F30,120);var two=rom.CreatePlacement(0x4F31,120);one.X=two.X=10000;one.Y=two.Y=5000;overlapTwo.ObjectsIn(120).AddRange([one,two]);Reject(()=>rom.ExportPlacements(overlapTwo,true));
var badLayer=rom.CloneLevel(46);var malformedEgg=rom.CreatePlacement(0x4F30,120);malformedEgg.X=10000;malformedEgg.Y=5000;malformedEgg.Layer^=1;badLayer.ObjectsIn(120).Add(malformedEgg);Reject(()=>rom.ExportPlacements(badLayer,true));
var groupSource=rom.CloneLevel(46);var group=new List<Placement>();for(int i=0;i<8;i++){var p=rom.CreatePlacement((ushort)(0x4F30+i),120);p.X=10000+i*64;p.Y=5000;groupSource.ObjectsIn(120).Add(p);group.Add(p);}
var clip=ObjectGroups.Copy(group,46,120);string groupBefore=State(groupSource);var copies=ObjectGroups.PreparePaste(rom,groupSource,120,clip,15000,8000);
Need(State(groupSource)==groupBefore && copies.Count==8 && copies.Select(p=>p.Id).Distinct().Count()==8 && !copies.Any(p=>group.Any(s=>s.Id==p.Id)),"Prepared same-room paste mutated source or IDs");
Need(copies.Select(p=>p.ObjectId).SequenceEqual(SpecialEditorObjects.Eggs),"Copied egg colors changed");
float old=group[0].X;group[0].X+=1000;var independent=ObjectGroups.PreparePaste(rom,groupSource,120,clip,18000,8000);Need(independent[1].X-independent[0].X==64,"Clipboard tracks later source changes");group[0].X=old;
var destination=rom.CloneLevel(Root(102));string destBefore=State(destination);var cross=ObjectGroups.PreparePaste(rom,destination,102,clip,10000,5000);Need(cross.Count==8 && State(destination)==destBefore,"Cross-room compatible paste failed or mutated target");destination.ObjectsIn(102).AddRange(cross);rom.ExportPlacements(destination,true);
var blocked=rom.CloneLevel(43);string blockedBefore=State(blocked);Reject(()=>ObjectGroups.PreparePaste(rom,blocked,43,clip,1000,500));Need(State(blocked)==blockedBefore,"Rejected group paste mutated room");
var capacity=rom.CloneLevel(46);var cap=capacity.ObjectsIn(120);var filler=cap.First(p=>p.ObjectId<0x8000 && p.ObjectId!=0x4001 && !SpecialEditorObjects.NativeEgg(p.ObjectId));while(cap.Count<RoomProject.MaxObjects)cap.Add(filler.Copy());string capBefore=State(capacity);Reject(()=>ObjectGroups.PreparePaste(rom,capacity,120,clip,10000,5000));Need(State(capacity)==capBefore,"Capacity rejection mutated room");
var mixed=rom.CloneLevel(46);mixed.ObjectsIn(120).AddRange(group.Select(p=>p.Copy()));int theme=RoomMusic.Tracks.First(t=>t.Id!=rom.ReadRoomMusic(120)).Id;rom.SetRoomMusic(mixed,120,theme);
var map=rom.ReadTerrain(120).Single();int changed=Enumerable.Range(0,map.RawTiles.Length).First(i=>map.RawTiles[i]!=65535 && TerrainEditing.ReferencedCells(map).Contains(i));
rom.SetTerrainCells(mixed,120,map.ResourceId,new Dictionary<int,(ushort,ushort)>{{changed,(65535,0)}});
Need(mixed.Schema==4 && mixed.AllRooms().Single(r=>r.SourceZone==120).MusicId==theme,"Music+terrain schema changed incorrectly");var mixedPayload=rom.ExportPlacements(mixed,true);Need(mixedPayload[7]=='5',"Mixed export is not V5");
var store=new ProjectStore(Path.Combine(output,"validation-special-core/content"));store.Save(mixed);var loaded=store.Levels().Single(l=>l.Id==mixed.Id);Need(State(loaded)==State(mixed),"Mixed special/music/terrain save lost data");
var duplicate=store.Duplicate(loaded);duplicate.ObjectsIn(120).First(p=>SpecialEditorObjects.IsEgg(p.ObjectId)).X+=32;rom.SetRoomMusic(duplicate,120,null);Need(State(loaded)==State(mixed),"Duplicate shares egg/music data");
// Native fixture uses only fresh eggs. Keep white/orange at Y600, above the observed floor/player Y545, for bounded callback validation.
var fixture=rom.CloneLevel(46);var fixtureRoom=fixture.ObjectsIn(120);fixtureRoom.RemoveAll(ObjectGroups.Editable);var positions=new List<object>();
for(int i=0;i<8;i++){
 int slot=i switch {0=>4,4=>0,2=>6,6=>2,_=>i};
 var p=rom.CreatePlacement((ushort)(0x4F30+i),120);p.X=48+(slot%4)*64;p.Y=i is 0 or 2?600:176+(slot/4)*64;fixtureRoom.Add(p);
 positions.Add(new{id=p.Id,objectId=p.ObjectId,color=SpecialEditorObjects.Color(p.ObjectId),name=SpecialEditorObjects.Names[i],x=p.X,y=p.Y,layer=p.Layer});
}
rom.SetRoomMusic(fixture,120,2);var terrain=rom.ReadTerrain(120).Single();int marker=TerrainEditing.ReferencedCells(terrain).First(i=>terrain.RawTiles[i]!=65535);rom.SetTerrainCells(fixture,120,terrain.ResourceId,new Dictionary<int,(ushort,ushort)>{{marker,(65535,0)}});
var export=rom.ExportPlacements(fixture,true);var normalized=Clone(fixture);foreach(var room in normalized.AllRooms())foreach(var item in room.Objects)if(SpecialEditorObjects.IsEgg(item.ObjectId))item.ObjectId=SpecialEditorObjects.Alias(rom.SourceObjects(room.SourceZone));var expected=rom.BuildDataOnly(normalized);
File.WriteAllBytes(Path.Combine(output,"special-runtime.yslevel"),export);File.WriteAllBytes(Path.Combine(output,"special-runtime.z64"),expected);File.WriteAllText(Path.Combine(output,"special-runtime.level.json"),State(fixture));
File.WriteAllText(Path.Combine(output,"special-runtime.json"),JsonSerializer.Serialize(new{rootZone=46,roomZone=120,stage=4,diagnosticZone=120,alias=SpecialEditorObjects.Alias(rom.SourceObjects(120)),musicId=RoomMusic.Selected(fixture,120),objects=positions,payloadSha256=Hash(export),expectedDataSha256=Hash(expected),sourceRomSha256=originalHash,customEggsOnly=true},ProjectStore.Json));
Need(Hash(File.ReadAllBytes(romPath))==originalHash,"Source ROM changed");
var result=new{passed=true,compatibleRooms=rooms,compatiblePaletteCases,unsupportedPaletteCases,eightEggColors=true,nativeTemplateLayers=true,sourceCompatiblePlacement=true,customOnlyRuntimeRequired=true,incompatibleRoomRejected=true,nativeAndCustomOverlapRejected=true,customOverlapRejected=true,nativeLayerEnforced=true,groupFreshIds=true,groupClipboardIndependent=true,compatibleCrossRoomPaste=true,incompatiblePasteAtomic=true,capacityRejectAtomic=true,musicTerrainV5=true,persistence=true,duplicateIsolation=true,sourceRomUnchanged=true,rejected,runtimeFixture="special-runtime.yslevel",expectedFixture="special-runtime.z64"};
File.WriteAllText(Path.Combine(output,"special-core.json"),JsonSerializer.Serialize(result,ProjectStore.Json));Console.WriteLine(JsonSerializer.Serialize(result));
static string Hash(byte[] bytes)=>Convert.ToHexStringLower(SHA256.HashData(bytes));
