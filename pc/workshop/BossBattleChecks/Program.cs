using System.Security.Cryptography;
using System.Text.Json;
using YoshiWorkshop;

string output=Path.GetFullPath(args.Length==0?"../../build/boss-battle-preparation":args[0]);Directory.CreateDirectory(output);
int assertions=0,rejections=0;
void Need(bool ok,string error){assertions++;if(!ok)throw new Exception(error);}
void Reject(Action action){try{action();}catch(InvalidDataException){rejections++;return;}throw new Exception("Invalid boss-battle operation admitted.");}
string Json<T>(T value)=>BossBattleJson.Serialize(value);
void Atomic(BossBattleSession s,Action action){string before=Json(s);Reject(action);Need(Json(s)==before,"Rejected session operation changed state.");}
var config=new BossBattleConfiguration();var plan=BossBattlePlanner.Create(0x123456789ABCDEF0UL,config);plan.Validate();
string[] nativeNames=["Vert","Rouge","Jaune","Bleu","Bleu pâle","Rose","Noir","Blanc","Orange","Violet","Turquoise","Lime-céleri","Brun","Vert foncé"];
int[] legacyColors=[0,5,4,2,3,1,7,6,8,9,10,11,12,13];
for(int color=0;color<14;color++){Need(BossBattleYoshiColors.FrenchName(color)==nativeNames[color],"Native Yoshi colour name differs");Need(BossBattleYoshiColors.FromLegacyPicker(color)==legacyColors[color],"Legacy picker selection did not preserve its named Yoshi");}
Need(legacyColors.Distinct().Count()==14,"Legacy colour conversion aliases two Yoshis");Reject(()=>BossBattleYoshiColors.FromLegacyPicker(-1));Reject(()=>BossBattleYoshiColors.FromLegacyPicker(14));Reject(()=>BossBattleYoshiColors.FrenchName(14));
Need(plan.Encounters.Count==10 && plan.Encounters[0] is {Family:BossBattleFamily.YellowCandy,Variant:"solo",GenieCompanion:false,ArenaSlot:0} && plan.Encounters[9] is {Family:BossBattleFamily.RedCandyFinal,Variant:"final",GenieCompanion:false},"First/final battle contract changed.");
Need(plan.Encounters.Take(6).Select(e=>e.Family).Distinct().Count()==6 && plan.Encounters.Skip(6).Take(3).All(e=>e.Family!=BossBattleFamily.RedCandyFinal),"Guaranteed family coverage or final-only rule changed.");
Need(Json(plan)==Json(BossBattlePlanner.Create(plan.Seed,config)),"Seed is not deterministic.");
var distinct=new HashSet<string>();var purple=new HashSet<int>();var fishVariants=new HashSet<string>();var fishCompanions=new HashSet<string>();int replacements=0,eligible=0;bool companionSeen=false,soloWithoutCompanionSeen=false;
var onlyLaterCandy=config with{RandomFamilies=[BossBattleFamily.YellowCandy],VariantPools=config.VariantPools.ToDictionary(p=>p.Key,p=>p.Key==BossBattleFamily.YellowCandy?new List<string>{"solo"}:p.Value.ToList())};
for(ulong seed=0;seed<300;seed++){
 var p=BossBattlePlanner.Create(seed,config);Need(p.Encounters[0].GenieCompanion==false,"First candy received a genie.");Need(p.Encounters.Take(6).Select(e=>e.Family).Distinct().Count()==6,"Some seed omitted a family.");
 distinct.Add(string.Join(',',p.Encounters.Skip(1).Take(5).Select(e=>e.Family)));
 foreach(var fish in p.Encounters.Where(e=>e.Family==BossBattleFamily.GiantFish)){
  var bosses=fish.Combatants(p.Configuration);var swimmers=bosses.Where(b=>b.Kind.EndsWith("-fish",StringComparison.Ordinal)).ToArray();fishVariants.Add(fish.Variant);purple.Add(fish.PurpleFishCount);
  int count=fish.Variant=="three-one-hit"?3:fish.Variant=="two-two-hits"?2:1;
  Need(swimmers.Length==count && bosses.Count==(count==3?3:2) && swimmers.All(b=>b.Kind is "red-fish" or "green-fish" or "purple-fish"),"Approved fish count and red/green/purple roster differ.");
  Need(swimmers.All(b=>b.Hits==(count==3?1:count==2?2:b.Hits) && (count!=1 || b.Hits is 2 or 3)),"Fish per-member HP differs from its approved variant.");
  Need(fish.PurpleFishCount==swimmers.Count(b=>b.Kind=="purple-fish") && (fish.PurpleFishCount==1?fish.SinglePurpleFishSlot is int slot && swimmers[slot].Kind=="purple-fish":fish.SinglePurpleFishSlot==null),"Remembered purple count/slot disagrees with the concrete roster.");
  if(count==1){var companion=bosses[^1];fishCompanions.Add(companion.Kind);Need(companion.Id=="fish-companion" && companion.Kind is "genie" or "invisible" && companion.Hits==2 && fish.GenieCompanion==(companion.Kind=="genie"),"Solo fish requires exactly one remembered two-HP companion.");}
  else for(int i=0;i<count;i++){eligible++;if(swimmers[i].Kind!=new[]{"red-fish","green-fish","purple-fish"}[i])replacements++;}
 }
 foreach(var candy in BossBattlePlanner.Create(seed,onlyLaterCandy).Encounters.Skip(6).Take(3)){
  companionSeen|=candy.GenieCompanion;soloWithoutCompanionSeen|=!candy.GenieCompanion;Need(candy.Combatants(config).Count==(candy.GenieCompanion?2:1),"Genie companion count incorrect.");if(candy.GenieCompanion)Need(candy.Combatants(config).Last().Hits==2,"Companion genie must take two hits.");
 }
 Need(p.Encounters.Where(e=>e.Family==BossBattleFamily.YellowCandy && e.Variant!="solo").All(e=>!e.GenieCompanion),"Multiple candy encounter received companion chance.");
}
Need(distinct.Count>50 && purple.Contains(0) && purple.Contains(1) && purple.All(n=>n is >=0 and <=3) && fishVariants.SetEquals(["three-one-hit","two-two-hits","single-with-companion"]) && fishCompanions.SetEquals(["genie","invisible"]) && companionSeen && soloWithoutCompanionSeen,"Deterministic plan variation not exercised.");
Need(eligible>300 && replacements>0 && replacements*100>eligible && replacements*100<eligible*10,"Per-fish replacement sample is absent or outside the bounded five-percent regression range.");
Need(string.Join('|',plan.Encounters.Where(e=>e.Family!=BossBattleFamily.GiantFish).Select(e=>$"{e.Number}:{e.Family}:{e.Variant}:{e.ArenaSlot}:{e.GenieCompanion}"))=="1:YellowCandy:solo:0:False|3:PinkWhiteMonsters:four-hits::False|4:GiantLips:with-invisible::False|5:Invisible:solo::False|6:Genie:two-two-hits::False|7:YellowCandy:solo::False|8:Invisible:two-two-hits::False|9:PinkWhiteMonsters:four-hits::False|10:RedCandyFinal:final::False","New fish choices changed the previously verified C# non-fish seed topology.");
using(var manifest=JsonDocument.Parse(Json(BossBattleCatalog.Manifest()))){var fish=manifest.RootElement.GetProperty("fish");var violet=manifest.RootElement.GetProperty("purpleFish");Need(fish.GetProperty("replacementPercentPerFish").GetInt32()==5 && fish.GetProperty("otherColorsEquiprobable").GetBoolean() && fish.GetProperty("companionHits").GetInt32()==2,"Manifest does not describe the active per-fish replacement and companion rules.");Need(!violet.GetProperty("emitsWater").GetBoolean() && violet.GetProperty("phaseOrder").EnumerateArray().Select(x=>x.GetString()).SequenceEqual(new[]{"arc","vertical"}),"Purple manifest retains the obsolete water cycle.");}
foreach(var family in new[]{BossBattleFamily.Invisible,BossBattleFamily.Genie}){
 var v=BossBattleCatalog.Family(family).Variants;Need(v[1].Combatants.Count==2 && v[1].Combatants.All(b=>b.Hits==2) && v[2].Combatants.Count==4 && v[2].Combatants.All(b=>b.Hits==1),"Invisible/genie health contract changed.");
}
Need(BossBattleCatalog.Families.Where(f=>f.Family!=BossBattleFamily.DoubleBlargg).All(f=>f.Arenas.Count==3 && f.Arenas.All(a=>a.Jungle && !a.GeometryImplemented)),"Legacy jungle arenas falsely implemented or not three.");
Need(BossBattleCatalog.Family(BossBattleFamily.DoubleBlargg).Arenas.Count==3 && BossBattleCatalog.Family(BossBattleFamily.DoubleBlargg).Arenas.All(a=>!a.Jungle && !a.GeometryImplemented),"Blargg cave arena planning is invalid or falsely marked implemented.");
Need(BossBattleCatalog.Family(BossBattleFamily.PinkWhiteMonsters).Variants.All(v=>v.Combatants.Count==2 && v.NoFlowers && v.NoEggBlocks),"2-3 arena sources changed.");
Need(BossBattleCatalog.Family(BossBattleFamily.RedCandyFinal).Variants.Single().Combatants.Single().Size==BossBattleSize.Native,"Final candy size shrank.");
Need(plan.Encounters.Where(e=>e.Number!=1).Any(e=>e.ArenaSlot==null),"Unknown arena choices were invented.");
Need(BossBattleAdmission.Missing(plan,new()).Contains("native-runtime-unverified") && BossBattleAdmission.Missing(plan,new()).Contains("random-distribution-unconfirmed"),"Incomplete native admission not blocked.");
Reject(()=>BossBattleAdmission.RequireReady(plan,new()));
Reject(()=>BossBattlePlanner.Create(1,config with{RandomFamilies=[BossBattleFamily.RedCandyFinal]}));
var oldRules=BossBattleJson.Clone(plan) with{RulesVersion="boss-draft-1"};string oldRulesBytes=Json(oldRules);Reject(oldRules.Validate);Need(Json(oldRules)==oldRulesBytes,"Rejecting an old rule set changed its saved data.");
var badFish=BossBattleJson.Clone(plan);int fishIndex=badFish.Encounters.FindIndex(e=>e.Family==BossBattleFamily.GiantFish);badFish.Encounters[fishIndex]=badFish.Encounters[fishIndex] with{FishCombatants=null};Reject(badFish.Validate);
badFish=BossBattleJson.Clone(plan);var forgedFish=badFish.Encounters[fishIndex].FishCombatants!.ToArray();forgedFish[0]=forgedFish[0] with{Kind="blue-fish",Hits=99};badFish.Encounters[fishIndex]=badFish.Encounters[fishIndex] with{FishCombatants=forgedFish};Reject(badFish.Validate);
Reject(()=>BossBattlePlanner.Create(1,config with{SmallCandyScale=1}));
Reject(()=>BossBattlePlanner.Create(1,config with{RedYellowWindowSeconds=double.NaN}));
// Explicit synthetic values exercise positive admission; these are not game defaults.
var readyConfig=config with{
 RandomPolicy=BossRandomPolicy.UniformConfirmed,SmallCandyScale=.6,RedYellowWindowSeconds=2,RedYellowHealing=1,
 ArenaSlots=BossBattleCatalog.Families.Where(f=>f.Family!=BossBattleFamily.DoubleBlargg).ToDictionary(f=>f.Family,f=>1),
 HitOverrides=BossBattleCatalog.Families.SelectMany(f=>f.Variants.SelectMany(v=>v.Combatants.Where(b=>!b.Hits.HasValue).Select(b=>new KeyValuePair<string,int>($"{f.Family}/{v.Id}/{b.Id}",3)))).ToDictionary(p=>p.Key,p=>p.Value),
 BirdCounts=BossBattleCatalog.Families.SelectMany(f=>f.Variants.Select(v=>$"{f.Family}/{v.Id}")).ToDictionary(k=>k,k=>2),
 BirdRespawnSeconds=BossBattleCatalog.Families.SelectMany(f=>f.Variants.Select(v=>$"{f.Family}/{v.Id}")).ToDictionary(k=>k,k=>5d)
};
var readyPlan=BossBattlePlanner.Create(1,readyConfig);
var capabilities=new BossBattleCapabilities{NativeIntegrationVerified=true,RuntimeSha256=new string('b',64),VerifiedArenas=BossBattleCatalog.Families.SelectMany(f=>f.Arenas).Select(a=>a.Id).ToHashSet(),Features=new(){"native-results","boss-session-transport","persistent-rest-tree","tree-no-invincibility","renewable-enemies-zero-score","purple-fish-alternating-jumps","yellow-birds","red-candy-color-healing","red-candy-30s-fire-roll","red-candy-native-size","yellow-candy-genie-companion","actor:purple-fish"}};
foreach(var f in BossBattleCatalog.Families)foreach(var v in f.Variants){string key=$"{f.Family}/{v.Id}";capabilities.Features.Add("encounter:"+key);capabilities.Features.Add("arena-no-flowers:"+key);capabilities.Features.Add("arena-no-egg-blocks:"+key);foreach(var b in v.Combatants)capabilities.Features.Add("actor:"+b.Kind);}
Need(BossBattleAdmission.Missing(readyPlan,capabilities).Count==0,"Fully supplied synthetic admission is permanently blocked.");
foreach(string capability in new[]{"native-results","tree-no-invincibility","encounter:YellowCandy/solo","red-candy-30s-fire-roll"}){var copy=capabilities with{Features=capabilities.Features.Where(x=>x!=capability).ToHashSet()};Need(BossBattleAdmission.Missing(readyPlan,copy).Count>0,"Missing runtime capability admitted.");}
var noAttestation=capabilities with{NativeIntegrationVerified=false};Need(BossBattleAdmission.Missing(readyPlan,noAttestation).Contains("native-runtime-unverified"),"Unverified native attestation admitted.");
Reject(()=>(plan with{Configuration=null!}).Validate());
Reject(()=>(plan with{Encounters=null!}).Validate());
Reject(()=>(config with{VariantPools=null!}).Validate());
Reject(()=>(config with{RandomFamilies=null!}).Validate());
var nullPool=BossBattleJson.Clone(config);nullPool.VariantPools[BossBattleFamily.Genie]=null!;Reject(nullPool.Validate);
var altered=BossBattleJson.Clone(plan);altered.Encounters[0]=altered.Encounters[0] with{GenieCompanion=true};Reject(altered.Validate);
var alteredSeed=BossBattleJson.Clone(plan) with{Seed=plan.Seed+1};Reject(alteredSeed.Validate);
var slots=new[]{new BossTreeSlot("11111111111141118111111111111111",0x4030),new BossTreeSlot("22222222222242228222222222222222",0x4F00)};
var session=BossBattleSession.Create(plan,13,slots,BossEvidenceKind.SimulationQa);var ledger=new BossBattleChallengeLedger();
Atomic(session,()=>session.CollectTreeFruit(slots[0].Id));Atomic(session,session.UseRestPot);
var nativeIncomplete=BossBattleSession.Create(plan,0,slots);Atomic(nativeIncomplete,()=>nativeIncomplete.EnterCombat(new()));
var nativeWithoutHash=BossBattleJson.Clone(nativeIncomplete);nativeWithoutHash.Phase=BossBattlePhase.Combat;nativeWithoutHash.ExpectedToken=Guid.NewGuid().ToString("N");Reject(nativeWithoutHash.Validate);
var failedWithoutHash=BossBattleJson.Clone(nativeIncomplete);failedWithoutHash.Phase=BossBattlePhase.Failed;Reject(failedWithoutHash.Validate);
var badReadyHash=BossBattleJson.Clone(nativeIncomplete);badReadyHash.RuntimeSha256="not-a-native-sha";Reject(badReadyHash.Validate);
BossBattleCombatReceipt Receipt(BossBattleSession s,string token,bool won=true)=>new(){Token=token,EncounterId=s.CurrentCombat!.Id,RunId=s.RunId,PlanFingerprint=s.PlanFingerprint,YoshiColor=s.YoshiColor,Evidence=s.Evidence,RuntimeSha256=s.RuntimeSha256,Won=won,DefeatedCombatants=s.CurrentCombat.Combatants(s.Plan.Configuration).Select(b=>b.Id).ToList()};
for(int i=0;i<10;i++){
 Need(session.CombatIndex==i && session.Phase==BossBattlePhase.ReadyForCombat,"Wrong next encounter phase.");string token=session.EnterCombat(new());
 Need(!session.RecordCombatResult(Receipt(session,new string('0',32))),"Wrong callback token accepted.");
 Need(!session.RecordCombatResult(Receipt(session,token) with{YoshiColor=(session.YoshiColor+1)%14}),"Other Yoshi callback accepted.");
 Need(!session.RecordCombatResult(Receipt(session,token) with{RunId=Guid.NewGuid().ToString("N")}),"Other run callback accepted.");
 Need(!session.RecordCombatResult(Receipt(session,token) with{PlanFingerprint=new string('f',64)}),"Other plan callback accepted.");
 var missing=Receipt(session,token) with{DefeatedCombatants=[]};Need(!session.RecordCombatResult(missing),"Won without defeating every boss.");
 var result=Receipt(session,token);Need(session.RecordCombatResult(result),"Model combat result rejected.");Need(!session.RecordCombatResult(result),"Duplicate callback advanced the run twice.");
 if(i==9)break;
 Need(session.Phase==BossBattlePhase.Rest && !session.TreeGrantsInvincibility,"Missing safe rest phase.");
 if(i==0){Atomic(session,session.RegenerateTree);var pickup=session.CollectTreeFruit(slots[0].Id);Need(!pickup.GrantInvincibility && pickup.Cycle==0 && session.RemainingTreeFruit().Count==1,"Tree pickup/reset incorrect.");Atomic(session,()=>session.CollectTreeFruit(slots[0].Id));Atomic(session,session.RegenerateTree);}
 if(i==1){Need(session.RemainingTreeFruit().Count==1,"Room revisit regenerated consumed fruit.");Atomic(session,session.RegenerateTree);session.CollectTreeFruit(slots[1].Id);Need(session.RemainingTreeFruit().Count==0,"Consumed batch is not empty");session.RegenerateTree();session.CollectTreeFruit(slots[0].Id);Need(session.ConsumedTreeFruitIds.Count==3,"Regeneration forgot consumption history.");Atomic(session,session.RegenerateTree);}
 if(i==2){Need(session.RemainingTreeFruit().Count==1,"Partial second batch did not persist across combats");session.CollectTreeFruit(slots[1].Id);session.RegenerateTree();Need(session.TreeRegenerations==2 && session.RemainingTreeFruit().Count==2,"Global regen counter incorrect.");}
 if(i>2)Atomic(session,session.RegenerateTree);
 string save=Path.Combine(output,"simulation-session.json");BossBattleStore.SaveSession(save,session);session=BossBattleStore.LoadSession(save);Need(session.CombatIndex==i+1 && session.TreeRegenerations==Math.Min(i,2),"Persistent session state lost.");
 session.UseRestPot();Need(!ledger.TryRecord(session,_=>true),"Partial/QA run awarded success.");
}
Need(session.Phase==BossBattlePhase.Completed && session.Results.Count==10 && !ledger.TryRecord(session,_=>true) && ledger.CompletedYoshis.Count==0,"QA completion awarded a real challenge.");
Atomic(session,session.UseRestPot);Atomic(session,()=>session.EnterCombat(new()));
var failed=BossBattleSession.Create(plan,0,slots,BossEvidenceKind.SimulationQa);string failureToken=failed.EnterCombat(new());Need(failed.RecordCombatResult(Receipt(failed,failureToken,false)) && failed.Phase==BossBattlePhase.Failed && !ledger.TryRecord(failed,_=>true),"Failed session awarded a challenge.");Atomic(failed,()=>failed.EnterCombat(new()));
var absentResults=BossBattleJson.Clone(session);absentResults.Results=null!;Reject(absentResults.Validate);
var absentPlan=BossBattleJson.Clone(session);absentPlan.Plan=null!;Reject(absentPlan.Validate);
var corrupt=BossBattleJson.Clone(session);corrupt.YoshiColor=14;Reject(corrupt.Validate);corrupt=BossBattleJson.Clone(session);corrupt.Results[1]=corrupt.Results[0];Reject(corrupt.Validate);corrupt=BossBattleJson.Clone(session);corrupt.TreeRegenerations=3;Reject(corrupt.Validate);
var skippedBatch=BossBattleJson.Clone(session);skippedBatch.ConsumedTreeFruitIds.Remove($"0:{slots[1].Id}");Reject(skippedBatch.Validate);
var finalBatch=BossBattleJson.Clone(session);finalBatch.CombatIndex=9;finalBatch.Results.RemoveAt(9);finalBatch.Phase=BossBattlePhase.Rest;finalBatch.CollectTreeFruit(slots[0].Id);finalBatch.CollectTreeFruit(slots[1].Id);Atomic(finalBatch,finalBatch.RegenerateTree);Need(finalBatch.RemainingTreeFruit().Count==0,"Tree supplied a fourth batch after the global two-regeneration quota");
// This is a pure model fixture with explicitly synthetic capability attestation.
// It proves verifier gating only; it is never written to a personal achievement file.
var syntheticNative=BossBattleJson.Clone(session);syntheticNative.Evidence=BossEvidenceKind.NativeObserved;syntheticNative.RuntimeSha256=new string('a',64);syntheticNative.Results=syntheticNative.Results.Select(x=>x with{Evidence=BossEvidenceKind.NativeObserved,RuntimeSha256=syntheticNative.RuntimeSha256}).ToList();syntheticNative.Validate();
Need(!ledger.TryRecord(syntheticNative,null) && !ledger.TryRecord(syntheticNative,_=>false),"Unchecked receipts awarded success.");Need(ledger.TryRecord(syntheticNative,_=>true) && !ledger.TryRecord(syntheticNative,_=>true) && ledger.CompletedYoshis.SetEquals([13]),"Verifier-qualified challenge not once-only.");
var replayColor=BossBattleJson.Clone(syntheticNative);replayColor.YoshiColor=0;Reject(replayColor.Validate);
var replayRun=BossBattleJson.Clone(syntheticNative);replayRun.RunId=Guid.NewGuid().ToString("N");Reject(replayRun.Validate);
foreach(int color in Enumerable.Range(0,14)){
 var s=BossBattleJson.Clone(syntheticNative);s.RunId=Guid.NewGuid().ToString("N");s.YoshiColor=color;
 s.Results=s.Results.Select(r=>r with{RunId=s.RunId,YoshiColor=color,Token=Guid.NewGuid().ToString("N")}).ToList();
 Need(ledger.TryRecord(s,_=>true),"A Yoshi challenge is missing.");
}Need(ledger.CompletedYoshis.Count==14 && ledger.ConsumedNativeReceiptTokens.Count==150,"All fourteen Yoshi challenges and receipt replay guard not represented.");
assertions+=BossBattlePresentationChecks.Run(output);
File.WriteAllText(Path.Combine(output,"boss-battle-manifest.json"),Json(BossBattleCatalog.Manifest()));File.WriteAllText(Path.Combine(output,"deterministic-plan.json"),Json(plan));
File.WriteAllText(Path.Combine(output,"verification.json"),Json(new{passed=true,scope="Pure model tests only; synthetic combat receipts, no native gameplay, no personal files",assertions,rejections,seeds=300,deterministicPlan=true,guaranteedSixFamilies=true,firstCandyAlwaysSolo=true,genieOnlyLaterSoloCandy=true,exactFishAndGroupHits=true,fishColors=new[]{"red","green","purple"},fishReplacementPercentPerMember=5,replacementSample=new{replacements,eligible},purpleAlternatesArcAndVerticalWithoutWater=true,fishCompanionHits=2,csharpNonFishSeedTopologyPreserved=true,oldRulesRejectedWithoutMutation=true,unknownParametersRemainNull=true,nativeAdmissionBlocked=true,treeConsumptionPersists=true,globalTreeRegenerationLimit=2,treeInvincibility=false,onceOnlyCallbacks=true,qaCannotAwardChallenges=true,externalNativeVerifierRequired=true,receiptsBoundToRunPlanAndColor=true,nativeReceiptReplayRejected=true,yoshiChallenges=14,nativePlayabilityProven=false,sourceFiles=Directory.GetFiles(Path.GetFullPath(Path.Combine(AppContext.BaseDirectory,"../../../../Core")),"BossBattle*.cs").ToDictionary(p=>Path.GetFileName(p)!,p=>Convert.ToHexStringLower(SHA256.HashData(File.ReadAllBytes(p))))}));
Console.WriteLine($"BossBattle model PASS: {assertions} assertions, {rejections} refusals, deterministic10 fights, persistent tree and QA-safe challenges. Native playability remains unproved.");
