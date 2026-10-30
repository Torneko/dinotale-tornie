using System.Diagnostics;

using System.ComponentModel;

using System.Drawing.Drawing2D;

using System.Drawing.Imaging;

using System.Text.Json;

using YoshiWorkshop;



namespace WorkshopApp;



public sealed partial class WorkshopWindow : Form {

    readonly string pcRoot;

    readonly ProjectStore store;

    readonly VanillaRom rom;

    readonly NativeRunner runner;

    readonly ContentPortTest contentTest;

    readonly SpecialContentRunner specialRunner;

    readonly Label status=new() {Dock=DockStyle.Bottom,Height=32,Padding=new(12,8,0,0),Text="Prototype PC • éditeur de placements du 1-1 • solo jouable"};

    readonly ListBox levels=new() {Dock=DockStyle.Fill,DisplayMember="Name"};

    readonly ListBox games=new() {Dock=DockStyle.Fill,DisplayMember="Name"};

    readonly ListBox gameLevels=new() {Dock=DockStyle.Fill,DisplayMember="Name"};

    readonly TextBox levelName=new() {Width=230,PlaceholderText="Nom du niveau"};

    readonly TextBox gameName=new() {Width=230,PlaceholderText="Nom de la partie"};

    readonly ComboBox palette=new() {Width=190,DropDownStyle=ComboBoxStyle.DropDownList,DisplayMember="Text"};

    readonly TextBox objectSearch=new() {Width=220,PlaceholderText="Chercher un objet ou son ID"};

    readonly ComboBox objectCategory=new() {Width=145,DropDownStyle=ComboBoxStyle.DropDownList};

    readonly Label objectDetails=new() {Dock=DockStyle.Top,Height=48,Padding=new(8),AutoEllipsis=true};

    readonly NumericUpDown x=new() {Minimum=-100000,Maximum=100000,DecimalPlaces=1,Width=105};

    readonly NumericUpDown y=new() {Minimum=-100000,Maximum=100000,DecimalPlaces=1,Width=105};

    readonly CheckBox interactions=new() {Text="Autoriser les interactions entre Yoshis",AutoSize=true};

    readonly LevelCanvas canvas=new() {Dock=DockStyle.Fill};

    readonly Stack<string> undo=[];

    readonly Stack<string> redo=[];

    LevelProject? current;

    CustomGame? game;

    bool updating,dirty;

    readonly TabControl tabs=new() {Dock=DockStyle.Fill};

    readonly Label selection=new() {AutoSize=true,Text="Sélectionne un objet dans le niveau."};



    public WorkshopWindow(string root) {

        pcRoot=root;store=new(Path.Combine(root,"content"));runner=new(root);contentTest=new(root);specialRunner=new(root);

        rom=new(specialRunner.OriginalRom);

        sessions=new(root,specialRunner,rom);

        EditorVisuals.AssetRoot=Directory.Exists(Path.Combine(root,"assets/editor"))?Path.Combine(root,"assets/editor"):Path.Combine(root,"build/special-assets");

        Text="DinoTale — Atelier 0.15.0";Width=1460;Height=920;MinimumSize=new(1160,740);KeyPreview=true;

        Font=new("Segoe UI",10);BackColor=Color.FromArgb(246,244,232);ForeColor=Color.FromArgb(32,48,42);

        BuildShell();

        foreach(var list in new[]{levels,gameLevels}){list.DrawMode=DrawMode.OwnerDrawFixed;list.ItemHeight=64;list.IntegralHeight=false;list.DrawItem+=DrawLevelItem;}

        BuildHome();BuildEditor();BuildGames();BuildCoop();BuildNotes();

        RefreshLevels();RefreshGames();

        if(levels.Items.Count>0)levels.SelectedIndex=0;

        canvas.SelectionChanged+=()=>ShowSelection();

        canvas.BeforeEdit+=Remember;

        canvas.Edited+=()=>{dirty=true;ShowSelection();RefreshSceneObjects();};

        sessions.Changed+=()=>{if(IsHandleCreated && !IsDisposed)BeginInvoke(RefreshSessionStatus);};

        sessions.Error+=message=>{if(IsHandleCreated && !IsDisposed)BeginInvoke(()=>{status.Text="Partie en pause : "+message;RefreshSessionStatus();});};

        KeyDown+=EditorKeys;

        Shown+=(s,e)=>{canvas.Fit();captureTimer.Start();};
        captureTimer.Tick+=(s,e)=>RefreshCaptureHotkey();

        FormClosed+=(s,e)=>{captureTimer.Stop();captureTimer.Dispose();UnregisterCaptureHotkey();EditorVisuals.Clear();};

        FormClosing+=(s,e)=>{if(specialRunner.IsRunning){e.Cancel=true;WindowState=FormWindowState.Minimized;status.Text="L’Atelier accompagne ta partie ; ferme d’abord la fenêtre de jeu.";}else if(!ConfirmDiscard())e.Cancel=true;};

    }

    public void SelectSection(bool customGames){tabs.SelectedIndex=customGames?2:1;}

    static Button Button(string text,Action action,int width=150) {

        var b=new Button {Text=text,AutoSize=false,Width=width,Height=36,FlatStyle=FlatStyle.Flat,BackColor=Color.White};

        b.Click+=(s,e)=>action();return b;

    }

    void Safe(Action action) {try {action();}catch(Exception e){MessageBox.Show(this,e.Message,"Atelier PC",MessageBoxButtons.OK,MessageBoxIcon.Information);}}

    void DrawLevelItem(object? sender,DrawItemEventArgs e){

        if(sender is not ListBox list || e.Index<0 || e.Index>=list.Items.Count || list.Items[e.Index] is not LevelProject item)return;

        e.DrawBackground();var rect=new Rectangle(e.Bounds.X+5,e.Bounds.Y+5,90,50);

        string icon=store.IconPath(item.Id);

        if(File.Exists(icon)){using var img=Image.FromFile(icon);e.Graphics.DrawImage(img,rect);}

        else{using var brush=new SolidBrush(Color.FromArgb(211,231,204));e.Graphics.FillRectangle(brush,rect);TextRenderer.DrawText(e.Graphics,VanillaRom.SourceCode(item.SourceZone),Font,rect,Color.DarkOliveGreen,TextFormatFlags.HorizontalCenter|TextFormatFlags.VerticalCenter);}

        TextRenderer.DrawText(e.Graphics,item.Name,e.Font??Font,new Rectangle(e.Bounds.X+102,e.Bounds.Y+9,Math.Max(10,e.Bounds.Width-108),45),e.ForeColor,TextFormatFlags.EndEllipsis|TextFormatFlags.VerticalCenter);

        e.DrawFocusRectangle();

    }

    FlowLayoutPanel Bar(params Control[] controls) {var p=new FlowLayoutPanel {Dock=DockStyle.Top,AutoSize=true,Padding=new(8),WrapContents=true};p.Controls.AddRange(controls);return p;}

    TabPage Page(string name) {var p=new TabPage(name){BackColor=BackColor,Padding=new(12)};tabs.TabPages.Add(p);return p;}

    static Label Note(string text,int height=65)=>new(){Text=text,Dock=DockStyle.Top,Height=height,Padding=new(8),AutoEllipsis=true};

    void BuildHome(){

        var p=Page("Accueil");

        p.Controls.Add(Note("Duplique l’un des 24 niveaux et modifie les objets de ses salles. Compose ta traversée et retrouve ta progression à la prochaine session. Le jeu d’origine et tes créations restent séparés.",90));

        p.Controls.Add(Bar(Button("Créer ou modifier un niveau",()=>tabs.SelectedIndex=1,270),Button("Mes parties personnalisées",()=>tabs.SelectedIndex=2,270)));

        p.Controls.Add(Note("Dans l’éditeur : fruits illustrés, Maskass hybrides, liste des objets, grille et aimantation. Pendant un test, F8 prend une capture du jeu pour l’icône de ton niveau.",90));

        p.Controls.Add(new Label{Text="Ton atelier d’histoires",Dock=DockStyle.Top,Height=90,Padding=new(15),Font=new("Segoe UI",27,FontStyle.Bold)});

    }



    string RomPath()=>specialRunner.OriginalRom;
    void BuildEditor()=>BuildExtendedEditor();

    void RefreshPalette()=>RefreshExtendedPalette();

    void BuildGames() {

        var p=Page("Parties personnalisées");

        var split=new SplitContainer {Size=new(1200,600),Dock=DockStyle.Fill,SplitterDistance=280,Panel1MinSize=240,FixedPanel=FixedPanel.Panel1};

        split.Panel1.Controls.Add(games);split.Panel1.Controls.Add(Bar(Button("Nouvelle partie",()=>{game=new();gameName.Text=game.Name;interactions.Checked=false;RefreshGameLevels();},190)));

        games.SelectedIndexChanged+=(s,e)=>{if(games.SelectedItem is CustomGame g){game=g;gameName.Text=g.Name;interactions.Checked=g.Coop.PlayerInteractions;RefreshGameLevels();RefreshSessionStatus();}};

        split.Panel2.Controls.Add(gameLevels);

        split.Panel2.Controls.Add(Bar(Button("Ajouter le niveau de l’éditeur",()=>Safe(AddToGame),255),Button("Monter",()=>MoveGameLevel(-1),90),Button("Descendre",()=>MoveGameLevel(1),110),Button("Retirer de la liste",RemoveGameLevel,160)));

        split.Panel2.Controls.Add(Bar(gameName,Button("Enregistrer la partie",()=>Safe(SaveGame),190),Button("Tester le niveau choisi",()=>Safe(TestGameLevel),210),Button("Jouer la partie",()=>Safe(()=>StartCustomGame(false)),165),Button("Reprendre",()=>Safe(()=>StartCustomGame(true)),120)));

        p.Controls.Add(split);p.Controls.Add(Note("Compose et sauvegarde une liste de niveaux. Chaque copie garde son décor d’origine. Après une victoire, la partie passe au niveau suivant ; « Reprendre » continue une traversée interrompue.",72));

    }

    void BuildCoop() {

        var p=Page("Coopération");

        var text="Deux modes prévus :\n\n• Local : deux manettes sur le même PC.\n• Réseau : un joueur héberge, l’autre rejoint sa partie.\n\nRègles retenues : fruits comptés ensemble, ennemis communs.\nLes Yoshis ne se bloquent pas par défaut.\nLes œufs traversent toujours le coéquipier et ne lui font aucun dégât.\n\nLa coop jouable n’est pas encore disponible dans ce prototype.";

        p.Controls.Add(Note("L’option d’interaction est mémorisée pour la partie ; les œufs resteront sans effet sur le coéquipier même si elle est activée.",65));

        p.Controls.Add(Bar(interactions,Button("Sauver pour la partie choisie",()=>Safe(()=>{if(game==null)throw new InvalidOperationException("Choisir d’abord une partie personnalisée.");game.Coop.PlayerInteractions=interactions.Checked;SaveGame();}),265)));

        p.Controls.Add(Note(text,330));

    }

    void BuildNotes() {

        var p=Page("Projet & contenu");

        p.AutoScroll=true;

        p.Controls.Add(Note("Le projet PC reste séparé de la ROM N64 9.2. Les six nouveaux fruits peuvent être placés et mangés dans tes niveaux personnalisés. Les six couleurs de Maskass et l’enchaînement des niveaux sont disponibles. La coop jouable reste à développer.",90));

        p.Controls.Add(Note("Favoris affichés dans la pause :\nNouveaux Yoshis : deux fruits classiques (+2) + leur hybride (+4 points et +4 de vie).\nYoshis d’origine : leur fruit classique + trois hybrides (+2).\nRouge/rose : pêche, mûre, prune • Jaune : pêche, poire, kiwi\nVert : bleuet, poire, prune • Bleu/bleu pâle : mûre, bleuet, kiwi\nBlanc et noir gardent +2 pour les hybrides. Ennemis : moyenne des deux couleurs, arrondie au-dessus ; bonus de vie converti en points ×2 si tués.",180));

        var gallery=new FlowLayoutPanel{Dock=DockStyle.Top,Height=175,WrapContents=false,AutoScroll=true,Padding=new(8)};

        foreach(var entry in new[]{("peche","Pêche · orange"),("mure","Mûre · violet"),("bleuet","Bleuet · turquoise"),("poire-verte","Poire · lime-céleri"),("prune","Prune · brun"),("kiwi","Kiwi · vert foncé")}){

            var card=new Panel{Width=155,Height=150};

            var caption=new Label{Dock=DockStyle.Bottom,Height=40,Text=entry.Item2,TextAlign=ContentAlignment.MiddleCenter};

            var picture=new PictureBox{Dock=DockStyle.Fill,SizeMode=PictureBoxSizeMode.Zoom};

            string path=Path.Combine(EditorVisuals.AssetRoot,entry.Item1+"-24x21.png");

            if(File.Exists(path)){using var source=Image.FromFile(path);picture.Image=new Bitmap(source);picture.Disposed+=(s,e)=>picture.Image?.Dispose();}

            card.Controls.Add(picture);card.Controls.Add(caption);gallery.Controls.Add(card);

        }

        p.Controls.Add(gallery);

        p.Controls.Add(Note("Teintes des fruits assorties aux Yoshis — aperçu des textures du jeu",48));

    }

    void RefreshLevels(string? selected=null) {

        updating=true;levels.Items.Clear();foreach(var l in store.Levels())levels.Items.Add(l);

        if(selected!=null)levels.SelectedItem=levels.Items.Cast<LevelProject>().FirstOrDefault(l=>l.Id==selected);

        updating=false;

    }

    void RefreshGames(){string? id=game?.Id;games.Items.Clear();foreach(var g in store.Games())games.Items.Add(g);if(id!=null)games.SelectedItem=games.Items.Cast<CustomGame>().FirstOrDefault(g=>g.Id==id);}

    bool ConfirmDiscard()=>!dirty || MessageBox.Show(this,"Des modifications ne sont pas enregistrées. Continuer ?","Niveau modifié",MessageBoxButtons.YesNo,MessageBoxIcon.Question)==DialogResult.Yes;

    void LoadLevel(LevelProject l){current=JsonSerializer.Deserialize<LevelProject>(JsonSerializer.Serialize(l,ProjectStore.Json),ProjectStore.Json)!;updating=true;levelName.Text=current.Name;updating=false;dirty=false;undo.Clear();redo.Clear();RefreshRoomChoices(current.SourceZone);}

    void NewFromVanilla(){if(!ConfirmDiscard())return;var template=sourceLevel.SelectedItem as LevelTemplate??throw new InvalidOperationException("Choisir un niveau original.");if(!template.Supported)throw new InvalidOperationException(template.Detail);var l=rom.CloneLevel(template.SourceZone);store.Save(l);RefreshLevels(l.Id);LoadLevel(l);}

    void Duplicate(){if(current==null)throw new InvalidOperationException("Choisir un niveau.");var l=store.Duplicate(current);store.Save(l);RefreshLevels(l.Id);LoadLevel(l);}

    void SaveLevel(){if(current==null)throw new InvalidOperationException("Ouvrir un niveau dans l’éditeur.");store.Save(current);dirty=false;RefreshLevels(current.Id);status.Text="Niveau enregistré : "+current.Name;}

    void EnsureGameClosed(){if(runner.IsRunning || contentTest.IsRunning || specialRunner.IsRunning)throw new InvalidOperationException("Fermer le jeu avant de lancer un autre essai.");}

    void LaunchLevel(LevelProject level){EnsureGameClosed();activeSoloId=level.Id;if(specialRunner.Available)specialRunner.Launch(level.Id,rom.ExportPlacements(level,true));else runner.Launch(File.ReadAllBytes(RomPath()),level.Id,rom.ExportPlacements(level));}

    void TestLevel(){SaveLevel();LaunchLevel(current!);status.Text="Test ouvert : Start lance le niveau personnalisé ; choisis ton Yoshi. F8 capture son icône.";}

    void Remember(){if(current==null)return;undo.Push(CaptureEditState());redo.Clear();}

    void Undo(){if(current==null || undo.Count==0)return;redo.Push(CaptureEditState());Restore(undo.Pop());}

    void Redo(){if(current==null || redo.Count==0)return;undo.Push(CaptureEditState());Restore(redo.Pop());}

    void Restore(string json)=>RestoreEditState(json);

    void ShowSelection(){updating=true;var o=canvas.Selected;selection.Text=o==null?"Sélectionne un objet.":ObjectCatalog.Get(ObjectCatalog.PaletteId(o)).Name;x.Enabled=y.Enabled=o!=null && o.ObjectId<0x8000 && o.ObjectId!=0x4001;if(o!=null){x.Value=Math.Clamp((decimal)o.X,x.Minimum,x.Maximum);y.Value=Math.Clamp((decimal)o.Y,y.Minimum,y.Maximum);}variantChoice.Enabled=o!=null && ObjectCatalog.IsMaskass(o.ObjectId);variantChoice.SelectedIndex=o?.HybridVariant is int c?c+1:0;updating=false;SyncSceneSelection();}

    void UpdateCoordinate(bool horizontal){if(updating || canvas.Selected==null)return;Remember();if(horizontal)canvas.Selected.X=(float)x.Value;else canvas.Selected.Y=(float)y.Value;dirty=true;canvas.Invalidate();}

    void DuplicateObject(){if(canvas.Selected==null || current==null)return;if(canvas.Selected.ObjectId>=0x8000)throw new InvalidOperationException("Les couches de décor sont conservées dans cette version.");if(canvas.Selected.ObjectId==0x4001)throw new InvalidOperationException("Un seul point de départ de Yoshi est nécessaire.");CheckObjectCapacity();Remember();var p=canvas.Selected.Copy();p.X+=24;SceneObjects.Add(p);canvas.Selected=p;dirty=true;canvas.Invalidate();RefreshSceneObjects();ShowSelection();}

    void RemoveObject(){if(canvas.Selected==null || current==null)return;if(canvas.Selected.ObjectId>=0x8000)throw new InvalidOperationException("Les couches de décor sont conservées dans cette version.");if(canvas.Selected.ObjectId==0x4001)throw new InvalidOperationException("Conserver le point de départ de Yoshi.");Remember();SceneObjects.Remove(canvas.Selected);canvas.Selected=null;dirty=true;canvas.Invalidate();RefreshSceneObjects();ShowSelection();}

    void AddObject(){if(current==null || palette.SelectedItem is not PaletteItem item)return;CheckObjectCapacity();Remember();var pos=canvas.WorldCenter;var p=rom.CreatePlacement(item.Id,activeRoom);p.X=Math.Clamp(pos.X,-100000,100000);p.Y=Math.Clamp(pos.Y,-100000,100000);SceneObjects.Add(p);canvas.Selected=p;dirty=true;canvas.Invalidate();RefreshSceneObjects();ShowSelection();}

    void CapturePreview(){SaveLevel();using var image=canvas.Snapshot();image.Save(store.IconPath(current!.Id),ImageFormat.Png);current.Icon=Path.GetFileName(store.IconPath(current.Id));store.Save(current);levels.Invalidate();gameLevels.Invalidate();status.Text="Icône enregistrée depuis l’aperçu de l’éditeur.";}

    void ImportIcon(){

        if(current==null)throw new InvalidOperationException("Choisir un niveau dans l’éditeur.");

        using var picker=new OpenFileDialog {Title="Choisir une capture pour l’icône du niveau",Filter="Image PNG|*.png|Image JPEG|*.jpg;*.jpeg"};

        if(picker.ShowDialog(this)!=DialogResult.OK)return;

        using var original=Image.FromFile(picker.FileName);

        if(original.Width>16384 || original.Height>16384)throw new InvalidDataException("Choisir une image de moins de 16384 pixels de côté.");

        using var icon=new Bitmap(320,180);using var graphics=Graphics.FromImage(icon);

        graphics.Clear(Color.FromArgb(233,241,231));graphics.InterpolationMode=InterpolationMode.HighQualityBicubic;

        float scale=Math.Min(320f/original.Width,180f/original.Height);float width=original.Width*scale,height=original.Height*scale;

        graphics.DrawImage(original,(320-width)/2,(180-height)/2,width,height);

        SaveLevel();icon.Save(store.IconPath(current.Id),ImageFormat.Png);current.Icon=Path.GetFileName(store.IconPath(current.Id));store.Save(current);

        levels.Invalidate();gameLevels.Invalidate();status.Text="Capture importée comme icône du niveau.";

    }

    void SaveGame(){if(game==null)throw new InvalidOperationException("Créer ou sélectionner une partie.");game.Name=gameName.Text;game.Coop.PlayerInteractions=interactions.Checked;store.Save(game);RefreshGames();status.Text="Partie personnalisée enregistrée.";}

    void AddToGame(){if(game==null || current==null)throw new InvalidOperationException("Ouvrir un niveau dans l’éditeur et créer une partie.");SaveLevel();game.LevelIds.Add(current.Id);RefreshGameLevels();}

    void RefreshGameLevels(){gameLevels.Items.Clear();if(game==null)return;var known=store.Levels().ToDictionary(l=>l.Id);foreach(var id in game.LevelIds)if(known.TryGetValue(id,out var l))gameLevels.Items.Add(l);}

    void MoveGameLevel(int delta){if(game==null)return;int i=gameLevels.SelectedIndex,j=i+delta;if(i<0 || j<0 || j>=game.LevelIds.Count)return;(game.LevelIds[i],game.LevelIds[j])=(game.LevelIds[j],game.LevelIds[i]);RefreshGameLevels();gameLevels.SelectedIndex=j;}

    void RemoveGameLevel(){int i=gameLevels.SelectedIndex;if(game==null || i<0)return;game.LevelIds.RemoveAt(i);RefreshGameLevels();}

    void TestGameLevel(){if(gameLevels.SelectedItem is not LevelProject l)throw new InvalidOperationException("Choisir un niveau dans la partie.");LaunchLevel(l);}

    sealed record PaletteItem(ushort Id,string Text);

}



