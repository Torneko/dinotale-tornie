using System.ComponentModel;
using System.Drawing.Drawing2D;
using System.Drawing.Imaging;
using YoshiWorkshop;
namespace WorkshopApp;

public sealed record TerrainRegion(ushort ResourceId,int X,int Y,int Width,int Height);
public sealed partial class LevelCanvas {
    TerrainRegion? terrainRegion;
    TerrainSection? terrainSectionClipboard;
    Bitmap? terrainSectionImage;
    bool sectionStampPressed;
    public event Action? TerrainSectionSelectionChanged,TerrainStampCancelled;
    public event Action<NativeTerrainMap,Point>? TerrainSectionPlaced;
    [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)] public int TerrainRoom {get;set;}
    [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)] public bool SectionApplyCollision {get;set;}=true;
    public TerrainRegion? SelectedTerrainRegion=>terrainRegion;
    public void ClearTerrainSectionSelection(){if(terrainRegion==null)return;terrainRegion=null;TerrainSectionSelectionChanged?.Invoke();Invalidate();}
    void FinishTerrainSectionSelection(){
        var map=strokeMap!;
        int x=Math.Min(strokeStart.X,lastTile.X),y=Math.Min(strokeStart.Y,lastTile.Y),w=Math.Abs(lastTile.X-strokeStart.X)+1,h=Math.Abs(lastTile.Y-strokeStart.Y)+1;
        bool valid=!strokeOverflow && (long)w*h<=TerrainSections.MaxCells;
        for(int dy=0;valid && dy<h;dy++)for(int dx=0;dx<w;dx++)if(!map.TryCell(x+dx,y+dy,out _)){valid=false;break;}
        CancelTerrainGesture();
        if(!valid){TerrainMessage?.Invoke("Choisir une zone de terrain complète, de 4096 tuiles maximum.");return;}
        terrainRegion=new(map.ResourceId,x,y,w,h);TerrainSectionSelectionChanged?.Invoke();Invalidate();
    }
    public void SetTerrainSectionClipboard(TerrainSection section,NativeTerrainMap map){
        var image=new Bitmap(section.Width*16,section.Height*16,PixelFormat.Format32bppArgb);
        for(int y=0;y<section.Height;y++)for(int x=0;x<section.Width;x++){
            ushort tile=map.Resolve(section.Cells[y*section.Width+x].Tile);if(tile==65535)continue;
            for(int py=0;py<16;py++)for(int px=0;px<16;px++)image.SetPixel(x*16+px,y*16+py,Color.FromArgb(unchecked((int)map.Palette[map.Pixels[tile*256+py*16+px]])));
        }
        terrainSectionImage?.Dispose();terrainSectionImage=image;terrainSectionClipboard=section;Invalidate();
    }
    bool CancelTerrainStamp(){if(terrainTool!=TerrainTool.SectionStamp)return false;CancelTerrainGesture();terrainHover=null;sectionStampPressed=false;TerrainStampCancelled?.Invoke();Invalidate();return true;}
    void DrawTerrainSections(Graphics g){
        if(terrainRegion is TerrainRegion region && terrainTool==TerrainTool.SectionSelect){
            var map=TerrainMaps.FirstOrDefault(m=>m.ResourceId==region.ResourceId);if(map!=null){var box=SectionBox(map,region.X,region.Y,region.Width,region.Height);using var fill=new SolidBrush(Color.FromArgb(35,12,135,125));using var line=new Pen(Color.Teal,2){DashStyle=DashStyle.Dash};g.FillRectangle(fill,box);g.DrawRectangle(line,box.X,box.Y,box.Width,box.Height);}
        }
        if(terrainTool!=TerrainTool.SectionStamp || terrainSectionClipboard is not TerrainSection section || terrainSectionImage==null || terrainHover is not Point pointer)return;
        if(!HitTerrain(pointer,out var target,out var point))return;
        bool valid=true;try{TerrainSections.PreparePaste(section,target!,TerrainRoom,point.X,point.Y,SectionApplyCollision);}catch(InvalidDataException){valid=false;}
        var area=SectionBox(target!,point.X,point.Y,section.Width,section.Height);
        using var attrs=new ImageAttributes();attrs.SetColorMatrix(new ColorMatrix{Matrix33=.62f});
        var old=g.InterpolationMode;g.InterpolationMode=InterpolationMode.NearestNeighbor;
        g.DrawImage(terrainSectionImage,Rectangle.Round(area),0,0,terrainSectionImage.Width,terrainSectionImage.Height,GraphicsUnit.Pixel,attrs);g.InterpolationMode=old;
        using var border=new Pen(valid?Color.Teal:Color.IndianRed,2);g.DrawRectangle(border,area.X,area.Y,area.Width,area.Height);
    }
    RectangleF SectionBox(NativeTerrainMap map,int x,int y,int w,int h)=>new(panX+(map.WorldX+x*16)*zoom,panY+(map.WorldY+y*16)*zoom,w*16*zoom,h*16*zoom);
    public void CheckTerrainSectionEscape(){var message=new Message();ProcessCmdKey(ref message,Keys.Escape);}
    public void CheckTerrainSectionHeldEscape(Point point){OnMouseDown(new(MouseButtons.Left,1,point.X,point.Y,0));CheckTerrainSectionEscape();OnMouseUp(new(MouseButtons.Left,1,point.X,point.Y,0));}
    public void CheckTerrainSectionHover(Point point)=>OnMouseMove(new(MouseButtons.None,0,point.X,point.Y,0));
    public void CheckTerrainSectionCaptureLoss(Point from,Point to){OnMouseDown(new(MouseButtons.Left,1,from.X,from.Y,0));OnMouseMove(new(MouseButtons.Left,0,to.X,to.Y,0));Capture=false;OnMouseUp(new(MouseButtons.Left,1,to.X,to.Y,0));}
}
