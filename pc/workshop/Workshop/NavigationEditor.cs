using System.Text.Json;
using YoshiWorkshop;
namespace WorkshopApp;
public sealed partial class WorkshopWindow {
    static NumericUpDown NavigationNumber()=>new(){Minimum=-100000,Maximum=100000,DecimalPlaces=1,Increment=16,Width=215};
    readonly NumericUpDown entryX=NavigationNumber(),entryY=NavigationNumber(),arrivalX=NavigationNumber(),arrivalY=NavigationNumber();
    readonly ComboBox passageDestination=new(){DropDownStyle=ComboBoxStyle.DropDownList,Width=215,DropDownWidth=300};
    readonly CheckBox passageFollowsEntry=new(){Text="Suivre l’entrée de la salle",AutoSize=true,Checked=true};
    readonly Label entrySummary=new(){AutoSize=true,MaximumSize=new(215,0),Width=215};
    readonly Button applyEntry=new(){Text="Appliquer X / Y",Width=215,Height=36},applyPassage=new(){Text="Appliquer le passage",Width=215,Height=36};
    bool navigationUpdating;
    TabPage? entryTab,passagesTab;
    sealed record DestinationOption(int Room,string Label){public override string ToString()=>Label;}
    void BuildNavigationEditor(TabControl inspector){
        entryTab=new TabPage("Entrées");var entryColumn=Column(Caption("Entrée de cette salle"),entrySummary,Caption("Position X"),entryX,Caption("Position Y (vers le bas)"),entryY,applyEntry,
            Button("Placer au centre",()=>Safe(()=>ChangeEntry(canvas.WorldCenter.X,canvas.WorldCenter.Y)),215),
            Button("Voir l’entrée",()=>Safe(FocusNativeEntry),215),Button("Rétablir l’origine",()=>Safe(RestoreNativeSpawn),215),
            new Label{AutoSize=true,MaximumSize=new(215,0),Text="Le point règle le départ et l’arrivée des pots qui suivent cette entrée. Choisir une zone du terrain visible, puis enregistrer et tester."});
        entryColumn.Padding=new(5);entryTab.Controls.Add(entryColumn);inspector.TabPages.Add(entryTab);
        passagesTab=new TabPage("Passages");passageList.Dock=DockStyle.None;passageList.Width=215;passageList.Height=108;passageSummary.Dock=DockStyle.None;passageSummary.AutoSize=true;passageSummary.MaximumSize=new(215,0);passageSummary.Padding=Padding.Empty;visitPassage.Dock=DockStyle.None;visitPassage.Width=215;visitPassage.Height=38;visitPassage.Text="Voir la destination";
        var column=Column(Caption("Passage sélectionné"),passageList,passageSummary,Caption("Salle de destination"),passageDestination,passageFollowsEntry,
            Caption("Arrivée X"),arrivalX,Caption("Arrivée Y"),arrivalY,applyPassage,Button("Rétablir le passage",()=>Safe(ResetSelectedPassage),215),visitPassage);
        column.Padding=new(5);passagesTab.Controls.Add(column);inspector.TabPages.Add(passagesTab);
        applyEntry.Click+=(s,e)=>Safe(()=>ChangeEntry((float)entryX.Value,(float)entryY.Value));applyPassage.Click+=(s,e)=>Safe(ApplySelectedPassage);
        passageFollowsEntry.CheckedChanged+=(s,e)=>{if(!navigationUpdating)RefreshArrivalInputs();};
        passageDestination.SelectedIndexChanged+=(s,e)=>{if(!navigationUpdating)RefreshArrivalInputs(true);};
        passageList.SelectedIndexChanged+=(s,e)=>{if(!passageUpdating)Safe(SelectPassage);};passageList.DoubleClick+=(s,e)=>Safe(VisitPassage);visitPassage.Click+=(s,e)=>Safe(VisitPassage);
    }
    void RefreshEntryControls(){
        navigationUpdating=true;try{
            bool has=current!=null;entryX.Enabled=entryY.Enabled=applyEntry.Enabled=has;if(!has){entrySummary.Text="Choisir une salle.";return;}
            var room=current!.AllRooms().Single(r=>r.SourceZone==activeRoom);RoomEntry point;
            try{point=rom.EffectiveEntry(current,activeRoom);}catch(InvalidDataException){var native=SceneObjects.Single(p=>p.ObjectId==0x4001);point=new(native.X,native.Y);}
            entryX.Value=(decimal)point.X;entryY.Value=(decimal)point.Y;entrySummary.Text=RoomLabel(activeRoom)+"\n"+(room.Entry==null?"Entrée d’origine conservée.":"Entrée personnalisée.");
        }finally{navigationUpdating=false;}
    }
    void ChangeEntry(float px,float py){
        if(current==null)return;string before=CaptureEditState();if(!rom.SetRoomEntry(current,activeRoom,px,py)){RefreshEntryControls();return;}
        CommitNavigation(before,"Point d’entrée modifié. Les pots qui le suivent utiliseront cette position.");
    }
    void CommitNavigation(string before,string message){undo.Push(before);redo.Clear();dirty=true;RefreshSceneObjects();RefreshEntryControls();ShowSelection();canvas.Invalidate();status.Text=message;}
    void RefreshPassageControls(){
        navigationUpdating=true;try{
            passageDestination.Items.Clear();var row=passageList.SelectedItem as PassageRow;bool can=current!=null && row!=null && rom.CanEditPassage(row.Placement);
            if(current!=null)foreach(var room in current.AllRooms())passageDestination.Items.Add(new DestinationOption(room.SourceZone,RoomLabel(room.SourceZone)));
            passageDestination.Enabled=passageFollowsEntry.Enabled=applyPassage.Enabled=can;
            visitPassage.Enabled=row!=null && current!=null && current.AllRooms().Any(r=>r.SourceZone==row.Passage.Destination);
            if(row==null){arrivalX.Enabled=arrivalY.Enabled=false;return;}
            var route=row.Placement.Passage;int target=route?.DestinationRoom??row.Passage.Destination;
            passageDestination.SelectedItem=passageDestination.Items.Cast<DestinationOption>().FirstOrDefault(r=>r.Room==target);passageFollowsEntry.Checked=route is {ArrivalX:null};
            arrivalX.Value=Math.Clamp((decimal)(route?.ArrivalX??row.Passage.ArrivalX),arrivalX.Minimum,arrivalX.Maximum);arrivalY.Value=Math.Clamp((decimal)(route?.ArrivalY??row.Passage.ArrivalY),arrivalY.Minimum,arrivalY.Maximum);
            arrivalX.Enabled=arrivalY.Enabled=can && !passageFollowsEntry.Checked;
        }finally{navigationUpdating=false;}
    }
    void RefreshArrivalInputs(bool destinationChanged=false){
        if(current==null || passageList.SelectedItem is not PassageRow row)return;bool can=rom.CanEditPassage(row.Placement);arrivalX.Enabled=arrivalY.Enabled=can && !passageFollowsEntry.Checked;
        if((destinationChanged || passageFollowsEntry.Checked) && passageDestination.SelectedItem is DestinationOption destination){
            try{var point=rom.EffectiveEntry(current,destination.Room);arrivalX.Value=(decimal)point.X;arrivalY.Value=(decimal)point.Y;}catch(InvalidDataException){passageSummary.Text="Définir d’abord une entrée dans cette salle, ou choisir une arrivée précise.";}
        }
    }
    void ApplySelectedPassage(){
        if(current==null || passageList.SelectedItem is not PassageRow row || passageDestination.SelectedItem is not DestinationOption destination)return;
        string before=CaptureEditState();if(!rom.SetPassage(current,activeRoom,row.Placement.Id,destination.Room,passageFollowsEntry.Checked?null:(float)arrivalX.Value,passageFollowsEntry.Checked?null:(float)arrivalY.Value))return;
        CommitNavigation(before,"Passage modifié pour cette copie. Les autres pots conservent leurs réglages.");
    }
    void ResetSelectedPassage(){
        if(current==null || passageList.SelectedItem is not PassageRow row)return;string before=CaptureEditState();if(!rom.SetPassage(current,activeRoom,row.Placement.Id,null,null,null))return;CommitNavigation(before,"Destination et arrivée d’origine rétablies pour ce passage.");
    }
}
