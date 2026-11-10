using System.Text.Json;

namespace YoshiWorkshop;

public enum BossBattlePhase { ReadyForCombat, Combat, Rest, Completed, Failed }
public enum BossEvidenceKind { NativeObserved, SimulationQa }
public sealed record BossTreeSlot(string Id,ushort ObjectId);
public sealed record BossTreePickup(string SlotId,ushort ObjectId,int Cycle,bool GrantInvincibility=false);
public sealed record BossBattleCombatReceipt {
    public string Token {get;init;}="";
    public string EncounterId {get;init;}="";
    public string RunId {get;init;}="";
    public string PlanFingerprint {get;init;}="";
    public int YoshiColor {get;init;}
    public BossEvidenceKind Evidence {get;init;}
    public string? RuntimeSha256 {get;init;}
    public bool Won {get;init;}
    public List<string> DefeatedCombatants {get;init;}=[];
}
public sealed class BossBattleSession {
    public int Schema {get;set;}=1;
    public string RunId {get;set;}=Guid.NewGuid().ToString("N");
    public BossBattlePlan Plan {get;set;}=new();
    public string PlanFingerprint {get;set;}="";
    public int YoshiColor {get;set;}
    public BossEvidenceKind Evidence {get;set;}
    public BossBattlePhase Phase {get;set;}=BossBattlePhase.ReadyForCombat;
    public int CombatIndex {get;set;}
    public string ExpectedToken {get;set;}="";
    public string? RuntimeSha256 {get;set;}
    public List<BossBattleCombatReceipt> Results {get;set;}=[];
    public List<BossTreeSlot> TreeSlots {get;set;}=[];
    public int TreeRegenerations {get;set;}
    public HashSet<string> ConsumedTreeFruitIds {get;set;}=[];
    public bool TreeGrantsInvincibility=>false;
    public BossBattleEncounter? CurrentCombat=>CombatIndex<Plan.Encounters.Count?Plan.Encounters[CombatIndex]:null;
    public static BossBattleSession Create(BossBattlePlan plan,int yoshiColor,IEnumerable<BossTreeSlot> slots,BossEvidenceKind evidence=BossEvidenceKind.NativeObserved){
        var s=new BossBattleSession{Plan=BossBattleJson.Clone(plan),YoshiColor=yoshiColor,Evidence=evidence,TreeSlots=slots.Select(x=>x with{}).ToList()};s.PlanFingerprint=s.Plan.Fingerprint();s.Validate();return s;
    }
    static void Need(bool ok,string error){if(!ok)throw new InvalidDataException(error);}
    static bool Token(string s)=>s is {Length:32} && s.All(Uri.IsHexDigit);
    string FruitKey(string slot)=>$"{TreeRegenerations}:{slot}";
    public IReadOnlyList<BossTreeSlot> RemainingTreeFruit(){Validate();return TreeSlots.Where(s=>!ConsumedTreeFruitIds.Contains(FruitKey(s.Id))).ToArray();}
    public void Validate(){
        if(Plan is null || Results is null || TreeSlots is null || ConsumedTreeFruitIds is null)throw new InvalidDataException("État ou collections de bataille absents.");
        if(Results.Any(r=>r is null || r.DefeatedCombatants is null) || TreeSlots.Any(s=>s is null) || ConsumedTreeFruitIds.Any(s=>s is null))throw new InvalidDataException("Élément de bataille absent.");
        Plan.Validate();Need(Schema==1 && Token(RunId) && PlanFingerprint==Plan.Fingerprint() && YoshiColor is >=0 and <14 && Enum.IsDefined(Phase) && Enum.IsDefined(Evidence),"État de bataille invalide.");
        Need(CombatIndex is >=0 and <=10 && Results.Count==CombatIndex && TreeRegenerations is >=0 and <=2,"Progression de bataille incohérente.");
        Need((Phase==BossBattlePhase.Completed)==(CombatIndex==10) && (Phase==BossBattlePhase.Combat?Token(ExpectedToken):ExpectedToken==""),"Transition de bataille incohérente.");
        Need(Phase!=BossBattlePhase.Rest || CombatIndex is >0 and <10,"Repos hors de la bataille.");
        if(Evidence==BossEvidenceKind.NativeObserved && (Phase is BossBattlePhase.Combat or BossBattlePhase.Failed || Results.Count>0 || RuntimeSha256!=null))Need(RuntimeSha256 is {Length:64} && RuntimeSha256.All(Uri.IsHexDigit),"Session native commencée sans provenance du moteur.");
        if(Evidence==BossEvidenceKind.SimulationQa)Need(RuntimeSha256==null,"Une simulation ne possède pas de provenance native.");
        Need(TreeSlots.Count is >=1 and <=64 && TreeSlots.All(s=>Token(s.Id) && BossBattleCatalog.ValidFruit(s.ObjectId)) && TreeSlots.Select(s=>s.Id).Distinct().Count()==TreeSlots.Count,"Fruits du Lucky Tree invalides.");
        foreach(string key in ConsumedTreeFruitIds){var parts=key.Split(':');Need(parts.Length==2 && int.TryParse(parts[0],out int cycle) && cycle>=0 && cycle<=TreeRegenerations && parts[0]==cycle.ToString(System.Globalization.CultureInfo.InvariantCulture) && TreeSlots.Any(s=>s.Id==parts[1]),"Mémoire du Lucky Tree invalide.");}
        for(int cycle=0;cycle<TreeRegenerations;cycle++)Need(TreeSlots.All(s=>ConsumedTreeFruitIds.Contains($"{cycle}:{s.Id}")),"Régénération avant la consommation du lot précédent.");
        Need(Results.Select(x=>x.Token).Distinct().Count()==Results.Count,"Résultat de combat dupliqué.");
        for(int i=0;i<Results.Count;i++){
            var result=Results[i];var e=Plan.Encounters[i];var required=e.Combatants(Plan.Configuration).Select(b=>b.Id).Order().ToArray();
            Need(Token(result.Token) && result.EncounterId==e.Id && result.RunId==RunId && result.PlanFingerprint==PlanFingerprint && result.YoshiColor==YoshiColor && result.Won && result.Evidence==Evidence && result.DefeatedCombatants.Order().SequenceEqual(required),"Victoire incomplète ou résultat d’une autre bataille.");
            if(Evidence==BossEvidenceKind.NativeObserved)Need(RuntimeSha256 is {Length:64} && RuntimeSha256.All(Uri.IsHexDigit) && result.RuntimeSha256==RuntimeSha256,"Résultat natif sans provenance.");
        }
    }
    public string EnterCombat(BossBattleCapabilities capabilities){
        Validate();Need(Phase==BossBattlePhase.ReadyForCombat,"Utilise le pot de repos avant le prochain combat.");
        if(Evidence==BossEvidenceKind.NativeObserved){BossBattleAdmission.RequireReady(Plan,capabilities);if(RuntimeSha256!=null)Need(RuntimeSha256==capabilities.RuntimeSha256,"Le moteur de la session a changé.");}
        RuntimeSha256=Evidence==BossEvidenceKind.NativeObserved?capabilities.RuntimeSha256:null;
        ExpectedToken=Guid.NewGuid().ToString("N");Phase=BossBattlePhase.Combat;return ExpectedToken;
    }
    public bool RecordCombatResult(BossBattleCombatReceipt receipt){
        Validate();if(receipt is null || receipt.DefeatedCombatants is null || Phase!=BossBattlePhase.Combat || receipt.Token!=ExpectedToken || receipt.EncounterId!=CurrentCombat!.Id || receipt.RunId!=RunId || receipt.PlanFingerprint!=PlanFingerprint || receipt.YoshiColor!=YoshiColor || receipt.Evidence!=Evidence || Results.Any(r=>r.Token==receipt.Token))return false;
        if(Evidence==BossEvidenceKind.NativeObserved && receipt.RuntimeSha256!=RuntimeSha256)return false;
        if(!receipt.Won){ExpectedToken="";Phase=BossBattlePhase.Failed;return true;}
        var required=CurrentCombat.Combatants(Plan.Configuration).Select(b=>b.Id).Order().ToArray();if(!receipt.DefeatedCombatants.Order().SequenceEqual(required))return false;
        Results.Add(BossBattleJson.Clone(receipt));CombatIndex++;ExpectedToken="";Phase=CombatIndex==10?BossBattlePhase.Completed:BossBattlePhase.Rest;return true;
    }
    public void UseRestPot(){Validate();Need(Phase==BossBattlePhase.Rest,"Le pot ne mène au combat suivant que depuis le repos.");Phase=BossBattlePhase.ReadyForCombat;}
    public BossTreePickup CollectTreeFruit(string slotId){
        Validate();Need(Phase==BossBattlePhase.Rest,"Le Lucky Tree est réservé à la pièce de repos.");var slot=TreeSlots.SingleOrDefault(s=>s.Id==slotId)??throw new InvalidDataException("Fruit du Lucky Tree inconnu.");
        Need(!ConsumedTreeFruitIds.Contains(FruitKey(slot.Id)),"Ce fruit a déjà été mangé pendant ce cycle.");ConsumedTreeFruitIds.Add(FruitKey(slot.Id));return new(slot.Id,slot.ObjectId,TreeRegenerations,false);
    }
    public void RegenerateTree(){Validate();Need(Phase==BossBattlePhase.Rest && TreeRegenerations<BossBattleCatalog.TreeRegenerationLimit,"Deux régénérations maximum pour toute la session.");Need(RemainingTreeFruit().Count==0,"Mange tous les fruits du lot avant sa régénération.");TreeRegenerations++;}
}

public sealed class BossBattleChallengeLedger {
    public int Schema {get;set;}=1;
    public HashSet<int> CompletedYoshis {get;set;}=[];
    public HashSet<string> CompletedRunIds {get;set;}=[];
    public HashSet<string> ConsumedNativeReceiptTokens {get;set;}=[];
    public void Validate(){if(CompletedYoshis is null || CompletedRunIds is null || ConsumedNativeReceiptTokens is null)throw new InvalidDataException("Collections de succès absentes.");if(Schema!=1 || CompletedYoshis.Any(y=>y is <0 or >=14) || CompletedRunIds.Concat(ConsumedNativeReceiptTokens).Any(s=>s is not {Length:32} || !s.All(Uri.IsHexDigit)))throw new InvalidDataException("Succès de bataille invalides.");}
    // The native integration must supply its own receipt verifier. A model-only
    // simulation, a QA run or an unchecked Completed flag can never award this.
    public bool TryRecord(BossBattleSession session,Func<BossBattleCombatReceipt,bool>? verifyNativeReceipt){
        Validate();session.Validate();if(session.Evidence!=BossEvidenceKind.NativeObserved || session.Phase!=BossBattlePhase.Completed || session.Results.Count!=10 || verifyNativeReceipt==null || CompletedRunIds.Contains(session.RunId) || session.Results.Any(r=>ConsumedNativeReceiptTokens.Contains(r.Token) || !verifyNativeReceipt(r)))return false;
        CompletedRunIds.Add(session.RunId);foreach(var receipt in session.Results)ConsumedNativeReceiptTokens.Add(receipt.Token);CompletedYoshis.Add(session.YoshiColor);return true;
    }
}
public static class BossBattleStore {
    // Explicit destination only; no implicit personal save, EEPROM or creation path.
    public static void SaveSession(string path,BossBattleSession session){session.Validate();var full=Path.GetFullPath(path);Directory.CreateDirectory(Path.GetDirectoryName(full)!);var temp=full+".tmp-"+Guid.NewGuid().ToString("N");try{File.WriteAllText(temp,BossBattleJson.Serialize(session));File.Move(temp,full,true);}finally{if(File.Exists(temp))File.Delete(temp);}}
    public static BossBattleSession LoadSession(string path){var s=JsonSerializer.Deserialize<BossBattleSession>(File.ReadAllText(path),BossBattleJson.Options)??throw new InvalidDataException("Session de bataille absente.");s.Validate();return s;}
}
