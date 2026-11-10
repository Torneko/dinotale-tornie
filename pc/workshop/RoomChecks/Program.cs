using System.Buffers.Binary;
using System.Diagnostics;
using System.Security.Cryptography;
using System.Text.Json;
using YoshiWorkshop;
string pc=Path.GetFullPath(args[0]),outdir=Path.Combine(pc,"build/editor-0140");Directory.CreateDirectory(outdir);
string original=new SpecialContentRunner(pc).OriginalRom;var rom=new VanillaRom(original);byte[] untouched=File.ReadAllBytes(original);
var templates=rom.Templates;var report=new List<object>();
if(templates.Count!=24 || templates.Any(t=>!t.Supported))throw new Exception("Unsupported sources: "+string.Join(", ",templates.Where(t=>!t.Supported).Select(t=>t.Code+": "+t.Detail)));
string native=Path.Combine(pc,"build/native-special/WorkshopDataChecks.exe");
int roomTotal=0;foreach(var template in templates){
    var level=rom.CloneLevel(template.SourceZone);var independent=rom.CloneLevel(template.SourceZone);
    if(level.Id==independent.Id || level.Objects[0].Id==independent.Objects[0].Id)throw new Exception("Source clone shares identity.");
    foreach(var room in level.AllRooms()){float originalX=independent.ObjectsIn(room.SourceZone).First(p=>p.ObjectId!=0x4001).X;room.Objects.First(p=>p.ObjectId!=0x4001).X+=16;if(independent.ObjectsIn(room.SourceZone).First(p=>p.ObjectId!=0x4001).X!=originalX)throw new Exception("Room clone is not independent.");roomTotal++;}
    string file=Path.Combine(outdir,"source-"+template.Code+".yslevel");byte[] payload=rom.ExportPlacements(level,true);File.WriteAllBytes(file,payload);
    byte[] expected=rom.BuildDataOnly(level);string result=Path.Combine(outdir,"native-parity.z64");
    var start=new ProcessStartInfo(native){UseShellExecute=false,RedirectStandardOutput=true,RedirectStandardError=true};foreach(string arg in new[]{original,file,result})start.ArgumentList.Add(arg);
    using(var p=Process.Start(start)!){string stdout=p.StandardOutput.ReadToEnd(),stderr=p.StandardError.ReadToEnd();p.WaitForExit();if(p.ExitCode!=0)throw new Exception(template.Code+": "+stderr);}
    if(!File.ReadAllBytes(result).SequenceEqual(expected))throw new Exception("Editor/native mismatch: "+template.Code);
    if(!expected.AsSpan(0x1000,0xA1C90-0x1000).SequenceEqual(untouched.AsSpan(0x1000,0xA1C90-0x1000)) || !expected.AsSpan(0xB6BCC0,0xEBDA70-0xB6BCC0).SequenceEqual(untouched.AsSpan(0xB6BCC0,0xEBDA70-0xB6BCC0)))throw new Exception("Native code changed.");
    report.Add(new{template.Code,template.SourceZone,template.Objects,rooms=level.Rooms.Count+1,cloneIndependent=true,nativeParity=true,nativeCodePreserved=true});Console.WriteLine("Source "+template.Code+" native parity passed.");
}

if(roomTotal!=113)throw new Exception("Expected 113 distinct editable rooms.");
var multi=rom.CloneLevel(43);if(multi.Schema!=2 || multi.Rooms.Single().SourceZone!=81)throw new Exception("Wrong room correspondence.");
var baseMaskass=multi.Objects.First(p=>ObjectCatalog.IsMaskass(p.ObjectId) && rom.SourceObjects(81).Contains(p.ObjectId));
var a=baseMaskass.Copy();a.X=200;a.Y=288;a.HybridVariant=0;multi.Objects.Add(a);var b=a.Copy();b.HybridVariant=1;multi.Rooms[0].Objects.Add(b);var bridge=multi.Objects.Single(p=>p.ObjectId==0x4208);bridge.X=120;bridge.Y=456;foreach(int xpos in new[]{160,200,240}){var extraJar=bridge.Copy();extraJar.X=xpos;multi.Objects.Add(extraJar);}
byte[] v3=rom.ExportPlacements(multi,true);if(!v3.AsSpan(0,8).SequenceEqual("YSLEVEL3"u8))throw new Exception("Missing multi-room format.");
File.WriteAllBytes(Path.Combine(outdir,"same-position-maskass.yslevel"),v3);
var specialCheck=new ProcessStartInfo(native){UseShellExecute=false};foreach(var arg in new[]{original,Path.Combine(outdir,"same-position-maskass.yslevel"),Path.Combine(outdir,"special-v3.z64"),"--special"})specialCheck.ArgumentList.Add(arg);using(var p=Process.Start(specialCheck)!){p.WaitForExit();if(p.ExitCode!=0)throw new Exception("Special multi-room rejected.");}
var duplicate= new ProjectStore(Path.Combine(outdir,"unused-store")).Duplicate(multi);if(duplicate.AllRooms().SelectMany(r=>r.Objects).Any(p=>multi.AllRooms().SelectMany(r=>r.Objects).Any(q=>p.Id==q.Id)))throw new Exception("Multi-room duplication reused identities.");
// Duplicating a saved level must keep a valid name even at the maximum length.
var duplicateNameStore=new ProjectStore(Path.Combine(outdir,"duplicate-name-store"));
string suffix=" \u2014 copie";
string[] duplicateNames=["Court",new string('N',92),new string('N',93),new string('N',100),new string('N',91)+"\U0001F95A"+new string('N',7)];
foreach(string name in duplicateNames){
    var named=rom.CloneLevel(43);named.Name=name;duplicateNameStore.Save(named);string before=JsonSerializer.Serialize(named,ProjectStore.Json);
    var copy=duplicateNameStore.Duplicate(named);duplicateNameStore.Save(copy);var reloaded=duplicateNameStore.Levels().Single(p=>p.Id==copy.Id);
    if(reloaded.Name!=copy.Name || copy.Name.Length>100 || !copy.Name.EndsWith(suffix) || name.Length<=100-suffix.Length && copy.Name!=name+suffix)throw new Exception("Duplicate name is invalid or was not preserved.");
    int suffixStart=copy.Name.Length-suffix.Length;if(suffixStart>0 && char.IsHighSurrogate(copy.Name[suffixStart-1]))throw new Exception("Duplicate split a Unicode character.");
    var again=duplicateNameStore.Duplicate(copy);duplicateNameStore.Save(again);
    if(JsonSerializer.Serialize(named,ProjectStore.Json)!=before)throw new Exception("Duplicate changed original long-named level.");
}
Console.WriteLine($"Duplicate name checks passed: {duplicateNames.Length} cases, including 100-character and Unicode names.");
void RejectRoom(Action action){try{action();}catch(InvalidDataException){return;}throw new Exception("Invalid multi-room accepted.");}
var bad=rom.CloneLevel(43);bad.Rooms.Add(rom.ReadRoom(106));RejectRoom(()=>rom.ExportPlacements(bad,true));bad=rom.CloneLevel(43);bad.Rooms.Add(rom.ReadRoom(81));RejectRoom(()=>rom.ExportPlacements(bad,true));bad=rom.CloneLevel(43);bad.Rooms[0].Objects.RemoveAll(p=>p.ObjectId==0x4001);RejectRoom(()=>rom.ExportPlacements(bad,true));
foreach(int room in new[]{43,81}){var moved=rom.CloneLevel(43);moved.ObjectsIn(room).Single(p=>p.ObjectId==0x4001).X+=16;RejectRoom(()=>rom.ExportPlacements(moved,true));}
var capacity=rom.CloneFirstLevel();while(capacity.Objects.Count<149)capacity.Objects.Add(capacity.Objects.First(p=>p.ObjectId==0x4030).Copy());rom.ExportPlacements(capacity,true);capacity.Objects.Add(capacity.Objects.First(p=>p.ObjectId==0x4030).Copy());RejectRoom(()=>rom.ExportPlacements(capacity,true));
var legacy=rom.CloneFirstLevel();string entry=JsonSerializer.Serialize(legacy.Objects,ProjectStore.Json);byte[] old=rom.ExportPlacements(legacy,true);if(!old.AsSpan(0,8).SequenceEqual("YSLEVEL1"u8))throw new Exception("Legacy V1 changed.");rom.IncludeOriginalRooms(legacy);if(JsonSerializer.Serialize(legacy.Objects,ProjectStore.Json)!=entry)throw new Exception("Upgrade changed existing placements.");
// A literal, editable fixture that enters the original pot and checks its target.
var pot=rom.CloneLevel(43);var jar=pot.Objects.Single(p=>p.ObjectId==0x4208);jar.X=120;jar.Y=456;foreach(int xpos in new[]{160,200,240}){var extraJar=jar.Copy();extraJar.X=xpos;pot.Objects.Add(extraJar);}
var fruit=pot.Rooms.Single().Objects.First(p=>p.ObjectId is >=0x4030 and <=0x4033);fruit.ObjectId=0x4F05;
var marker=rom.CreatePlacement(0x4F12,81);marker.X=360;marker.Y=288;pot.Rooms.Single().Objects.Add(marker);
File.WriteAllBytes(Path.Combine(outdir,"real-pot.yslevel"),rom.ExportPlacements(pot,true));File.WriteAllText(Path.Combine(outdir,"real-pot.json"),JsonSerializer.Serialize(pot,ProjectStore.Json));

var custom=rom.CloneFirstLevel();var spawn=custom.Objects.Single(p=>p.ObjectId==0x4001);
foreach(var id in ObjectCatalog.SpecialMaskass){var p=rom.CreatePlacement(id);p.X=spawn.X+220+(id-0x4F10)*70;p.Y=spawn.Y+12;custom.Objects.Add(p);}
foreach(var id in ObjectCatalog.SpecialFruits){var p=rom.CreatePlacement(id);p.X=spawn.X+180+(id-0x4F00)*40;p.Y=spawn.Y+40;custom.Objects.Add(p);}
foreach(var p in custom.Objects.Where(p=>p.HybridVariant.HasValue)){p.X=spawn.X+120+p.HybridVariant!.Value*12;p.Y=spawn.Y+8;}
var victory=rom.CloneFirstLevel();victory.Objects.RemoveAll(p=>p.ObjectId<0x8000 && p.ObjectId!=0x4001);var apple=rom.CreatePlacement(0x4030);apple.X=spawn.X+50;apple.Y=spawn.Y+32;victory.Objects.Add(apple);File.WriteAllBytes(Path.Combine(outdir,"victory-fixture.yslevel"),rom.ExportPlacements(victory,true));File.WriteAllText(Path.Combine(outdir,"victory-fixture.json"),JsonSerializer.Serialize(victory,ProjectStore.Json));
var victoryMulti=rom.CloneLevel(43);victoryMulti.Objects=victory.Objects.Select(p=>p.Copy()).ToList();File.WriteAllText(Path.Combine(outdir,"victory-multiroom.json"),JsonSerializer.Serialize(victoryMulti,ProjectStore.Json));
byte[] v2=rom.ExportPlacements(custom,true);if(!v2.AsSpan(0,8).SequenceEqual("YSLEVEL2"u8) || v2.Length!=16+custom.Objects.Count*16)throw new Exception("Wrong extended payload layout.");
string extended=Path.Combine(outdir,"six-maskass-six-fruits.yslevel");File.WriteAllBytes(extended,v2);File.WriteAllText(Path.Combine(outdir,"six-maskass-six-fruits.json"),JsonSerializer.Serialize(custom,ProjectStore.Json));
var extendedStart=new ProcessStartInfo(native){UseShellExecute=false,RedirectStandardOutput=true,RedirectStandardError=true};foreach(string arg in new[]{original,extended,Path.Combine(outdir,"native-v2.z64"),"--special"})extendedStart.ArgumentList.Add(arg);
using(var p=Process.Start(extendedStart)!){string stdout=p.StandardOutput.ReadToEnd(),stderr=p.StandardError.ReadToEnd();p.WaitForExit();if(p.ExitCode!=0)throw new Exception(stderr);}
void Reject(Action operation){try{operation();}catch(InvalidDataException){return;}throw new Exception("Invalid custom project accepted.");}
var invalid=rom.CloneFirstLevel();invalid.SourceZone=67;Reject(()=>rom.ExportPlacements(invalid,true));invalid=rom.CloneFirstLevel();invalid.Objects.First(p=>p.ObjectId==0x4030).HybridVariant=1;Reject(()=>rom.ExportPlacements(invalid,true));
invalid=rom.CloneFirstLevel();var wrong=rom.CreatePlacement(0x4F10);invalid.Objects.Add(wrong);var overlap=wrong.Copy();overlap.HybridVariant=1;invalid.Objects.Add(overlap);Reject(()=>rom.ExportPlacements(invalid,true));
var state=new CustomSessionState{GameId=Guid.NewGuid().ToString("N"),GameName="Deux copies",Levels=[rom.CloneFirstLevel(),rom.CloneLevel(46)]};state.Validate();state.Phase="running";state.ExpectedToken=Guid.NewGuid().ToString("N");
var complete=new CustomLevelResult{Token=state.ExpectedToken,Completed=true,Fruits=30,SelectedColor=10};if(!state.Commit(complete) || state.Index!=1 || state.PreferredColor!=10 || state.Phase!="paused" || state.Commit(complete))throw new Exception("Completion duplicate/continuation rules failed.");
state=JsonSerializer.Deserialize<CustomSessionState>(JsonSerializer.Serialize(state,ProjectStore.Json),ProjectStore.Json)!;state.Validate();state.Phase="running";state.ExpectedToken=Guid.NewGuid().ToString("N");
if(state.Commit(new(){Token=complete.Token,Completed=true,Fruits=30}) || state.Commit(new(){Token=state.ExpectedToken,Completed=true,Fruits=29}))throw new Exception("Stale/incomplete result accepted.");
if(!state.Commit(new(){Token=state.ExpectedToken,Completed=true,Fruits=30,SelectedColor=10}) || state.Phase!="completed" || state.Index!=2)throw new Exception("Final completion failed.");
if(!File.ReadAllBytes(original).SequenceEqual(untouched))throw new Exception("Original ROM modified.");
var results=new{sourceCount=24,editableRoomCount=roomTotal,duplicateNameCases=duplicateNames.Length,duplicateLongNamesSaveReload=true,duplicateUnicodePreserved=true,multiRoomNativeParity=true,wrongRoomRejected=true,duplicateRoomRejected=true,missingSecondarySpawnRejected=true,movedNativeEntryRejected=true,max149ObjectsEnforced=true,legacyUpgradePreservesEntry=true,samePositionDifferentRoomsAccepted=true,sources=report,legacyV1Preserved=true,extendedV2Accepted=true,sixMaskassVariantsExported=true,sixFruitsPreserved=true,invalidSourceRejected=true,invalidVariantRejected=true,conflictingOverlapRejected=true,sessionRoundTrip=true,duplicateCompletionRejected=true,staleTokenRejected=true,incompleteVictoryRejected=true,preferredYoshiRetained=true,nativeGameplayValidation="pending",originalRomPreserved=true};
File.WriteAllText(Path.Combine(outdir,"core-verification.json"),JsonSerializer.Serialize(results,ProjectStore.Json));Console.WriteLine("Editor data/session checks passed.");

