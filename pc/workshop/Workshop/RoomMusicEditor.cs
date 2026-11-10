using System.Text.Json;
using YoshiWorkshop;
namespace WorkshopApp;
public sealed partial class WorkshopWindow {
    readonly ComboBox roomMusicChoice=new(){DropDownStyle=ComboBoxStyle.DropDownList,Width=215,DropDownWidth=440,MaxDropDownItems=18};
    readonly Label roomMusicSummary=new(){AutoSize=true,MaximumSize=new(215,0),Width=215,Text="Choisir une salle pour sa musique."};
    TabPage? roomMusicTab;
    bool roomMusicUpdating;
    sealed record MusicOption(int? Id,string Label){public override string ToString()=>Label;}
    void BuildRoomMusic(TabControl inspector){
        roomMusicTab=new TabPage("Musique");
        roomMusicChoice.Items.Add(new MusicOption(null,"Musique d’origine"));foreach(var track in RoomMusic.Tracks)roomMusicChoice.Items.Add(new MusicOption(track.Id,track.Label));
        var column=Column(Caption("Musique de cette salle"),roomMusicChoice,roomMusicSummary,
            Button("Musique d’origine",()=>roomMusicChoice.SelectedIndex=0,215),
            new Label{AutoSize=true,MaximumSize=new(215,0),Width=215,Text="Le choix s’applique à la salle affichée. Les autres salles conservent leur réglage.\n\nEnregistre puis teste le niveau pour écouter la musique dans le jeu. Annuler rétablit le choix précédent."});
        column.Padding=new(5);foreach(Control c in column.Controls)if(!c.AutoSize)c.Width=215;roomMusicTab.Controls.Add(column);inspector.TabPages.Add(roomMusicTab);
        roomMusicChoice.SelectedIndexChanged+=(s,e)=>{if(!roomMusicUpdating)Safe(ChangeRoomMusic);};
    }
    void RefreshRoomMusic(){
        roomMusicUpdating=true;try{
            roomMusicChoice.Enabled=current!=null;
            int? chosen=current==null?null:RoomMusic.Selected(current,activeRoom);
            roomMusicChoice.SelectedItem=roomMusicChoice.Items.Cast<MusicOption>().FirstOrDefault(o=>o.Id==chosen);
            if(current==null){roomMusicSummary.Text="Choisir une salle pour sa musique.";return;}
            int native=rom.ReadRoomMusic(activeRoom);string original=RoomMusic.Tracks.FirstOrDefault(t=>t.Id==native)?.Label??$"Musique {native}";
            roomMusicSummary.Text=$"{RoomLabel(activeRoom)}\nOrigine : {original}\n"+(chosen==null?"La musique d’origine est conservée.":"Choix personnalisé pour cette salle.");
        }finally{roomMusicUpdating=false;}
    }
    void ChangeRoomMusic(){
        if(current==null || roomMusicChoice.SelectedItem is not MusicOption option)return;
        string before=CaptureEditState();if(!rom.SetRoomMusic(current,activeRoom,option.Id)){RefreshRoomMusic();return;}
        undo.Push(before);redo.Clear();dirty=true;RefreshRoomMusic();
        status.Text=option.Id==null?"Musique d’origine rétablie pour cette salle.":"Musique de la salle modifiée. Enregistre puis teste pour l’écouter.";
    }
    public void RunRoomMusicChecks(){
        if(!pcRoot.Contains("validation",StringComparison.OrdinalIgnoreCase))throw new InvalidOperationException("Copie QA isolée requise.");
        void Phase(string label){Directory.CreateDirectory(Path.Combine(pcRoot,"build/editor-0200"));File.AppendAllText(Path.Combine(pcRoot,"build/editor-0200/ui-music-phases.log"),label+" dirty="+dirty+"\n");}
        Phase("begin");
        void Need(bool condition,string message){if(!condition)throw new Exception(message);}
        string State()=>JsonSerializer.Serialize(current,ProjectStore.Json);
        sourceLevel.SelectedItem=sourceLevel.Items.Cast<LevelTemplate>().Single(t=>t.SourceZone==43);NewFromVanilla();Phase("copy");
        if(roomMusicTab?.Parent is TabControl inspector)inspector.SelectedTab=roomMusicTab;
        undo.Clear();redo.Clear();string original=State();var placements=JsonSerializer.Serialize(current!.Objects,ProjectStore.Json);
        int track=RoomMusic.Tracks.First(t=>t.Id!=0 && t.Id!=rom.ReadRoomMusic(43)).Id;
        roomMusicChoice.SelectedItem=roomMusicChoice.Items.Cast<MusicOption>().Single(o=>o.Id==track);
        Need(RoomMusic.Selected(current!,43)==track && RoomMusic.Selected(current!,81)==null && undo.Count==1,"Room music choice did not isolate primary");
        Phase("primary");string changed=State();Need(JsonSerializer.Serialize(current!.Objects,ProjectStore.Json)==placements,"Music changed placements");
        roomMusicChoice.SelectedItem=roomMusicChoice.Items.Cast<MusicOption>().Single(o=>o.Id==track);Need(undo.Count==1,"Repeated choice added history");
        Undo();Need(State()==original && roomMusicChoice.SelectedItem is MusicOption {Id:null},"Music undo failed");Redo();Need(State()==changed,"Music redo failed");
        Phase("undo-redo");roomChoice.SelectedIndex=1;Need(roomMusicChoice.SelectedItem is MusicOption {Id:null},"Other room inherited override");
        int secondary=RoomMusic.Tracks.First(t=>t.Id!=track && t.Id!=rom.ReadRoomMusic(81)).Id;roomMusicChoice.SelectedItem=roomMusicChoice.Items.Cast<MusicOption>().Single(o=>o.Id==secondary);Need(RoomMusic.Selected(current!,81)==secondary && RoomMusic.Selected(current!,43)==track,"Secondary theme leaked to another room");
        Phase("secondary");SaveLevel();string id=current!.Id,saved=State();LoadLevel(store.Levels().Single(l=>l.Id==id));Need(State()==saved && rom.ExportPlacements(current!,true)[7]=='5',"Music lost on save/reopen");
        Phase("reloaded");var copy=store.Duplicate(current!);rom.SetRoomMusic(copy,43,null);Need(RoomMusic.Selected(current!,43)==track,"Duplicate shares music");
        Phase("duplicate");roomMusicChoice.SelectedIndex=0;Need(RoomMusic.Selected(current!,43)==null && RoomMusic.Selected(current!,81)==secondary,"Reset primary cleared secondary");
        roomChoice.SelectedIndex=1;roomMusicChoice.SelectedIndex=0;Need(!current!.AllRooms().Any(r=>r.MusicId.HasValue) && rom.ExportPlacements(current!,true)[7]=='3',"Clear all music did not restore legacy format");Undo();Need(RoomMusic.Selected(current!,81)==secondary,"Reset music not undoable");
        Phase("complete");Directory.CreateDirectory(Path.Combine(pcRoot,"build/editor-0200"));File.WriteAllText(Path.Combine(pcRoot,"build/editor-0200/ui-music.json"),JsonSerializer.Serialize(new{roomChoices=true,primaryAndSecondaryIsolation=true,secondaryTheme=true,placementsUntouched=true,noOpNoHistory=true,undoRedo=true,resetUndoable=true,saveReload=true,duplicateIsolation=true,legacyFormatAfterClear=true,v5Export=true},ProjectStore.Json));
        roomChoice.SelectedIndex=0;canvas.FocusAt(240,400,1.2f);SaveLevel();
    }
}
