using System.Buffers.Binary;
using System.Security.Cryptography;
using System.Diagnostics;
using System.Text.Json;
using YoshiWorkshop;

string pc=Path.GetFullPath(args[0]);
string reference=Path.Combine(pc,"..","work/yoshis-story-decomp/baseroms/us/baserom.z64");
byte[] before=File.ReadAllBytes(reference);
var rom=new VanillaRom(reference);
var source=rom.CloneFirstLevel();
source.Id="33333333333343338333333333333333";
if(source.Objects.Count!=123 || source.Objects.Count(p=>p.ObjectId==0x4001)!=1)throw new Exception("Native import differs.");
var objectTypes=source.Objects.Select(o=>o.ObjectId).Distinct().ToArray();
if(objectTypes.Length!=37 || objectTypes.Any(id=>!ObjectCatalog.All.ContainsKey(id)))throw new Exception("First-level catalog is incomplete.");
if(!objectTypes.Where(id=>!ObjectCatalog.Get(id).Identified).Order().SequenceEqual(new ushort[]{0x414F,0x44CA,0x4732}))throw new Exception("Unknown native roles must stay explicit.");
if(ObjectCatalog.Get(0x45B6).Name!="Piment" || ObjectCatalog.Get(0x4034).Name!="Melon" || ObjectCatalog.Get(0x4035).Name!="Cœur")throw new Exception("Native melon/heart identification differs.");
var store=new ProjectStore(Path.Combine(pc,"build/self-test"));
store.Save(source);var copy=store.Duplicate(source);
if(copy.Id==source.Id || copy.Objects[0].Id==source.Objects[0].Id)throw new Exception("Clone reuses identity.");
copy.Id="44444444444444448444444444444444";
float old=source.Objects[0].X;copy.Objects[0].X+=64;store.Save(copy);
if(source.Objects[0].X!=old)throw new Exception("Clone changed vanilla source.");
var loaded=store.Levels().Single(l=>l.Id==copy.Id);
if(loaded.Objects[0].X!=old+64)throw new Exception("Save lost coordinates.");
var specialLevel=store.Duplicate(source);
foreach(ushort id in ObjectCatalog.SpecialFruits){
    var fruit=rom.CreatePlacement(id);fruit.X=100+(id-0x4F00)*40;specialLevel.Objects.Add(fruit);
    if(!ObjectCatalog.Get(id).Identified || ObjectCatalog.Get(id).Category!="Fruits" || fruit.Layer!=source.Objects.First(p=>p.ObjectId==0x4030).Layer)throw new Exception("Special fruit template/catalog differs.");
}
if(ObjectCatalog.SpecialFruits.Count()!=6)throw new Exception("Special fruit count differs.");
byte[] specialPayload=rom.ExportPlacements(specialLevel,true);
if(BinaryPrimitives.ReadUInt32BigEndian(specialPayload.AsSpan(12))!=129)throw new Exception("Special placements were lost.");
bool vanillaRejected=false;try{rom.ExportPlacements(specialLevel);}catch(InvalidDataException){vanillaRejected=true;}
if(!vanillaRejected)throw new Exception("Special fruits accepted by vanilla runtime export.");
vanillaRejected=false;try{rom.BuildDataOnly(specialLevel);}catch(InvalidDataException){vanillaRejected=true;}
if(!vanillaRejected)throw new Exception("Special fruits accepted in vanilla ROM export.");
specialLevel.Objects.Last().ObjectId=0x4F06;
vanillaRejected=false;try{rom.ExportPlacements(specialLevel,true);}catch(InvalidDataException){vanillaRejected=true;}
if(!vanillaRejected)throw new Exception("Unsupported special ID accepted.");
byte[] derived=rom.BuildDataOnly(loaded);
int Bank(uint ptr)=>0xB47A10+(int)(ptr&0xFFFFFF);
uint U32(int at)=>BinaryPrimitives.ReadUInt32BigEndian(derived.AsSpan(at,4));
int info=Bank(U32(0xA84A0+43*8+4));int meta=Bank(U32(Bank(U32(info+0x5C))));int objects=Bank(U32(meta+4));
if(BinaryPrimitives.ReadUInt16BigEndian(derived.AsSpan(meta))!=123 || BitConverter.Int32BitsToSingle((int)U32(objects+4))!=old+64)throw new Exception("Export not linked into native level table.");
if(!File.ReadAllBytes(reference).SequenceEqual(before))throw new Exception("Original ROM changed.");
bool rejected=false;loaded.Objects[0].X=float.NaN;
try{rom.BuildDataOnly(loaded);}catch(InvalidDataException){rejected=true;}
if(!rejected)throw new Exception("Invalid coordinates accepted.");
loaded.Objects[0].X=old+64;
rejected=false;try{store.LevelPath("../escape");}catch(InvalidDataException){rejected=true;}
if(!rejected)throw new Exception("Unsafe path accepted.");
var game=new CustomGame {Id="55555555555545558555555555555555",Name="Essai custom",LevelIds=[source.Id,copy.Id]};store.Save(game);
if(store.Games().Single(g=>g.Id==game.Id).LevelIds.Count!=2)throw new Exception("Game order lost.");
var rules=new CoopRules();var team=new TeamProgress();
if(!rules.LocalTwoControllers || !rules.HostedNetwork || !rules.SharedFruitCounter || rules.EggsHitTeammate || rules.EggsBlockedByTeammate)throw new Exception("Requested cooperative policies not retained.");
if(!team.Collect("apple-1") || team.Collect("apple-1") || !team.Collect("apple-2") || team.FruitCount!=2)throw new Exception("Shared fruits counted twice.");
if(!team.Defeat("enemy-1") || team.Defeat("enemy-1"))throw new Exception("Enemy counted twice.");
if(TeamProgress.PlayersCollide(rules) || TeamProgress.EggCollidesWithTeammate(rules))throw new Exception("Default teammate collisions.");
rules.PlayerInteractions=true;
if(!TeamProgress.PlayersCollide(rules) || TeamProgress.EggCollidesWithTeammate(rules))throw new Exception("Egg policy follows interaction toggle.");
var runner=new NativeRunner(pc);
string engine=Path.Combine(pc,"build/native/DinoTale.exe");
byte[] engineBefore=File.ReadAllBytes(engine);
byte[] payload=rom.ExportPlacements(loaded);
string profile=runner.Prepare(before,copy.Id,payload);
if(!File.ReadAllBytes(Path.Combine(profile,"yoshisstory.n64.us.z64")).SequenceEqual(before))throw new Exception("Native profile ROM is not the original.");
if(!File.ReadAllBytes(engine).SequenceEqual(engineBefore))throw new Exception("Preparing a level modified the engine.");
if(!File.Exists(Path.Combine(profile,"portable.txt")))throw new Exception("No isolated native profile.");
rejected=false;try{runner.Prepare(derived,copy.Id,payload);}catch(InvalidDataException){rejected=true;}
if(!rejected)throw new Exception("Patched ROM accepted by source-built launcher.");
string nativeOutput=Path.Combine(pc,"build/self-test/native-loader-output.z64");
var nativeStart=new ProcessStartInfo(Path.Combine(pc,"build/native/WorkshopDataChecks.exe")){UseShellExecute=false};
nativeStart.ArgumentList.Add(reference);nativeStart.ArgumentList.Add(Path.Combine(profile,"level.yslevel"));nativeStart.ArgumentList.Add(nativeOutput);
using(var check=Process.Start(nativeStart)!){check.WaitForExit();if(check.ExitCode!=0)throw new Exception("Native data loader checks failed.");}
if(!File.ReadAllBytes(nativeOutput).SequenceEqual(derived))throw new Exception("Native and editor placement loaders disagree.");
// Use an isolated test root: never initialize or replace a user's save here.
string seedRoot=Path.Combine(pc,"build/save-seed-checks",Guid.NewGuid().ToString("N"));
string seedFolder=Path.Combine(seedRoot,"save-imports");Directory.CreateDirectory(seedFolder);
byte[] testSeed=Enumerable.Range(0,2048).Select(n=>(byte)(n*37)).ToArray();
File.WriteAllBytes(Path.Combine(seedFolder,"default.bin"),testSeed);
File.WriteAllText(Path.Combine(seedFolder,"current-import.json"),JsonSerializer.Serialize(new {convertedSha256=Convert.ToHexString(SHA256.HashData(testSeed)).ToLowerInvariant()}));
string testProfile=Path.Combine(seedRoot,"profiles/new-level");
if(!NativeSaveSeed.Initialize(seedRoot,testProfile))throw new Exception("New profile did not inherit imported progress.");
string testSave=Path.Combine(testProfile,"saves/yoshisstory.n64.us.bin");
if(!File.ReadAllBytes(testSave).SequenceEqual(testSeed))throw new Exception("Imported seed data changed.");
byte[] progressed=(byte[])testSeed.Clone();progressed[100]^=42;File.WriteAllBytes(testSave,progressed);
if(NativeSaveSeed.Initialize(seedRoot,testProfile) || !File.ReadAllBytes(testSave).SequenceEqual(progressed))throw new Exception("Existing PC progress was replaced by the seed.");
File.WriteAllBytes(Path.Combine(seedFolder,"default.bin"),new byte[2048]);
rejected=false;try{NativeSaveSeed.Initialize(seedRoot,Path.Combine(seedRoot,"profiles/bad-seed"));}catch(InvalidDataException){rejected=true;}
if(!rejected || File.Exists(Path.Combine(seedRoot,"profiles/bad-seed/saves/yoshisstory.n64.us.bin")))throw new Exception("Unverified seed was installed.");
File.WriteAllText(Path.Combine(pc,"build/save-seed-verification.json"),JsonSerializer.Serialize(new {newProfilesInheritImportedProgress=true,existingProgressPreserved=true,unverifiedSeedRejected=true,testRoot=seedRoot},ProjectStore.Json));
string contentSeedRoot=Path.Combine(pc,"build/content-seed-checks",Guid.NewGuid().ToString("N"));
string contentSeedDir=Path.Combine(contentSeedRoot,"save-imports");Directory.CreateDirectory(contentSeedDir);
byte[] earnedSeed=File.ReadAllBytes(Path.Combine(pc,"save-imports/content-default.bin"));
File.WriteAllBytes(Path.Combine(contentSeedDir,"content-default.bin"),earnedSeed);
File.Copy(Path.Combine(pc,"save-imports/content-import.json"),Path.Combine(contentSeedDir,"content-import.json"));
string earnedProfile=Path.Combine(contentSeedRoot,"personal");
if(!ContentSaveSeed.Initialize(contentSeedRoot,earnedProfile))throw new Exception("Earned Yoshi seed was not installed.");
string earnedSave=Path.Combine(earnedProfile,"saves/yoshisstory.n64.us.bin");
if(!File.ReadAllBytes(earnedSave).SequenceEqual(earnedSeed))throw new Exception("Earned unlock save changed.");
byte[] newerProgress=(byte[])earnedSeed.Clone();newerProgress[200]^=1;File.WriteAllBytes(earnedSave,newerProgress);
if(ContentSaveSeed.Initialize(contentSeedRoot,earnedProfile) || !File.ReadAllBytes(earnedSave).SequenceEqual(newerProgress))throw new Exception("Existing earned Yoshi progress was replaced.");
File.WriteAllBytes(Path.Combine(contentSeedDir,"content-default.bin"),new byte[2048]);
rejected=false;try{ContentSaveSeed.Initialize(contentSeedRoot,Path.Combine(contentSeedRoot,"bad"));}catch(InvalidDataException){rejected=true;}
if(!rejected)throw new Exception("Unverified content save accepted.");
File.WriteAllBytes(Path.Combine(contentSeedDir,"content-default.bin"),earnedSeed);
File.WriteAllText(Path.Combine(contentSeedDir,"content-import.json"),JsonSerializer.Serialize(new{seedSha256=Convert.ToHexStringLower(SHA256.HashData(earnedSeed)),testUnlocksUsed=true,earnedUnlocksPreserved=true}));
rejected=false;try{ContentSaveSeed.Initialize(contentSeedRoot,Path.Combine(contentSeedRoot,"forced-unlocks"));}catch(InvalidDataException){rejected=true;}
if(!rejected)throw new Exception("Forced test unlocks accepted as personal progress.");
File.WriteAllText(Path.Combine(pc,"build/content-seed-verification.json"),JsonSerializer.Serialize(new{earnedSeedInstalled=true,existingContentProgressPreserved=true,unverifiedSeedRejected=true,testUnlockSeedRejected=true,nativeObjectTypesCataloged=objectTypes.Length,unconfirmedObjectRoles=3,testRoot=contentSeedRoot},ProjectStore.Json));
string latest=Path.Combine(contentSeedRoot,"profiles/content-personal/saves/yoshisstory.n64.us.bin");Directory.CreateDirectory(Path.GetDirectoryName(latest)!);
// Test an evolved but complete personal save. Neither an archive nor the
// all-unlocked test fixture may replace this newer source.
byte[] recent=(byte[])earnedSeed.Clone();recent[200]^=1;recent[1224]^=1;File.WriteAllBytes(latest,recent);
string customProfile=Path.Combine(contentSeedRoot,"profiles/custom-content-check");
if(!SpecialSaveSeed.Initialize(contentSeedRoot,customProfile))throw new Exception("Custom profile did not copy recent personal progress.");
string customSave=Path.Combine(customProfile,"saves/yoshisstory.n64.us.bin");
if(!File.ReadAllBytes(customSave).SequenceEqual(recent))throw new Exception("Custom profile inherited stale progress.");
byte[] customProgress=(byte[])recent.Clone();customProgress[300]^=7;File.WriteAllBytes(customSave,customProgress);
if(SpecialSaveSeed.Initialize(contentSeedRoot,customProfile) || !File.ReadAllBytes(customSave).SequenceEqual(customProgress))throw new Exception("Custom progress was reseeded.");
if(!File.ReadAllBytes(latest).SequenceEqual(recent))throw new Exception("Preparing custom progress changed personal save.");
File.WriteAllText(Path.Combine(pc,"build/special-editor-verification.json"),JsonSerializer.Serialize(new{sixFruitsCataloged=true,independentPlacementTemplates=true,specialExportEnabled=true,vanillaExportsRejectSpecialIds=true,unsupportedIdsRejected=true,newCustomProfilesCopyLatestPersonalProgress=true,existingCustomProgressPreserved=true,personalSourceUnchanged=true},ProjectStore.Json));
var specialRunner=new SpecialContentRunner(pc);
if(specialRunner.Available){
    string personalSave=Path.Combine(pc,"profiles/content-personal/saves/yoshisstory.n64.us.bin");
    byte[] personalBefore=File.ReadAllBytes(personalSave);
    string specialProfile=specialRunner.Prepare(copy.Id,rom.ExportPlacements(source,true));
    if(!File.ReadAllBytes(Path.Combine(specialProfile,"yoshisstory.n64.us.z64")).SequenceEqual(before))throw new Exception("Special profile changed original ROM.");
    if(!File.ReadAllBytes(Path.Combine(specialProfile,"level.yslevel")).SequenceEqual(rom.ExportPlacements(source,true)))throw new Exception("Special profile lost placements.");
    string demo=specialRunner.Prepare("demo",File.ReadAllBytes(Path.Combine(pc,"build/special-port/six-fruits-demo.yslevel")),true);
    if(specialProfile==demo || !File.ReadAllBytes(personalSave).SequenceEqual(personalBefore))throw new Exception("Special profile preparation modified personal progression.");
    File.WriteAllText(Path.Combine(pc,"build/special-launcher-verification.json"),JsonSerializer.Serialize(new{verifiedEngineSelected=true,originalRomPreserved=true,customPlacementsPreserved=true,personalSourceUnchanged=true,demoAndCustomProfilesSeparate=true,specialProfile,demo},ProjectStore.Json));
}
var report=new {importedNativeObjects=123,cloneIsIndependent=true,roundTripCoordinates=true,nativeLevelPointersVerified=true,originalRomUnchanged=true,profileRomUnchanged=true,engineUnchanged=true,nativeLoaderMatchesEditor=true,nativeMalformedFilesRejected=6,invalidCoordinatesRejected=true,pathTraversalRejected=true,customGameOrderRetained=true,sharedFruitDeduplication=true,sharedEnemyDeduplication=true,teammateEggsAlwaysPass=true,localAndHostedModesConfigured=true,profile,derivedRomSha256=Convert.ToHexString(SHA256.HashData(derived)).ToLowerInvariant(),gameplayValidation="pending",coopEngineIntegration="pending"};
File.WriteAllText(Path.Combine(pc,"build/core-verification.json"),JsonSerializer.Serialize(report,ProjectStore.Json));
Console.WriteLine(JsonSerializer.Serialize(report,ProjectStore.Json));
