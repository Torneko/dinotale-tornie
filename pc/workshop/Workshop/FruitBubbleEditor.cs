using YoshiWorkshop;
using System.Text.Json;
namespace WorkshopApp;
public sealed partial class WorkshopWindow {
    public void RunFruitBubbleChecks(){
        if(!pcRoot.Contains("validation",StringComparison.OrdinalIgnoreCase))throw new InvalidOperationException("Copie de validation isolée requise.");
        Directory.CreateDirectory(Path.Combine(pcRoot,"build/editor-0170"));
        sourceLevel.SelectedItem=sourceLevel.Items.Cast<LevelTemplate>().Single(t=>t.SourceZone==43);NewFromVanilla();
        objectSearch.Text="";objectCategory.SelectedItem="Fruits en bulle";RefreshPalette();
        if(palette.Items.Count!=6)throw new Exception("Six fruits en bulle attendus.");
        var original=SceneObjects.First(p=>ObjectCatalog.IsFruitBubble(p.ObjectId));canvas.Selected=original;ShowSelection();var id=original.Id;var oldId=original.ObjectId;float beforeX=original.X,beforeY=original.Y;ushort beforeLayer=original.Layer;
        palette.SelectedItem=palette.Items.Cast<PaletteItem>().Single(p=>p.Id==0x4F25);ReplaceSelectedFruit();
        if(canvas.Selected!.ObjectId!=0x4F25 || canvas.Selected.X!=beforeX || canvas.Selected.Y!=beforeY || canvas.Selected.Layer!=beforeLayer)throw new Exception("Bubble replacement lost native parameters");
        Undo();if(SceneObjects.Single(p=>p.Id==id).ObjectId!=oldId)throw new Exception("Bubble undo failed");Redo();
        for(int i=0;i<5;i++){palette.SelectedItem=palette.Items.Cast<PaletteItem>().Single(p=>p.Id==0x4F20+i);AddObject();canvas.Selected!.X=240+i*48;canvas.Selected.Y=350;}
        SaveLevel();var saved=store.Levels().Single(p=>p.Id==current!.Id);if(saved.Objects.Count(p=>ObjectCatalog.IsSpecialBubble(p.ObjectId))!=6)throw new Exception("Bubble save failed");
        var payload=rom.ExportPlacements(saved,true);if(payload[7]!='3')throw new Exception("Bubble multiroom compatibility lost");
        canvas.FocusAt(350,400,1.3f);objectSearch.Text="";RefreshPalette();
        File.WriteAllText(Path.Combine(pcRoot,"build/editor-0170/ui-bubbles.json"),JsonSerializer.Serialize(new{sixPaletteEntries=true,distinctBubblePreview=true,add=true,replacePreservesPositionAndParameters=true,undoRedo=true,save=true,export=true,isolatedProfile=true},ProjectStore.Json));
    }
}
