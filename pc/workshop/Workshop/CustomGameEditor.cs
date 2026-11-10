using System.Text.Json;
using YoshiWorkshop;
namespace WorkshopApp;

public sealed partial class WorkshopWindow {
    bool gameUpdating,gameDirty;
    readonly Label gameSaveStatus=new(){Dock=DockStyle.Top,Height=32,Padding=new(8),Text="Choisis ou crée une partie."};
    // Checks inject the two answers only inside the existing isolated UI suite.
    Func<string,bool>? discardChoiceForChecks;
    sealed record GameLevelRow(int Index,string Id,LevelProject? Level){
        public string Name=>Level?.Name ?? "Niveau introuvable · "+Id;
    }
    bool ConfirmUnsaved(bool modified,string title,string message){
        if(!modified)return true;
        if(automatedChecks){
            if(discardChoiceForChecks!=null)return discardChoiceForChecks(title);
            throw new InvalidOperationException("Le contrôle tente d’abandonner des modifications : "+title);
        }
        return MessageBox.Show(this,message,title,MessageBoxButtons.YesNo,MessageBoxIcon.Question)==DialogResult.Yes;
    }
    bool ConfirmGameDiscard()=>ConfirmUnsaved(gameDirty,"Partie personnalisée modifiée","Des modifications de la partie personnalisée ne sont pas enregistrées. Continuer ?");
    void MarkGameDirty(){gameDirty=true;gameSaveStatus.Text="Modifications de la partie non enregistrées.";}
    void RefreshGames(){
        bool before=gameUpdating;gameUpdating=true;
        try{string? id=game?.Id;games.Items.Clear();foreach(var saved in store.Games())games.Items.Add(saved);if(id!=null)games.SelectedItem=games.Items.Cast<CustomGame>().FirstOrDefault(g=>g.Id==id);}
        finally{gameUpdating=before;}
    }
    void LoadCustomGame(CustomGame selected){
        bool before=gameUpdating;gameUpdating=true;
        try{
            game=JsonSerializer.Deserialize<CustomGame>(JsonSerializer.Serialize(selected,ProjectStore.Json),ProjectStore.Json)!;
            gameName.Text=game.Name;interactions.Checked=game.Coop.PlayerInteractions;gameDirty=false;
            gameSaveStatus.Text="Partie enregistrée.";RefreshGameLevels();RefreshSessionStatus();
        }finally{gameUpdating=before;}
    }
    void SelectCustomGame(){
        if(gameUpdating || games.SelectedItem is not CustomGame selected || selected.Id==game?.Id)return;
        if(!ConfirmGameDiscard()){
            bool before=gameUpdating;gameUpdating=true;
            try{games.SelectedItem=games.Items.Cast<CustomGame>().FirstOrDefault(g=>g.Id==game?.Id);}
            finally{gameUpdating=before;}
            return;
        }
        LoadCustomGame(selected);
    }
    void NewCustomGame(){
        if(!ConfirmGameDiscard())return;
        LoadCustomGame(new());bool before=gameUpdating;gameUpdating=true;
        try{games.ClearSelected();}finally{gameUpdating=before;}
        MarkGameDirty();
    }
    void SaveGame(){
        if(game==null)throw new InvalidOperationException("Créer ou sélectionner une partie.");
        game.Name=gameName.Text;game.Coop.PlayerInteractions=interactions.Checked;store.Save(game);
        gameDirty=false;gameSaveStatus.Text="Partie enregistrée.";RefreshGames();status.Text="Partie personnalisée enregistrée.";
    }
    void AddToGame(){
        if(game==null || current==null)throw new InvalidOperationException("Ouvrir un niveau dans l’éditeur et créer une partie.");
        SaveLevel();game.LevelIds.Add(current.Id);RefreshGameLevels();gameLevels.SelectedIndex=gameLevels.Items.Count-1;MarkGameDirty();
    }
    void RefreshGameLevels(){
        int selected=gameLevels.SelectedIndex;gameLevels.Items.Clear();if(game==null)return;
        var known=store.Levels().ToDictionary(l=>l.Id);
        for(int i=0;i<game.LevelIds.Count;i++){string id=game.LevelIds[i];gameLevels.Items.Add(new GameLevelRow(i,id,known.GetValueOrDefault(id)));}
        if(selected>=0 && gameLevels.Items.Count>0)gameLevels.SelectedIndex=Math.Min(selected,gameLevels.Items.Count-1);
    }
    GameLevelRow? SelectedGameLevel()=>game!=null && gameLevels.SelectedItem is GameLevelRow row && row.Index>=0 && row.Index<game.LevelIds.Count && game.LevelIds[row.Index]==row.Id?row:null;
    void MoveGameLevel(int delta){
        if(game==null || SelectedGameLevel() is not {} row)return;int i=row.Index,j=i+delta;
        if(j<0 || j>=game.LevelIds.Count)return;
        (game.LevelIds[i],game.LevelIds[j])=(game.LevelIds[j],game.LevelIds[i]);RefreshGameLevels();gameLevels.SelectedIndex=j;MarkGameDirty();
    }
    void RemoveGameLevel(){
        if(game==null || SelectedGameLevel() is not {} row)return;
        game.LevelIds.RemoveAt(row.Index);RefreshGameLevels();MarkGameDirty();
    }
    void TestGameLevel(){
        if(SelectedGameLevel() is not {} row)throw new InvalidOperationException("Choisir un niveau dans la partie.");
        if(row.Level==null)throw new InvalidOperationException("Ce niveau est introuvable. Rétablis son fichier ou retire cette ligne de la partie.");
        LaunchLevel(row.Level);
    }
}
