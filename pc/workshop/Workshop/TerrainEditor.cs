using System.Text.Json;
using YoshiWorkshop;
namespace WorkshopApp;
public sealed partial class WorkshopWindow {
    readonly CheckBox terrainChoice=new(){Text="Afficher le terrain",Checked=true,AutoSize=true};
    readonly CheckBox collisionChoice=new(){Text="Tuiles de collision",AutoSize=true};
    readonly Label terrainSummary=new(){Width=235,Height=85,Text="Choisis un niveau et une salle."};
    readonly ListBox passageList=new(){Dock=DockStyle.Fill,IntegralHeight=false};
    readonly System.Windows.Forms.Button visitPassage=new(){Text="Voir la salle de destination",Dock=DockStyle.Bottom,Height=42,Enabled=false};
    readonly Label passageSummary=new(){Dock=DockStyle.Bottom,Height=76,Padding=new(7),Text="Choisis un passage pour le repérer dans la salle."};
    bool passageUpdating;
    sealed record PassageRow(Placement Placement,NativePassage Passage,string Label){public override string ToString()=>Label;}
    void BuildTerrainPanels(TabControl inspector){
        inspector.Multiline=true;var view=new TabPage("Vue");view.Controls.Add(Column(Caption("Terrain de la salle"),terrainChoice,collisionChoice,Button("Voir le point d’entrée",FocusNativeEntry,235),terrainSummary,new Label{Width=235,Height=175,Text="Les décors en profondeur et les éléments animés se vérifient dans le jeu. Les repères montrent les tuiles de collision, pas la forme exacte des pentes."}));inspector.TabPages.Add(view);
        terrainChoice.CheckedChanged+=(s,e)=>{canvas.ShowTerrain=terrainChoice.Checked;canvas.Invalidate();};collisionChoice.CheckedChanged+=(s,e)=>{canvas.ShowCollisions=collisionChoice.Checked;canvas.Invalidate();};
        BuildTerrainPainting(inspector);BuildTerrainSections(inspector);BuildRoomMusic(inspector);BuildSpecialObjects(inspector);
        BuildNavigationEditor(inspector);
    }
    void LoadRoomTerrain(bool palette=true){
        try{var maps=rom.ReadTerrain(activeRoom);var changes=current?.AllRooms().Single(r=>r.SourceZone==activeRoom).Terrain??[];canvas.SetTerrain(maps.Select(m=>m.Preview(changes)).ToList());if(palette)LoadTerrainPalette(maps);terrainSummary.Text=maps.Count==0?"Aucun terrain principal disponible pour cet aperçu.":$"Terrain de {RoomLabel(activeRoom)}.\nTuiles et collisions modifiables.\nLes dimensions sont conservées.";}
        catch(InvalidDataException){canvas.SetTerrain([]);LoadTerrainPalette([]);terrainSummary.Text="L’aperçu de cette salle n’est pas disponible ; les objets restent éditables et le test du jeu conserve son terrain.";}
    }
    void FocusNativeEntry(){if(current==null)return;var point=rom.EffectiveEntry(current,activeRoom);canvas.FocusAt(point.X,point.Y);}
    void RefreshPassages(){
        if(current==null)return;string? selected=(passageList.SelectedItem as PassageRow)?.Placement.Id;passageUpdating=true;passageList.Items.Clear();
        foreach(var p in SceneObjects){var route=rom.EffectivePassage(current,activeRoom,p);if(route==null)continue;bool local=NativeRooms.Contains(current.SourceZone,route.Destination);string destination=local?RoomLabel(route.Destination):"Scène d’origine";string name=ObjectCatalog.Get(p.ObjectId) is {Identified:true} info?info.Name:"Passage";passageList.Items.Add(new PassageRow(p,route,name+" → "+destination));}
        passageList.SelectedItem=passageList.Items.Cast<PassageRow>().FirstOrDefault(p=>p.Placement.Id==selected);passageUpdating=false;visitPassage.Enabled=false;passageSummary.Text=passageList.Items.Count==0?"Aucun passage transportant vers une autre salle dans cette sélection.":"Choisis un passage pour régler sa destination et son arrivée.";RefreshPassageControls();
    }
    void SelectPassage(){if(current==null || passageList.SelectedItem is not PassageRow row){RefreshPassageControls();return;}canvas.Selected=row.Placement;canvas.FocusAt(row.Placement.X,row.Placement.Y);ShowSelection();bool present=current.AllRooms().Any(r=>r.SourceZone==row.Passage.Destination);visitPassage.Enabled=present;passageSummary.Text=present?"Ce passage mène à "+RoomLabel(row.Passage.Destination)+".":NativeRooms.Contains(current.SourceZone,row.Passage.Destination)?"Cette ancienne création ne contient pas encore la salle : utilise Ajouter les autres salles.":"Ce passage mène à une scène conservée dans le jeu, hors des salles éditables.";if(!rom.CanEditPassage(row.Placement))passageSummary.Text+=" Ce type de passage conserve ses réglages natifs.";RefreshPassageControls();}
    void VisitPassage(){if(current==null || passageList.SelectedItem is not PassageRow row || !current.AllRooms().Any(r=>r.SourceZone==row.Passage.Destination))return;roomChoice.SelectedItem=roomChoice.Items.Cast<RoomOption>().Single(r=>r.Zone==row.Passage.Destination);canvas.FocusAt(row.Passage.ArrivalX,row.Passage.ArrivalY);}
    void CheckTerrainIntegration(){
        sourceLevel.SelectedItem=sourceLevel.Items.Cast<LevelTemplate>().Single(t=>t.SourceZone==43);NewFromVanilla();string before=JsonSerializer.Serialize(current,ProjectStore.Json);
        if(canvas.TerrainMaps.Count!=1 || canvas.TerrainMaps[0].ResourceId!=0x8067 || passageList.Items.Count==0)throw new Exception("Terrain/passages du 1-1 absents.");
        terrainChoice.Checked=false;if(canvas.ShowTerrain)throw new Exception("Terrain non masquable.");terrainChoice.Checked=true;collisionChoice.Checked=true;if(!canvas.ShowCollisions)throw new Exception("Repères de collision absents.");collisionChoice.Checked=false;
        passageList.SelectedItem=passageList.Items.Cast<PassageRow>().First(p=>p.Placement.ObjectId==0x4208);SelectPassage();if(!visitPassage.Enabled || canvas.Selected?.ObjectId!=0x4208)throw new Exception("Pot non repéré.");VisitPassage();if(activeRoom!=81 || canvas.TerrainMaps.Single().ResourceId!=0x8068 || canvas.WorldCenter.X!=rom.EffectivePassage(current!,43,current!.Objects.Single(p=>p.ObjectId==0x4208))!.ArrivalX || canvas.WorldCenter.Y!=rom.EffectivePassage(current!,43,current!.Objects.Single(p=>p.ObjectId==0x4208))!.ArrivalY)throw new Exception("Destination du pot incorrecte.");
        if(JsonSerializer.Serialize(current,ProjectStore.Json)!=before)throw new Exception("La prévisualisation modifie le niveau.");
        SaveLevel();if(terrainChoice.Parent?.Parent is TabPage view && view.Parent is TabControl inspector)inspector.SelectedTab=view;Directory.CreateDirectory(Path.Combine(pcRoot,"build/editor-0150"));File.WriteAllText(Path.Combine(pcRoot,"build/editor-0150/ui-terrain.json"),JsonSerializer.Serialize(new{terrainLoaded=true,terrainToggle=true,collisionToggle=true,passageSelectCentersObject=true,potDestinationNavigation=true,secondaryTerrainLoaded=true,nativeArrivalPositionUsed=true,previewNeverEditsProject=true},ProjectStore.Json));
    }
}
