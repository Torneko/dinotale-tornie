using System.Security.Cryptography;
using System.Text.Json;
using YoshiWorkshop;

// Run only against an extracted disposable archive, never a player profile.
string root=Path.GetFullPath(args[0]);
if(!root.Contains("portable-validation",StringComparison.OrdinalIgnoreCase))throw new Exception("Disposable extraction required.");
string personal=Path.Combine(root,"profiles/content-personal");
string[] protectedNames=["saves/yoshisstory.n64.us.bin","pc-achievements.json","pc-collection.json","pc-hybrid-records.json","pc-hybrid-finds.json"];
string Hash(string path)=>Convert.ToHexStringLower(SHA256.HashData(File.ReadAllBytes(path)));
var before=protectedNames.Where(n=>File.Exists(Path.Combine(personal,n))).ToDictionary(n=>n,n=>Hash(Path.Combine(personal,n)));
var runner=new SpecialContentRunner(root);runner.Verify();
var rom=new VanillaRom(Path.Combine(root,"../work/yoshis-story-decomp/baseroms/us/baserom.z64"));
var level=rom.CloneFirstLevel();level.Objects.Add(rom.CreatePlacement(0x4F00));
string profile=runner.Prepare("77777777777747778777777777777777",rom.ExportPlacements(level,true));
foreach(var entry in before)if(Hash(Path.Combine(profile,entry.Key))!=entry.Value)throw new Exception("Progression ledger not copied: "+entry.Key);
if(JsonDocument.Parse(File.ReadAllText(Path.Combine(profile,"sound.json"))).RootElement.GetProperty("main_volume").GetInt32()!=10)throw new Exception("Custom test volume differs.");
// A played custom ledger must survive the next launch.
string customLedger=Path.Combine(profile,"pc-achievements.json");
File.WriteAllText(customLedger,"{\"portableCheck\":true}");
runner.Prepare("77777777777747778777777777777777",rom.ExportPlacements(level,true));
if(File.ReadAllText(customLedger)!="{\"portableCheck\":true}")throw new Exception("Existing custom ledger overwritten.");
foreach(var entry in before)if(Hash(Path.Combine(personal,entry.Key))!=entry.Value)throw new Exception("Personal source modified.");
Console.WriteLine(JsonSerializer.Serialize(new{relocatedRoot=root,verifiedSpecialRuntime=true,customLevelPrepared=true,allProgressLedgersInherited=true,existingCustomProgressPreserved=true,personalSourceUnchanged=true,volumePercent=10,profile}));
