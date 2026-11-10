using System.Security.Cryptography;
using System.Text;
using System.Text.Json.Serialization;

namespace YoshiWorkshop;

public enum BossRandomPolicy { UniformDraft, UniformConfirmed }
public enum FirstCandyPolicy { AlwaysSolo }
public sealed record BossBattleConfiguration {
    // Omit false so every existing persisted configuration/fingerprint stays exact.
    [JsonIgnore(Condition=JsonIgnoreCondition.WhenWritingDefault)]
    public bool IncludeBlargg {get;init;}
    public static BossBattleConfiguration WithBlargg()=>new(){
        IncludeBlargg=true,RandomFamilies=BossBattleCatalog.BlarggRandomFamilies.ToList(),
        VariantPools=BossBattleCatalog.BlarggRandomFamilies.ToDictionary(f=>f,f=>BossBattleCatalog.Family(f).Variants.Select(v=>v.Id).ToList())
    };
    public BossRandomPolicy RandomPolicy {get;init;}=BossRandomPolicy.UniformDraft;
    public FirstCandyPolicy FirstCandy {get;init;}=FirstCandyPolicy.AlwaysSolo;
    public List<BossBattleFamily> RandomFamilies {get;init;}=BossBattleCatalog.OrdinaryFamilies.ToList();
    public Dictionary<BossBattleFamily,List<string>> VariantPools {get;init;}=BossBattleCatalog.OrdinaryFamilies.ToDictionary(f=>f,f=>BossBattleCatalog.Family(f).Variants.Select(v=>v.Id).ToList());
    public Dictionary<BossBattleFamily,int> ArenaSlots {get;init;}=[];
    public Dictionary<string,int> HitOverrides {get;init;}=[];
    public double? SmallCandyScale {get;init;}
    public double? RedYellowWindowSeconds {get;init;}
    public int? RedYellowHealing {get;init;}
    public Dictionary<string,int> BirdCounts {get;init;}=[];
    public Dictionary<string,double> BirdRespawnSeconds {get;init;}=[];
    public void Validate(){
        if(RandomFamilies is null || VariantPools is null || ArenaSlots is null || HitOverrides is null || BirdCounts is null || BirdRespawnSeconds is null)throw new InvalidDataException("Collections de configuration absentes.");
        var allowed=IncludeBlargg?BossBattleCatalog.BlarggRandomFamilies:BossBattleCatalog.OrdinaryFamilies;
        if(!Enum.IsDefined(RandomPolicy) || !Enum.IsDefined(FirstCandy) || RandomFamilies.Count==0 || RandomFamilies.Distinct().Count()!=RandomFamilies.Count || RandomFamilies.Any(f=>!allowed.Contains(f)))throw new InvalidDataException("Politique de tirage invalide.");
        if(VariantPools.Count!=allowed.Count || allowed.Any(f=>!VariantPools.TryGetValue(f,out var pool) || pool is null || pool.Count==0 || pool.Distinct().Count()!=pool.Count || pool.Any(id=>!BossBattleCatalog.Family(f).Variants.Any(v=>v.Id==id))))throw new InvalidDataException("Variantes disponibles invalides.");
        if(ArenaSlots.Any(p=>!Enum.IsDefined(p.Key) || p.Key==BossBattleFamily.DoubleBlargg && !IncludeBlargg || p.Value is <0 or >2) || HitOverrides.Any(p=>string.IsNullOrWhiteSpace(p.Key) || p.Value is <1 or >1000))throw new InvalidDataException("Paramètres de combat invalides.");
        if(SmallCandyScale.HasValue && (!double.IsFinite(SmallCandyScale.Value) || SmallCandyScale.Value<=0 || SmallCandyScale.Value>=1) || RedYellowWindowSeconds.HasValue && (!double.IsFinite(RedYellowWindowSeconds.Value) || RedYellowWindowSeconds.Value<=0) || RedYellowHealing is <1 or >1000)throw new InvalidDataException("Paramètres du bonbon invalides.");
        if(BirdCounts.Any(p=>string.IsNullOrWhiteSpace(p.Key) || p.Value is <1 or >1000) || BirdRespawnSeconds.Any(p=>string.IsNullOrWhiteSpace(p.Key) || !double.IsFinite(p.Value) || p.Value<=0))throw new InvalidDataException("Approvisionnement en oiseaux invalide.");
    }
}

public sealed record BossBattleEncounter(int Number,BossBattleFamily Family,string Variant,int? ArenaSlot,
    bool GenieCompanion,int PurpleFishCount,int? SinglePurpleFishSlot) {
    public string Id=>$"combat-{Number}";
    [JsonIgnore(Condition=JsonIgnoreCondition.WhenWritingNull)]
    public IReadOnlyList<BossCombatantSpec>? FishCombatants {get;init;}
    public IReadOnlyList<BossCombatantSpec> Combatants(BossBattleConfiguration configuration){
        if(Family==BossBattleFamily.GiantFish)return FishCombatants?.ToArray()??throw new InvalidDataException("Composition des poissons absente.");
        var bosses=BossBattleCatalog.Variant(Family,Variant).Combatants.Select(b=>b with{Hits=b.Hits??(configuration.HitOverrides.TryGetValue($"{Family}/{Variant}/{b.Id}",out int hits)?hits:null)}).ToList();
        if(GenieCompanion)bosses.Add(new("genie-companion","genie",2));return bosses;
    }
}
public sealed record BossBattlePlan {
    public string RulesVersion {get;init;}=BossBattleCatalog.RulesVersion;
    public ulong Seed {get;init;}
    public BossBattleConfiguration Configuration {get;init;}=new();
    public List<BossBattleEncounter> Encounters {get;init;}=[];
    public string Fingerprint()=>Convert.ToHexStringLower(SHA256.HashData(Encoding.UTF8.GetBytes(BossBattleJson.Serialize(this))));
    public void Validate(){
        if(Configuration is null || Encounters is null || Encounters.Any(e=>e is null))throw new InvalidDataException("Configuration ou rencontres absentes.");
        if(RulesVersion!=(Configuration.IncludeBlargg?BossBattleCatalog.BlarggRulesVersion:BossBattleCatalog.RulesVersion))throw new InvalidDataException("Ce parcours utilise d’anciennes règles. Préparer un nouveau parcours ; le fichier existant est conservé.");
        Configuration.Validate();var expected=BossBattlePlanner.Create(Seed,Configuration);
        if(BossBattleJson.Serialize(Encounters)!=BossBattleJson.Serialize(expected.Encounters))throw new InvalidDataException("Le plan ne correspond pas à sa graine et à ses règles.");
    }
}

public static class BossBattlePlanner {
    // SplitMix64 and rejection sampling keep persisted seeds deterministic across runtimes.
    struct Random(ulong seed){ulong state=seed;public ulong State=>state;public ulong Next(){ulong z=unchecked(state+=0x9E3779B97F4A7C15UL);z=unchecked((z^(z>>30))*0xBF58476D1CE4E5B9UL);z=unchecked((z^(z>>27))*0x94D049BB133111EBUL);return z^(z>>31);}public int Pick(int count){if(count<=0)throw new ArgumentOutOfRangeException(nameof(count));ulong n=(uint)count,limit=unchecked(0UL-n)%n,v;do{v=Next();}while(v<limit);return(int)(v%n);}}
    public static BossBattlePlan Create(ulong seed,BossBattleConfiguration configuration){
        configuration.Validate();var config=BossBattleJson.Clone(configuration);var random=new Random(seed);var encounters=new List<BossBattleEncounter>();
        string Variant(BossBattleFamily family){var pool=config.VariantPools[family];return pool[random.Pick(pool.Count)];}
        BossBattleEncounter Encounter(int number,BossBattleFamily family,string variant){
            bool companion=family==BossBattleFamily.YellowCandy && variant=="solo" && number!=1 && random.Pick(100)<10;
            List<BossCombatantSpec>? fish=null;
            if(family==BossBattleFamily.GiantFish){
                // Retain the former single main-stream fish draw. Per-fish
                // replacements and companions use a separate deterministic
                // stream, so later C# families/variants keep their old draws.
                _=random.Pick(100);
                int variantIndex=variant switch{"three-one-hit"=>0,"two-two-hits"=>1,"single-with-companion"=>2,_=>throw new InvalidDataException("Variante de poissons inconnue.")};
                var colors=new Random(unchecked(random.State^((ulong)number*0xD6E8FEB86659FD93UL)^((ulong)variantIndex*0xA0761D6478BD642FUL)));
                string[] kinds=["red-fish","green-fish","purple-fish"];fish=[];
                int count=variantIndex==0?3:variantIndex==1?2:1;
                for(int i=0;i<count;i++){
                    int color=variantIndex==2?colors.Pick(3):i;
                    if(colors.Pick(100)<5)color=(color+1+colors.Pick(2))%3;
                    int hits=variantIndex==0?1:variantIndex==1?2:2+colors.Pick(2);
                    fish.Add(new($"fish-{i+1}",kinds[color],hits));
                }
                if(variantIndex==2){companion=colors.Pick(2)==0;fish.Add(new("fish-companion",companion?"genie":"invisible",2));}
            }
            int? arena=number==1?0:config.ArenaSlots.TryGetValue(family,out int slot)?slot:null;
            int purple=fish?.Count(b=>b.Kind=="purple-fish")??0;
            return new(number,family,variant,arena,companion,purple,purple==1?fish!.FindIndex(b=>b.Kind=="purple-fish"):null){FishCombatants=fish?.AsReadOnly()};
        }
        encounters.Add(Encounter(1,BossBattleFamily.YellowCandy,"solo"));
        var guaranteed=(config.IncludeBlargg?BossBattleCatalog.BlarggGuaranteedFamilies:BossBattleCatalog.OrdinaryFamilies).Where(f=>f!=BossBattleFamily.YellowCandy).ToList();
        for(int i=guaranteed.Count-1;i>0;i--){int j=random.Pick(i+1);(guaranteed[i],guaranteed[j])=(guaranteed[j],guaranteed[i]);}
        foreach(var family in guaranteed)encounters.Add(Encounter(encounters.Count+1,family,Variant(family)));
        for(int i=0;i<3;i++){var family=config.RandomFamilies[random.Pick(config.RandomFamilies.Count)];encounters.Add(Encounter(encounters.Count+1,family,Variant(family)));}
        encounters.Add(Encounter(10,BossBattleFamily.RedCandyFinal,"final"));
        return new(){RulesVersion=config.IncludeBlargg?BossBattleCatalog.BlarggRulesVersion:BossBattleCatalog.RulesVersion,Seed=seed,Configuration=config,Encounters=encounters};
    }
}

public sealed record BossBattleCapabilities {
    public bool NativeIntegrationVerified {get;init;}
    public string? RuntimeSha256 {get;init;}
    public HashSet<string> Features {get;init;}=[];
    public HashSet<string> VerifiedArenas {get;init;}=[];
}
public static class BossBattleAdmission {
    public static IReadOnlyList<string> Missing(BossBattlePlan plan,BossBattleCapabilities capabilities){
        plan.Validate();var missing=new HashSet<string>();var c=plan.Configuration;
        if(!capabilities.NativeIntegrationVerified || capabilities.RuntimeSha256 is not string hash || hash.Length!=64 || !hash.All(Uri.IsHexDigit))missing.Add("native-runtime-unverified");
        if(c.RandomPolicy!=BossRandomPolicy.UniformConfirmed)missing.Add("random-distribution-unconfirmed");
        foreach(var feature in new[]{"native-results","boss-session-transport","persistent-rest-tree","tree-no-invincibility","renewable-enemies-zero-score"})if(!capabilities.Features.Contains(feature))missing.Add("feature:"+feature);
        foreach(var e in plan.Encounters){
            string key=$"{e.Family}/{e.Variant}";var variant=BossBattleCatalog.Variant(e.Family,e.Variant);
            if(!capabilities.Features.Contains("encounter:"+key))missing.Add("encounter-unverified:"+key);
            if(e.Family==BossBattleFamily.YellowCandy && e.GenieCompanion && !capabilities.Features.Contains("yellow-candy-genie-companion"))missing.Add("feature:yellow-candy-genie-companion");
            if(e.ArenaSlot is not int slot)missing.Add($"arena-selection:{e.Family}");
            else if(!capabilities.VerifiedArenas.Contains(BossBattleCatalog.Family(e.Family).Arenas[slot].Id))missing.Add($"arena-unverified:{e.Family}/{slot}");
            foreach(var boss in e.Combatants(c)){
                if(!boss.Hits.HasValue)missing.Add($"hits:{key}/{boss.Id}");
                if(!capabilities.Features.Contains("actor:"+boss.Kind))missing.Add("actor:"+boss.Kind);
                if(boss.Size==BossBattleSize.SmallerUnspecified && !c.SmallCandyScale.HasValue)missing.Add("small-candy-size");
            }
            if(e.PurpleFishCount>0 && !capabilities.Features.Contains("purple-fish-alternating-jumps"))missing.Add("feature:purple-fish-alternating-jumps");
            if(variant.Birds!=BossBirdSupply.Unspecified || e.Family==BossBattleFamily.RedCandyFinal){
                if(!c.BirdCounts.ContainsKey(key) || !c.BirdRespawnSeconds.ContainsKey(key))missing.Add("bird-supply:"+key);
                if(!capabilities.Features.Contains("yellow-birds"))missing.Add("feature:yellow-birds");
            }
            if(variant.NoFlowers && !capabilities.Features.Contains("arena-no-flowers:"+key))missing.Add("feature:arena-no-flowers:"+key);
            if(variant.NoEggBlocks && !capabilities.Features.Contains("arena-no-egg-blocks:"+key))missing.Add("feature:arena-no-egg-blocks:"+key);
        }
        if(!c.RedYellowWindowSeconds.HasValue || !c.RedYellowHealing.HasValue)missing.Add("red-candy-yellow-window-and-healing");
        foreach(var feature in new[]{"red-candy-color-healing","red-candy-30s-fire-roll","red-candy-native-size"})if(!capabilities.Features.Contains(feature))missing.Add("feature:"+feature);
        return missing.Order(StringComparer.Ordinal).ToArray();
    }
    public static void RequireReady(BossBattlePlan plan,BossBattleCapabilities capabilities){var missing=Missing(plan,capabilities);if(missing.Count>0)throw new InvalidDataException("Bataille native indisponible : "+string.Join(", ",missing));}
}
