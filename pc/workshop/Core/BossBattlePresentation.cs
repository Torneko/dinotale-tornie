using System.Text.Json.Serialization;

namespace YoshiWorkshop;

// A presentation snapshot only: hidden rows contain no encounter parameters or
// link back to the persisted plan. Null labels are omitted from exported JSON.
public sealed record BossEncounterView(
    int Number,
    string Title,
    bool Revealed,
    [property:JsonIgnore(Condition=JsonIgnoreCondition.WhenWritingNull)] string? FamilyLabel=null,
    [property:JsonIgnore(Condition=JsonIgnoreCondition.WhenWritingNull)] string? VariantLabel=null);

public static class BossBattlePresentation {
    public static IReadOnlyList<BossEncounterView> Preparation(BossBattlePlan plan){
        ArgumentNullException.ThrowIfNull(plan);plan.Validate();
        return Project(plan,_=>false);
    }
    public static IReadOnlyList<BossEncounterView> Session(BossBattleSession session){
        ArgumentNullException.ThrowIfNull(session);session.Validate();
        int completed=session.Results.Count;
        return Project(session.Plan,e=>e.Number<=completed || session.Phase==BossBattlePhase.Combat && e.Number==session.CombatIndex+1);
    }
    static IReadOnlyList<BossEncounterView> Project(BossBattlePlan plan,Func<BossBattleEncounter,bool> reveal)=>Array.AsReadOnly(plan.Encounters.Select(e=>{
        if(!reveal(e))return new BossEncounterView(e.Number,$"Combat {e.Number}",false);
        var family=BossBattleCatalog.Family(e.Family);var variant=BossBattleCatalog.Variant(e.Family,e.Variant);
        return new BossEncounterView(e.Number,$"Combat {e.Number} — {family.Label}",true,family.Label,variant.Label);
    }).ToArray());
}
