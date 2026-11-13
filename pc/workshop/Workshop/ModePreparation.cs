using System.Security.Cryptography;
using System.Text.Json;
using YoshiWorkshop;
namespace WorkshopApp;

public sealed partial class WorkshopWindow {
    public sealed class ModeSetup {
        public int Schema {get;set;}=2;
        public int FirstColor {get;set;}
        public int SecondColor {get;set;}=5;
        public string LevelKey {get;set;}="vanilla-43";
        public bool PlayerInteractions {get;set;}
        public int RespawnSeconds {get;set;}=5;
        public string PeerAddress {get;set;}="127.0.0.1";
        public int HostPort {get;set;}
        public int JoinPort {get;set;}=42042;
        public ulong BossSeed {get;set;}=1;
        public int BossYoshi {get;set;}
        public int BossTerrain {get;set;}
        public void UpgradeLegacyColors(){
            if(Schema!=1)return;
            int first=BossBattleYoshiColors.FromLegacyPicker(FirstColor),second=BossBattleYoshiColors.FromLegacyPicker(SecondColor),boss=BossBattleYoshiColors.FromLegacyPicker(BossYoshi);
            FirstColor=first;SecondColor=second;BossYoshi=boss;Schema=2;
        }
        public void Validate(){
            if(Schema!=2 || BossTerrain is <0 or >2 || FirstColor is <0 or >13 || SecondColor is <0 or >13 || BossYoshi is <0 or >17 || RespawnSeconds is <1 or >60 || HostPort is <0 or >65535 || JoinPort is <1 or >65535 || LevelKey is null || PeerAddress is null || LevelKey.Length>100 || PeerAddress.Length>100)throw new InvalidDataException("Réglages des modes invalides ; fichier conservé.");
        }
    }
    public sealed class BossPreparationDocument {
        public int Schema {get;set;}=2;
        public int YoshiColor {get;set;}
        public BossBattlePlan Plan {get;set;}=new();
        public void UpgradeLegacyColors(){if(Schema==1){YoshiColor=BossBattleYoshiColors.FromLegacyPicker(YoshiColor);Schema=2;}}
        public void Validate(){if(Schema!=2 || Plan is null || YoshiColor is <0 or >17)throw new InvalidDataException("Préparation de bataille invalide.");Plan.Validate();}
    }
    sealed record YoshiChoice(int Index,string Name,Color Body){public override string ToString()=>Name;}
    static readonly YoshiChoice[] ModeYoshis=[
        new(0,"Vert",Color.ForestGreen),new(1,"Rouge",Color.Crimson),new(2,"Jaune",Color.Gold),new(3,"Bleu",Color.RoyalBlue),new(4,"Bleu pâle",Color.DeepSkyBlue),new(5,"Rose",Color.HotPink),new(6,"Noir",Color.FromArgb(45,40,50)),new(7,"Blanc",Color.WhiteSmoke),
        new(8,"Orange",Color.FromArgb(239,139,48)),new(9,"Violet",Color.FromArgb(146,66,192)),new(10,"Turquoise",Color.FromArgb(33,188,185)),new(11,"Lime-céleri",Color.FromArgb(174,205,87)),new(12,"Brun",Color.FromArgb(135,85,47)),new(13,"Vert foncé",Color.FromArgb(37,117,67))];
    static readonly YoshiChoice[] SoloYoshis=[..ModeYoshis.Select(y=>y.Index==6?new YoshiChoice(6,"Blanc",Color.WhiteSmoke):y.Index==7?new YoshiChoice(7,"Noir",Color.FromArgb(45,40,50)):y),new(14,"Or",Color.FromArgb(223,185,75)),new(15,"Argent",Color.FromArgb(189,200,214)),new(16,"Feu",Color.FromArgb(255,107,16)),new(17,"Eau",Color.FromArgb(25,165,255))];
    ModeSetup modeSetup=new();
    string ModeDirectory=>Path.Combine(pcRoot,"content","mode-preparations");
    string ModeSetupPath=>Path.Combine(ModeDirectory,"settings.json");
    string BossPlanPath=>Path.Combine(ModeDirectory,"boss-battle-plan.json");
    string? modeLoadProblem;
    void LoadModeSetup(){
        if(!File.Exists(ModeSetupPath))return;
        try{if(new FileInfo(ModeSetupPath).Length>65536)throw new InvalidDataException("Réglages trop volumineux.");var loaded=JsonSerializer.Deserialize<ModeSetup>(File.ReadAllText(ModeSetupPath),ProjectStore.Json)??throw new InvalidDataException("Réglages absents.");loaded.UpgradeLegacyColors();loaded.Validate();modeSetup=loaded;}catch(Exception e)when(e is IOException or JsonException or InvalidDataException){modeLoadProblem="Les réglages précédents sont illisibles et ont été conservés. "+e.Message;}
    }
    void SaveModeDocument<T>(string path,T value,JsonSerializerOptions? options=null){
        Directory.CreateDirectory(ModeDirectory);string temp=path+".tmp-"+Guid.NewGuid().ToString("N");
        try{File.WriteAllText(temp,JsonSerializer.Serialize(value,options??ProjectStore.Json));if(File.Exists(path))File.Copy(path,path+".before-"+DateTime.UtcNow.ToString("yyyyMMddTHHmmssfffffff")+"-"+Guid.NewGuid().ToString("N"),false);File.Move(temp,path,true);}finally{if(File.Exists(temp))File.Delete(temp);}
    }
    void PersistModeSetup(){modeSetup.Validate();SaveModeDocument(ModeSetupPath,modeSetup);}
    ComboBox YoshiPicker(int selected,bool solo=false){
        var box=new ComboBox{DropDownStyle=ComboBoxStyle.DropDownList,Width=250,DrawMode=DrawMode.OwnerDrawFixed,ItemHeight=30};
        var rights=new KnownYoshis0355.Snapshot(KnownYoshis0355.OrdinaryMask);int previous=0;bool refreshing=false;
        void RefreshKnown(){
            if(!solo)return;int wanted=box.SelectedIndex>=0?box.SelectedIndex:selected;
            rights=KnownYoshis0355.ReadProfile(Path.Combine(pcRoot,"profiles","content-personal"));
            refreshing=true;try{box.BeginUpdate();box.Items.Clear();foreach(var y in SoloYoshis)box.Items.Add(rights.Known(y.Index)?y:new YoshiChoice(y.Index,"???",Color.DimGray));box.SelectedIndex=rights.Initial(wanted);previous=box.SelectedIndex;}finally{box.EndUpdate();refreshing=false;}
        }
        if(solo)RefreshKnown();else{foreach(var y in ModeYoshis)box.Items.Add(y);box.SelectedIndex=selected;}
        box.SelectedIndexChanged+=(s,e)=>{if(!solo||refreshing)return;if(!rights.Known(box.SelectedIndex)){refreshing=true;try{box.SelectedIndex=rights.Initial(previous);}finally{refreshing=false;}}else previous=box.SelectedIndex;};
        box.DropDown+=(s,e)=>RefreshKnown();
        box.KeyDown+=(s,e)=>{
            if(!solo)return;
            int? next=e.KeyCode switch{Keys.Down or Keys.Right=>rights.Next(box.SelectedIndex,1),Keys.Up or Keys.Left=>rights.Next(box.SelectedIndex,-1),Keys.Home=>0,Keys.End=>rights.Last(),Keys.PageDown=>rights.Next(box.SelectedIndex,1),Keys.PageUp=>rights.Next(box.SelectedIndex,-1),_=>null};
            if(next!=null){box.SelectedIndex=next.Value;e.Handled=e.SuppressKeyPress=true;}
        };
        box.DrawItem+=(s,e)=>{if(e.Index<0)return;e.DrawBackground();if(box.Items[e.Index] is not YoshiChoice y)return;bool known=!solo||rights.Known(y.Index);using var color=new SolidBrush(known?y.Body:Color.WhiteSmoke);e.Graphics.FillEllipse(color,e.Bounds.X+7,e.Bounds.Y+6,18,18);e.Graphics.DrawEllipse(Pens.DimGray,e.Bounds.X+7,e.Bounds.Y+6,18,18);if(!known)TextRenderer.DrawText(e.Graphics,"?",e.Font??box.Font,new Rectangle(e.Bounds.X+7,e.Bounds.Y+5,18,20),Color.DimGray,TextFormatFlags.HorizontalCenter|TextFormatFlags.VerticalCenter|TextFormatFlags.NoPadding);TextRenderer.DrawText(e.Graphics,known?y.Name:"???",e.Font??box.Font,new Point(e.Bounds.X+34,e.Bounds.Y+5),e.ForeColor);e.DrawFocusRectangle();};return box;
    }
    static Label ModeText(string text,int width=480,int size=10,bool bold=false)=>new(){Text=text,AutoSize=true,MaximumSize=new(width,0),Margin=new(3,6,3,10),Font=new("Segoe UI",size,bold?FontStyle.Bold:FontStyle.Regular)};
    static FlowLayoutPanel ModeColumn(params Control[] controls){var c=new FlowLayoutPanel{Dock=DockStyle.Fill,FlowDirection=FlowDirection.TopDown,WrapContents=false,AutoScroll=true,Padding=new(12)};c.Controls.AddRange(controls);return c;}
    static Panel ModeField(string label,Control input,int width=400){var p=new Panel{Width=width,Height=72,Margin=new(3,3,3,7)};p.Controls.Add(new Label{Text=label,Dock=DockStyle.Top,Height=27});input.Location=new(0,29);input.Width=Math.Min(input.Width,width-10);p.Controls.Add(input);input.BringToFront();return p;}
    public void SelectPreparationSection(bool boss){tabs.SelectedIndex=boss?4:3;}
    async Task ModeAction(Func<Task> action){try{await action();}catch(OperationCanceledException){}catch(Exception e){if(automatedChecks)throw;if(coopUiClosing || IsDisposed)return;string message=e is System.Net.Sockets.SocketException?"Connexion impossible. Vérifie l’adresse IP, le port et que le salon est ouvert.":e.Message.Contains("differ",StringComparison.OrdinalIgnoreCase)?"Le niveau, la version du jeu ou les règles ne correspondent pas à ceux de l’hôte.":e.Message;status.Text=message;MessageBox.Show(this,message,"Préparation de partie",MessageBoxButtons.OK,MessageBoxIcon.Information);}}
}
