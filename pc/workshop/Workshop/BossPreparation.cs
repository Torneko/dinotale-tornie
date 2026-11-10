using System.Text.Json;
using YoshiWorkshop;
namespace WorkshopApp;
public sealed partial class WorkshopWindow {
    readonly NumericUpDown bossSeed=new(){Minimum=0,Maximum=ulong.MaxValue,Width=250};
    ComboBox bossYoshi=null!;
    readonly ComboBox bossTerrain=new(){DropDownStyle=ComboBoxStyle.DropDownList,Width=300};
    readonly DataGridView bossPreview=new(){Dock=DockStyle.Fill,ReadOnly=true,AllowUserToAddRows=false,AllowUserToDeleteRows=false,AllowUserToResizeRows=false,RowHeadersVisible=false,AutoSizeColumnsMode=DataGridViewAutoSizeColumnsMode.Fill,SelectionMode=DataGridViewSelectionMode.FullRowSelect,MultiSelect=false,BackgroundColor=Color.FromArgb(248,240,216),BorderStyle=BorderStyle.None};
    readonly Label bossDescription=new(){Dock=DockStyle.Fill,Padding=new(12),AutoEllipsis=false};
    readonly Label bossPreparationStatus=ModeText("Prépare puis enregistre une épreuve de dix rencontres surprises.",410);
    Button bossSave=null!;BossBattlePlan? bossPlan;
    void BuildBossPreparation(){
        var page=Page("Boss Battle");var split=new TableLayoutPanel{Dock=DockStyle.Fill,ColumnCount=2,RowCount=1};split.ColumnStyles.Add(new(SizeType.Absolute,440));split.ColumnStyles.Add(new(SizeType.Percent,100));
        bossYoshi=YoshiPicker(modeSetup.BossYoshi);bossSeed.Value=modeSetup.BossSeed;bossTerrain.Items.AddRange(new object[]{"Terrain jungle 1 — référence","Terrain jungle 2 — prévu","Terrain jungle 3 — prévu"});bossTerrain.SelectedIndex=modeSetup.BossTerrain;
        bossSave=Button("Enregistrer mon parcours",()=>Safe(SaveBossPreparation),315);
        var settings=ModeColumn(ModeText("Ta bataille de dix boss",410,20,true),ModeText("Cette page prépare et enregistre un parcours dans l’Atelier. Elle ne lance pas la bataille native et ne valide pas la jouabilité des terrains ou des rencontres.",410),ModeText("Dix rencontres surprises t’attendent. Les adversaires à venir restent cachés pour conserver la découverte.",410),ModeField("Yoshi prévu pour le défi",bossYoshi),ModeField("Numéro du parcours",bossSeed),ModeField("Terrain prévu pour les rencontres",bossTerrain),
            Button("Préparer ce parcours",()=>Safe(GenerateBossPreview),315),Button("Nouveau parcours",()=>Safe(()=>{bossSeed.Value=System.Security.Cryptography.RandomNumberGenerator.GetInt32(int.MaxValue);GenerateBossPreview();}),315),bossSave,bossPreparationStatus,
            ModeText("Entre les combats : une pièce de repos, le Lucky Tree sans invincibilité et un pot vers le combat suivant. Les fruits consommés restent mémorisés. Deux régénérations sont prévues sur la session entière.",410),
            ModeText("Un défi de victoire complète est prévu pour chacun des quatorze Yoshis. Enregistrer un parcours ne débloque aucun succès.",410),
            ModeText("Tu peux préparer un autre parcours sans modifier les niveaux d’origine.",410));
        foreach(string name in new[]{"Combat","Rencontre"})bossPreview.Columns.Add(name,name);bossPreview.Columns[0].FillWeight=35;bossPreview.Columns[1].FillWeight=180;foreach(DataGridViewColumn column in bossPreview.Columns)column.SortMode=DataGridViewColumnSortMode.NotSortable;bossPreview.RowTemplate.Height=34;bossPreview.ColumnHeadersHeight=34;
        var right=new TableLayoutPanel{Dock=DockStyle.Fill,ColumnCount=1,RowCount=3,Padding=new(12)};right.RowStyles.Add(new(SizeType.Absolute,52));right.RowStyles.Add(new(SizeType.Percent,100));right.RowStyles.Add(new(SizeType.Absolute,210));right.Controls.Add(ModeText("Parcours surprise",700,16,true),0,0);right.Controls.Add(bossPreview,0,1);right.Controls.Add(bossDescription,0,2);split.Controls.Add(settings,0,0);split.Controls.Add(right,1,0);page.Controls.Add(split);
        bossPreview.SelectionChanged+=(s,e)=>ShowBossEncounter();
        if(File.Exists(BossPlanPath)){
            try{if(new FileInfo(BossPlanPath).Length>262144)throw new InvalidDataException("Parcours trop volumineux.");var saved=JsonSerializer.Deserialize<BossPreparationDocument>(File.ReadAllText(BossPlanPath),BossBattleJson.Options)??throw new InvalidDataException("Parcours absent.");saved.UpgradeLegacyColors();saved.Validate();bossPlan=saved.Plan;bossSeed.Value=(decimal)saved.Plan.Seed;bossYoshi.SelectedIndex=saved.YoshiColor;bossTerrain.SelectedIndex=saved.Plan.Configuration.ArenaSlots.GetValueOrDefault(BossBattleFamily.YellowCandy,0);RenderBossPreview();bossPreparationStatus.Text="Parcours enregistré retrouvé.";}
            catch(Exception e)when(e is IOException or InvalidDataException or JsonException){bossPreparationStatus.Text="Le parcours enregistré ne peut pas être repris ; son fichier est conservé. "+e.Message;}
        }
        if(bossPlan==null)GenerateBossPreview();
    }
    void GenerateBossPreview(){
        var configuration=new BossBattleConfiguration{ArenaSlots=BossBattleCatalog.Families.Where(f=>f.Family!=BossBattleFamily.DoubleBlargg).ToDictionary(f=>f.Family,f=>bossTerrain.SelectedIndex)};
        bossPlan=BossBattlePlanner.Create((ulong)bossSeed.Value,configuration);RenderBossPreview();bossPreparationStatus.Text="Parcours préparé. Les dix adversaires restent cachés ; leurs identités seront découvertes pendant les combats.";
    }
    void RenderBossPreview(){
        bossPreview.Rows.Clear();if(bossPlan==null)return;
        foreach(var encounter in BossBattlePresentation.Preparation(bossPlan))bossPreview.Rows.Add(encounter.Number,"À découvrir");if(bossPreview.Rows.Count>0)bossPreview.Rows[0].Selected=true;ShowBossEncounter();
    }
    void ShowBossEncounter(){
        if(bossPlan==null || bossPreview.CurrentRow==null)return;int i=bossPreview.CurrentRow.Index;var hidden=BossBattlePresentation.Preparation(bossPlan);if(i<0 || i>=hidden.Count)return;
        bossDescription.Text=$"{hidden[i].Title} — rencontre surprise\n\nL’adversaire sera découvert à l’entrée du combat. La liste ne révèle ni son identité, ni sa variante, ni ses caractéristiques.\n\nLe numéro du parcours permet de retrouver la même épreuve en gardant la surprise.";
    }

    void SaveBossPreparation(){
        if(bossPlan==null || bossPlan.Seed!=(ulong)bossSeed.Value || bossPlan.Configuration.ArenaSlots.GetValueOrDefault(BossBattleFamily.YellowCandy,0)!=bossTerrain.SelectedIndex)GenerateBossPreview();var saved=new BossPreparationDocument{YoshiColor=((YoshiChoice)bossYoshi.SelectedItem!).Index,Plan=bossPlan!};saved.Validate();SaveModeDocument(BossPlanPath,saved,BossBattleJson.Options);modeSetup.BossSeed=(ulong)bossSeed.Value;modeSetup.BossYoshi=saved.YoshiColor;modeSetup.BossTerrain=bossTerrain.SelectedIndex;PersistModeSetup();bossPreparationStatus.Text="Parcours enregistré. Tu retrouveras le même numéro et les mêmes dix rencontres à la réouverture.";status.Text="Préparation Boss Battle enregistrée.";
    }
}
