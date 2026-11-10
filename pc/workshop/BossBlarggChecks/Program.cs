using System.Text.Json;
using YoshiWorkshop;

string output=Path.GetFullPath(args[0]),baseline=Path.GetFullPath(args[1]);
if(Directory.Exists(output))throw new InvalidOperationException("Refuse overwriting a previous isolated result.");
Directory.CreateDirectory(output);int assertions=0,rejections=0;
void Need(bool yes,string why){assertions++;if(!yes)throw new Exception(why);}
void Reject(Action action){try{action();}catch(InvalidDataException){rejections++;return;}throw new Exception("Invalid opt-in configuration admitted.");}
string Json<T>(T value)=>BossBattleJson.Serialize(value);
var legacy=new BossBattleConfiguration();
Need(!legacy.IncludeBlargg && Json(legacy)==File.ReadAllText(Path.Combine(baseline,"legacy-config.json")),"Default legacy config JSON changed.");
Need(!Json(legacy).Contains("IncludeBlargg",StringComparison.Ordinal),"Disabled flag changed persisted JSON.");
Need(Enum.GetValues<BossBattleFamily>().Select(f=>(int)f).SequenceEqual(Enumerable.Range(0,8)),"Family numeric identities shifted.");
Need((int)BossBattleFamily.RedCandyFinal==6 && (int)BossBattleFamily.DoubleBlargg==7,"Appended family changed final identity.");
Need(BossBattleCatalog.OrdinaryFamilies.Select(f=>(int)f).SequenceEqual(Enumerable.Range(0,6)),"Legacy six-family order changed.");
using var captured=JsonDocument.Parse(File.ReadAllText(Path.Combine(baseline,"legacy-plans.json")));
int legacyPlans=0;
foreach(var row in captured.RootElement.EnumerateArray()){
 ulong seed=row.GetProperty("seed").GetUInt64();var p=BossBattlePlanner.Create(seed,legacy);p.Validate();
 Need(Json(p)==row.GetProperty("serialized").GetString(),$"Legacy serialized plan changed at seed{seed}.");
 Need(p.Fingerprint()==row.GetProperty("fingerprint").GetString(),$"Legacy fingerprint changed at seed{seed}.");
 Need(p.RulesVersion=="boss-draft-2" && !p.Encounters.Any(e=>e.Family==BossBattleFamily.DoubleBlargg),"Legacy mode includes new rules or a new family.");legacyPlans++;
}
Need(legacyPlans==302,"Incomplete pre-change legacy corpus.");
using(var uiCases=JsonDocument.Parse(File.ReadAllText(Path.Combine(baseline,"legacy-ui-configs.json"))))foreach(var row in uiCases.RootElement.EnumerateArray()){
 int slot=row.GetProperty("slot").GetInt32();var uiConfig=new BossBattleConfiguration{ArenaSlots=BossBattleCatalog.Families.Where(f=>f.Family!=BossBattleFamily.DoubleBlargg).ToDictionary(f=>f.Family,f=>slot)};
 var uiPlan=BossBattlePlanner.Create(42,uiConfig);uiPlan.Validate();
 Need(uiConfig.ArenaSlots.Count==7 && !uiConfig.ArenaSlots.ContainsKey(BossBattleFamily.DoubleBlargg),"Legacy UI includes a disabled cave slot.");
 Need(Json(uiConfig)==row.GetProperty("configuration").GetString() && Json(uiPlan)==row.GetProperty("plan").GetString() && uiPlan.Fingerprint()==row.GetProperty("fingerprint").GetString(),"Legacy UI configuration changed JSON or seed plan.");
}

var expanded=BossBattleConfiguration.WithBlargg();expanded.Validate();
var guaranteed=new HashSet<BossBattleFamily>{BossBattleFamily.YellowCandy,BossBattleFamily.Invisible,BossBattleFamily.Genie,BossBattleFamily.GiantLips,BossBattleFamily.GiantFish,BossBattleFamily.DoubleBlargg};
Need(expanded.IncludeBlargg && expanded.RandomFamilies.Count==7 && expanded.RandomFamilies.Contains(BossBattleFamily.PinkWhiteMonsters) && expanded.RandomFamilies.Contains(BossBattleFamily.DoubleBlargg) && !expanded.RandomFamilies.Contains(BossBattleFamily.RedCandyFinal),"Expanded random pool is wrong.");
Need(expanded.VariantPools.Count==7 && expanded.VariantPools[BossBattleFamily.DoubleBlargg].SequenceEqual(new[]{"double-three-hits"}),"Blargg mechanics invented or missing.");
var specs=BossBattleCatalog.Family(BossBattleFamily.DoubleBlargg);
Need(specs.Arenas.Select(a=>a.Slot).SequenceEqual(new[]{0,1,2}) && specs.Arenas.All(a=>!a.Jungle && !a.GeometryImplemented),"Cave slots absent or falsely certified.");
Need(specs.Variants.Single().Combatants.Count==2 && specs.Variants.Single().Combatants.All(b=>b.Kind=="blargg" && b.Hits==3 && b.Size==BossBattleSize.Native),"Expected two native-sized three-hit members.");
var randomSeen=new HashSet<BossBattleFamily>();int randomMonsters=0,randomBlargg=0;
for(ulong seed=0;seed<500;seed++){
 var p=BossBattlePlanner.Create(seed,expanded);p.Validate();
 Need(p.RulesVersion==BossBattleCatalog.BlarggRulesVersion && p.Encounters.Count==10,"Expanded plan count/version wrong.");
 Need(p.Encounters[0] is{Family:BossBattleFamily.YellowCandy,Variant:"solo",GenieCompanion:false} && p.Encounters[^1] is{Family:BossBattleFamily.RedCandyFinal,Variant:"final"},"First/final invariant changed.");
 Need(p.Encounters.Take(6).Select(e=>e.Family).ToHashSet().SetEquals(guaranteed),"Guaranteed six include the wrong replacement.");
 Need(p.Encounters.Count(e=>e.Family==BossBattleFamily.RedCandyFinal)==1 && p.Encounters.Skip(6).Take(3).All(e=>e.Family!=BossBattleFamily.RedCandyFinal),"Final boss appeared in random pool.");
 foreach(var e in p.Encounters.Where(e=>e.Family==BossBattleFamily.DoubleBlargg))Need(e.Variant=="double-three-hits" && e.Combatants(p.Configuration).Count==2 && e.Combatants(p.Configuration).All(b=>b.Hits==3),"Blargg roster changed after planning.");
 foreach(var e in p.Encounters.Skip(6).Take(3)){randomSeen.Add(e.Family);if(e.Family==BossBattleFamily.PinkWhiteMonsters)randomMonsters++;if(e.Family==BossBattleFamily.DoubleBlargg)randomBlargg++;}
 var clone=BossBattleJson.Clone(p);clone.Validate();Need(Json(clone)==Json(p) && clone.Fingerprint()==p.Fingerprint(),"Expanded persistence roundtrip changed plan.");
 Need(Json(p)==Json(BossBattlePlanner.Create(seed,expanded)),"Expanded plan not deterministic.");
 Need(BossBattlePresentation.Preparation(p).All(v=>!v.Revealed && v.FamilyLabel==null && v.VariantLabel==null),"Preparation revealed upcoming bosses.");
}
Need(randomSeen.SetEquals(expanded.RandomFamilies) && randomMonsters>0 && randomBlargg>0,"Random pool failed to retain both Monsters and Blargg.");
foreach(int slot in Enumerable.Range(0,3)){
 var config=expanded with{ArenaSlots=new(){[BossBattleFamily.DoubleBlargg]=slot}};
 var p=BossBattlePlanner.Create(100,config);Need(p.Encounters.Where(e=>e.Family==BossBattleFamily.DoubleBlargg).All(e=>e.ArenaSlot==slot),"Explicit cave slot was lost.");
}
Reject(()=>(legacy with{IncludeBlargg=true}).Validate());
Reject(()=>(expanded with{IncludeBlargg=false}).Validate());
Reject(()=>(legacy with{RandomFamilies=[BossBattleFamily.DoubleBlargg]}).Validate());
Reject(()=>(legacy with{ArenaSlots=new(){[BossBattleFamily.DoubleBlargg]=0}}).Validate());
Reject(()=>(expanded with{RandomFamilies=[BossBattleFamily.RedCandyFinal]}).Validate());
Reject(()=>(expanded with{ArenaSlots=new(){[BossBattleFamily.DoubleBlargg]=3}}).Validate());
var invalidPool=BossBattleJson.Clone(expanded);invalidPool.VariantPools[BossBattleFamily.DoubleBlargg]=["four-hits"];Reject(invalidPool.Validate);
var modern=BossBattlePlanner.Create(42,expanded);var oldPlan=BossBattlePlanner.Create(42,legacy);
foreach(var bad in new[]{modern with{RulesVersion=BossBattleCatalog.RulesVersion},oldPlan with{RulesVersion=BossBattleCatalog.BlarggRulesVersion},modern with{RulesVersion="boss-blargg-0"}}){string prior=Json(bad);Reject(bad.Validate);Need(Json(bad)==prior,"Version rejection mutated the input plan.");}
var forged=BossBattleJson.Clone(modern);int index=forged.Encounters.FindIndex(e=>e.Family==BossBattleFamily.DoubleBlargg);forged.Encounters[index]=forged.Encounters[index] with{Variant="solo"};Reject(forged.Validate);
var missing=BossBattleAdmission.Missing(modern,new());Need(missing.Contains("native-runtime-unverified") && missing.Contains("encounter-unverified:DoubleBlargg/double-three-hits") && missing.Contains("actor:blargg"),"Unverified native Blargg falsely admitted.");
var session=BossBattleSession.Create(modern,0,[new BossTreeSlot("11111111111141118111111111111111",0x4030)],BossEvidenceKind.SimulationQa);
string file=Path.Combine(output,"model-session.json");BossBattleStore.SaveSession(file,session);string saved=File.ReadAllText(file);var reloaded=BossBattleStore.LoadSession(file);
Need(reloaded.Plan.Fingerprint()==session.Plan.Fingerprint() && File.ReadAllText(file)==saved,"Reading new session changed saved file.");
var malformed=BossBattleJson.Clone(session);malformed.Plan=malformed.Plan with{RulesVersion=BossBattleCatalog.RulesVersion};string badFile=Path.Combine(output,"invalid-version-session.json");File.WriteAllText(badFile,Json(malformed));string badBytes=File.ReadAllText(badFile);Reject(()=>BossBattleStore.LoadSession(badFile));Need(File.ReadAllText(badFile)==badBytes,"Rejecting invalid session rewrote its file.");
File.WriteAllText(Path.Combine(output,"verification.json"),Json(new{passed=true,scope="Pure C# plan/data checks; no game or native bodies",assertions,rejections,legacyPlans,legacyJsonAndFingerprintsUnchanged=true,legacyUiSevenSlotConfigurations=3,expandedSeeds=500,randomMonsters,randomBlargg,idsPreserved=true,optInDefaultOff=true,ruleVersionPairedWithFlag=true,threeCaveSlots=true,twoThreeHitMembers=true,hiddenFutureBosses=true,nativeAdmissionBlocked=true,invalidFilePreserved=true,personalProgressAccessed=false}));
Console.WriteLine($"Blargg C# PASS: {assertions} assertions,{rejections} rejections,{legacyPlans} legacy fingerprints unchanged. No native gameplay proven.");
