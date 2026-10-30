using YoshiWorkshop;

using System.Text.Json;



namespace WorkshopApp;

static class Program {

    [STAThread] static void Main(string[] args) {

        ApplicationConfiguration.Initialize();

        var directory=new DirectoryInfo(AppContext.BaseDirectory);

        while(!File.Exists(Path.Combine(directory.FullName,"upstream-lock.json")) && !File.Exists(Path.Combine(directory.FullName,"portable-package.json")) && directory.Parent!=null)directory=directory.Parent;

        string root=args.Length>0 && !args[0].StartsWith("--")?Path.GetFullPath(args[0]):directory.FullName;

        bool check=args.Contains("--startup-check");

        Application.SetUnhandledExceptionMode(UnhandledExceptionMode.ThrowException);

        try {

            int importArgument=Array.IndexOf(args,"--import-rom-file");
            if(importArgument>=0){if(importArgument+1>=args.Length)throw new ArgumentException("ROM path missing.");Console.WriteLine(PublicRomInstall.Install(root,args[importArgument+1]));return;}
            if(args.Contains("--setup-rom") || PublicRomInstall.Needed(root)){
                using var importer=new RomImportForm(root);if(importer.ShowDialog()!=DialogResult.OK){Environment.ExitCode=2;return;}
                if(args.Contains("--play-after-setup")){var start=new System.Diagnostics.ProcessStartInfo(Path.Combine(root,"build/native-special/DinoTale.exe")){WorkingDirectory=Path.Combine(root,"profiles/content-personal"),UseShellExecute=false};start.ArgumentList.Add("--workshop-main-menu");System.Diagnostics.Process.Start(start);return;}
                if(args.Contains("--setup-rom"))return;
            }

            if(args.Contains("--test-yoshis")){new ContentPortTest(root).Launch();return;}

            if(args.Contains("--personal-yoshis")){new ContentPortTest(root).Launch(personal:true);return;}

            if(args.Contains("--test-special-fruits")){new SpecialContentRunner(root).LaunchDemo();return;}

            if(args.Contains("--prepare-example")){

                var store=new ProjectStore(Path.Combine(root,"content"));

                var rom=new VanillaRom(new SpecialContentRunner(root).OriginalRom);

                var level=rom.CloneFirstLevel();level.Id="11111111111141118111111111111111";level.Name="Mon premier 1-1";

                level.Objects.First(p=>p.ObjectId==0x4030).X=200;

                if(!File.Exists(store.LevelPath(level.Id))){

                    store.Save(level);using var canvas=new LevelCanvas {Width=960,Height=540,Level=level};canvas.Fit();

                    using var image=canvas.Snapshot();image.Save(store.IconPath(level.Id),System.Drawing.Imaging.ImageFormat.Png);

                    level.Icon=Path.GetFileName(store.IconPath(level.Id));store.Save(level);

                }

                var game=new CustomGame {Id="22222222222242228222222222222222",Name="Ma première partie",LevelIds=[level.Id]};

                if(!store.Games().Any(g=>g.Id==game.Id))store.Save(game);

                return;

            }

            var form=new WorkshopWindow(root);

            if(args.Contains("--editor") || args.Contains("--custom-games"))form.SelectSection(args.Contains("--custom-games"));

            form.Shown+=(s,e)=>{

                var names=form.SectionNames;

                if(names.Length!=5 || form.ClientSize.Width<900 || form.ClientSize.Height<600)throw new InvalidOperationException("Fenêtre initiale invalide.");

                Directory.CreateDirectory(Path.Combine(root,"build"));

                File.WriteAllText(Path.Combine(root,"build/ui-startup-verification.json"),JsonSerializer.Serialize(new {windowCreated=true,shown=true,form.ClientSize.Width,form.ClientSize.Height,menus=names,visualReview="pending",mode=check?"startup-check":"interactive"},ProjectStore.Json));

                if(args.Contains("--render-editor-preview")){

                    form.SelectSection(false);form.BeginInvoke(()=>{if(args.Contains("--editor-integration-check"))form.RunIntegrationChecks();form.RenderPreview(Path.Combine(root,"build/editor-0150/atelier-preview.png"));form.Close();});

                }else if(check)form.BeginInvoke(()=>form.Close());

            };

            Application.Run(form);

        }

        catch(Exception e) {

            Directory.CreateDirectory(Path.Combine(root,"build"));

            File.WriteAllText(Path.Combine(root,"build/ui-startup-error.txt"),e.ToString());

            Environment.ExitCode=1;

            if(!check)MessageBox.Show(e.Message,"Atelier PC",MessageBoxButtons.OK,MessageBoxIcon.Error);

        }

    }

}

