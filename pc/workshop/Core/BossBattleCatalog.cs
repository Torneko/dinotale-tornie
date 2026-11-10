using System.Text.Json;
using System.Text.Json.Serialization;

namespace YoshiWorkshop;

public enum BossBattleFamily { YellowCandy, Invisible, Genie, GiantLips, GiantFish, PinkWhiteMonsters, RedCandyFinal, DoubleBlargg }
public enum BossBattleSize { Native, SmallerUnspecified }
public enum BossBirdSupply { Unspecified, Reference, MoreThanReference, LessThanReference, Required }
public sealed record BossCombatantSpec(string Id,string Kind,int? Hits,BossBattleSize Size=BossBattleSize.Native);
public sealed record BossVariantSpec(string Id,string Label,IReadOnlyList<BossCombatantSpec> Combatants,
    BossBirdSupply Birds=BossBirdSupply.Unspecified,bool NoFlowers=false,bool NoEggBlocks=false);
public sealed record BossArenaSpec(int Slot,string Id,string Label,bool Jungle=true,bool GeometryImplemented=false);
public sealed record BossFamilySpec(BossBattleFamily Family,string Label,string? NativeSource,
    IReadOnlyList<BossVariantSpec> Variants,IReadOnlyList<BossArenaSpec> Arenas);

public static class BossBattleCatalog {
    public const int CombatCount=10,YoshiCount=14,TreeRegenerationLimit=2;
    public const string RulesVersion="boss-draft-2",BlarggRulesVersion="boss-blargg-1";
    static BossCombatantSpec B(string id,string kind,int? hits=null,BossBattleSize size=BossBattleSize.Native)=>new(id,kind,hits,size);
    static BossVariantSpec V(string id,string label,BossCombatantSpec[] bosses,BossBirdSupply birds=BossBirdSupply.Unspecified,bool emptySources=false)=>new(id,label,Array.AsReadOnly(bosses),birds,emptySources,emptySources);
    static BossFamilySpec F(BossBattleFamily family,string label,string? source,params BossVariantSpec[] variants)=>new(family,label,source,Array.AsReadOnly(variants),
        Array.AsReadOnly(Enumerable.Range(0,3).Select(i=>new BossArenaSpec(i,$"{family.ToString().ToLowerInvariant()}-terrain-{i+1}",i==0?"Terrain de référence à intégrer":$"Terrain jungle {i+1} à concevoir")).ToArray()));
    public static readonly IReadOnlyList<BossFamilySpec> Families=Array.AsReadOnly(new[]{
        F(BossBattleFamily.YellowCandy,"Bonbon jaune","3-1",
            V("solo","Bonbon jaune standard",[B("candy-1","yellow-candy")]),
            V("two-small","Deux petits bonbons",[B("candy-1","yellow-candy",size:BossBattleSize.SmallerUnspecified),B("candy-2","yellow-candy",size:BossBattleSize.SmallerUnspecified)]),
            V("three-small","Trois petits bonbons",[B("candy-1","yellow-candy",size:BossBattleSize.SmallerUnspecified),B("candy-2","yellow-candy",size:BossBattleSize.SmallerUnspecified),B("candy-3","yellow-candy",size:BossBattleSize.SmallerUnspecified)])),
        F(BossBattleFamily.Invisible,"Boss invisible",null,
            V("solo","Boss invisible d’origine",[B("invisible-1","invisible")]),
            V("two-two-hits","Deux boss, deux coups chacun",[B("invisible-1","invisible",2),B("invisible-2","invisible",2)]),
            V("four-one-hit","Quatre boss, un coup chacun",Enumerable.Range(1,4).Select(i=>B($"invisible-{i}","invisible",1)).ToArray())),
        F(BossBattleFamily.Genie,"Génie",null,
            V("solo","Génie d’origine",[B("genie-1","genie")]),
            V("two-two-hits","Deux génies, deux coups chacun",[B("genie-1","genie",2),B("genie-2","genie",2)]),
            V("four-one-hit","Quatre génies, un coup chacun",Enumerable.Range(1,4).Select(i=>B($"genie-{i}","genie",1)).ToArray())),
        F(BossBattleFamily.GiantLips,"Lèvres géantes",null,
            V("solo","Lèvres géantes d’origine",[B("lips-1","giant-lips")]),
            V("with-small-candy","Lèvres et petit bonbon",[B("lips-1","giant-lips"),B("candy-companion","yellow-candy",size:BossBattleSize.SmallerUnspecified)]),
            V("with-invisible","Lèvres et boss invisible",[B("lips-1","giant-lips"),B("invisible-companion","invisible")])),
        F(BossBattleFamily.GiantFish,"Poissons géants",null,
            V("three-one-hit","Trois poissons, un coup chacun",[B("fish-1","red-fish",1),B("fish-2","green-fish",1),B("fish-3","purple-fish",1)],BossBirdSupply.Required),
            V("two-two-hits","Deux poissons, deux coups chacun",[B("fish-1","red-fish",2),B("fish-2","green-fish",2)],BossBirdSupply.Required),
            V("single-with-companion","Un poisson et un compagnon",[B("fish-1","red-fish"),B("fish-companion","genie",2)],BossBirdSupply.Required)),
        F(BossBattleFamily.PinkWhiteMonsters,"Monstres roses et blancs","2-3 — salle exacte à identifier",
            V("four-hits","Deux monstres, quatre coups chacun",[B("monster-1","pink-white-monster",4),B("monster-2","pink-white-monster",4)],BossBirdSupply.Reference,true),
            V("three-hits-more-birds","Trois coups chacun, plus d’oiseaux",[B("monster-1","pink-white-monster",3),B("monster-2","pink-white-monster",3)],BossBirdSupply.MoreThanReference,true),
            V("five-hits-fewer-birds","Cinq coups chacun, moins d’oiseaux",[B("monster-1","pink-white-monster",5),B("monster-2","pink-white-monster",5)],BossBirdSupply.LessThanReference,true)),
        F(BossBattleFamily.RedCandyFinal,"Bonbon rouge final","Taille du bonbon jaune standard du 3-1",
            V("final","Bonbon rouge final",[B("red-candy-1","red-candy")],BossBirdSupply.Required)),
        new BossFamilySpec(BossBattleFamily.DoubleBlargg,"Double Blargg","2-2",
            Array.AsReadOnly(new[]{V("double-three-hits","Deux Blarggs, trois coups chacun",[B("blargg-1","blargg",3),B("blargg-2","blargg",3)],BossBirdSupply.Required)}),
            Array.AsReadOnly(Enumerable.Range(0,3).Select(i=>new BossArenaSpec(i,$"doubleblargg-cavern-{i+1}",i==0?"Caverne et lave à intégrer":$"Terrain de caverne {i+1} à concevoir",Jungle:false)).ToArray()))
    });
    // Keep the legacy six-family order stable for saved boss-draft-2 plans.
    public static readonly IReadOnlyList<BossBattleFamily> OrdinaryFamilies=Array.AsReadOnly(Families.Where(f=>f.Family is not (BossBattleFamily.RedCandyFinal or BossBattleFamily.DoubleBlargg)).Select(f=>f.Family).ToArray());
    public static readonly IReadOnlyList<BossBattleFamily> BlarggRandomFamilies=Array.AsReadOnly(Families.Where(f=>f.Family!=BossBattleFamily.RedCandyFinal).Select(f=>f.Family).ToArray());
    public static readonly IReadOnlyList<BossBattleFamily> BlarggGuaranteedFamilies=Array.AsReadOnly(new[]{BossBattleFamily.YellowCandy,BossBattleFamily.Invisible,BossBattleFamily.Genie,BossBattleFamily.GiantLips,BossBattleFamily.GiantFish,BossBattleFamily.DoubleBlargg});
    public static BossFamilySpec Family(BossBattleFamily family)=>Families.SingleOrDefault(f=>f.Family==family)??throw new InvalidDataException("Famille de boss inconnue.");
    public static BossVariantSpec Variant(BossBattleFamily family,string id)=>Family(family).Variants.SingleOrDefault(v=>v.Id==id)??throw new InvalidDataException("Variante de boss inconnue.");
    public static bool ValidFruit(ushort id)=>id is >=0x4030 and <=0x4034 or >=0x4F00 and <=0x4F05;
    public static object Manifest()=>new {
        rulesVersion=RulesVersion,preparationOnly=true,nativePlayabilityProven=false,combatCount=CombatCount,yoshiChallenges=YoshiCount,
        families=Families,guaranteedOrder="YellowCandy solo; shuffled five other families; three random fights; RedCandyFinal",
        terrainPolicy="Three arena slots per family: legacy jungle and optional Blargg cavern/lava. No claim that geometry or every mechanics/terrain combination exists.",
        optionalBlargg=new{defaultEnabled=false,rulesVersion=BlarggRulesVersion,members=2,hitsEach=3,guaranteedReplacement="PinkWhiteMonsters",monstersRemainRandom=true,randomFamilies=BlarggRandomFamilies},
        randomPolicy="Explicit configurable uniform draft; distribution must be confirmed before production admission.",
        firstCandyPolicy="Confirmed: first candy always alone. The 10% genie rule applies only to later eligible solo yellow candy encounters.",
        fish=new{colors=new[]{"red-fish","green-fish","purple-fish"},replacementPercentPerFish=5,otherColorsEquiprobable=true,rememberedForEncounter=true,soloHits=new[]{2,3},companionKinds=new[]{"genie","invisible"},companionHits=2,nativeDefaultBirds=3,nativeDefaultBirdRespawnSeconds=3},
        purpleFish=new{phaseOrder=new[]{"arc","vertical"},emitsWater=false},
        redCandy=new{finalOnly=true,nativeYellowSize=true,redHealing=0,yellowWindowSeconds=(double?)null,yellowHealing=(int?)null,fireIntervalSeconds=30,fireChancePercent=20,firstFireCheckSeconds=30,birdsRequired=true},
        restRoom=new{empty=true,luckyTree=true,nextCombatFlowerPot=true,invincibility=false,treeRegenerations=TreeRegenerationLimit,quotaScope="Whole session; explicit implementation assumption for the quota scope",fruitMemory="Stable fruit slot IDs and consumption cycle retained across every combat/rest visit"},
        renewableEnemies=new{eatenScore=0,killedScore=0},unknowns="Null values and unresolved arena/bird parameters block native admission. No Bowser."
    };
}

public static class BossBattleJson {
    public static readonly JsonSerializerOptions Options=new(){WriteIndented=true,Converters={new JsonStringEnumConverter()}};
    public static string Serialize<T>(T value)=>JsonSerializer.Serialize(value,Options);
    public static T Clone<T>(T value)=>JsonSerializer.Deserialize<T>(Serialize(value),Options)??throw new InvalidDataException("Données de bataille absentes.");
}
