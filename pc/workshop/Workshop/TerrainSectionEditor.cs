using YoshiWorkshop;
namespace WorkshopApp;
public sealed partial class WorkshopWindow {
    TerrainSection? sectionClipboard;
    readonly Button copyTerrainSection=new(){Text="Copier la zone · Ctrl+C",Width=235,Height=38,FlatStyle=FlatStyle.Flat,BackColor=Color.White,Enabled=false};
    readonly Button pasteTerrainSection=new(){Text="Placer la copie · Ctrl+V",Width=235,Height=38,FlatStyle=FlatStyle.Flat,BackColor=Color.White,Enabled=false};
    readonly CheckBox sectionCollision=new(){Text="Coller les collisions",Checked=true,AutoSize=true};
    readonly Label sectionSelectionInfo=new(){Width=235,Height=65,Text="Aucune zone sélectionnée."};
    readonly Label sectionClipboardInfo=new(){Width=235,Height=90,Text="Aucune section copiée."};
    TabPage? terrainSectionsTab;
    bool syncingSectionCollision;
    void BuildTerrainSections(TabControl inspector){
        terrainSectionsTab=new TabPage("Sections");
        var contents=Column(Caption("Copier une section"),
            Button("Sélectionner une zone",()=>{terrainToolChoice.SelectedIndex=(int)TerrainTool.SectionSelect;canvas.Focus();},235),sectionSelectionInfo,copyTerrainSection,
            Caption("Reproduire la section"),sectionClipboardInfo,sectionCollision,pasteTerrainSection,
            new Label{Width=235,Height=150,Text="Trace un rectangle dans le terrain, puis copie-le. Déplace l’aperçu et clique pour poser la section. Échap quitte la pose. Annuler rétablit tout le collage.\n\nLa copie s’utilise dans la même salle d’origine, y compris dans une autre création de ce niveau."});
        contents.Padding=new(5);foreach(Control item in contents.Controls)if(!item.AutoSize)item.Width=215;terrainSectionsTab.Controls.Add(contents);
        inspector.TabPages.Add(terrainSectionsTab);
        copyTerrainSection.Click+=(s,e)=>Safe(CopyTerrainSection);pasteTerrainSection.Click+=(s,e)=>Safe(StartTerrainSectionPaste);
        canvas.TerrainSectionSelectionChanged+=RefreshTerrainSectionControls;
        canvas.TerrainSectionPlaced+=(map,point)=>Safe(()=>ApplyTerrainSection(map,point));
        canvas.TerrainStampCancelled+=()=>{terrainToolChoice.SelectedIndex=(int)TerrainTool.SectionSelect;status.Text="Pose abandonnée ; le terrain est conservé.";};
        sectionCollision.CheckedChanged+=(s,e)=>{if(syncingSectionCollision)return;syncingSectionCollision=true;paintCollision.Checked=sectionCollision.Checked;canvas.SectionApplyCollision=sectionCollision.Checked;syncingSectionCollision=false;canvas.Invalidate();};
        paintCollision.CheckedChanged+=(s,e)=>{if(syncingSectionCollision)return;syncingSectionCollision=true;sectionCollision.Checked=paintCollision.Checked;canvas.SectionApplyCollision=paintCollision.Checked;syncingSectionCollision=false;canvas.Invalidate();};
    }
    void RefreshTerrainSectionControls(){
        var region=canvas.SelectedTerrainRegion;canvas.TerrainRoom=activeRoom;
        sectionSelectionInfo.Text=region==null?"Aucune zone sélectionnée.":$"Zone : {region.Width} × {region.Height} tuiles\n{region.Width*16} × {region.Height*16} pixels";
        copyTerrainSection.Enabled=current!=null && region!=null && canvas.TerrainTool==TerrainTool.SectionSelect;
        bool compatible=current!=null && sectionClipboard!=null && sectionClipboard.SourceRoom==activeRoom && canvas.TerrainMaps.Any(m=>m.ResourceId==sectionClipboard.ResourceId);
        pasteTerrainSection.Enabled=compatible;
        sectionClipboardInfo.Text=sectionClipboard==null?"Aucune section copiée.":$"Copie : {sectionClipboard.Width} × {sectionClipboard.Height} tuiles\n"+(compatible?"Prête à placer dans cette salle.":"Reviens dans la salle d’origine pour la placer.");
    }
    void CopyTerrainSection(){
        if(current==null || canvas.SelectedTerrainRegion is not TerrainRegion region || canvas.TerrainTool!=TerrainTool.SectionSelect)throw new InvalidOperationException("Trace d’abord une zone avec Sélectionner une section.");
        canvas.CancelTerrainGesture();var map=canvas.TerrainMaps.Single(m=>m.ResourceId==region.ResourceId);
        var next=TerrainSections.Capture(map,activeRoom,region.X,region.Y,region.Width,region.Height);
        canvas.SetTerrainSectionClipboard(next,map);sectionClipboard=next;RefreshTerrainSectionControls();
        status.Text=$"Section {next.Width} × {next.Height} copiée. Placer la copie affiche un aperçu avant le clic.";
    }
    void StartTerrainSectionPaste(){
        if(current==null || sectionClipboard==null)throw new InvalidOperationException("Copie d’abord une section de terrain.");
        if(sectionClipboard.SourceRoom!=activeRoom || !canvas.TerrainMaps.Any(m=>m.ResourceId==sectionClipboard.ResourceId))throw new InvalidOperationException("Cette section doit être placée dans sa salle d’origine.");
        terrainToolChoice.SelectedIndex=(int)TerrainTool.SectionStamp;canvas.Focus();status.Text="Déplace l’aperçu puis clique pour poser. Bord rouge : zone incompatible. Échap : abandonner.";
    }
    void ApplyTerrainSection(NativeTerrainMap map,Point point){
        if(current==null || sectionClipboard==null)return;
        var next=TerrainSections.PreparePaste(sectionClipboard,map,activeRoom,point.X,point.Y,sectionCollision.Checked);
        string before=CaptureEditState();if(!rom.SetTerrainCells(current,activeRoom,map.ResourceId,next)){status.Text="Cette section est déjà identique ; aucune modification.";return;}
        undo.Push(before);redo.Clear();dirty=true;LoadRoomTerrain(false);RefreshTerrainSectionControls();
        status.Text=$"Section {sectionClipboard.Width} × {sectionClipboard.Height} posée. Un seul Annuler rétablit le terrain.";
    }
    bool HandleTerrainSectionKeys(KeyEventArgs e){
        if(canvas.TerrainTool is not (TerrainTool.SectionSelect or TerrainTool.SectionStamp) || !e.Control)return false;
        Action? action=e.KeyCode==Keys.C?CopyTerrainSection:e.KeyCode==Keys.V?StartTerrainSectionPaste:null;
        if(action==null)return false;Safe(action);e.SuppressKeyPress=true;return true;
    }
}
