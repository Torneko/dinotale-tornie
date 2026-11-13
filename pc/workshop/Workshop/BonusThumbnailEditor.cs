using System.Buffers.Binary;
using System.Drawing.Drawing2D;
using System.Drawing.Imaging;
using System.IO.Hashing;
using System.Text.Json;
using YoshiWorkshop;

namespace WorkshopApp;

public sealed partial class WorkshopWindow {
    readonly ComboBox bonusThumbnailSlot=new(){DropDownStyle=ComboBoxStyle.DropDownList,Width=130};
    readonly PictureBox bonusThumbnailPicture=new(){Dock=DockStyle.Fill,SizeMode=PictureBoxSizeMode.Zoom,BackColor=Color.FromArgb(28,40,34)};
    readonly Label bonusThumbnailEmpty=new(){Dock=DockStyle.Fill,TextAlign=ContentAlignment.MiddleCenter,BackColor=Color.FromArgb(28,40,34),ForeColor=Color.FromArgb(234,239,222)};
    readonly Label bonusThumbnailMessage=new(){Width=1000,Height=58,AutoEllipsis=true};
    BonusThumbnailStore? bonusThumbnailStore;
    TabPage? bonusThumbnailPage;
    bool bonusThumbnailsEnglish;
    string BonusThumbnailText(string fr,string en)=>bonusThumbnailsEnglish?en:fr;

    void BuildBonusThumbnails() {
        try {
            string preferences=Path.Combine(pcRoot,"profiles/content-personal/book-menu.json");
            if(File.Exists(preferences)&&new FileInfo(preferences).Length<=16384) {
                using var parsed=JsonDocument.Parse(File.ReadAllText(preferences));
                bonusThumbnailsEnglish=parsed.RootElement.TryGetProperty("language",out var language)&&language.ValueKind==JsonValueKind.String&&language.GetString()=="en";
            }
        } catch(Exception e)when(e is IOException or UnauthorizedAccessException or JsonException){}
        bonusThumbnailStore=new(pcRoot);
        bonusThumbnailPage=new TabPage(BonusThumbnailText("Vignettes Bonus","Bonus thumbnails")){BackColor=BackColor};
        var content=new FlowLayoutPanel{Dock=DockStyle.Fill,FlowDirection=FlowDirection.TopDown,WrapContents=false,AutoScroll=true,Padding=new(18)};
        content.Controls.Add(new Label{Width=1000,Height=64,Text=BonusThumbnailText("Importe une capture PNG enregistrée pendant un essai de Bonus. Son aperçu apparaîtra dans le menu des Bonus accessibles. Choisis une scène qui ne révèle pas la finale.","Import a PNG screenshot saved during a Bonus test. Its preview will appear in the menu for accessible Bonus courses. Choose a scene that keeps the finale a surprise.")});
        var controls=new FlowLayoutPanel{Width=1000,Height=47,WrapContents=false};
        controls.Controls.Add(new Label{Text=BonusThumbnailText("Niveau bonus","Bonus course"),AutoSize=true,Margin=new(0,9,12,0)});
        controls.Controls.Add(bonusThumbnailSlot);
        controls.Controls.Add(Button(BonusThumbnailText("Importer une capture PNG","Import PNG screenshot"),()=>Safe(ImportBonusThumbnail),235));
        controls.Controls.Add(Button(BonusThumbnailText("Actualiser","Refresh"),()=>Safe(RefreshBonusThumbnail),135));
        content.Controls.Add(controls);
        var frame=new Panel{Width=640,Height=360,BorderStyle=BorderStyle.FixedSingle};frame.Controls.Add(bonusThumbnailPicture);frame.Controls.Add(bonusThumbnailEmpty);content.Controls.Add(frame);content.Controls.Add(bonusThumbnailMessage);
        bonusThumbnailPage.Controls.Add(content);tabs.TabPages.Add(bonusThumbnailPage);
        foreach(int slot in Enumerable.Range(1,6))bonusThumbnailSlot.Items.Add($"7-{slot}");
        bonusThumbnailSlot.SelectedIndexChanged+=(_,_)=>Safe(RefreshBonusThumbnail);
        bonusThumbnailSlot.SelectedIndex=0;
        FormClosed+=(_,_)=>{var old=bonusThumbnailPicture.Image;bonusThumbnailPicture.Image=null;old?.Dispose();};
    }
    public void SelectBonusThumbnails(){if(bonusThumbnailPage!=null)tabs.SelectedTab=bonusThumbnailPage;}
    void RefreshBonusThumbnail() {
        if(bonusThumbnailStore==null||bonusThumbnailSlot.SelectedIndex<0)return;
        int slot=bonusThumbnailSlot.SelectedIndex+1;
        var previous=bonusThumbnailPicture.Image;bonusThumbnailPicture.Image=null;previous?.Dispose();
        byte[]? bytes;
        try {bytes=bonusThumbnailStore.Read(slot);}
        catch(Exception e)when(e is InvalidDataException or IOException or UnauthorizedAccessException) {
            bonusThumbnailEmpty.Text=BonusThumbnailText("Aperçu indisponible","Preview unavailable");bonusThumbnailEmpty.Visible=true;bonusThumbnailEmpty.BringToFront();
            bonusThumbnailMessage.Text=BonusThumbnailText("L’aperçu enregistré n’a pas pu être lu. Son fichier a été conservé.","The saved preview could not be read. Its file has been preserved.");return;
        }
        if(bytes==null) {
            bonusThumbnailEmpty.Text=BonusThumbnailText("Aucune capture importée","No imported screenshot");bonusThumbnailEmpty.Visible=true;bonusThumbnailEmpty.BringToFront();
            bonusThumbnailMessage.Text=BonusThumbnailText("Choisis une capture de ton essai pour ce Bonus.","Choose a screenshot from your test for this Bonus.");return;
        }
        using var stream=new MemoryStream(bytes,false);using var image=Image.FromStream(stream);
        bonusThumbnailPicture.Image=new Bitmap(image);bonusThumbnailEmpty.Visible=false;bonusThumbnailPicture.BringToFront();
        bonusThumbnailMessage.Text=BonusThumbnailText("Aperçu enregistré pour ","Saved preview for ")+$"7-{slot}.";
    }
    public static bool ImportBonusScreenshot0358(BonusThumbnailStore store,int slot,Stream? selected,bool english=false) {
        if(selected==null)return false;
        byte[] encoded=NormalizeBonusScreenshot0358(selected,english);store.Import(slot,encoded);return true;
    }
    public static byte[] NormalizeBonusScreenshot0358(Stream input,bool english=false) {
        string Text(string fr,string en)=>english?en:fr;
        if(!input.CanRead||!input.CanSeek)throw new InvalidDataException(Text("Capture PNG illisible.","PNG screenshot cannot be read."));
        if(input.Length is <33 or >16_777_216)throw new InvalidDataException(Text("Choisis une capture PNG de16Mio maximum.","Choose a PNG screenshot no larger than16MiB."));
        input.Position=0;byte[] header=new byte[33];input.ReadExactly(header);
        uint width=BinaryPrimitives.ReadUInt32BigEndian(header.AsSpan(16,4)),height=BinaryPrimitives.ReadUInt32BigEndian(header.AsSpan(20,4));
        bool validHeader=header.AsSpan(0,8).SequenceEqual(new byte[]{137,80,78,71,13,10,26,10})&&BinaryPrimitives.ReadUInt32BigEndian(header.AsSpan(8,4))==13&&header.AsSpan(12,4).SequenceEqual(new byte[]{73,72,68,82})&&Crc32.HashToUInt32(header.AsSpan(12,17))==BinaryPrimitives.ReadUInt32BigEndian(header.AsSpan(29,4));
        if(!validHeader||width is 0 or >8192||height is 0 or >8192||(ulong)width*height>16_777_216)throw new InvalidDataException(Text("Choisis une capture PNG de8192pixels de côté et16millions de pixels maximum.","Choose a PNG screenshot up to8192pixels per side and16million pixels."));
        input.Position=0;using var original=Image.FromStream(input,false,true);
        if(original.Width!=width||original.Height!=height)throw new InvalidDataException(Text("Dimensions PNG incohérentes.","PNG dimensions do not match."));
        using var normalized=new Bitmap(320,180,PixelFormat.Format32bppArgb);
        using(var drawing=Graphics.FromImage(normalized)) {
            drawing.Clear(Color.FromArgb(255,18,31,24));drawing.InterpolationMode=InterpolationMode.HighQualityBicubic;
            float scale=Math.Min(320f/original.Width,180f/original.Height);float w=original.Width*scale,h=original.Height*scale;
            drawing.DrawImage(original,(320-w)/2,(180-h)/2,w,h);
        }
        using var encoded=new MemoryStream();normalized.Save(encoded,ImageFormat.Png);return encoded.ToArray();
    }
    void ImportBonusThumbnail() {
        if(bonusThumbnailStore==null||bonusThumbnailSlot.SelectedIndex<0)return;
        int slot=bonusThumbnailSlot.SelectedIndex+1;
        using var picker=new OpenFileDialog{Title=BonusThumbnailText("Choisir une capture pour ","Choose a screenshot for ")+$"7-{slot}",Filter="Image PNG|*.png",CheckFileExists=true,Multiselect=false};
        if(picker.ShowDialog(this)!=DialogResult.OK){ImportBonusScreenshot0358(bonusThumbnailStore,slot,null,bonusThumbnailsEnglish);return;}
        using var input=new FileStream(picker.FileName,FileMode.Open,FileAccess.Read,FileShare.Read);
        ImportBonusScreenshot0358(bonusThumbnailStore,slot,input,bonusThumbnailsEnglish);
        RefreshBonusThumbnail();status.Text=BonusThumbnailText("Aperçu du Bonus enregistré : ","Bonus preview saved: ")+$"7-{slot}.";bonusThumbnailMessage.Text+=BonusThumbnailText(" Si DinoTale était déjà ouvert, relance-le pour actualiser cet aperçu."," If DinoTale was already open, restart it to refresh this preview.");
    }
}
