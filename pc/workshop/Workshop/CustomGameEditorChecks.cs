using System.Text.Json;
using YoshiWorkshop;
namespace WorkshopApp;

public sealed partial class WorkshopWindow {
    void CheckCustomGameEditing(){
        if(!automatedChecks || !pcRoot.Contains("validation",StringComparison.OrdinalIgnoreCase))throw new InvalidOperationException("Copie QA isolée requise.");
        void Need(bool value,string message){if(!value)throw new Exception(message);}
        string State()=>JsonSerializer.Serialize(game,ProjectStore.Json);
        var first=store.Games().Single(g=>g.Id==game!.Id);
        var second=new CustomGame{Name="Autre traversée de validation",LevelIds=[..first.LevelIds]};store.Save(second);RefreshGames();LoadCustomGame(first);
        int prompts=0;bool allow=false;discardChoiceForChecks=title=>{Need(title=="Partie personnalisée modifiée","Wrong unsaved prompt");prompts++;return allow;};
        try{
            gameName.Text="Nom en attente";Need(gameDirty && !dirty,"Game name did not mark only the game dirty");
            Need(games.Items.Cast<CustomGame>().Single(g=>g.Id==first.Id).Name==first.Name && store.Games().Single(g=>g.Id==first.Id).Name==first.Name,"Draft changed saved/list model");
            string pending=State();games.SelectedItem=games.Items.Cast<CustomGame>().Single(g=>g.Id==second.Id);
            Need(prompts==1 && game!.Id==first.Id && State()==pending && gameName.Text=="Nom en attente" && games.SelectedItem is CustomGame restoredSelection && restoredSelection.Id==first.Id,"Cancelled switch discarded the draft");
            allow=true;games.SelectedItem=games.Items.Cast<CustomGame>().Single(g=>g.Id==second.Id);
            Need(prompts==2 && game!.Id==second.Id && !gameDirty && gameName.Text==second.Name,"Accepted switch did not load saved game");
            interactions.Checked=!interactions.Checked;Need(gameDirty && !dirty,"Game option did not mark only the game dirty");
            allow=false;string beforeNew=State();NewCustomGame();Need(prompts==3 && State()==beforeNew && gameDirty,"Cancelled new game discarded options");
            allow=true;NewCustomGame();Need(prompts==4 && game!.Id!=second.Id && gameDirty && games.SelectedIndex==-1,"New unsaved game not tracked");
            allow=false;var closing=new FormClosingEventArgs(CloseReason.UserClosing,false);OnFormClosing(closing);
            Need(closing.Cancel && prompts==5 && !IsDisposed,"Closing ignored unsaved game");
            allow=true;closing=new FormClosingEventArgs(CloseReason.UserClosing,false);OnFormClosing(closing);Need(!closing.Cancel && prompts==6,"Accepted closing remained blocked");
            LoadCustomGame(first);AddToGame();Need(gameDirty && game!.LevelIds.Count==first.LevelIds.Count+1,"Adding a level not tracked");
            gameName.Text="Partie conservée";interactions.Checked=true;SaveGame();Need(!gameDirty && store.Games().Single(g=>g.Id==first.Id).LevelIds.Count==first.LevelIds.Count+1,"Save failed to persist/clear dirty");
            LoadCustomGame(second);LoadCustomGame(store.Games().Single(g=>g.Id==first.Id));Need(gameName.Text=="Partie conservée" && interactions.Checked,"Saved name/options lost on reload");
            string saved=State();gameName.Text="";bool rejected=false;try{SaveGame();}catch(InvalidDataException){rejected=true;}
            Need(rejected && gameDirty && JsonSerializer.Serialize(store.Games().Single(g=>g.Id==first.Id),ProjectStore.Json)==saved,"Invalid save lost dirty state or changed disk");
            LoadCustomGame(first);
            string missing=Guid.NewGuid().ToString("N"),a=first.LevelIds[0],b=first.LevelIds[1];
            var incomplete=new CustomGame{Name="Référence absente",LevelIds=[missing,a,b,a]};LoadCustomGame(incomplete);
            Need(!File.Exists(store.LevelPath(missing)) && gameLevels.Items.Count==4 && gameLevels.Items[0] is GameLevelRow {Level:null} row && row.Name.Contains("introuvable"),"Missing level hidden from list");
            gameLevels.SelectedIndex=1;RemoveGameLevel();Need(game!.LevelIds.SequenceEqual(new[]{missing,b,a}) && gameDirty,"Removal used filtered index");
            gameLevels.SelectedIndex=2;MoveGameLevel(-1);Need(game.LevelIds.SequenceEqual(new[]{missing,a,b}) && gameLevels.SelectedIndex==1,"Move-up used filtered index");
            gameLevels.SelectedIndex=0;MoveGameLevel(1);Need(game.LevelIds.SequenceEqual(new[]{a,missing,b}) && gameLevels.SelectedIndex==1,"Missing row could not move down");
            gameLevels.SelectedIndex=2;MoveGameLevel(-1);Need(game.LevelIds.SequenceEqual(new[]{a,b,missing}),"Move across missing reference changed wrong row");
            gameLevels.SelectedIndex=2;string boundary=State();MoveGameLevel(1);Need(State()==boundary,"Boundary move changed game");
            gameLevels.SelectedIndex=2;rejected=false;try{TestGameLevel();}catch(InvalidOperationException error){rejected=error.Message.Contains("introuvable");}
            Need(rejected && !specialRunner.IsRunning && !runner.IsRunning,"Missing level reached native launch");
            rejected=false;try{SaveGame();}catch(InvalidDataException){rejected=true;}
            Need(rejected && gameDirty && gameLevels.Items.Count==3 && !store.Games().Any(g=>g.Id==incomplete.Id),"Missing reference save was accepted or draft lost");
            RemoveGameLevel();SaveGame();Need(!gameDirty && store.Games().Single(g=>g.Id==incomplete.Id).LevelIds.SequenceEqual(new[]{a,b}),"Removing missing row did not repair the game");
            LoadCustomGame(store.Games().Single(g=>g.Id==first.Id));RefreshGames();Need(!gameDirty && !dirty,"Checks left an untracked draft");
            Directory.CreateDirectory(Path.Combine(pcRoot,"build/editor-0270"));
            File.WriteAllText(Path.Combine(pcRoot,"build/editor-0270/ui-custom-games.json"),JsonSerializer.Serialize(new{gameDirtyIndependent=true,draftCloneIsolation=true,cancelSwitchPreservesDraft=true,acceptSwitchLoadsSaved=true,newGameGuard=true,closeGuard=true,addMarksDirty=true,saveReload=true,failedSaveKeepsDirty=true,missingReferenceVisible=true,stableRemoveIndex=true,stableMoveIndices=true,boundaryNoOp=true,missingLaunchRejectedBeforeProcess=true,missingSaveRejected=true,missingRepairSaved=true,nativeGameLaunched=false},ProjectStore.Json));
        }finally{discardChoiceForChecks=null;}
    }
}
