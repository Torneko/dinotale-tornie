using System.Text.Json;
using YoshiWorkshop;

static class BossBattlePresentationChecks {
    public static int Run(string output){
        int assertions=0;
        void Need(bool ok,string error){assertions++;if(!ok)throw new Exception(error);}
        string Json<T>(T value)=>BossBattleJson.Serialize(value);
        void Hidden(BossEncounterView row){
            Need(!row.Revealed && row.Title==$"Combat {row.Number}" && row.FamilyLabel==null && row.VariantLabel==null,"Hidden row contains a spoiler.");
            using var json=JsonDocument.Parse(Json(row));var fields=json.RootElement.EnumerateObject().Select(p=>p.Name).Order().ToArray();
            Need(fields.SequenceEqual(new[]{"Number","Revealed","Title"}),"Hidden DTO exposes encounter metadata in JSON.");
        }
        void Visible(BossBattlePlan p,BossEncounterView row){
            var encounter=p.Encounters[row.Number-1];
            Need(row.Revealed && row.FamilyLabel==BossBattleCatalog.Family(encounter.Family).Label && row.VariantLabel==BossBattleCatalog.Variant(encounter.Family,encounter.Variant).Label,"Visible row describes the wrong completed/current encounter.");
        }
        void View(BossBattleSession session,int visible){
            var rows=BossBattlePresentation.Session(session);Need(rows.Count==10 && rows.Count(r=>r.Revealed)==visible,"Wrong reveal count.");
            for(int n=0;n<10;n++){Need(rows[n].Number==n+1,"Presentation order changed.");if(n<visible)Visible(session.Plan,rows[n]);else Hidden(rows[n]);}
        }
        var config=new BossBattleConfiguration();var plan=BossBattlePlanner.Create(42,config);string original=Json(plan);
        var prep=BossBattlePresentation.Preparation(plan);Need(prep.Count==10 && prep.Select(r=>r.Number).SequenceEqual(Enumerable.Range(1,10)),"Preparation lost encounter slots.");
        foreach(var row in prep)Hidden(row);
        Need(!prep[0].Revealed && !prep[9].Revealed,"First or final boss was spoiled.");
        Need(Json(plan)==original,"Presentation changed the persisted plan.");
        var propertyNames=typeof(BossEncounterView).GetProperties().Select(p=>p.Name).Order().ToArray();
        Need(propertyNames.SequenceEqual(new[]{"FamilyLabel","Number","Revealed","Title","VariantLabel"}),"Presentation DTO gained raw encounter data.");
        var slots=new[]{new BossTreeSlot("11111111111141118111111111111111",0x4030)};
        var session=BossBattleSession.Create(plan,0,slots,BossEvidenceKind.SimulationQa);View(session,0);
        for(int i=0;i<10;i++){
            View(session,i);string token=session.EnterCombat(new());View(session,i+1);
            string path=Path.Combine(output,"presentation-simulation-session.json");BossBattleStore.SaveSession(path,session);var loaded=BossBattleStore.LoadSession(path);
            Need(Json(BossBattlePresentation.Session(loaded))==Json(BossBattlePresentation.Session(session)),"Save/reload changed spoiler visibility.");
            var receipt=new BossBattleCombatReceipt{Token=token,EncounterId=session.CurrentCombat!.Id,RunId=session.RunId,PlanFingerprint=session.PlanFingerprint,YoshiColor=session.YoshiColor,Evidence=BossEvidenceKind.SimulationQa,Won=true,DefeatedCombatants=session.CurrentCombat.Combatants(session.Plan.Configuration).Select(b=>b.Id).ToList()};
            Need(session.RecordCombatResult(receipt),"Simulation fixture did not advance.");View(session,i+1);
            if(i<9){Need(session.Phase==BossBattlePhase.Rest,"Inter-combat fixture missed rest phase.");Hidden(BossBattlePresentation.Session(session)[i+1]);session.UseRestPot();View(session,i+1);}
        }
        Need(session.Phase==BossBattlePhase.Completed && BossBattlePresentation.Session(session).All(r=>r.Revealed),"A completed session should reveal all encountered bosses.");
        Need(BossBattlePresentation.Preparation(session.Plan).All(r=>!r.Revealed),"Preparation leaked a previous session’s reveal state.");
        Need(prep.All(r=>!r.Revealed),"A detached preparation DTO changed as the session advanced.");
        File.WriteAllText(Path.Combine(output,"boss-presentation-hidden.json"),Json(prep));
        File.WriteAllText(Path.Combine(output,"boss-presentation-completed.json"),Json(BossBattlePresentation.Session(session)));
        File.WriteAllText(Path.Combine(output,"boss-presentation-verification.json"),Json(new{passed=true,scope="Presentation projection and simulated session transitions only; no native battle or achievement",assertions,preparationTenHidden=true,firstAndFinalHidden=true,hiddenJsonHasNoEncounterMetadata=true,currentAndCompletedOnly=true,restAndReadyHideNext=true,saveReloadVisibilityStable=true,completedSessionRevealsPast=true,preparationDetached=true,nativeGameplayValidated=false}));
        return assertions;
    }
}
