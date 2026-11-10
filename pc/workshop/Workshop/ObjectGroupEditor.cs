using YoshiWorkshop;
namespace WorkshopApp;
public sealed partial class WorkshopWindow {
    ObjectClipboard? objectClipboard;
    readonly ToolTip groupTips=new();
    readonly Label groupCount=new(){AutoSize=true,Padding=new(3,9,0,0),Text="Aucune sélection"};
    Button? copyGroup,pasteGroup,duplicateGroup,removeGroup;
    FlowLayoutPanel BuildGroupTools(){
        copyGroup=Button("Copier",()=>Safe(CopySelectedObjects),82);pasteGroup=Button("Coller",()=>Safe(PasteSelectedObjects),82);
        duplicateGroup=Button("Dupliquer",()=>Safe(DuplicateSelectedObjects),102);removeGroup=Button("Supprimer",()=>Safe(RemoveSelectedObjects),102);
        var all=Button("Tout",()=>{canvas.SelectAllObjects();canvas.Focus();},65);
        groupTips.SetToolTip(all,"Ctrl+A · sélectionner les objets modifiables de la salle");groupTips.SetToolTip(copyGroup,"Ctrl+C · copier la sélection dans l’éditeur");groupTips.SetToolTip(pasteGroup,"Ctrl+V · coller au centre de la vue, si la salle possède les ressources");groupTips.SetToolTip(duplicateGroup,"Ctrl+D · créer une copie du groupe, légèrement décalée");groupTips.SetToolTip(removeGroup,"Suppr · retirer la sélection ; Annuler la restaure");
        return Bar(all,copyGroup,pasteGroup,duplicateGroup,removeGroup,groupCount);
    }
    void RefreshGroupButtons(){bool objects=canvas.TerrainTool==TerrainTool.Objects;bool editable=objects && canvas.SelectedObjects.Count>0 && canvas.SelectedObjects.All(ObjectGroups.Editable);if(copyGroup!=null)copyGroup.Enabled=editable;if(duplicateGroup!=null)duplicateGroup.Enabled=editable;if(removeGroup!=null)removeGroup.Enabled=editable;if(pasteGroup!=null)pasteGroup.Enabled=objects && current!=null && objectClipboard!=null;groupCount.Text=canvas.SelectedObjects.Count==0?"Aucune sélection":$"{canvas.SelectedObjects.Count} objet(s)";}
    void RequireObjectMode(){if(canvas.TerrainTool!=TerrainTool.Objects)throw new InvalidOperationException("Choisir Déplacer les objets dans l’onglet Terrain.");canvas.CancelObjectGesture();}
    void SelectAllSceneObjects(){if(sceneObjects.ContainsFocus)canvas.SetObjectSelection(sceneObjects.Items.Cast<ObjectRow>().Select(r=>r.Item).Where(ObjectGroups.Editable));else canvas.SelectAllObjects();}
    void CopySelectedObjects(){RequireObjectMode();if(current==null)return;objectClipboard=ObjectGroups.Copy(canvas.SelectedObjects,current.SourceZone,activeRoom);RefreshGroupButtons();status.Text=$"{objectClipboard.Count} objet(s) copié(s) dans l’éditeur. Coller les place au centre de la vue.";}
    void CutSelectedObjects(){CopySelectedObjects();RemoveSelectedObjects();}
    void PasteSelectedObjects(){RequireObjectMode();if(current==null || objectClipboard==null)return;var center=canvas.WorldCenter;var copies=ObjectGroups.PreparePaste(rom,current,activeRoom,objectClipboard,center.X,center.Y);AddObjectGroup(copies,"collé(s)");}
    void DuplicateSelectedObjects(){RequireObjectMode();if(current==null || canvas.SelectedObjects.Count==0)return;var clip=ObjectGroups.Copy(canvas.SelectedObjects,current.SourceZone,activeRoom);var center=ObjectGroups.Center(canvas.SelectedObjects);var copies=ObjectGroups.PreparePaste(rom,current,activeRoom,clip,center.X+24,center.Y+24);AddObjectGroup(copies,"dupliqué(s)");}
    void AddObjectGroup(List<Placement> copies,string action){Remember();SceneObjects.AddRange(copies);canvas.SetObjectSelection(copies,copies.FirstOrDefault(),false);dirty=true;RefreshSceneObjects();ShowSelection();canvas.Invalidate();status.Text=$"{copies.Count} objet(s) {action}. Un seul Annuler restaure tout le groupe.";canvas.Focus();}
    void RemoveSelectedObjects(){RequireObjectMode();if(current==null || canvas.SelectedObjects.Count==0)return;var selected=canvas.SelectedObjects.ToArray();if(selected.Any(p=>!ObjectGroups.Editable(p)))throw new InvalidOperationException("Le départ de Yoshi et les couches de décor sont protégés.");Remember();foreach(var p in selected)SceneObjects.Remove(p);canvas.SetObjectSelection([],null,false);dirty=true;RefreshSceneObjects();ShowSelection();canvas.Invalidate();status.Text=$"{selected.Length} objet(s) retiré(s). Annuler restaure le groupe.";}
}
