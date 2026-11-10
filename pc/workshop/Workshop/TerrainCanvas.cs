using System.ComponentModel;
using System.Drawing.Drawing2D;
using System.Drawing.Imaging;
using System.Runtime.InteropServices;
using YoshiWorkshop;
namespace WorkshopApp;
public sealed partial class LevelCanvas {
    readonly List<(NativeTerrainMap Map,Bitmap Image)> terrainImages=[];
    [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)] public bool ShowTerrain {get;set;}=true;
    [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)] public bool ShowCollisions {get;set;}
    public IReadOnlyList<NativeTerrainMap> TerrainMaps=>terrainImages.Select(t=>t.Map).ToList();
    public void SetTerrain(IReadOnlyList<NativeTerrainMap> maps){
        CancelTerrainGesture();ClearTerrainSectionSelection();foreach(var old in terrainImages)old.Image.Dispose();terrainImages.Clear();
        try{foreach(var map in maps){var bitmap=new Bitmap(map.Width,map.Height,PixelFormat.Format32bppArgb);var locked=bitmap.LockBits(new(0,0,map.Width,map.Height),ImageLockMode.WriteOnly,PixelFormat.Format32bppArgb);try{int[] pixels=map.Rasterize();for(int y=0;y<map.Height;y++)Marshal.Copy(pixels,y*map.Width,locked.Scan0+y*locked.Stride,map.Width);}finally{bitmap.UnlockBits(locked);}terrainImages.Add((map,bitmap));}}
        catch{CancelTerrainGesture();foreach(var old in terrainImages)old.Image.Dispose();terrainImages.Clear();throw;}Invalidate();
    }
    void DrawTerrain(Graphics g,int width,int height){
        if(ShowTerrain){var old=g.InterpolationMode;var offset=g.PixelOffsetMode;g.InterpolationMode=InterpolationMode.NearestNeighbor;g.PixelOffsetMode=PixelOffsetMode.Half;foreach(var terrain in terrainImages){var m=terrain.Map;g.DrawImage(terrain.Image,new RectangleF(panX+m.WorldX*zoom,panY+m.WorldY*zoom,m.Width*zoom,m.Height*zoom));}g.InterpolationMode=old;g.PixelOffsetMode=offset;}
        if(ShowCollisions){using var fill=new SolidBrush(Color.FromArgb(65,12,190,180));using var line=new Pen(Color.FromArgb(180,0,95,90));foreach(var terrain in terrainImages){var m=terrain.Map;int left=Math.Clamp((int)MathF.Floor((-panX/zoom-m.WorldX)/16),0,m.Width/16),right=Math.Clamp((int)MathF.Ceiling(((width-panX)/zoom-m.WorldX)/16),0,m.Width/16),top=Math.Clamp((int)MathF.Floor((-panY/zoom-m.WorldY)/16),0,m.Height/16),bottom=Math.Clamp((int)MathF.Ceiling(((height-panY)/zoom-m.WorldY)/16),0,m.Height/16);for(int y=top;y<bottom;y++)for(int x=left;x<right;x++){int cell=m.CellIndex(x,y);if(cell<0 || m.Collisions[cell]==0)continue;var box=new RectangleF(panX+(m.WorldX+x*16)*zoom,panY+(m.WorldY+y*16)*zoom,16*zoom,16*zoom);g.FillRectangle(fill,box);if(zoom>=.4f)g.DrawRectangle(line,box.X,box.Y,box.Width,box.Height);}}}
    }
    public void FocusAt(float x,float y,float scale=1){zoom=Math.Clamp(scale,.025f,4);panX=Width/2f-x*zoom;panY=Height/2f-y*zoom;Invalidate();ViewChanged?.Invoke();}
    protected override void Dispose(bool disposing){if(disposing){terrainSectionImage?.Dispose();foreach(var t in terrainImages)t.Image.Dispose();terrainImages.Clear();}base.Dispose(disposing);}
}
