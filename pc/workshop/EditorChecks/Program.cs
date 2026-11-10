using System.Buffers.Binary;
using System.Diagnostics;
using System.Security.Cryptography;
using System.Text.Json;
using YoshiWorkshop;
string pc=Path.GetFullPath(args[0]),outdir=Path.Combine(pc,"build/editor-0130");Directory.CreateDirectory(outdir);
string original=new SpecialContentRunner(pc).OriginalRom;var rom=new VanillaRom(original);byte[] untouched=File.ReadAllBytes(original);
var templates=rom.Templates;var report=new List<object>();
if(templates.Count!=24 || templates.Any(t=>!t.Supported))throw new Exception("Unsupported sources: "+string.Join(", ",templates.Where(t=>!t.Supported).Select(t=>t.Code+": "+t.Detail)));
string native=Path.Combine(pc,"build/native-special/WorkshopDataChecks.exe");
foreach(var template in templates){
    var level=rom.CloneLevel(template.SourceZone);var independent=rom.CloneLevel(template.SourceZone);
    if(level.Id==independent.Id || level.Objects[0].Id==independent.Objects[0].Id)throw new Exception("Source clone shares identity.");
    float x=independent.Objects[0].X;level.Objects[0].X+=16;if(independent.Objects[0].X!=x)throw new Exception("Source clone is not independent.");
    string file=Path.Combine(outdir,"source-"+template.Code+".yslevel");byte[] payload=rom.ExportPlacements(level,true);File.WriteAllBytes(file,payload);
    byte[] expected=rom.BuildDataOnly(level);string result=Path.Combine(outdir,"native-parity.z64");
    var start=new ProcessStartInfo(native){UseShellExecute=false,RedirectStandardOutput=true,RedirectStandardError=true};foreach(string arg in new[]{original,file,result})start.ArgumentList.Add(arg);
    using(var p=Process.Start(start)!){string stdout=p.StandardOutput.ReadToEnd(),stderr=p.StandardError.ReadToEnd();p.WaitForExit();if(p.ExitCode!=0)throw new Exception(template.Code+": "+stderr);}
    if(!File.ReadAllBytes(result).SequenceEqual(expected))throw new Exception("Editor/native mismatch: "+template.Code);
    if(!expected.AsSpan(0x1000,0xA1C90-0x1000).SequenceEqual(untouched.AsSpan(0x1000,0xA1C90-0x1000)) || !expected.AsSpan(0xB6BCC0,0xEBDA70-0xB6BCC0).SequenceEqual(untouched.AsSpan(0xB6BCC0,0xEBDA70-0xB6BCC0)))throw new Exception("Native code changed.");
    report.Add(new{template.Code,template.SourceZone,template.Objects,cloneIndependent=true,nativeParity=true,nativeCodePreserved=true});Console.WriteLine("Source "+template.Code+" native parity passed.");
}
var custom=rom.CloneFirstLevel();var spawn=custom.Objects.Single(p=>p.ObjectId==0x4001);
foreach(var id in ObjectCatalog.SpecialMaskass){var p=rom.CreatePlacement(id);p.X=spawn.X+220+(id-0x4F10)*70;p.Y=spawn.Y+12;custom.Objects.Add(p);}
foreach(var id in ObjectCatalog.SpecialFruits){var p=rom.CreatePlacement(id);p.X=spawn.X+180+(id-0x4F00)*40;p.Y=spawn.Y+40;custom.Objects.Add(p);}
foreach(var p in custom.Objects.Where(p=>p.HybridVariant.HasValue)){p.X=spawn.X+120+p.HybridVariant!.Value*12;p.Y=spawn.Y+8;}
var victory=rom.CloneFirstLevel();victory.Objects.RemoveAll(p=>p.ObjectId<0x8000 && p.ObjectId!=0x4001);var apple=rom.CreatePlacement(0x4030);apple.X=spawn.X+50;apple.Y=spawn.Y+32;victory.Objects.Add(apple);File.WriteAllBytes(Path.Combine(outdir,"victory-fixture.yslevel"),rom.ExportPlacements(victory,true));File.WriteAllText(Path.Combine(outdir,"victory-fixture.json"),JsonSerializer.Serialize(victory,ProjectStore.Json));
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
var results=new{sourceCount=24,sources=report,legacyV1Preserved=true,extendedV2Accepted=true,sixMaskassVariantsExported=true,sixFruitsPreserved=true,invalidSourceRejected=true,invalidVariantRejected=true,conflictingOverlapRejected=true,sessionRoundTrip=true,duplicateCompletionRejected=true,staleTokenRejected=true,incompleteVictoryRejected=true,preferredYoshiRetained=true,nativeGameplayValidation="pending",originalRomPreserved=true};
File.WriteAllText(Path.Combine(outdir,"core-verification.json"),JsonSerializer.Serialize(results,ProjectStore.Json));Console.WriteLine("Editor data/session checks passed.");
