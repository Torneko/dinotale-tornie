using System.Text.Json;
using YoshiWorkshop;
namespace WorkshopApp;
public sealed partial class WorkshopWindow {
    readonly ComboBox roomChoice=new(){DropDownStyle=ComboBoxStyle.DropDownList,Width=170};
    readonly System.Windows.Forms.Button includeRooms=new(){Text="Ajouter les autres salles",Width=240,Height=36,FlatStyle=FlatStyle.Flat,BackColor=Color.White};
    int activeRoom=43;
    sealed record RoomOption(int Zone,string Label){public override string ToString()=>Label;}
    sealed class EditorHistory {public LevelProject Level {get;set;}=new();public int Room {get;set;}public string? Selected {get;set;}}
    List<Placement> SceneObjects=>current?.ObjectsIn(activeRoom)??[];
    string RoomLabel(int zone)=>zone==current?.SourceZone?"Entrée":"Salle "+(NativeRooms.For(current!.SourceZone).ToList().IndexOf(zone)+1);
    void ConfigureRoomEditor(){
        roomChoice.SelectedIndexChanged+=(s,e)=>{if(updating || current==null || roomChoice.SelectedItem is not RoomOption r)return;BindRoom(r.Zone);};
        includeRooms.Click+=(s,e)=>Safe(()=>{if(current==null)return;Remember();rom.IncludeOriginalRooms(current);dirty=true;RefreshRoomChoices(activeRoom);status.Text="Les salles d’origine sont ajoutées à ta copie. Tes objets modifiés sont conservés.";});
    }
    void RefreshRoomChoices(int? preferred=null){
        if(current==null)return;bool before=updating;updating=true;roomChoice.Items.Clear();foreach(var r in current.AllRooms())roomChoice.Items.Add(new RoomOption(r.SourceZone,RoomLabel(r.SourceZone)));
        int next=preferred.HasValue && current.AllRooms().Any(r=>r.SourceZone==preferred.Value)?preferred.Value:current.SourceZone;
        roomChoice.SelectedItem=roomChoice.Items.Cast<RoomOption>().Single(r=>r.Zone==next);includeRooms.Enabled=current.Rooms.Count+1<NativeRooms.For(current.SourceZone).Count;updating=before;BindRoom(next);
    }
    void BindRoom(int zone){
        if(current==null)return;activeRoom=zone;canvas.SceneCaption=$"Niveau {VanillaRom.SourceCode(current.SourceZone)} · {RoomLabel(zone)}";
        canvas.Level=new LevelProject{Id=current.Id,Name=current.Name,SourceZone=current.SourceZone,Objects=SceneObjects};LoadRoomTerrain();canvas.Fit();RefreshExtendedCategories();RefreshSceneObjects();ShowSelection();
    }
    void RestoreNativeSpawn(){if(current==null)throw new InvalidOperationException("Choisis un niveau.");var native=rom.ReadRoom(activeRoom).Objects.Single(p=>p.ObjectId==0x4001);var p=SceneObjects.Single(p=>p.ObjectId==0x4001);Remember();p.X=native.X;p.Y=native.Y;p.Layer=native.Layer;dirty=true;canvas.Selected=p;ShowSelection();canvas.Invalidate();status.Text="Point d’entrée d’origine rétabli.";}
    void CheckObjectCapacity(){if(SceneObjects.Count>=RoomProject.MaxObjects)throw new InvalidOperationException("Cette salle a atteint sa limite actuelle de 149 objets. Tu peux remplacer un fruit, ou retirer un objet avant d’en ajouter un autre.");}
    static bool LooseFruit(ushort id)=>id is >=0x4030 and <=0x4034 or >=0x4F00 and <=0x4F05;
    void ReplaceSelectedFruit(){
        if(current==null || canvas.Selected is not Placement p || palette.SelectedItem is not PaletteItem choice || !LooseFruit(p.ObjectId) || !LooseFruit(choice.Id))throw new InvalidOperationException("Sélectionne un fruit posé dans la salle, puis son remplacement dans la palette.");
        var replacement=rom.CreatePlacement(choice.Id,activeRoom);Remember();p.ObjectId=replacement.ObjectId;p.Layer=replacement.Layer;p.HybridVariant=null;dirty=true;canvas.Invalidate();RefreshSceneObjects();ShowSelection();status.Text="Fruit remplacé en conservant sa position.";
    }
    string CaptureEditState()=>JsonSerializer.Serialize(new EditorHistory{Level=current!,Room=activeRoom,Selected=canvas.Selected?.Id},ProjectStore.Json);
    void RestoreEditState(string json){
        var state=JsonSerializer.Deserialize<EditorHistory>(json,ProjectStore.Json)!;current=state.Level;updating=true;levelName.Text=current.Name;updating=false;dirty=true;RefreshRoomChoices(state.Room);canvas.Selected=SceneObjects.FirstOrDefault(p=>p.Id==state.Selected);ShowSelection();canvas.Invalidate();
    }
    void CheckRoomEditorIntegration(){
        sourceLevel.SelectedItem=sourceLevel.Items.Cast<LevelTemplate>().Single(t=>t.SourceZone==43);NewFromVanilla();
        if(roomChoice.Items.Count!=2 || current!.Rooms.Single().SourceZone!=81)throw new Exception("Les deux salles du 1-1 ne sont pas disponibles.");
        string rootBefore=JsonSerializer.Serialize(current.Objects,ProjectStore.Json);roomChoice.SelectedIndex=1;
        if(activeRoom!=81 || canvas.Level!.Objects!=current.Rooms[0].Objects)throw new Exception("Le canevas ne montre pas la salle choisie.");
        var p=SceneObjects.First(o=>o.ObjectId is >=0x4030 and <=0x4034);canvas.Selected=p;ShowSelection();float old=p.X;x.Value=(decimal)old+32;
        if(SceneObjects.Single(o=>o.Id==p.Id).X!=old+32 || JsonSerializer.Serialize(current.Objects,ProjectStore.Json)!=rootBefore)throw new Exception("La modification de la salle secondaire touche l’entrée.");
        Undo();if(activeRoom!=81 || SceneObjects.Single(o=>o.Id==p.Id).X!=old)throw new Exception("Annulation dans la mauvaise salle.");Redo();if(activeRoom!=81 || SceneObjects.Single(o=>o.Id==p.Id).X!=old+32)throw new Exception("Rétablissement multi-salle incorrect.");
        var spawn=SceneObjects.Single(o=>o.ObjectId==0x4001);canvas.Selected=spawn;ShowSelection();if(x.Enabled || y.Enabled)throw new Exception("Départ encore déplaçable.");float sy=spawn.Y;canvas.Nudge(0,16);if(spawn.Y!=sy)throw new Exception("Départ déplacé au clavier.");spawn.X+=32;RestoreNativeSpawn();if(spawn.X!=rom.ReadRoom(activeRoom).Objects.Single(o=>o.ObjectId==0x4001).X)throw new Exception("Restauration du départ incorrecte.");Undo();if(SceneObjects.Single(o=>o.ObjectId==0x4001).X!=spawn.X+32)throw new Exception("Restauration du départ non annulable.");Redo();
        canvas.Selected=SceneObjects.Single(o=>o.Id==p.Id);float oldY=canvas.Selected.Y;canvas.Nudge(0,16);if(canvas.Selected.Y!=oldY+16)throw new Exception("Déplacement vertical inversé.");Undo();
        int count=SceneObjects.Count;string fruitId=p.Id;var selected=SceneObjects.Single(o=>o.Id==fruitId);ushort originalKind=selected.ObjectId;canvas.Selected=selected;
        objectCategory.SelectedIndex=0;objectSearch.Text="kiwi";RefreshPalette();palette.SelectedItem=palette.Items.Cast<PaletteItem>().Single(i=>i.Id==0x4F05);ReplaceSelectedFruit();
        if(SceneObjects.Count!=count || SceneObjects.Single(o=>o.Id==fruitId).ObjectId!=0x4F05)throw new Exception("Remplacement du fruit incorrect.");Undo();if(SceneObjects.Single(o=>o.Id==fruitId).ObjectId!=originalKind)throw new Exception("Remplacement non annulable.");Redo();
        SaveLevel();string id=current.Id;LoadLevel(store.Levels().Single(l=>l.Id==id));roomChoice.SelectedIndex=1;if(SceneObjects.Single(o=>o.Id==p.Id).X!=old+32)throw new Exception("Salle secondaire perdue à la relecture.");
        var legacy=rom.CloneFirstLevel();store.Save(legacy);LoadLevel(legacy);if(roomChoice.Items.Count!=1 || !includeRooms.Enabled)throw new Exception("Création ancienne mal reconnue.");
        string original=JsonSerializer.Serialize(current!.Objects,ProjectStore.Json);includeRooms.PerformClick();if(roomChoice.Items.Count!=2 || JsonSerializer.Serialize(current.Objects,ProjectStore.Json)!=original)throw new Exception("La conversion modifie les anciens placements.");Undo();if(roomChoice.Items.Count!=1 || current!.Schema!=1)throw new Exception("Conversion non annulable.");SaveLevel();
        Directory.CreateDirectory(Path.Combine(pcRoot,"build/editor-0150"));File.WriteAllText(Path.Combine(pcRoot,"build/editor-0150/ui-rooms.json"),JsonSerializer.Serialize(new{roomSelection=true,secondaryRoomEdit=true,entryUntouched=true,roomAwareUndoRedo=true,roomSaveReload=true,legacyImport=true,conversionUndoable=true,fruitReplacement=true,nativeEntryLocked=true,nativeEntryRestoreUndoable=true,positiveYDownNudge=true},ProjectStore.Json));
    }
}
