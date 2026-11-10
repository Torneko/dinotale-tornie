using System.Diagnostics;

using System.Drawing.Imaging;

using System.Runtime.InteropServices;

using YoshiWorkshop;

namespace WorkshopApp;



public sealed partial class WorkshopWindow {

    readonly CustomGameSession sessions;

    readonly ComboBox sourceLevel=new(){DropDownStyle=ComboBoxStyle.DropDownList,Width=240};

    readonly ComboBox variantChoice=new(){DropDownStyle=ComboBoxStyle.DropDownList,Width=220};

    readonly ListBox sceneObjects=new(){SelectionMode=SelectionMode.MultiExtended,Dock=DockStyle.Fill,DrawMode=DrawMode.OwnerDrawFixed,ItemHeight=43,IntegralHeight=false};

    readonly TextBox sceneSearch=new(){Dock=DockStyle.Top,PlaceholderText="Chercher dans ce niveau",Height=30};

    readonly Label sessionStatus=new(){Dock=DockStyle.Top,Height=45,Padding=new(10),Text="Choisis une partie pour voir sa progression."};

    readonly Label sourceSummary=new(){Dock=DockStyle.Top,Height=52,Padding=new(8),Text="Une copie indépendante, avec le décor original."};

    readonly CheckBox gridChoice=new(){Text="Grille",Checked=true,AutoSize=true};

    readonly CheckBox technicalChoice=new(){Text="Repères techniques",AutoSize=true};

    readonly ComboBox snapChoice=new(){DropDownStyle=ComboBoxStyle.DropDownList,Width=125};

    bool paletteUpdating,sceneUpdating,hotkeyRegistered;

    string? activeSoloId;

    public string[] SectionNames=>tabs.TabPages.Cast<TabPage>().Select(p=>p.Text).ToArray();

    void BuildShell(){

        var shell=new TableLayoutPanel{Dock=DockStyle.Fill,ColumnCount=1,RowCount=3,BackColor=Color.FromArgb(248,240,216)};

        shell.RowStyles.Add(new(SizeType.Absolute,78));shell.RowStyles.Add(new(SizeType.Percent,100));shell.RowStyles.Add(new(SizeType.Absolute,35));

        var header=new TableLayoutPanel{Dock=DockStyle.Fill,BackColor=Color.FromArgb(48,128,135),Padding=new(18,7,18,5),ColumnCount=1,RowCount=2};
        header.RowStyles.Add(new(SizeType.Percent,65));header.RowStyles.Add(new(SizeType.Percent,35));
        header.Controls.Add(new Label{Dock=DockStyle.Fill,Text="DinoTale   /   Atelier de jeu et de création",ForeColor=Color.White,Font=new("Segoe UI",22,FontStyle.Bold),TextAlign=ContentAlignment.MiddleLeft},0,0);
        header.Controls.Add(new Label{Dock=DockStyle.Fill,Text="Prépare tes parties · compose tes traversées · garde tes histoires",ForeColor=Color.FromArgb(239,242,212),Font=new("Segoe UI",10),TextAlign=ContentAlignment.MiddleLeft},0,1);
        tabs.Dock=DockStyle.Fill;tabs.Padding=new(15,7);status.Dock=DockStyle.Fill;status.Text="Atelier 0.27.0 · créations séparées du jeu original";

        shell.Controls.Add(header,0,0);shell.Controls.Add(tabs,0,1);shell.Controls.Add(status,0,2);Controls.Add(shell);

    }

    static FlowLayoutPanel Column(params Control[] controls){var p=new FlowLayoutPanel{Dock=DockStyle.Fill,FlowDirection=FlowDirection.TopDown,WrapContents=false,AutoScroll=true,Padding=new(9)};p.Controls.AddRange(controls);return p;}

    static Label Caption(string text)=>new(){Text=text,AutoSize=false,Width=235,Height=30,Font=new("Segoe UI",10,FontStyle.Bold),Padding=new(0,5,0,0)};

    void BuildExtendedEditor(){

        var page=Page("Éditeur de niveaux");ConfigureRoomEditor();

        var library=new Panel{Dock=DockStyle.Fill};levels.Dock=DockStyle.Fill;library.Controls.Add(levels);

        var create=Column(Caption("Partir d’un original"),sourceLevel,Button("Créer une copie",()=>Safe(NewFromVanilla),240),Button("Dupliquer ma sélection",()=>Safe(Duplicate),240),includeRooms);create.Dock=DockStyle.Top;create.Height=195;library.Controls.Add(create);

        levels.SelectedIndexChanged+=(sender,e)=>{
            if(updating || levels.SelectedItem is not LevelProject l || l.Id==current?.Id)return;
            if(ConfirmDiscard())LoadLevel(l);else{updating=true;levels.SelectedItem=levels.Items.Cast<LevelProject>().FirstOrDefault(p=>p.Id==current?.Id);updating=false;}
        };
        foreach(var source in rom.Templates)sourceLevel.Items.Add(source);sourceLevel.SelectedIndex=0;

        var inspector=new TabControl{Dock=DockStyle.Fill};

        var objectsPage=new TabPage("Objets");objectsPage.Controls.Add(sceneObjects);objectsPage.Controls.Add(sceneSearch);inspector.TabPages.Add(objectsPage);

        sceneSearch.TextChanged+=(s,e)=>RefreshSceneObjects();

        sceneObjects.DrawItem+=DrawSceneObject;sceneObjects.SelectedIndexChanged+=(s,e)=>{if(sceneUpdating)return;var items=sceneObjects.SelectedItems.Cast<ObjectRow>().Select(r=>r.Item).ToArray();canvas.CancelObjectGesture();canvas.SetObjectSelection(items,items.LastOrDefault(),false);if(canvas.Selected!=null)canvas.CenterOn(canvas.Selected);ShowSelection();};

        var palettePage=new TabPage("Ajouter");inspector.TabPages.Add(palettePage);palette.DrawMode=DrawMode.OwnerDrawFixed;palette.ItemHeight=43;palette.DropDownHeight=400;palette.Width=238;palette.DrawItem+=DrawPaletteItem;

        objectSearch.Width=238;objectCategory.Width=238;objectDetails.Dock=DockStyle.None;objectDetails.Width=238;objectDetails.Height=138;

        palettePage.Controls.Add(Column(Caption("Trouver un objet"),objectSearch,objectCategory,palette,Button("Ajouter au centre de la vue",()=>Safe(AddObject),238),Button("Remplacer le fruit sélectionné",()=>Safe(ReplaceSelectedFruit),238),objectDetails));

        var propertiesPage=new TabPage("Modifier");inspector.TabPages.Add(propertiesPage);selection.Dock=DockStyle.None;selection.Width=235;selection.Height=45;

        variantChoice.Items.Add("Couleur native");foreach(var name in new[]{"Orange","Violet","Turquoise","Lime-céleri","Brun","Vert foncé"})variantChoice.Items.Add(name);variantChoice.SelectedIndex=0;

        variantChoice.SelectedIndexChanged+=(s,e)=>{if(updating || canvas.SelectedObjects.Count!=1 || canvas.Selected is not Placement p || !ObjectCatalog.IsMaskass(p.ObjectId))return;Remember();p.HybridVariant=variantChoice.SelectedIndex==0?null:variantChoice.SelectedIndex-1;dirty=true;canvas.Invalidate();RefreshSceneObjects();};

        propertiesPage.Controls.Add(Column(selection,Caption("Position X"),x,Caption("Position Y (vers le bas)"),y,Caption("Couleur du Maskass"),variantChoice,Button("Rétablir le point d’entrée",()=>Safe(RestoreNativeSpawn),235),Button("Dupliquer la sélection",()=>Safe(DuplicateObject),235),Button("Retirer la sélection",()=>Safe(RemoveObject),235),new Label{Width=235,Height=95,Text="Choisis une salle au-dessus de la vue. L’onglet Terrain permet de peindre les tuiles et leurs collisions.",Padding=new(0,14,0,0)}));

        BuildTerrainPanels(inspector);

        var center=new Panel{Dock=DockStyle.Fill,Padding=new(8,0,8,0)};center.Controls.Add(canvas);

        snapChoice.Items.AddRange(new object[]{"Sans aimantation","Grille 16","Grille 32","Grille 64"});snapChoice.SelectedIndex=0;

        gridChoice.CheckedChanged+=(s,e)=>{canvas.ShowGrid=gridChoice.Checked;canvas.Invalidate();};technicalChoice.CheckedChanged+=(s,e)=>{canvas.ShowTechnical=technicalChoice.Checked;canvas.Invalidate();RefreshSceneObjects();};snapChoice.SelectedIndexChanged+=(s,e)=>canvas.SnapSize=snapChoice.SelectedIndex switch{1=>16,2=>32,3=>64,_=>0};

        var groupTools=BuildGroupTools();center.Controls.Add(groupTools);
        var tools=Bar(roomChoice,Button("Vue d’ensemble",canvas.Fit,145),Button("Annuler",Undo,90),Button("Rétablir",Redo,90),gridChoice,snapChoice,technicalChoice);center.Controls.Add(tools);

        var columns=new TableLayoutPanel{Dock=DockStyle.Fill,ColumnCount=3,RowCount=1};columns.ColumnStyles.Add(new(SizeType.Absolute,270));columns.ColumnStyles.Add(new(SizeType.Percent,100));columns.ColumnStyles.Add(new(SizeType.Absolute,275));columns.Controls.Add(library,0,0);columns.Controls.Add(center,1,0);columns.Controls.Add(inspector,2,0);page.Controls.Add(columns);

        levelName.Width=260;page.Controls.Add(sourceSummary);page.Controls.Add(Bar(levelName,Button("Enregistrer",()=>Safe(SaveLevel),125),Button("Tester mon niveau",()=>Safe(TestLevel),160),Button("Icône de l’aperçu",()=>Safe(CapturePreview),160),Button("Importer une icône",()=>Safe(ImportIcon),170),Button("Capture du jeu · F8",()=>status.Text="Dans la fenêtre de ton test, appuie sur F8 : la capture devient l’icône du niveau joué.",170)));

        objectSearch.TextChanged+=(s,e)=>RefreshPalette();objectCategory.SelectedIndexChanged+=(s,e)=>RefreshPalette();palette.SelectedIndexChanged+=(s,e)=>{if(palette.SelectedItem is PaletteItem item)objectDetails.Text=ObjectCatalog.Get(item.Id).Description;};

        x.ValueChanged+=(s,e)=>UpdateCoordinate(true);y.ValueChanged+=(s,e)=>UpdateCoordinate(false);
        levelName.TextChanged+=(s,e)=>{if(!updating && current!=null){current.Name=levelName.Text;dirty=true;levels.Invalidate();}};
        RefreshExtendedCategories();

    }

    void RefreshSourceSummary(){if(current!=null)sourceSummary.Text=$"Source {VanillaRom.SourceCode(current.SourceZone)} · {current.Rooms.Count+1} salles · {RoomLabel(activeRoom)} · {SceneObjects.Count} objets. Maj-clic : sélection multiple | rectangle sur le vide | clic droit : parcourir | molette : zoomer.";}

    void RefreshExtendedCategories(){

        paletteUpdating=true;string? selected=objectCategory.SelectedItem?.ToString();objectCategory.Items.Clear();objectCategory.Items.Add("Toutes les catégories");

        foreach(var cat in rom.Palette(current==null?43:activeRoom,specialRunner.Available).Select(id=>ObjectCatalog.Get(id).Category).Distinct().Order())objectCategory.Items.Add(cat);

        objectCategory.SelectedItem=selected;if(objectCategory.SelectedIndex<0)objectCategory.SelectedIndex=0;paletteUpdating=false;RefreshExtendedPalette();

        RefreshSourceSummary();

    }

    void RefreshExtendedPalette(){

        if(paletteUpdating)return;ushort? selected=(palette.SelectedItem as PaletteItem)?.Id;palette.Items.Clear();string query=objectSearch.Text.Trim(),category=objectCategory.SelectedItem?.ToString()??"Toutes les catégories";

        foreach(var id in rom.Palette(current==null?43:activeRoom,specialRunner.Available)){

            var info=ObjectCatalog.Get(id);if(category!="Toutes les catégories" && info.Category!=category)continue;if(query.Length>0 && !info.DisplayName.Contains(query,StringComparison.CurrentCultureIgnoreCase))continue;palette.Items.Add(new PaletteItem(id,info.DisplayName));

        }

        palette.SelectedItem=palette.Items.Cast<PaletteItem>().FirstOrDefault(p=>p.Id==selected);if(palette.SelectedIndex<0 && palette.Items.Count>0)palette.SelectedIndex=0;if(palette.Items.Count==0)objectDetails.Text="Aucun objet compatible ne correspond à cette recherche.";

    }

    sealed record ObjectRow(Placement Item,string Text){public override string ToString()=>Text;}

    void RefreshSceneObjects(){
        sceneUpdating=true;string query=sceneSearch.Text.Trim();sceneObjects.Items.Clear();
        if(current!=null)foreach(var p in SceneObjects){if(!technicalChoice.Checked && p.ObjectId>=0x8000)continue;string text=ObjectCatalog.Get(ObjectCatalog.PaletteId(p)).DisplayName;if(query.Length>0 && !text.Contains(query,StringComparison.CurrentCultureIgnoreCase))continue;sceneObjects.Items.Add(new ObjectRow(p,text));}
        SyncSceneSelection();sceneUpdating=false;RefreshPassages();
    }
    void SyncSceneSelection(){bool before=sceneUpdating;sceneUpdating=true;for(int i=0;i<sceneObjects.Items.Count;i++)sceneObjects.SetSelected(i,canvas.SelectedObjects.Contains(((ObjectRow)sceneObjects.Items[i]).Item));sceneUpdating=before;}

    void DrawSceneObject(object? sender,DrawItemEventArgs e){if(e.Index<0 || e.Index>=sceneObjects.Items.Count || sceneObjects.Items[e.Index] is not ObjectRow row)return;e.DrawBackground();EditorVisuals.Draw(e.Graphics,new(e.Bounds.X+6,e.Bounds.Y+5,31,31),row.Item);TextRenderer.DrawText(e.Graphics,row.Text,Font,new Rectangle(e.Bounds.X+45,e.Bounds.Y+5,e.Bounds.Width-48,33),e.ForeColor,TextFormatFlags.EndEllipsis|TextFormatFlags.VerticalCenter);e.DrawFocusRectangle();}

    void DrawPaletteItem(object? sender,DrawItemEventArgs e){if(e.Index<0 || e.Index>=palette.Items.Count || palette.Items[e.Index] is not PaletteItem item)return;e.DrawBackground();EditorVisuals.Draw(e.Graphics,new(e.Bounds.X+4,e.Bounds.Y+5,30,30),new Placement{ObjectId=item.Id});TextRenderer.DrawText(e.Graphics,item.Text,Font,new Rectangle(e.Bounds.X+42,e.Bounds.Y+4,e.Bounds.Width-44,34),e.ForeColor,TextFormatFlags.EndEllipsis|TextFormatFlags.VerticalCenter);e.DrawFocusRectangle();}

    void EditorKeys(object? sender,KeyEventArgs e){
        if(e.Control && e.KeyCode==Keys.S){Safe(SaveLevel);e.SuppressKeyPress=true;return;}
        if(!canvas.ContainsFocus && !sceneObjects.ContainsFocus)return;
        Action? history=e.Control && e.KeyCode==Keys.Z?Undo:e.Control && e.KeyCode==Keys.Y?Redo:null;
        if(history!=null){canvas.CancelObjectGesture();canvas.CancelTerrainGesture();history();e.SuppressKeyPress=true;return;}
        if(canvas.ContainsFocus && HandleTerrainSectionKeys(e))return;
        if(canvas.TerrainTool!=TerrainTool.Objects)return;
        Action? action=e.Control && e.KeyCode==Keys.A?SelectAllSceneObjects:e.Control && e.KeyCode==Keys.C?()=>Safe(CopySelectedObjects):e.Control && e.KeyCode==Keys.V?()=>Safe(PasteSelectedObjects):e.Control && e.KeyCode==Keys.X?()=>Safe(CutSelectedObjects):e.Control && e.KeyCode==Keys.D?()=>Safe(DuplicateObject):e.KeyCode==Keys.Delete?()=>Safe(RemoveObject):null;
        if(action!=null){canvas.CancelObjectGesture();action();e.SuppressKeyPress=true;return;}
        if(!canvas.ContainsFocus)return;int step=e.Shift?16:1;
        if(e.KeyCode is Keys.Left or Keys.Right or Keys.Up or Keys.Down){canvas.CancelObjectGesture();canvas.Nudge(e.KeyCode==Keys.Left?-step:e.KeyCode==Keys.Right?step:0,e.KeyCode==Keys.Up?-step:e.KeyCode==Keys.Down?step:0);e.SuppressKeyPress=true;}
    }

    void StartCustomGame(bool resume){if(game==null)throw new InvalidOperationException("Créer ou choisir une partie.");EnsureGameClosed();if(!resume && dirty && current!=null && game.LevelIds.Contains(current.Id))SaveLevel();SaveGame();sessions.Start(game,store,resume);RefreshSessionStatus();status.Text=resume?"Reprise de la partie personnalisée.":"Partie ouverte : Start, puis choisis ton Yoshi. Les victoires enchaînent les niveaux.";}

    void RefreshSessionStatus(){if(game==null)return;try{var state=sessions.State?.GameId==game.Id?sessions.State:sessions.Read(game.Id);sessionStatus.Text=state==null?"Aucune traversée commencée pour cette partie.":$"{state.GameName} · {state.Index}/{state.Levels.Count} niveaux terminés · {(state.Phase=="completed"?"Terminée":state.Phase=="running"?"En cours":"Reprenable")}";}catch(Exception e){sessionStatus.Text=e.Message;}}

    public void RunIntegrationChecks(){
        if(!pcRoot.Contains("validation",StringComparison.OrdinalIgnoreCase))throw new InvalidOperationException("Utiliser une copie de validation isolée.");
        if(sourceLevel.Items.Count!=24 || string.IsNullOrWhiteSpace(sourceLevel.Text))throw new Exception("Bibliothèque des 24 sources non affichée.");
        sourceLevel.SelectedItem=sourceLevel.Items.Cast<LevelTemplate>().Single(t=>t.SourceZone==43);NewFromVanilla();
        int initial=current!.Objects.Count;objectCategory.SelectedItem="Maskass hybrides";objectSearch.Text="turquoise";RefreshPalette();
        palette.SelectedItem=palette.Items.Cast<PaletteItem>().Single(i=>i.Id==0x4F12);AddObject();
        if(current.Objects.Count!=initial+1 || canvas.Selected?.HybridVariant!=2)throw new Exception("Ajout du Maskass turquoise incorrect.");
        Undo();if(current!.Objects.Count!=initial)throw new Exception("Annulation incorrecte.");Redo();if(current!.Objects.Count!=initial+1)throw new Exception("Rétablissement incorrect.");
        canvas.Selected=current.Objects.Single(p=>p.HybridVariant==2);ShowSelection();variantChoice.SelectedIndex=6;if(canvas.Selected.HybridVariant!=5)throw new Exception("Modification de couleur incorrecte.");
        float before=canvas.Selected.X;x.Value=(decimal)before+16;if(canvas.Selected.X!=before+16)throw new Exception("Coordonnée incorrecte.");
        SaveLevel();string first=current.Id;CapturePreview();if(!File.Exists(store.IconPath(first)))throw new Exception("Icône absente.");
        sourceLevel.SelectedItem=sourceLevel.Items.Cast<LevelTemplate>().Single(t=>t.SourceZone==46);NewFromVanilla();if(current!.SourceZone!=46)throw new Exception("Mauvaise source clonée.");string second=current.Id;
        game=new CustomGame{Name="Traversée de validation",LevelIds=[first,second]};gameName.Text=game.Name;SaveGame();
        if(store.Games().Single(g=>g.Id==game!.Id).LevelIds.Count!=2)throw new Exception("Ordre de partie perdu.");
        var saved=store.Levels().Single(l=>l.Id==first);if(saved.Objects.Single(p=>p.HybridVariant.HasValue).HybridVariant!=5)throw new Exception("Couleur non sauvegardée.");
        CheckCustomGameEditing();
        CheckRoomEditorIntegration();CheckTerrainIntegration();CheckTerrainPaintingIntegration();
        File.WriteAllText(Path.Combine(pcRoot,"build/editor-0150/ui-integration.json"),System.Text.Json.JsonSerializer.Serialize(new{sourceChoices=24,paletteSearch=true,maskassVariantAddEditSave=true,undoRedo=true,coordinateEdit=true,previewIcon=true,secondSourceClone=true,customGameOrder=true,isolatedRoot=pcRoot},ProjectStore.Json));
    }
    public void RenderPreview(string path){Directory.CreateDirectory(Path.GetDirectoryName(path)!);using var bitmap=new Bitmap(Width,Height);DrawToBitmap(bitmap,new Rectangle(0,0,Width,Height));bitmap.Save(path,ImageFormat.Png);}



    const int CaptureKey=0xD130;

    [DllImport("user32.dll",SetLastError=true)]static extern bool RegisterHotKey(IntPtr hwnd,int id,uint modifiers,uint key);

    [DllImport("user32.dll")]static extern bool UnregisterHotKey(IntPtr hwnd,int id);

    [DllImport("user32.dll")]static extern IntPtr GetForegroundWindow();

    [DllImport("user32.dll")]static extern bool GetClientRect(IntPtr hwnd,out NativeRect rect);

    [DllImport("user32.dll")]static extern bool ClientToScreen(IntPtr hwnd,ref NativePoint point);

    [StructLayout(LayoutKind.Sequential)]struct NativeRect{public int Left,Top,Right,Bottom;}

    [StructLayout(LayoutKind.Sequential)]struct NativePoint{public int X,Y;}

    readonly System.Windows.Forms.Timer captureTimer=new(){Interval=200};
    void RefreshCaptureHotkey(){var p=specialRunner.Running;bool own=false;try{if(p is {HasExited:false}){p.Refresh();own=p.MainWindowHandle!=IntPtr.Zero && GetForegroundWindow()==p.MainWindowHandle;}}catch(InvalidOperationException){}if(own && !hotkeyRegistered)RegisterCaptureHotkey();else if(!own && hotkeyRegistered)UnregisterCaptureHotkey();}
    void RegisterCaptureHotkey(){hotkeyRegistered=RegisterHotKey(Handle,CaptureKey,0x4000,(uint)Keys.F8);if(!hotkeyRegistered)status.Text="F8 est déjà utilisé ; tu peux importer une capture comme icône.";}

    void UnregisterCaptureHotkey(){if(hotkeyRegistered){UnregisterHotKey(Handle,CaptureKey);hotkeyRegistered=false;}}

    protected override void WndProc(ref Message m){if(m.Msg==0x0312 && m.WParam.ToInt32()==CaptureKey){Safe(CaptureGameIcon);return;}base.WndProc(ref m);}

    void CaptureGameIcon(){

        var process=specialRunner.Running;string? id=sessions.CurrentLevelId??activeSoloId;

        if(process==null || process.HasExited || id==null)throw new InvalidOperationException("Lance d’abord le test d’un niveau de cet Atelier.");

        process.Refresh();IntPtr hwnd=process.MainWindowHandle;if(hwnd==IntPtr.Zero || GetForegroundWindow()!=hwnd){status.Text="Reviens dans la fenêtre de ton test puis appuie sur F8.";return;}

        if(!GetClientRect(hwnd,out var rect))throw new InvalidOperationException("Image du jeu indisponible.");int width=rect.Right-rect.Left,height=rect.Bottom-rect.Top;if(width<64 || height<64 || width>8192 || height>8192)throw new InvalidOperationException("Taille de fenêtre non prise en charge.");var origin=new NativePoint();if(!ClientToScreen(hwnd,ref origin))throw new InvalidOperationException("Position du jeu indisponible.");

        using var frame=new Bitmap(width,height);using(var g=Graphics.FromImage(frame))g.CopyFromScreen(origin.X,origin.Y,0,0,new Size(width,height),CopyPixelOperation.SourceCopy);

        using var icon=new Bitmap(320,180);using(var g=Graphics.FromImage(icon)){g.Clear(Color.Black);float scale=Math.Min(320f/width,180f/height);g.InterpolationMode=System.Drawing.Drawing2D.InterpolationMode.HighQualityBicubic;g.DrawImage(frame,(320-width*scale)/2,(180-height*scale)/2,width*scale,height*scale);}

        var level=store.Levels().FirstOrDefault(l=>l.Id==id)??throw new InvalidOperationException("Le niveau joué n’est plus dans la bibliothèque.");icon.Save(store.IconPath(id),ImageFormat.Png);level.Icon=Path.GetFileName(store.IconPath(id));store.Save(level);if(current?.Id==id)current.Icon=level.Icon;levels.Invalidate();gameLevels.Invalidate();status.Text="Capture du jeu enregistrée comme icône : "+level.Name;

    }

}

