using System.Drawing.Drawing2D;
using System.Text.Json;
using YoshiWorkshop;
namespace WorkshopApp;
public sealed partial class WorkshopWindow {
    readonly ComboBox terrainToolChoice=new(){DropDownStyle=ComboBoxStyle.DropDownList,Width=235};
    readonly ListBox terrainBrushes=new(){Dock=DockStyle.Fill,DrawMode=DrawMode.OwnerDrawFixed,ItemHeight=42,IntegralHeight=false};
    readonly CheckBox paintCollision=new(){Text="Appliquer aussi la collision",Checked=true,AutoSize=true};
    readonly Label paintSummary=new(){Dock=DockStyle.Bottom,Height=112,Padding=new(8),Text="Choisir une tuile du niveau, puis peindre dans la vue."};
    bool paintingUpdating;NativeTerrainMap? paletteMap;TabPage? terrainPaintingTab;
    sealed record TerrainBrush(ushort Tile,ushort Collision,int Number){public override string ToString()=>$"Tuile {Number} · "+(Collision==0?"sans collision":"avec collision")+(Tile>=32768 && Tile!=65535?" · animée":"");}
    void BuildTerrainPainting(TabControl inspector){
        var page=terrainPaintingTab=new TabPage("Terrain");var top=Column(Caption("Dessiner le terrain"),terrainToolChoice,paintCollision);top.Dock=DockStyle.Top;top.Height=112;
        var reset=Button("Rétablir le terrain de la salle",()=>Safe(ResetRoomTerrain),235);reset.Dock=DockStyle.Bottom;reset.Height=40;
        page.Controls.Add(terrainBrushes);page.Controls.Add(paintSummary);page.Controls.Add(reset);page.Controls.Add(top);inspector.TabPages.Add(page);
        terrainToolChoice.Items.AddRange(new object[]{"Déplacer les objets","Pinceau","Rectangle","Pipette","Gomme · tuile et collision","Collision seule","Sélectionner une section","Coller une section"});terrainToolChoice.SelectedIndex=0;
        terrainToolChoice.SelectedIndexChanged+=(s,e)=>{if(paintingUpdating)return;canvas.TerrainTool=(TerrainTool)Math.Max(0,terrainToolChoice.SelectedIndex);paintCollision.Enabled=canvas.TerrainTool is TerrainTool.Pencil or TerrainTool.Rectangle or TerrainTool.SectionStamp;RefreshGroupButtons();RefreshTerrainSectionControls();};
        terrainBrushes.DrawItem+=DrawTerrainBrush;terrainBrushes.SelectedIndexChanged+=(s,e)=>{if(paintingUpdating)return;if(terrainToolChoice.SelectedIndex is 0 or 3)terrainToolChoice.SelectedIndex=1;};
        canvas.TerrainCommitted+=stroke=>Safe(()=>ApplyTerrainStroke(stroke));canvas.TerrainPicked+=PickTerrain;canvas.TerrainMessage+=text=>status.Text=text;
    }
    bool BrushVisible(TerrainBrush brush){if(paletteMap==null)return false;ushort tile=paletteMap.Resolve(brush.Tile);return tile!=65535 && Enumerable.Range(0,256).Any(n=>(paletteMap.Palette[paletteMap.Pixels[tile*256+n]]>>24)!=0);}
    void LoadTerrainPalette(IReadOnlyList<NativeTerrainMap> maps){
        var old=terrainBrushes.SelectedItem as TerrainBrush;paintingUpdating=true;terrainBrushes.Items.Clear();paletteMap=maps.SingleOrDefault();
        if(paletteMap!=null){int number=0;foreach(var pair in paletteMap.RawTiles.Zip(paletteMap.Collisions).Distinct().Where(p=>p.First!=65535).OrderBy(p=>p.Second==0).ThenBy(p=>p.First).ThenBy(p=>p.Second))terrainBrushes.Items.Add(new TerrainBrush(pair.First,pair.Second,++number));}
        terrainBrushes.SelectedItem=terrainBrushes.Items.Cast<TerrainBrush>().FirstOrDefault(b=>b.Tile==old?.Tile && b.Collision==old?.Collision);if(terrainBrushes.SelectedIndex<0 && terrainBrushes.Items.Count>0)terrainBrushes.SelectedItem=terrainBrushes.Items.Cast<TerrainBrush>().FirstOrDefault(b=>b.Collision!=0 && BrushVisible(b))??terrainBrushes.Items[0];
        terrainToolChoice.Enabled=paletteMap!=null;terrainBrushes.Enabled=paletteMap!=null;paintingUpdating=false;
        paintSummary.Text=paletteMap==null?"Pas de terrain principal éditable dans cette salle.":"Pipette : copier une tuile.\nGomme : effacer tuile + collision.\nÉchap : annuler le geste.";
    }
    void DrawTerrainBrush(object? sender,DrawItemEventArgs e){
        if(e.Index<0 || paletteMap==null || terrainBrushes.Items[e.Index] is not TerrainBrush b)return;e.DrawBackground();
        ushort tile=paletteMap.Resolve(b.Tile);var box=new Rectangle(e.Bounds.X+5,e.Bounds.Y+5,32,32);
        e.Graphics.FillRectangle(Brushes.LightGray,box);if(tile!=65535){using var image=new Bitmap(16,16);for(int y=0;y<16;y++)for(int x=0;x<16;x++)image.SetPixel(x,y,Color.FromArgb(unchecked((int)paletteMap.Palette[paletteMap.Pixels[tile*256+y*16+x]])));var interpolation=e.Graphics.InterpolationMode;e.Graphics.InterpolationMode=InterpolationMode.NearestNeighbor;e.Graphics.DrawImage(image,box);e.Graphics.InterpolationMode=interpolation;}
        TextRenderer.DrawText(e.Graphics,b.ToString(),Font,new Rectangle(e.Bounds.X+43,e.Bounds.Y+4,e.Bounds.Width-46,34),e.ForeColor,TextFormatFlags.VerticalCenter|TextFormatFlags.EndEllipsis);e.DrawFocusRectangle();
    }
    void PickTerrain(NativeTerrainMap map,int cell){
        if(paletteMap?.ResourceId!=map.ResourceId)return;ushort tile=map.RawTiles[cell],mask=map.Collisions[cell];
        var brush=terrainBrushes.Items.Cast<TerrainBrush>().FirstOrDefault(b=>b.Tile==tile && b.Collision==mask);if(brush==null){brush=new(tile,mask,terrainBrushes.Items.Count+1);terrainBrushes.Items.Add(brush);}terrainBrushes.SelectedItem=brush;terrainToolChoice.SelectedIndex=1;status.Text="Tuile copiée, avec sa collision. Le pinceau est prêt.";
    }
    void ApplyTerrainStroke(TerrainStroke stroke){
        if(current==null || paletteMap==null || paletteMap.ResourceId!=stroke.ResourceId || terrainBrushes.SelectedItem is not TerrainBrush brush)return;
        var map=canvas.TerrainMaps.Single(m=>m.ResourceId==stroke.ResourceId);var next=new Dictionary<int,(ushort,ushort)>();
        foreach(int cell in stroke.Cells){ushort tile=stroke.Tool==TerrainTool.Eraser?(ushort)65535:stroke.Tool==TerrainTool.Collision?map.RawTiles[cell]:brush.Tile;ushort mask=stroke.Tool==TerrainTool.Eraser?(ushort)0:stroke.Tool==TerrainTool.Collision || paintCollision.Checked?brush.Collision:map.Collisions[cell];if(tile!=map.RawTiles[cell] || mask!=map.Collisions[cell])next[cell]=(tile,mask);}
        if(next.Count==0)return;string before=CaptureEditState();if(!rom.SetTerrainCells(current,activeRoom,stroke.ResourceId,next))return;undo.Push(before);redo.Clear();dirty=true;LoadRoomTerrain(false);status.Text=$"{next.Count} tuile(s) modifiée(s). Annuler rétablit ce geste en une fois.";
    }
    void ResetRoomTerrain(){if(current==null)return;var room=current.AllRooms().Single(r=>r.SourceZone==activeRoom);if(room.Terrain.Count==0)return;Remember();room.Terrain.Clear();dirty=true;LoadRoomTerrain(false);status.Text="Terrain de cette salle rétabli. Cette action peut être annulée.";}
    void CheckTerrainPaintingIntegration(){
        sourceLevel.SelectedItem=sourceLevel.Items.Cast<LevelTemplate>().Single(t=>t.SourceZone==43);NewFromVanilla();var native=rom.ReadTerrain(43).Single();var otherBefore=JsonSerializer.Serialize(current!.Rooms,ProjectStore.Json);
        var brush=terrainBrushes.Items.Cast<TerrainBrush>().First(b=>b.Collision!=0 && BrushVisible(b));terrainBrushes.SelectedItem=brush;terrainToolChoice.SelectedIndex=1;canvas.FocusAt(160,320,2);
        var start=canvas.TerrainScreenPoint(136,200);var end=canvas.TerrainScreenPoint(168,200);canvas.CheckTerrainMouse(start,end);
        if(current.Terrain.Count!=3 || JsonSerializer.Serialize(current.Rooms,ProjectStore.Json)!=otherBefore)throw new Exception("Terrain stroke isolation failed");var edited=canvas.TerrainMaps.Single();int cell=native.CellIndex(8,12);if(edited.RawTiles[cell]!=brush.Tile || edited.Collisions[cell]!=brush.Collision)throw new Exception("Painted terrain not shown");
        Undo();if(current!.Terrain.Count!=0)throw new Exception("Whole stroke undo failed");Redo();if(current!.Terrain.Count!=3)throw new Exception("Whole stroke redo failed");
        canvas.FocusAt(160,320,2);canvas.CheckTerrainMouse(start,end);Undo();if(current!.Terrain.Count!=0)throw new Exception("No-op stroke polluted undo history");Redo();
        canvas.FocusAt(160,320,2);string beforeCancel=JsonSerializer.Serialize(current,ProjectStore.Json);canvas.CheckTerrainAbandon(canvas.TerrainScreenPoint(232,200),canvas.TerrainScreenPoint(248,200));if(JsonSerializer.Serialize(current,ProjectStore.Json)!=beforeCancel)throw new Exception("Abandoned gesture changed the level");
        terrainToolChoice.SelectedIndex=5;terrainBrushes.SelectedItem=terrainBrushes.Items.Cast<TerrainBrush>().First(b=>b.Collision==0);canvas.CheckTerrainMouse(start,start);if(canvas.TerrainMaps.Single().RawTiles[cell]!=brush.Tile || canvas.TerrainMaps.Single().Collisions[cell]!=0)throw new Exception("Collision-only edit changed appearance");Undo();
        terrainToolChoice.SelectedIndex=4;canvas.FocusAt(160,320,2);canvas.CheckTerrainMouse(start,start);if(canvas.TerrainMaps.Single().Collisions[cell]!=0 || canvas.TerrainMaps.Single().RawTiles[cell]!=65535)throw new Exception("Terrain erase failed");Undo();
        paintCollision.Checked=false;terrainToolChoice.SelectedIndex=1;var bare=terrainBrushes.Items.Cast<TerrainBrush>().First(b=>b.Tile!=brush.Tile);terrainBrushes.SelectedItem=bare;canvas.FocusAt(160,320,2);canvas.CheckTerrainMouse(start,start);if(canvas.TerrainMaps.Single().Collisions[cell]!=brush.Collision)throw new Exception("Appearance-only paint changed collision");Undo();paintCollision.Checked=true;
        terrainToolChoice.SelectedIndex=3;canvas.FocusAt(160,320,2);canvas.CheckTerrainMouse(start,start);if(terrainBrushes.SelectedItem is not TerrainBrush copied || copied.Tile!=brush.Tile || copied.Collision!=brush.Collision || canvas.TerrainTool!=TerrainTool.Pencil)throw new Exception("Terrain eyedropper failed");
        ResetRoomTerrain();if(current!.Terrain.Count!=0)throw new Exception("Reset failed");Undo();if(current!.Terrain.Count!=3)throw new Exception("Reset not undoable");SaveLevel();var id=current.Id;LoadLevel(store.Levels().Single(l=>l.Id==id));if(current!.Terrain.Count!=3 || rom.ExportPlacements(current,true)[7]!='4')throw new Exception("Painted terrain lost after reload");
        terrainToolChoice.SelectedIndex=2;canvas.FocusAt(240,320,2);start=canvas.TerrainScreenPoint(232,200);end=canvas.TerrainScreenPoint(248,216);int count=current.Terrain.Count;canvas.CheckTerrainMouse(start,end);if(current.Terrain.Count!=count+4)throw new Exception("Rectangle stroke failed");Undo();
        roomChoice.SelectedIndex=1;if(current.Terrain.Count!=3 || canvas.TerrainMaps.Single().ResourceId!=0x8068)throw new Exception("Painted room leaked into its neighbour");roomChoice.SelectedIndex=0;terrainToolChoice.SelectedIndex=1;canvas.FocusAt(160,320,1.5f);SaveLevel();if(terrainPaintingTab?.Parent is TabControl inspector)inspector.SelectedTab=terrainPaintingTab;
        Directory.CreateDirectory(Path.Combine(pcRoot,"build/editor-0160"));File.WriteAllText(Path.Combine(pcRoot,"build/editor-0160/ui-terrain-painting.json"),JsonSerializer.Serialize(new{gestureCancel=true,noOpDoesNotPolluteUndo=true,collisionOnly=true,pencil=true,rectangle=true,picker=true,erase=true,appearanceWithoutCollision=true,oneUndoPerGesture=true,redo=true,roomIsolation=true,resetUndoable=true,persistence=true,v4Export=true},ProjectStore.Json));
    }
}
