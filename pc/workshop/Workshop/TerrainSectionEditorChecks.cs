using System.Text.Json;
using YoshiWorkshop;
namespace WorkshopApp;
public sealed partial class WorkshopWindow {
    public void RunTerrainSectionChecks(){
        if(!pcRoot.Contains("validation",StringComparison.OrdinalIgnoreCase))throw new InvalidOperationException("Utiliser une copie QA isolée.");
        if(terrainSectionsTab?.Parent is TabControl sectionInspector)sectionInspector.SelectedTab=terrainSectionsTab;
        void Need(bool condition,string message){if(!condition)throw new Exception(message);}
        string State()=>JsonSerializer.Serialize(current,ProjectStore.Json);
        void Reject(Action action){try{action();}catch(InvalidDataException){return;}catch(InvalidOperationException){return;}throw new Exception("Invalid section operation accepted");}
        sourceLevel.SelectedItem=sourceLevel.Items.Cast<LevelTemplate>().Single(t=>t.SourceZone==43);NewFromVanilla();
        var native=rom.ReadTerrain(43).Single();var solid=terrainBrushes.Items.Cast<TerrainBrush>().First(b=>b.Collision!=0 && BrushVisible(b));
        var pattern=new Dictionary<int,(ushort,ushort)>();for(int y=12;y<14;y++)for(int x=8;x<11;x++)pattern[native.CellIndex(x,y)]=(x==9 && y==12)?((ushort)65535,(ushort)0):(solid.Tile,solid.Collision);
        rom.SetTerrainCells(current!,43,native.ResourceId,pattern);LoadRoomTerrain(false);undo.Clear();redo.Clear();dirty=false;
        string before=State(),other=JsonSerializer.Serialize(current!.Rooms,ProjectStore.Json);
        Point Cell(int x,int y)=>canvas.TerrainScreenPoint(native.WorldX+x*16+8,native.WorldY+y*16+8);
        void View()=>canvas.FocusAt(native.WorldX+14*16,native.WorldY+15*16,1.7f);
        terrainToolChoice.SelectedIndex=6;View();canvas.CheckTerrainMouse(Cell(8,12),Cell(10,13));
        Need(canvas.SelectedTerrainRegion is {Width:3,Height:2} && copyTerrainSection.Enabled,"Rectangle selection failed");Need(State()==before && undo.Count==0 && !dirty,"Selection mutated level");
        copyTerrainSection.PerformClick();Need(sectionClipboard is {Count:6} && sectionClipboard.Cells[1].Tile==65535 && sectionClipboard.Cells[1].Collision==0,"Copied section missing empty cell");Need(State()==before && undo.Count==0,"Copy mutated level");
        var clip=sectionClipboard!;
        pasteTerrainSection.PerformClick();View();canvas.CheckTerrainSectionHover(Cell(16,12));Need(State()==before,"Preview changed terrain");
        var expected=TerrainSections.PreparePaste(clip,canvas.TerrainMaps.Single(),43,16,12,true);
        canvas.CheckTerrainMouse(Cell(16,12),Cell(16,12));var map=canvas.TerrainMaps.Single();Need(expected.All(v=>map.RawTiles[v.Key]==v.Value.Tile && map.Collisions[v.Key]==v.Value.Collision),"Stamped appearance/collision incorrect");
        Need(undo.Count==1 && JsonSerializer.Serialize(current.Rooms,ProjectStore.Json)==other,"Stamp not one undo / leaked room");string after=State();
        Undo();Need(State()==before && activeRoom==43,"Stamp undo incomplete");Redo();Need(State()==after && activeRoom==43,"Stamp redo incomplete");
        View();terrainToolChoice.SelectedIndex=7;int count=undo.Count;canvas.CheckTerrainMouse(Cell(16,12),Cell(16,12));Need(undo.Count==count && State()==after,"No-op stamp added history");
        View();canvas.CheckTerrainSectionHover(Cell(22,12));canvas.CheckTerrainSectionEscape();Need(State()==after && canvas.TerrainTool==TerrainTool.SectionSelect,"Escape did not abandon preview");
        terrainToolChoice.SelectedIndex=7;View();canvas.CheckTerrainSectionHeldEscape(Cell(22,12));Need(State()==after && canvas.TerrainTool==TerrainTool.SectionSelect && undo.Count==count,"Held mouse Escape did not abandon pose");
        terrainToolChoice.SelectedIndex=7;View();canvas.CheckTerrainSectionCaptureLoss(Cell(22,12),Cell(24,12));Need(State()==after && undo.Count==count,"Capture loss placed section");
        terrainToolChoice.SelectedIndex=6;View();canvas.CheckTerrainAbandon(Cell(20,12),Cell(22,13));Need(State()==after,"Selection cancellation modified level");
        sectionCollision.Checked=false;Need(!paintCollision.Checked && !canvas.SectionApplyCollision,"Collision controls out of sync");var original=canvas.TerrainMaps.Single();var masks=Enumerable.Range(0,6).Select(i=>original.Collisions[original.CellIndex(22+i%3,12+i/3)]).ToArray();
        pasteTerrainSection.PerformClick();View();canvas.CheckTerrainMouse(Cell(22,12),Cell(22,12));map=canvas.TerrainMaps.Single();Need(Enumerable.Range(0,6).All(i=>map.Collisions[map.CellIndex(22+i%3,12+i/3)]==masks[i]),"Visual-only paste changed collision");Undo();sectionCollision.Checked=true;
        before=State();count=undo.Count;map=canvas.TerrainMaps.Single();Reject(()=>ApplyTerrainSection(map,new(map.Width/16-1,12)));Need(State()==before && undo.Count==count,"Rejected boundary paste partially applied");
        roomChoice.SelectedIndex=1;Need(!pasteTerrainSection.Enabled && canvas.SelectedTerrainRegion==null,"Room switch kept stale selection/paste");string second=State();Reject(StartTerrainSectionPaste);Need(State()==second,"Cross-room paste changed data");roomChoice.SelectedIndex=0;
        var changed=clip.Cells[0];rom.SetTerrainCells(current!,43,native.ResourceId,new Dictionary<int,(ushort,ushort)>{{native.CellIndex(8,12),((ushort)65535,(ushort)0)}});LoadRoomTerrain(false);Need(clip.Cells[0]==changed,"Clipboard changed with source painting");
        SaveLevel();string id=current.Id,stored=State();LoadLevel(store.Levels().Single(l=>l.Id==id));Need(State()==stored && rom.ExportPlacements(current!,true)[7]=='4',"Section data lost after save/reopen");
        var exported=rom.ExportPlacements(current!,true);Directory.CreateDirectory(Path.Combine(pcRoot,"build/editor-0190"));File.WriteAllBytes(Path.Combine(pcRoot,"build/editor-0190/ui-section.yslevel"),exported);
        terrainToolChoice.SelectedIndex=6;canvas.FocusAt(native.WorldX+14*16,native.WorldY+26*16,1.7f);canvas.CheckTerrainMouse(Cell(2,29),Cell(5,29));copyTerrainSection.PerformClick();pasteTerrainSection.PerformClick();canvas.CheckTerrainSectionHover(Cell(10,26));if(terrainSectionsTab?.Parent is TabControl inspector)inspector.SelectedTab=terrainSectionsTab;
        File.WriteAllText(Path.Combine(pcRoot,"build/editor-0190/ui-sections.json"),JsonSerializer.Serialize(new{rectangleSelection=true,selectionAndCopyNeverMutate=true,emptyTilesCopied=true,previewNeverMutates=true,tilesAndCollisionPasted=true,oneUndoPerPaste=true,redo=true,noOpNoHistory=true,escapeCancels=true,heldMouseEscapeCancels=true,captureLossCancels=true,selectionCancel=true,visualOnlyPreservesCollision=true,collisionControlsSynchronized=true,invalidPasteAtomic=true,crossRoomRejected=true,roomSelectionCleared=true,secondaryRoomUntouched=true,immutableClipboard=true,saveReload=true,v4Export=true},ProjectStore.Json));
    }
}
