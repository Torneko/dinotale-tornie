using System.Text.Json;
using YoshiWorkshop;
namespace WorkshopApp;
public sealed partial class WorkshopWindow {
    public void RunObjectGroupChecks(){
        if(!pcRoot.Contains("validation",StringComparison.OrdinalIgnoreCase))throw new InvalidOperationException("Copie QA isolée requise.");
        Directory.CreateDirectory(Path.Combine(pcRoot,"build/editor-0180"));
        void Need(bool yes,string message){if(!yes)throw new Exception(message);}
        string State()=>JsonSerializer.Serialize(current,ProjectStore.Json);
        void Rejected(Action action){try{action();}catch(InvalidDataException){return;}catch(InvalidOperationException){return;}throw new Exception("Invalid UI group operation accepted");}
        sourceLevel.SelectedItem=sourceLevel.Items.Cast<LevelTemplate>().Single(t=>t.SourceZone==43);NewFromVanilla();
        current!.Objects.RemoveAll(p=>ObjectGroups.Editable(p));var ids=new List<string>();
        ushort[] kinds=[0x4030,0x4F00,0x4F02,0x4F12,0x4F25];
        for(int i=0;i<kinds.Length;i++){var p=rom.CreatePlacement(kinds[i]);p.X=160+i*64;p.Y=288;current.Objects.Add(p);ids.Add(p.Id);}
        RefreshRoomChoices(43);terrainToolChoice.SelectedIndex=0;snapChoice.SelectedIndex=1;canvas.FocusAt(288,288,1.5f);undo.Clear();redo.Clear();
        Placement Item(int n)=>SceneObjects.Single(p=>p.Id==ids[n]);
        var other=JsonSerializer.Serialize(current.Rooms,ProjectStore.Json);int before=undo.Count;
        canvas.CheckObjectGesture(canvas.TerrainScreenPoint(120,248),canvas.TerrainScreenPoint(310,320));
        Need(canvas.SelectedObjects.Count==3 && undo.Count==before,"Marquee selection failed or changed history");
        var from=canvas.TerrainScreenPoint(Item(1).X,Item(1).Y);var to=new Point(from.X+48,from.Y+24);
        canvas.CheckObjectGesture(from,to);Need(canvas.SelectedObjects.Count==3 && Item(0).X==192 && Item(1).X==256 && Item(2).X==320 && Item(1).Y==304,"Group drag/snap failed");Need(undo.Count==before+1,"Group stroke is not one history entry");
        Undo();Need(Item(0).X==160 && canvas.SelectedObjects.Count==3 && canvas.Selected?.Id==ids[1],"Undo lost group or primary selection");Redo();Need(Item(2).X==320 && canvas.SelectedObjects.Count==3,"Redo lost group");
        string unchanged=State();int history=undo.Count;from=canvas.TerrainScreenPoint(Item(1).X,Item(1).Y);
        canvas.CheckObjectGesture(from,from);Need(State()==unchanged && undo.Count==history,"Click polluted history");
        canvas.CheckObjectGesture(from,new(from.X+96,from.Y+48),cancel:true);Need(State()==unchanged && undo.Count==history,"Cancelled group gesture changed data/history");
        canvas.CheckObjectGesture(from,new(from.X+96,from.Y+48),loseCapture:true);Need(State()==unchanged && undo.Count==history,"Lost capture committed partial move");
        // Clicking an unaligned object must not snap it before a real drag.
        Item(4).X+=3;Item(4).Y+=3;float unalignedX=Item(4).X,unalignedY=Item(4).Y;var unaligned=canvas.TerrainScreenPoint(unalignedX,unalignedY);
        canvas.CheckObjectGesture(unaligned,unaligned);Need(Item(4).X==unalignedX && Item(4).Y==unalignedY && undo.Count==history,"Unaligned click snapped the object");
        canvas.CheckObjectGesture(unaligned,new(unaligned.X+1,unaligned.Y+1));Need(Item(4).X==unalignedX && Item(4).Y==unalignedY && undo.Count==history,"Mouse jitter snapped the object");
        canvas.CheckObjectGesture(unaligned,unaligned,via:new(unaligned.X+48,unaligned.Y+24));Need(Item(4).X==unalignedX && Item(4).Y==unalignedY && undo.Count==history,"Returning to drag origin changed position/history");
        Item(4).X-=3;Item(4).Y-=3;canvas.SetObjectSelection(ids.Take(3).Select(id=>SceneObjects.Single(p=>p.Id==id)),Item(1));Need(State()==unchanged,"Click checks changed unrelated data");
        canvas.Nudge(16,0);Need(Item(0).X==208 && Item(2).X==336,"Keyboard group move failed");Undo();Need(State()==unchanged,"Keyboard group undo failed");
        x.Value+=16;Need(Item(0).X==208 && Item(2).X==336,"Group coordinate edit failed");Undo();Need(State()==unchanged,"Coordinate group undo failed");
        var fourth=canvas.TerrainScreenPoint(Item(3).X,Item(3).Y);canvas.CheckObjectGesture(fourth,fourth,Keys.Shift);Need(canvas.SelectedObjects.Count==4,"Shift click did not add");
        var first=canvas.TerrainScreenPoint(Item(0).X,Item(0).Y);canvas.CheckObjectGesture(first,first,Keys.Shift);Need(canvas.SelectedObjects.Count==3 && !canvas.SelectedObjects.Contains(Item(0)),"Shift click did not remove");
        var spawn=SceneObjects.Single(p=>p.ObjectId==0x4001);var lockPoint=canvas.TerrainScreenPoint(spawn.X,spawn.Y);canvas.CheckObjectGesture(lockPoint,lockPoint,Keys.Shift);Need(canvas.SelectedObjects.Count==3,"Locked spawn added to group");
        // Use the native ListBox selection events, not a separate test model.
        sceneObjects.ClearSelected();for(int i=0;i<sceneObjects.Items.Count;i++)if(((ObjectRow)sceneObjects.Items[i]).Item.Id is string id && (id==ids[3] || id==ids[4]))sceneObjects.SetSelected(i,true);
        Need(canvas.SelectedObjects.Count==2 && canvas.SelectedObjects.Any(p=>p.HybridVariant==2),"List multi-selection not bound to canvas");
        CopySelectedObjects();Need(objectClipboard?.Count==2,"Copy failed");unchanged=State();int count=SceneObjects.Count;DuplicateSelectedObjects();var duplicates=canvas.SelectedObjects.Select(p=>p.Id).ToArray();
        Need(SceneObjects.Count==count+2 && duplicates.All(id=>!ids.Contains(id)) && canvas.SelectedObjects.Any(p=>p.HybridVariant==2) && canvas.SelectedObjects.Any(p=>p.ObjectId==0x4F25),"Duplicate lost content/identities");Undo();Need(State()==unchanged && canvas.SelectedObjects.Count==2,"Group duplicate undo failed");Redo();Need(canvas.SelectedObjects.Select(p=>p.Id).Order().SequenceEqual(duplicates.Order()),"Redo changed duplicate IDs");
        canvas.Nudge(32,0);unchanged=State();count=SceneObjects.Count;PasteSelectedObjects();Need(SceneObjects.Count==count+2 && canvas.SelectedObjects.Any(p=>p.HybridVariant==2) && canvas.SelectedObjects.Any(p=>p.ObjectId==0x4F25),"Paste failed");var pasted=canvas.SelectedObjects.Select(p=>p.Id).ToArray();Need(pasted.All(id=>!duplicates.Contains(id)),"Paste reused IDs");
        RemoveSelectedObjects();Need(SceneObjects.Count==count && canvas.SelectedObjects.Count==0,"Group delete failed");Undo();Need(canvas.SelectedObjects.Count==2 && pasted.All(id=>SceneObjects.Any(p=>p.Id==id)),"Delete undo failed");
        CutSelectedObjects();Need(SceneObjects.Count==count,"Cut failed");Undo();Need(canvas.SelectedObjects.Count==2,"Cut undo lost selection");
        // Failure must not create an undo entry or mutate the target room.
        var padding=new List<Placement>();while(SceneObjects.Count<149){var p=rom.CreatePlacement(0x4030);p.X=1000+padding.Count*32;p.Y=300;SceneObjects.Add(p);padding.Add(p);}unchanged=State();history=undo.Count;Rejected(PasteSelectedObjects);Need(State()==unchanged && undo.Count==history,"Rejected capacity paste mutated state");foreach(var p in padding)SceneObjects.Remove(p);
        var pot=rom.CreatePlacement(0x4208);pot.X=1000;pot.Y=300;SceneObjects.Add(pot);canvas.Selected=pot;CopySelectedObjects();roomChoice.SelectedIndex=1;unchanged=State();history=undo.Count;Rejected(PasteSelectedObjects);Need(State()==unchanged && undo.Count==history,"Rejected cross-room paste mutated state");roomChoice.SelectedIndex=0;SceneObjects.Remove(SceneObjects.Single(p=>p.Id==pot.Id));
        canvas.Selected=SceneObjects.Single(p=>p.ObjectId==0x4001);unchanged=State();history=undo.Count;Rejected(DuplicateSelectedObjects);Rejected(RemoveSelectedObjects);Need(State()==unchanged && undo.Count==history,"Protected spawn changed");
        Need(other==JsonSerializer.Serialize(current!.Rooms,ProjectStore.Json),"Other room was modified");
        SaveLevel();string levelId=current.Id;LoadLevel(store.Levels().Single(p=>p.Id==levelId));Need(SceneObjects.Any(p=>p.ObjectId==0x4F25) && SceneObjects.Any(p=>p.HybridVariant==2),"Special group save/reload failed");rom.ExportPlacements(current!,true);
        terrainToolChoice.SelectedIndex=1;Need(copyGroup?.Enabled==false && pasteGroup?.Enabled==false,"Object actions active during terrain painting");terrainToolChoice.SelectedIndex=0;
        canvas.SetObjectSelection(ids.Select(id=>SceneObjects.Single(p=>p.Id==id)));canvas.FocusAt(300,300,1.5f);ShowSelection();
        var report=new{unalignedClickNoMove=true,tinyJitterNoMove=true,returnToOriginNoOp=true,marquee=true,shiftClick=true,listSelection=true,groupDrag=true,snap=true,nudge=true,coordinateEdit=true,undoRedoSelection=true,oneUndoPerDrag=true,noOp=true,cancel=true,captureLoss=true,copy=true,paste=true,duplicate=true,cut=true,delete=true,uniqueIds=true,specialContentPreserved=true,capacityFailureAtomic=true,resourceFailureAtomic=true,spawnProtected=true,secondaryRoomUntouched=true,persistence=true,terrainModeGuard=true,passed=true};
        File.WriteAllText(Path.Combine(pcRoot,"build/editor-0180/ui-groups.json"),JsonSerializer.Serialize(report,ProjectStore.Json));
    }
}
