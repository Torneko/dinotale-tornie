using System.Text.Json;
using YoshiWorkshop;
namespace WorkshopApp;
public sealed partial class WorkshopWindow {
    readonly ComboBox specialObjectChoice=new(){DropDownStyle=ComboBoxStyle.DropDownList,Width=215,DropDownWidth=420,MaxDropDownItems=16};
    readonly Label specialAvailability=new(){AutoSize=true,MaximumSize=new(215,0),Width=215,Text="Choisir une salle."};
    readonly Label specialObjectInfo=new(){AutoSize=true,MaximumSize=new(215,0),Width=215};
    readonly Button addSpecialObject=new(){Text="Ajouter dans la vue",Width=215,Height=40,FlatStyle=FlatStyle.Flat,BackColor=Color.White,Enabled=false};
    readonly Button showSpecialRooms=new(){Text="Salles compatibles",Width=215,Height=36,FlatStyle=FlatStyle.Flat,BackColor=Color.White,Enabled=false};
    readonly ListBox specialRooms=new(){Width=215,Height=108,Visible=false,IntegralHeight=false};
    readonly Label specialRoomInfo=new(){AutoSize=true,MaximumSize=new(215,0),Width=215,Visible=false};
    readonly Button visitSpecialRoom=new(){Text="Afficher cette salle",Width=215,Height=36,FlatStyle=FlatStyle.Flat,BackColor=Color.White,Visible=false,Enabled=false};
    bool specialRoomsOpen;
    sealed record SpecialRoomOption(int Zone,string Label,IReadOnlyList<ushort> Objects){public override string ToString()=>Label;}
    TabPage? specialObjectsTab;
    sealed record SpecialOption(ushort Id,string Label){public override string ToString()=>Label;}
    void BuildSpecialObjects(TabControl inspector){
        specialObjectsTab=new TabPage("Spéciaux");var column=Column(Caption("Objets spéciaux"),specialObjectChoice,addSpecialObject,specialObjectInfo,showSpecialRooms,specialRooms,specialRoomInfo,visitSpecialRoom,specialAvailability);
        column.Padding=new(5);foreach(Control c in column.Controls)if(!c.AutoSize)c.Width=215;specialObjectsTab.Controls.Add(column);inspector.TabPages.Add(specialObjectsTab);
        specialObjectChoice.SelectedIndexChanged+=(s,e)=>{addSpecialObject.Enabled=current!=null && specialObjectChoice.SelectedItem!=null;specialObjectInfo.Text=specialObjectChoice.SelectedItem is SpecialOption o?ObjectCatalog.Get(o.Id).Description:"Aucun objet spécial disponible dans cette salle.";};
        addSpecialObject.Click+=(s,e)=>Safe(AddEditorSpecial);
        showSpecialRooms.Click+=(s,e)=>Safe(()=>{specialRoomsOpen=!specialRoomsOpen;RefreshSpecialRooms();});
        specialRooms.SelectedIndexChanged+=(s,e)=>RefreshSpecialRoomSelection();
        specialRooms.DoubleClick+=(s,e)=>Safe(VisitSpecialRoom);visitSpecialRoom.Click+=(s,e)=>Safe(VisitSpecialRoom);
    }
    void RefreshEditorSpecials(){
        ushort? previous=(specialObjectChoice.SelectedItem as SpecialOption)?.Id;specialObjectChoice.Items.Clear();
        if(current==null){addSpecialObject.Enabled=false;specialObjectInfo.Text="";specialAvailability.Text="Choisir une salle.";RefreshSpecialRooms();return;}
        var allowed=rom.SourceObjects(activeRoom);bool eggs=SpecialEditorObjects.Compatible(allowed) && specialRunner.Available;
        var ids=AvailableSpecialObjects(activeRoom);
        foreach(var id in ids)specialObjectChoice.Items.Add(new SpecialOption(id,ObjectCatalog.Get(id).Name));
        specialObjectChoice.SelectedItem=specialObjectChoice.Items.Cast<SpecialOption>().FirstOrDefault(o=>o.Id==previous);if(specialObjectChoice.SelectedIndex<0 && specialObjectChoice.Items.Count>0)specialObjectChoice.SelectedIndex=0;
        addSpecialObject.Enabled=specialObjectChoice.SelectedItem!=null;
        specialAvailability.Text=eggs?"Les dix couleurs d’œufs sont disponibles ici. Les trouvailles restent réservées à cette création et ne débloquent pas les Yoshis de la partie personnelle.":"Les œufs spéciaux sont disponibles dans les salles contenant déjà une bulle d’œuf d’origine : certaines salles du 2-1, 2-4, 3-2 et 3-3.";
        if(specialObjectChoice.Items.Count==0)specialObjectInfo.Text="Aucun objet spécial disponible dans cette salle.";
        RefreshSpecialRooms();
    }
    IReadOnlyList<ushort> AvailableSpecialObjects(int room){
        var allowed=rom.SourceObjects(room);bool eggs=SpecialEditorObjects.Compatible(allowed) && specialRunner.Available;
        return (eggs?SpecialEditorObjects.Eggs:Enumerable.Empty<ushort>()).Concat(SpecialEditorObjects.NativeSpecials.Where(allowed.Contains).Where(id=>ObjectCatalog.Get(id).Identified)).ToArray();
    }
    void RefreshSpecialRooms(){
        int? previous=(specialRooms.SelectedItem as SpecialRoomOption)?.Zone;specialRooms.BeginUpdate();
        try{
            specialRooms.Items.Clear();showSpecialRooms.Enabled=current!=null;
            specialRooms.Visible=specialRoomInfo.Visible=visitSpecialRoom.Visible=specialRoomsOpen && current!=null;
            showSpecialRooms.Text=specialRoomsOpen?"Masquer les salles":"Salles compatibles";
            if(current!=null && specialRoomsOpen){
                foreach(var room in current.AllRooms()){
                    var objects=AvailableSpecialObjects(room.SourceZone);if(objects.Count==0)continue;
                    string label=RoomLabel(room.SourceZone)+(room.SourceZone==activeRoom?" · salle actuelle":"");
                    specialRooms.Items.Add(new SpecialRoomOption(room.SourceZone,label,objects));
                }
                var choices=specialRooms.Items.Cast<SpecialRoomOption>();
                specialRooms.SelectedItem=choices.FirstOrDefault(o=>o.Zone==activeRoom)??choices.FirstOrDefault(o=>o.Zone==previous)??choices.FirstOrDefault();
            }
            specialRooms.Enabled=specialRooms.Items.Count>0;
        }finally{specialRooms.EndUpdate();}
        RefreshSpecialRoomSelection();
    }
    void RefreshSpecialRoomSelection(){
        var room=specialRooms.SelectedItem as SpecialRoomOption;visitSpecialRoom.Enabled=room!=null && room.Zone!=activeRoom;
        if(room==null){specialRoomInfo.Text=current==null?"Choisir un niveau.":"Aucun objet spécial disponible dans les salles incluses. Tu peux ajouter les autres salles d’origine depuis le sélecteur de salle.";return;}
        var names=new List<string>();if(room.Objects.Any(SpecialEditorObjects.IsEgg))names.Add("Œufs de Yoshi en bulle · 10 couleurs");
        names.AddRange(room.Objects.Where(id=>!SpecialEditorObjects.IsEgg(id)).Select(id=>ObjectCatalog.Get(id).Name));
        specialRoomInfo.Text=$"Niveau {VanillaRom.SourceCode(current!.SourceZone)} · {RoomLabel(room.Zone)}"+(room.Zone==activeRoom?" · salle actuelle":"")+"\n\n"+string.Join("\n",names);
    }
    void VisitSpecialRoom(){
        if(current==null || specialRooms.SelectedItem is not SpecialRoomOption destination || destination.Zone==activeRoom)return;
        var choice=roomChoice.Items.Cast<RoomOption>().FirstOrDefault(o=>o.Zone==destination.Zone);if(choice==null)return;
        roomChoice.SelectedItem=choice;status.Text="Salle affichée : "+RoomLabel(activeRoom)+". Choisis l’objet à ajouter dans la liste des spéciaux.";
    }
    void AddEditorSpecial(){
        if(current==null || specialObjectChoice.SelectedItem is not SpecialOption choice)return;CheckObjectCapacity();var item=rom.CreatePlacement(choice.Id,activeRoom);var at=canvas.WorldCenter;item.X=Math.Clamp(at.X,-100000,100000);item.Y=Math.Clamp(at.Y,-100000,100000);
        var candidate=JsonSerializer.Deserialize<LevelProject>(JsonSerializer.Serialize(current,ProjectStore.Json),ProjectStore.Json)!;candidate.ObjectsIn(activeRoom).Add(item);rom.ExportPlacements(candidate,true);
        Remember();SceneObjects.Add(item);dirty=true;canvas.Selected=item;RefreshSceneObjects();ShowSelection();canvas.Invalidate();status.Text="Objet spécial ajouté à cette création : "+ObjectCatalog.Get(choice.Id).Name;
    }
    public void RunEditorSpecialChecks(){
        if(!pcRoot.Contains("validation",StringComparison.OrdinalIgnoreCase))throw new InvalidOperationException("Copie QA isolée requise.");
        void Need(bool condition,string message){if(!condition)throw new Exception(message);}
        sourceLevel.SelectedItem=sourceLevel.Items.Cast<LevelTemplate>().Single(t=>t.SourceZone==46);NewFromVanilla();
        if(specialObjectsTab?.Parent is TabControl inspector)inspector.SelectedTab=specialObjectsTab;
        string beforeNavigation=JsonSerializer.Serialize(current,ProjectStore.Json);int beforeHistory=undo.Count;bool beforeDirty=dirty;
        if(specialRoomsOpen)showSpecialRooms.PerformClick();showSpecialRooms.PerformClick();
        Need(specialRooms.Visible && specialRooms.Items.Cast<SpecialRoomOption>().Any(o=>o.Zone==120 && o.Objects.Count(SpecialEditorObjects.IsEgg)==10),"Compatible secondary egg room absent");
        specialRooms.SelectedItem=specialRooms.Items.Cast<SpecialRoomOption>().Single(o=>o.Zone==120);
        Need(visitSpecialRoom.Enabled && specialRoomInfo.Text.Contains("10 couleurs"),"Compatible room description or navigation absent");visitSpecialRoom.PerformClick();
        Need(activeRoom==120 && specialRooms.SelectedItem is SpecialRoomOption {Zone:120} selectedRoom && selectedRoom.Label.Contains("salle actuelle") && !visitSpecialRoom.Enabled,"Compatible room did not become current");
        Need(JsonSerializer.Serialize(current,ProjectStore.Json)==beforeNavigation && undo.Count==beforeHistory && dirty==beforeDirty,"Room discovery changed project/history/dirty state");
        Need(specialObjectChoice.Items.Cast<SpecialOption>().Count(o=>SpecialEditorObjects.IsEgg(o.Id))==10,"Ten egg choices absent");
        string primary=JsonSerializer.Serialize(current!.Objects,ProjectStore.Json);undo.Clear();redo.Clear();var ids=new List<string>();
        for(int i=0;i<10;i++){canvas.FocusAt(48+i*40,240,1);specialObjectChoice.SelectedItem=specialObjectChoice.Items.Cast<SpecialOption>().Single(o=>o.Id==0x4F30+i);addSpecialObject.PerformClick();Need(canvas.Selected?.ObjectId==0x4F30+i,"Wrong egg added");ids.Add(canvas.Selected!.Id);}
        Need(undo.Count==10 && JsonSerializer.Serialize(current.Objects,ProjectStore.Json)==primary,"Egg addition history/room isolation failed");
        Undo();Need(!SceneObjects.Any(p=>p.Id==ids[9]),"Egg undo failed");Redo();Need(SceneObjects.Single(p=>p.Id==ids[9]).ObjectId==0x4F39,"Egg redo failed");
        SaveLevel();string id=current.Id;LoadLevel(store.Levels().Single(l=>l.Id==id));roomChoice.SelectedItem=roomChoice.Items.Cast<RoomOption>().Single(r=>r.Zone==120);Need(ids.All(key=>SceneObjects.Any(p=>p.Id==key)),"Eggs lost after reload");
        terrainToolChoice.SelectedIndex=0;canvas.SetObjectSelection(SceneObjects.Where(p=>ids.Take(2).Contains(p.Id)).ToArray());int before=SceneObjects.Count;DuplicateSelectedObjects();Need(SceneObjects.Count==before+2 && canvas.SelectedObjects.All(p=>SpecialEditorObjects.IsEgg(p.ObjectId)),"Egg group duplication failed");Undo();
        roomChoice.SelectedIndex=0;Need(!specialObjectChoice.Items.Cast<SpecialOption>().Any(o=>SpecialEditorObjects.IsEgg(o.Id)),"Incompatible room exposes eggs");
        roomChoice.SelectedItem=roomChoice.Items.Cast<RoomOption>().Single(r=>r.Zone==120);canvas.FocusAt(180,240,1.2f);SaveLevel();
        sourceLevel.SelectedItem=sourceLevel.Items.Cast<LevelTemplate>().Single(t=>t.SourceZone==35);NewFromVanilla();
        Need(specialRooms.Items.Cast<SpecialRoomOption>().All(o=>current!.AllRooms().Any(r=>r.SourceZone==o.Zone)),"Compatible room listing retained the previous level");
        Need(specialRooms.Items.Cast<SpecialRoomOption>().Any(o=>o.Zone==activeRoom && o.Objects.Contains((ushort)0x45E3)),"Current black carrier room absent from discovery");
        Need(specialObjectChoice.Items.Cast<SpecialOption>().Any(o=>o.Id==0x45E3) && specialObjectChoice.Items.Cast<SpecialOption>().Any(o=>o.Id==0x45E5),"Black carrier choices absent");
        specialObjectChoice.SelectedItem=specialObjectChoice.Items.Cast<SpecialOption>().Single(o=>o.Id==0x45E3);int blackBefore=SceneObjects.Count;addSpecialObject.PerformClick();Need(SceneObjects.Count==blackBefore+1 && canvas.Selected?.ObjectId==0x45E3,"Standalone black carrier add failed");Undo();Need(SceneObjects.Count==blackBefore,"Black carrier undo failed");SaveLevel();
                Directory.CreateDirectory(Path.Combine(pcRoot,"build/editor-0200"));File.WriteAllText(Path.Combine(pcRoot,"build/editor-0200/ui-specials.json"),JsonSerializer.Serialize(new{tenEggChoices=true,correctColors=true,primaryRoomUntouched=true,oneUndoPerAdd=true,undoRedo=true,saveReload=true,groupDuplication=true,incompatibleRoomHidden=true,blackCarrierChoices=true,blackCarrierAddUndo=true,compatibleRoomDiscovery=true,currentRoomMarked=true,compatibleRoomNavigation=true,discoveryDoesNotEditProject=true,compatibleRoomsRefreshWithLevel=true},ProjectStore.Json));
    }
}
