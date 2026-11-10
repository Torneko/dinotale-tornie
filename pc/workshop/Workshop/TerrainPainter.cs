using System.ComponentModel;
using System.Drawing.Drawing2D;
using YoshiWorkshop;
namespace WorkshopApp;
public enum TerrainTool {Objects,Pencil,Rectangle,Picker,Eraser,Collision,SectionSelect,SectionStamp}
public sealed record TerrainStroke(ushort ResourceId,IReadOnlyList<int> Cells,TerrainTool Tool);
public sealed partial class LevelCanvas {
    TerrainTool terrainTool;NativeTerrainMap? strokeMap;Point strokeStart,lastTile;bool strokeOverflow;TerrainTool strokeTool;
    readonly Dictionary<int,Point> strokeCells=[];
    [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)] public TerrainTool TerrainTool {get=>terrainTool;set{CancelObjectGesture();CancelTerrainGesture();terrainTool=value;Cursor=value==TerrainTool.Objects?Cursors.Default:Cursors.Cross;Invalidate();}}
    public event Action<TerrainStroke>? TerrainCommitted;
    public event Action<NativeTerrainMap,int>? TerrainPicked;
    public event Action<string>? TerrainMessage;
    Point? terrainHover;
    Point TilePoint(NativeTerrainMap map,Point location)=>new((int)MathF.Floor(((location.X-panX)/zoom-map.WorldX)/16),(int)MathF.Floor(((location.Y-panY)/zoom-map.WorldY)/16));
    bool HitTerrain(Point location,out NativeTerrainMap? map,out Point point){
        foreach(var item in terrainImages.AsEnumerable().Reverse()){var p=TilePoint(item.Map,location);if(item.Map.TryCell(p.X,p.Y,out _)){map=item.Map;point=p;return true;}}
        map=null;point=default;return false;
    }
    public bool CancelTerrainGesture(){bool active=strokeMap!=null || sectionStampPressed;sectionStampPressed=false;strokeMap=null;strokeCells.Clear();strokeOverflow=false;if(active)Capture=false;Invalidate();return active;}
    bool BeginTerrain(MouseEventArgs e){
        if(terrainTool==TerrainTool.Objects || e.Button!=MouseButtons.Left)return false;
        if(!HitTerrain(e.Location,out var map,out var point)){TerrainMessage?.Invoke("Cette zone n’a pas de tuiles éditables. Les dimensions restent celles du niveau d’origine.");return true;}
        if(terrainTool==TerrainTool.SectionStamp){sectionStampPressed=true;Capture=true;terrainHover=e.Location;Invalidate();return true;}
        if(terrainTool==TerrainTool.Picker){TerrainPicked?.Invoke(map!,map!.CellIndex(point.X,point.Y));return true;}
        strokeMap=map;strokeStart=lastTile=point;strokeTool=terrainTool;strokeOverflow=false;strokeCells.Clear();AddStrokeCell(point);Capture=true;Invalidate();return true;
    }
    void AddStrokeCell(Point p){
        if(strokeMap==null || !strokeMap.TryCell(p.X,p.Y,out int cell))return;
        if(strokeCells.Count>=4096 && !strokeCells.ContainsKey(cell)){strokeOverflow=true;return;}strokeCells.TryAdd(cell,p);
    }
    bool MoveTerrain(MouseEventArgs e){
        terrainHover=e.Location;if(strokeMap==null){if(terrainTool!=TerrainTool.Objects)Invalidate();return false;}
        var point=TilePoint(strokeMap,e.Location);point.X=Math.Clamp(point.X,0,strokeMap.Width/16-1);point.Y=Math.Clamp(point.Y,0,strokeMap.Height/16-1);
        if(strokeTool is TerrainTool.Rectangle or TerrainTool.SectionSelect){strokeCells.Clear();strokeOverflow=false;int x0=Math.Min(point.X,strokeStart.X),x1=Math.Max(point.X,strokeStart.X),y0=Math.Min(point.Y,strokeStart.Y),y1=Math.Max(point.Y,strokeStart.Y);if((long)(x1-x0+1)*(y1-y0+1)>4096)strokeOverflow=true;else for(int y=y0;y<=y1;y++)for(int x=x0;x<=x1;x++)AddStrokeCell(new(x,y));}
        else {int x=lastTile.X,y=lastTile.Y,dx=Math.Abs(point.X-x),dy=-Math.Abs(point.Y-y),sx=x<point.X?1:-1,sy=y<point.Y?1:-1,error=dx+dy;while(true){AddStrokeCell(new(x,y));if(x==point.X && y==point.Y)break;int twice=2*error;if(twice>=dy){error+=dy;x+=sx;}if(twice<=dx){error+=dx;y+=sy;}}}
        lastTile=point;Invalidate();return true;
    }
    bool EndTerrain(MouseEventArgs e){
        if(e.Button!=MouseButtons.Left)return false;
        if(sectionStampPressed){sectionStampPressed=false;Capture=false;if(HitTerrain(e.Location,out var target,out var point))TerrainSectionPlaced?.Invoke(target!,point);else TerrainMessage?.Invoke("La copie doit tenir entièrement dans le terrain de la salle.");Invalidate();return true;}
        if(strokeMap==null)return false;
        if(strokeTool==TerrainTool.SectionSelect){FinishTerrainSectionSelection();return true;}
        var action=new TerrainStroke(strokeMap.ResourceId,strokeCells.Keys.ToArray(),strokeTool);bool overflow=strokeOverflow;CancelTerrainGesture();
        if(overflow)TerrainMessage?.Invoke("Geste trop grand : dessiner par zones de 4096 tuiles maximum. Aucune modification appliquée.");else if(action.Cells.Count>0)TerrainCommitted?.Invoke(action);return true;
    }
    void DrawTerrainPointer(Graphics g){
        DrawTerrainSections(g);
        if(terrainTool is TerrainTool.Objects or TerrainTool.SectionStamp)return;
        if(strokeMap!=null){using var fill=new SolidBrush(Color.FromArgb(95,strokeTool==TerrainTool.Eraser?Color.IndianRed:Color.Teal));using var line=new Pen(Color.White,1);foreach(var p in strokeCells.Values){var box=new RectangleF(panX+(strokeMap.WorldX+p.X*16)*zoom,panY+(strokeMap.WorldY+p.Y*16)*zoom,16*zoom,16*zoom);g.FillRectangle(fill,box);g.DrawRectangle(line,box.X,box.Y,box.Width,box.Height);}}
        else if(terrainHover is Point location && HitTerrain(location,out var map,out var p)){using var dark=new Pen(Color.DarkSlateGray,3);using var light=new Pen(Color.White,1);var box=new RectangleF(panX+(map!.WorldX+p.X*16)*zoom,panY+(map.WorldY+p.Y*16)*zoom,16*zoom,16*zoom);g.DrawRectangle(dark,box.X,box.Y,box.Width,box.Height);g.DrawRectangle(light,box.X,box.Y,box.Width,box.Height);}
    }
    protected override bool ProcessCmdKey(ref Message msg,Keys keyData){if(keyData==Keys.Escape && (CancelTerrainStamp() || CancelObjectGesture() || CancelTerrainGesture()))return true;return base.ProcessCmdKey(ref msg,keyData);}
    protected override void OnMouseCaptureChanged(EventArgs e){base.OnMouseCaptureChanged(e);if(!Capture){if(strokeMap!=null || sectionStampPressed)CancelTerrainGesture();CancelObjectGesture();panning=false;}}
    protected override void OnMouseLeave(EventArgs e){base.OnMouseLeave(e);terrainHover=null;Invalidate();}
    public void CheckTerrainAbandon(Point from,Point to){OnMouseDown(new(MouseButtons.Left,1,from.X,from.Y,0));OnMouseMove(new(MouseButtons.Left,0,to.X,to.Y,0));CancelTerrainGesture();OnMouseUp(new(MouseButtons.Left,1,to.X,to.Y,0));}
    // Used by isolated integration checks through the same mouse handlers as play.
    public void CheckTerrainMouse(Point from,Point to){OnMouseDown(new(MouseButtons.Left,1,from.X,from.Y,0));OnMouseMove(new(MouseButtons.Left,0,to.X,to.Y,0));OnMouseUp(new(MouseButtons.Left,1,to.X,to.Y,0));}
    public Point TerrainScreenPoint(float worldX,float worldY)=>new((int)(panX+worldX*zoom),(int)(panY+worldY*zoom));
}
