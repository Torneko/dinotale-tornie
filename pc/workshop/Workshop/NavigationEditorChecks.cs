using System.Text.Json;
using YoshiWorkshop;
namespace WorkshopApp;
public sealed partial class WorkshopWindow {
    public void RunNavigationChecks(){
        if(!pcRoot.Contains("validation",StringComparison.OrdinalIgnoreCase))throw new InvalidOperationException("Copie QA isolée requise.");
        void Need(bool value,string message){if(!value)throw new Exception(message);}
        string State()=>JsonSerializer.Serialize(current,ProjectStore.Json);
        sourceLevel.SelectedItem=sourceLevel.Items.Cast<LevelTemplate>().Single(t=>t.SourceZone==43);NewFromVanilla();
        var sourceEntry=SceneObjects.Single(p=>p.ObjectId==0x4001);float originalX=sourceEntry.X,originalY=sourceEntry.Y;string original=State();string secondary=JsonSerializer.Serialize(current!.Rooms,ProjectStore.Json);
        canvas.Selected=sourceEntry;ShowSelection();
        string invalidBefore=State();bool dirtyBefore=dirty;int undoBefore=undo.Count,redoBefore=redo.Count;
        x.Value=-1;y.Value=100000;
        Need(State()==invalidBefore && dirty==dirtyBefore && undo.Count==undoBefore && redo.Count==redoBefore,"Rejected coordinate changed project/history");
        Need(x.Value==(decimal)originalX && y.Value==(decimal)originalY && entryX.Value==(decimal)rom.EffectiveEntry(current!,activeRoom).X && entryY.Value==(decimal)rom.EffectiveEntry(current!,activeRoom).Y,"Rejected coordinate did not restore both inspectors");
        Need(!updating && !navigationUpdating && !IsDisposed && status.Text.Contains("page de terrain"),"Rejected coordinate left editor unusable");
        if(entryTab?.Parent is TabControl inspector)inspector.SelectedTab=entryTab;
        undo.Clear();redo.Clear();entryX.Value=480;entryY.Value=440;applyEntry.PerformClick();
        Need(current.Entry==new RoomEntry(480,440) && SceneObjects.Single(p=>p.ObjectId==0x4001).X==480 && undo.Count==1,"Entry edit/history missing");
        Need(JsonSerializer.Serialize(current.Rooms,ProjectStore.Json)==secondary,"Primary entry changed another room");
        applyEntry.PerformClick();Need(undo.Count==1,"No-op entry added history");Undo();Need(State()==original,"Entry undo did not restore exact project");Redo();Need(current.Entry==new RoomEntry(480,440),"Entry redo failed");
        string changed=State();
        Undo();string withRedo=State();int redoKept=redo.Count,undoKept=undo.Count;bool dirtyKept=dirty;
        canvas.Selected=SceneObjects.Single(p=>p.ObjectId==0x4001);ShowSelection();x.Value=-1;
        Need(State()==withRedo && redo.Count==redoKept && undo.Count==undoKept && dirty==dirtyKept,"Invalid coordinate discarded redo/history");
        Redo();Need(State()==changed,"Invalid coordinate prevented valid redo");RestoreNativeSpawn();Need(current.Entry==null && SceneObjects.Single(p=>p.ObjectId==0x4001).X==originalX,"Entry reset failed");Undo();Need(State()==changed,"Entry reset not undoable");
        roomChoice.SelectedIndex=1;Need(entryX.Value==72 && entryY.Value==344,"Secondary native anchor unresolved");roomChoice.SelectedIndex=0;
        if(passagesTab?.Parent is TabControl tabs)tabs.SelectedTab=passagesTab;
        var pot=SceneObjects.Single(p=>p.ObjectId==0x4208);string potId=pot.Id;
        passageList.SelectedItem=passageList.Items.Cast<PassageRow>().Single(r=>r.Placement.Id==potId);SelectPassage();Need(passageDestination.Enabled && visitPassage.Enabled,"Pot controls disabled");
        passageDestination.SelectedItem=passageDestination.Items.Cast<DestinationOption>().Single(r=>r.Room==81);passageFollowsEntry.Checked=false;arrivalX.Value=104;arrivalY.Value=344;int history=undo.Count;applyPassage.PerformClick();
        Need(SceneObjects.Single(p=>p.Id==potId).Passage==new PassageOverride(81,104,344) && undo.Count==history+1,"Per-pot arrival not applied");
        applyPassage.PerformClick();Need(undo.Count==history+1,"No-op route added history");string routeState=State();Undo();Need(SceneObjects.Single(p=>p.Id==potId).Passage==null,"Route undo failed");Redo();Need(State()==routeState,"Route redo failed");
        terrainToolChoice.SelectedIndex=0;canvas.Selected=SceneObjects.Single(p=>p.Id==potId);DuplicateSelectedObjects();string siblingId=canvas.Selected!.Id;
        Need(siblingId!=potId && canvas.Selected.Passage==new PassageOverride(81,104,344) && !ReferenceEquals(canvas.Selected.Passage,SceneObjects.Single(p=>p.Id==potId).Passage),"Route duplicate shares state");
        passageList.SelectedItem=passageList.Items.Cast<PassageRow>().Single(r=>r.Placement.Id==potId);SelectPassage();passageDestination.SelectedItem=passageDestination.Items.Cast<DestinationOption>().Single(r=>r.Room==43);passageFollowsEntry.Checked=false;arrivalX.Value=1000;arrivalY.Value=440;applyPassage.PerformClick();
        Need(SceneObjects.Single(p=>p.Id==siblingId).Passage==new PassageOverride(81,104,344),"Changing source affected duplicate");
        SaveLevel();string id=current.Id,saved=State();LoadLevel(store.Levels().Single(l=>l.Id==id));Need(State()==saved && current.Entry==new RoomEntry(480,440),"Navigation lost on reload");
        passageList.SelectedItem=passageList.Items.Cast<PassageRow>().Single(r=>r.Placement.Id==potId);SelectPassage();Need(passageDestination.SelectedItem is DestinationOption {Room:43} && arrivalX.Value==1000,"Route controls not restored");
        VisitPassage();Need(activeRoom==43 && canvas.WorldCenter.X==1000 && canvas.WorldCenter.Y==440,"Destination navigation did not use edited arrival");
        roomChoice.SelectedIndex=1;Undo();Need(activeRoom==81,"Reload had stale history");roomChoice.SelectedIndex=0;
        passageList.SelectedItem=passageList.Items.Cast<PassageRow>().Single(r=>r.Placement.Id==potId);ResetSelectedPassage();Need(SceneObjects.Single(p=>p.Id==potId).Passage==null,"Route reset failed");Undo();Need(SceneObjects.Single(p=>p.Id==potId).Passage is {DestinationRoom:43},"Route reset undo failed");
        roomChoice.SelectedIndex=1;string before=State();ChangeEntry(104,344);Need(current.Rooms.Single().Entry==new RoomEntry(104,344) && current.Entry==new RoomEntry(480,440),"Secondary entry did not isolate room");Undo();Need(activeRoom==81 && State()==before,"Undo restored another room");Redo();Need(activeRoom==81 && current.Rooms.Single().Entry==new RoomEntry(104,344),"Secondary redo failed");
        SaveLevel();Need(rom.ExportPlacements(current,true)[7]=='6',"Navigation did not export V6");
        Directory.CreateDirectory(Path.Combine(pcRoot,"build/editor-0210"));File.WriteAllText(Path.Combine(pcRoot,"build/editor-0210/ui-navigation.json"),JsonSerializer.Serialize(new{entryControls=true,rejectedCoordinateControlsRestored=true,rejectedCoordinateHistoryPreserved=true,entryUndoRedo=true,entryResetUndoable=true,noOpHistory=true,secondaryRoomIsolation=true,resolvedNativeAnchor=true,perInstanceArrival=true,routeUndoRedo=true,duplicateIsolation=true,saveReload=true,visitEditedDestination=true,resetRouteUndoable=true,roomAwareHistory=true,v6Export=true},ProjectStore.Json));
        if(entryTab?.Parent is TabControl review)review.SelectedTab=entryTab;FocusNativeEntry();
    }
}
