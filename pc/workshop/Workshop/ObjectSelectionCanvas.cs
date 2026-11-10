using System.ComponentModel;
using System.Drawing.Drawing2D;
using YoshiWorkshop;
namespace WorkshopApp;
public sealed partial class LevelCanvas {
    readonly List<Placement> selectionItems=[];
    bool objectDragging,objectMarquee,marqueeAdditive;
    Point objectStart,objectPointer;PointF objectDelta;
    Placement[] selectionBeforeMarquee=[];
    Keys? checkObjectModifiers;
    Keys ObjectModifiers=>checkObjectModifiers??ModifierKeys;
    [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)] public IReadOnlyList<Placement> SelectedObjects=>selectionItems;
    public void SetObjectSelection(IEnumerable<Placement> objects,Placement? primary=null,bool notify=true){
        var list=objects.Where(p=>level?.Objects.Contains(p)==true).Distinct().ToArray();
        bool different=!list.SequenceEqual(selectionItems) || selected!=(primary!=null && list.Contains(primary)?primary:list.FirstOrDefault());
        selectionItems.Clear();selectionItems.AddRange(list);selected=primary!=null && list.Contains(primary)?primary:list.FirstOrDefault();Invalidate();if(notify && different)SelectionChanged?.Invoke();
    }
    public void SelectAllObjects(){CancelObjectGesture();SetObjectSelection(VisibleObjects.Where(ObjectGroups.Editable));}
    public bool MoveObjectSelection(float dx,float dy){
        if(selectionItems.Count==0 || selectionItems.Any(p=>!ObjectGroups.Editable(p)))return false;
        var delta=ObjectGroups.ClampOffset(selectionItems,dx,dy);if(delta==(0f,0f))return false;
        BeforeEdit?.Invoke();ObjectGroups.Translate(selectionItems,delta.X,delta.Y);Edited?.Invoke();Invalidate();return true;
    }
    PointF ObjectDisplayPosition(Placement p)=>objectDragging && selectionItems.Contains(p)?new(p.X+objectDelta.X,p.Y+objectDelta.Y):new(p.X,p.Y);
    void BeginObjectGesture(Point point){
        objectStart=objectPointer=point;objectDelta=PointF.Empty;
        var hit=VisibleObjects.Where(p=>Math.Abs(panX+p.X*zoom-point.X)<15 && Math.Abs(panY+p.Y*zoom-point.Y)<15).OrderBy(p=>Math.Abs(panX+p.X*zoom-point.X)+Math.Abs(panY+p.Y*zoom-point.Y)).FirstOrDefault();
        bool add=(ObjectModifiers&(Keys.Shift|Keys.Control))!=0;
        if(hit==null){selectionBeforeMarquee=selectionItems.ToArray();marqueeAdditive=add;objectMarquee=true;Capture=true;if(!add)SetObjectSelection([]);return;}
        if(add){if(!ObjectGroups.Editable(hit)){TerrainMessage?.Invoke("Le départ et le décor restent protégés.");return;}var list=selectionItems.Where(ObjectGroups.Editable).ToList();if(!list.Remove(hit))list.Add(hit);SetObjectSelection(list,list.Contains(hit)?hit:null);return;}
        if(selectionItems.Contains(hit))SetObjectSelection(selectionItems.ToArray(),hit);else SetObjectSelection([hit],hit);
        if(selectionItems.Any(p=>!ObjectGroups.Editable(p)))return;objectDragging=true;Capture=true;
    }
    Rectangle SelectionRectangle=>Rectangle.FromLTRB(Math.Min(objectStart.X,objectPointer.X),Math.Min(objectStart.Y,objectPointer.Y),Math.Max(objectStart.X,objectPointer.X),Math.Max(objectStart.Y,objectPointer.Y));
    void MoveObjectGesture(Point point){
        objectPointer=point;
        if(objectDragging && selected!=null){
            if(Math.Abs(point.X-objectStart.X)<=2 && Math.Abs(point.Y-objectStart.Y)<=2){objectDelta=PointF.Empty;Invalidate();return;}
            float dx=(point.X-objectStart.X)/zoom,dy=(point.Y-objectStart.Y)/zoom;
            if(SnapSize>0 && !ObjectModifiers.HasFlag(Keys.Alt)){dx=MathF.Round((selected.X+dx)/SnapSize)*SnapSize-selected.X;dy=MathF.Round((selected.Y+dy)/SnapSize)*SnapSize-selected.Y;}
            var delta=ObjectGroups.ClampOffset(selectionItems,dx,dy);objectDelta=new(delta.X,delta.Y);Invalidate();
        }else if(objectMarquee)Invalidate();
    }
    void EndObjectGesture(Point point){
        if(objectMarquee){objectPointer=point;var box=SelectionRectangle;var items=VisibleObjects.Where(ObjectGroups.Editable).Where(p=>box.Contains((int)MathF.Round(panX+p.X*zoom),(int)MathF.Round(panY+p.Y*zoom))).ToArray();var combined=marqueeAdditive?selectionBeforeMarquee.Concat(items).Where(ObjectGroups.Editable).Distinct().ToArray():items;objectMarquee=false;Capture=false;SetObjectSelection(combined);return;}
        if(objectDragging){MoveObjectGesture(point);var delta=objectDelta;objectDragging=false;objectDelta=PointF.Empty;Capture=false;MoveObjectSelection(delta.X,delta.Y);Invalidate();}
    }
    public bool CancelObjectGesture(){
        bool active=objectDragging||objectMarquee;var previous=selectionBeforeMarquee;bool rectangle=objectMarquee;
        objectDragging=objectMarquee=false;objectDelta=PointF.Empty;selectionBeforeMarquee=[];
        if(active){Capture=false;if(rectangle)SetObjectSelection(previous);Invalidate();}return active;
    }
    void DrawObjectSelectionOverlay(Graphics g){
        if(!objectMarquee)return;var box=SelectionRectangle;using var fill=new SolidBrush(Color.FromArgb(30,29,146,137));using var border=new Pen(Color.Teal,1){DashStyle=DashStyle.Dash};g.FillRectangle(fill,box);g.DrawRectangle(border,box);
    }
    // Isolated checks exercise the same handlers as the mouse, including cancellation.
    public void CheckObjectGesture(Point from,Point to,Keys modifiers=Keys.None,bool cancel=false,bool loseCapture=false,Point? via=null){
        checkObjectModifiers=modifiers;
        try{OnMouseDown(new(MouseButtons.Left,1,from.X,from.Y,0));if(via is Point middle)OnMouseMove(new(MouseButtons.Left,0,middle.X,middle.Y,0));OnMouseMove(new(MouseButtons.Left,0,to.X,to.Y,0));if(cancel)CancelObjectGesture();if(loseCapture)Capture=false;OnMouseUp(new(MouseButtons.Left,1,to.X,to.Y,0));}
        finally{checkObjectModifiers=null;}
    }
}
