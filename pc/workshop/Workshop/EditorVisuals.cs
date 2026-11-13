using System.Drawing.Drawing2D;

using System.ComponentModel;

using System.Drawing.Imaging;

using YoshiWorkshop;

namespace WorkshopApp;



public static class EditorVisuals {

    public static string AssetRoot {get;set;}="";

    static readonly Dictionary<string,Image> images=[];

    static readonly string[] fruits=["peche","mure","bleuet","poire-verte","prune","kiwi"];

    public static readonly Color[] Colors=[Color.FromArgb(239,139,48),Color.FromArgb(146,66,192),Color.FromArgb(33,188,185),Color.FromArgb(174,205,87),Color.FromArgb(135,85,47),Color.FromArgb(37,117,67)];

    public static void Draw(Graphics g,RectangleF box,Placement p){

        if(ObjectCatalog.IsSpecialFruit(p.ObjectId) || ObjectCatalog.IsSpecialBubble(p.ObjectId)){

            string path=Path.Combine(AssetRoot,fruits[p.ObjectId-(ObjectCatalog.IsSpecialBubble(p.ObjectId)?0x4F20:0x4F00)]+"-24x21.png");

            if(File.Exists(path)){

                if(!images.TryGetValue(path,out var img)){using var source=Image.FromFile(path);images[path]=img=new Bitmap(source);}

                var old=g.InterpolationMode;g.InterpolationMode=InterpolationMode.NearestNeighbor;bool bubbled=ObjectCatalog.IsSpecialBubble(p.ObjectId);var inside=box;if(bubbled)inside.Inflate(-box.Width*.12f,-box.Height*.12f);g.DrawImage(img,inside);g.InterpolationMode=old;
                if(bubbled){using var ring=new Pen(Color.DeepSkyBlue,Math.Max(1,box.Width/20));g.DrawEllipse(ring,box);g.DrawArc(Pens.White,box.X+box.Width*.12f,box.Y+box.Height*.08f,box.Width*.65f,box.Height*.7f,205,65);}return;

            }

        }

        float x=box.X,y=box.Y,w=box.Width,h=box.Height;

        using var outline=new Pen(Color.FromArgb(70,64,49),Math.Max(1,w/24));

        if(SpecialEditorObjects.IsEgg(p.ObjectId)){
            int eggColor=SpecialEditorObjects.Color(p.ObjectId);
            Color spotColor=eggColor switch {
                6 or 17=>Color.LightGray,
                7 or 16=>Color.FromArgb(40,35,50),
                14=>Color.FromArgb(223,185,75),
                15=>Color.FromArgb(189,200,214),
                _=>Colors[eggColor-8]
            };
            using var spot=new SolidBrush(spotColor);
            g.FillEllipse(Brushes.WhiteSmoke,x+w*.18f,y+h*.1f,w*.64f,h*.82f);g.DrawEllipse(outline,x+w*.18f,y+h*.1f,w*.64f,h*.82f);
            g.FillEllipse(spot,x+w*.3f,y+h*.2f,w*.2f,h*.23f);g.FillEllipse(spot,x+w*.53f,y+h*.45f,w*.2f,h*.24f);g.FillEllipse(spot,x+w*.27f,y+h*.62f,w*.18f,h*.19f);
            using var bubble=new Pen(Color.DeepSkyBlue,Math.Max(1,w/25));g.DrawEllipse(bubble,box);return;
        }
        if(ObjectCatalog.IsMaskass(p.ObjectId) || ObjectCatalog.IsSpecialMaskass(p.ObjectId) || p.ObjectId is 0x45E3 or 0x45E5 or 0x45B0 or 0x46AA or 0x46AE){

            int c=p.HybridVariant??(ObjectCatalog.IsSpecialMaskass(p.ObjectId)?p.ObjectId-0x4F10:-1);using var body=new SolidBrush(p.ObjectId is 0x45E3 or 0x45E5 or 0x45B0?Color.FromArgb(35,35,45):p.ObjectId is 0x46AA or 0x46AE?Color.WhiteSmoke:c>=0?Colors[c]:Color.FromArgb(217,77,77));

            g.FillEllipse(body,x+w*.12f,y+h*.2f,w*.76f,h*.7f);g.DrawEllipse(outline,x+w*.12f,y+h*.2f,w*.76f,h*.7f);

            g.FillEllipse(Brushes.WhiteSmoke,x+w*.22f,y+h*.1f,w*.56f,h*.61f);g.DrawEllipse(outline,x+w*.22f,y+h*.1f,w*.56f,h*.61f);

            g.FillEllipse(Brushes.Black,x+w*.34f,y+h*.27f,w*.08f,h*.16f);g.FillEllipse(Brushes.Black,x+w*.58f,y+h*.27f,w*.08f,h*.16f);g.FillEllipse(Brushes.Black,x+w*.45f,y+h*.5f,w*.1f,h*.12f);

            g.FillEllipse(Brushes.SlateBlue,x+w*.1f,y+h*.78f,w*.35f,h*.17f);g.FillEllipse(Brushes.SlateBlue,x+w*.55f,y+h*.78f,w*.35f,h*.17f);
            if(p.ObjectId is 0x45E3 or 0x45E5 or 0x45B0){g.DrawLine(outline,x+w*.25f,y+h*.06f,x+w*.75f,y+h*.06f);g.DrawLine(outline,x+w*.5f,y,x+w*.5f,y+h*.15f);}
            if(p.ObjectId==0x45E5){g.FillRectangle(Brushes.Gray,x+w*.7f,y+h*.6f,w*.3f,h*.3f);g.DrawLine(outline,x+w*.75f,y+h*.55f,x+w*.8f,y+h*.65f);g.DrawLine(outline,x+w*.95f,y+h*.55f,x+w*.9f,y+h*.65f);}
            if(p.ObjectId==0x45B0){g.FillEllipse(Brushes.Black,x+w*.7f,y+h*.6f,w*.3f,h*.3f);g.DrawLine(Pens.OrangeRed,x+w*.85f,y+h*.6f,x+w*.95f,y+h*.5f);}return;

        }

        if(p.ObjectId==0x4001){

            g.FillEllipse(Brushes.YellowGreen,x+w*.12f,y+h*.25f,w*.65f,h*.6f);g.FillEllipse(Brushes.WhiteSmoke,x+w*.35f,y+h*.42f,w*.35f,h*.35f);g.FillEllipse(Brushes.ForestGreen,x+w*.35f,y+h*.05f,w*.6f,h*.45f);g.FillEllipse(Brushes.White,x+w*.58f,y+h*.04f,w*.13f,h*.22f);g.FillEllipse(Brushes.Black,x+w*.64f,y+h*.06f,w*.05f,h*.1f);g.FillEllipse(Brushes.OrangeRed,x+w*.28f,y+h*.78f,w*.44f,h*.17f);return;

        }

        if(p.ObjectId==0x406F){g.FillEllipse(Brushes.Goldenrod,box);g.DrawEllipse(outline,box);using var inner=new Pen(Color.FromArgb(255,238,143),Math.Max(1,w/9));g.DrawEllipse(inner,x+w*.24f,y+h*.13f,w*.52f,h*.74f);return;}

        if(p.ObjectId is >=0x4030 and <=0x4034 || p.ObjectId is >=0x41D1 and <=0x41D5){

            int kind=p.ObjectId>=0x41D1?p.ObjectId-0x41D1:p.ObjectId-0x4030;Color[] classic=[Color.FromArgb(230,65,54),Color.Gold,Color.ForestGreen,Color.MediumPurple,Color.FromArgb(153,205,147)];using var fruit=new SolidBrush(classic[kind]);

            g.FillEllipse(fruit,x+w*.1f,y+h*.2f,w*.8f,h*.75f);g.DrawEllipse(outline,x+w*.1f,y+h*.2f,w*.8f,h*.75f);g.FillEllipse(Brushes.SeaGreen,x+w*.5f,y+h*.04f,w*.35f,h*.2f);if(p.ObjectId>=0x41D1){using var bubble=new Pen(Color.LightSkyBlue,2);g.DrawEllipse(bubble,box);}return;

        }

        Color color=p.ObjectId>=0x8000?Color.FromArgb(162,138,112):ObjectCatalog.Get(p.ObjectId).Identified?Color.FromArgb(93,153,180):Color.FromArgb(172,151,125);

        using var brush=new SolidBrush(color);g.FillRectangle(brush,box);g.DrawRectangle(outline,x,y,w,h);

        string text=p.ObjectId==0x4086?"E":p.ObjectId==0x4208?"P":p.ObjectId>=0x8000?"D":"?";

        using var font=new Font("Segoe UI",Math.Max(7,w*.36f),FontStyle.Bold,GraphicsUnit.Pixel);var format=new StringFormat{Alignment=StringAlignment.Center,LineAlignment=StringAlignment.Center};g.DrawString(text,font,Brushes.White,box,format);

    }

    public static void Clear(){foreach(var image in images.Values)image.Dispose();images.Clear();}

}



public sealed partial class LevelCanvas:Control {

    LevelProject? level;Placement? selected;

    [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]

    public LevelProject? Level {get=>level;set{CancelObjectGesture();CancelTerrainGesture();ClearTerrainSectionSelection();level=value;selectionItems.Clear();selected=null;Invalidate();}}

    [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]

    public Placement? Selected {get=>selected;set=>SetObjectSelection(value==null?[]:[value],value,false);}

    [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]

    public bool ShowGrid {get;set;}=true;

    [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]

    public bool ShowTechnical {get;set;}

    [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]

    public int SnapSize {get;set;}

    public event Action? SelectionChanged,BeforeEdit,Edited,ViewChanged;

    float zoom=.2f,panX=40,panY=40;

    bool panning;Point last;

    public PointF WorldCenter=>new((Width/2f-panX)/zoom,(Height/2f-panY)/zoom);

    [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
    public string? SceneCaption {get;set;}

    public string ZoomText=>$"{zoom*100:0}%";

    public LevelCanvas(){DoubleBuffered=true;BackColor=Color.FromArgb(250,244,222);SetStyle(ControlStyles.Selectable,true);TabStop=true;}

    IEnumerable<Placement> VisibleObjects=>level?.Objects.Where(p=>ShowTechnical || p.ObjectId<0x8000)??[];

    public void Fit(){var list=VisibleObjects.ToList();if(list.Count==0)return;float minX=list.Min(p=>p.X),maxX=list.Max(p=>p.X),minY=list.Min(p=>p.Y),maxY=list.Max(p=>p.Y);if(ShowTerrain || ShowCollisions)foreach(var item in terrainImages){var map=item.Map;minX=Math.Min(minX,map.WorldX);maxX=Math.Max(maxX,map.WorldX+map.Width);minY=Math.Min(minY,map.WorldY);maxY=Math.Max(maxY,map.WorldY+map.Height);}zoom=Math.Clamp(Math.Min(Math.Max(100,Width-100)/Math.Max(maxX-minX,200),Math.Max(100,Height-100)/Math.Max(maxY-minY,200)),.025f,2f);panX=(Width-(maxX-minX)*zoom)/2-minX*zoom;panY=(Height-(maxY-minY)*zoom)/2-minY*zoom;Invalidate();ViewChanged?.Invoke();}

    public void CenterOn(Placement p){panX=Width/2f-p.X*zoom;panY=Height/2f-p.Y*zoom;Invalidate();}

    public void Nudge(int dx,int dy)=>MoveObjectSelection(dx,dy);

    protected override void OnPaint(PaintEventArgs e){base.OnPaint(e);Draw(e.Graphics,Width,Height);}

    void Draw(Graphics g,int width,int height){

        g.Clear(BackColor);DrawTerrain(g,width,height);g.SmoothingMode=SmoothingMode.AntiAlias;

        if(ShowGrid){float step=64*zoom;while(step<12)step*=4;using var grid=new Pen(Color.FromArgb(ShowTerrain?60:255,225,214,184));for(float a=panX%step;a<width;a+=step)g.DrawLine(grid,a,0,a,height);for(float b=panY%step;b<height;b+=step)g.DrawLine(grid,0,b,width,b);}

        if(level==null){using var title=new Font("Segoe UI",18,FontStyle.Bold);g.DrawString("Choisis un original, puis crée ta copie.",title,Brushes.DarkSlateGray,28,40);g.DrawString("Le décor et les collisions sont conservés.\nAjoute ensuite tes fruits, ennemis et objets.",Font,Brushes.DarkSlateGray,28,88);return;}

        foreach(var p in VisibleObjects){var at=ObjectDisplayPosition(p);float x=panX+at.X*zoom,y=panY+at.Y*zoom;if(x< -40 || y< -40 || x>width+40 || y>height+40)continue;float size=p.ObjectId>=0x8000?12:Math.Clamp(28*zoom,17,40);var box=new RectangleF(x-size/2,y-size/2,size,size);EditorVisuals.Draw(g,box,p);

            if(selectionItems.Contains(p)){using var selectedPen=new Pen(Color.FromArgb(35,121,116),2);g.DrawRectangle(selectedPen,box.X-5,box.Y-5,box.Width+10,box.Height+10);if(p!=Selected || selectionItems.Count>1)continue;string name=ObjectCatalog.Get(ObjectCatalog.PaletteId(p)).Name;SizeF label=g.MeasureString(name,Font);var area=new RectangleF(x+size/2+9,y-13,label.Width+12,26);g.FillRectangle(Brushes.Ivory,area);g.DrawString(name,Font,Brushes.DarkSlateGray,area.X+6,area.Y+4);}

        }

        DrawObjectSelectionOverlay(g);
        DrawTerrainPointer(g);
        using var textBack=new SolidBrush(Color.FromArgb(230,255,253,242));g.FillRectangle(textBack,0,height-31,width,31);g.DrawString($"{SceneCaption??("Source "+VanillaRom.SourceCode(level.SourceZone))}  ·  {level.Objects.Count} objets  ·  {selectionItems.Count} sélectionné(s)  ·  {ZoomText}  ·  Bas = Y positif",Font,Brushes.DarkSlateGray,12,height-24);

    }

    protected override void OnMouseDown(MouseEventArgs e){base.OnMouseDown(e);Focus();last=e.Location;CancelObjectGesture();if(BeginTerrain(e))return;if(e.Button==MouseButtons.Right){panning=true;Capture=true;return;}if(e.Button==MouseButtons.Left)BeginObjectGesture(e.Location);}
    protected override void OnMouseMove(MouseEventArgs e){base.OnMouseMove(e);if(MoveTerrain(e))return;if(panning){panX+=e.X-last.X;panY+=e.Y-last.Y;last=e.Location;Invalidate();ViewChanged?.Invoke();}else MoveObjectGesture(e.Location);}
    protected override void OnMouseUp(MouseEventArgs e){base.OnMouseUp(e);if(EndTerrain(e))return;if(e.Button==MouseButtons.Left)EndObjectGesture(e.Location);if(panning){panning=false;Capture=false;}}

    protected override void OnMouseWheel(MouseEventArgs e){base.OnMouseWheel(e);float wx=(e.X-panX)/zoom,wy=(e.Y-panY)/zoom;zoom=Math.Clamp(zoom*(e.Delta>0?1.2f:1/1.2f),.025f,4);panX=e.X-wx*zoom;panY=e.Y-wy*zoom;Invalidate();ViewChanged?.Invoke();}

    public Bitmap Snapshot(){using var bitmap=new Bitmap(Math.Max(Width,1),Math.Max(Height,1));using(var g=Graphics.FromImage(bitmap))Draw(g,bitmap.Width,bitmap.Height);var small=new Bitmap(320,180);using var target=Graphics.FromImage(small);target.InterpolationMode=InterpolationMode.HighQualityBicubic;target.DrawImage(bitmap,0,0,320,180);return small;}

}

