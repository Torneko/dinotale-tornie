using System.Diagnostics;

using System.Text.Json;

namespace YoshiWorkshop;



public sealed class CustomLevelResult {

    public int Schema {get;set;}=1;

    public string Token {get;set;}="";

    public bool Completed {get;set;}

    public int Fruits {get;set;}

    public int SelectedColor {get;set;}

    public bool Advance {get;set;}=true;

}

public sealed class CustomSessionState {

    public int Schema {get;set;}=1;

    public string RunId {get;set;}=Guid.NewGuid().ToString("N");

    public string GameId {get;set;}="";

    public string GameName {get;set;}="";

    public string Phase {get;set;}="paused";

    public int ProcessId {get;set;}
    public int Index {get;set;}

    public int PreferredColor {get;set;}=-1;

    public string ExpectedToken {get;set;}="";

    public List<LevelProject> Levels {get;set;}=[];

    public List<CustomLevelResult> Results {get;set;}=[];

    public void Validate(){

        ProjectStore.ValidateId(RunId);ProjectStore.ValidateId(GameId);

        if(Schema!=1 || Levels.Count is <1 or >64 || Index<0 || Index>Levels.Count || Results.Count!=Index || PreferredColor is <-1 or >13 || Phase is not ("running" or "paused" or "completed"))throw new InvalidDataException("Suivi de partie invalide ; fichier conservé.");

        if((Phase=="completed")!=(Index==Levels.Count) || ProcessId<0)throw new InvalidDataException("État de reprise incohérent.");
        foreach(var r in Results)ProjectStore.ValidateId(r.Token);
        if(ExpectedToken.Length>0)ProjectStore.ValidateId(ExpectedToken);

        foreach(var level in Levels)ProjectStore.Validate(level);

        if(Results.Any(r=>r.Schema!=1 || !r.Completed || r.Fruits!=30 || r.SelectedColor is <0 or >13) || Results.Select(r=>r.Token).Distinct().Count()!=Results.Count)throw new InvalidDataException("Résultats personnalisés invalides.");

    }

    public bool Commit(CustomLevelResult r){

        Validate();

        if(Phase!="running" || Index>=Levels.Count || r.Schema!=1 || r.Token!=ExpectedToken || !r.Completed || r.Fruits!=30 || r.SelectedColor is <0 or >13 || Results.Any(x=>x.Token==r.Token))return false;

        Results.Add(r);Index++;PreferredColor=r.SelectedColor;ExpectedToken="";Phase=Index==Levels.Count?"completed":"paused";return true;

    }

}

public sealed class CustomGameSession(string root,SpecialContentRunner runner,VanillaRom rom) {

    readonly object gate=new();

    public CustomSessionState? State {get;private set;}

    public event Action? Changed;

    public event Action<string>? Error;

    // Bounded automation is restricted to disposable validation roots.
    public Func<int,IEnumerable<string>>? DiagnosticArguments {get;set;}

    public bool IsRunning=>runner.IsRunning;

    public string? CurrentLevelId {get{lock(gate)return State is {Phase:"running"} s && s.Index<s.Levels.Count?s.Levels[s.Index].Id:null;}}

    public string SessionPath(string gameId){ProjectStore.ValidateId(gameId);return Path.Combine(root,"profiles","custom-game-"+gameId,"custom-session.json");}

    void Persist(){

        State!.Validate();string path=SessionPath(State.GameId);Directory.CreateDirectory(Path.GetDirectoryName(path)!);string temp=path+".tmp";File.WriteAllText(temp,JsonSerializer.Serialize(State,ProjectStore.Json));if(File.Exists(path))File.Copy(path,path+".bak",true);File.Move(temp,path,true);

    }

    public CustomSessionState? Read(string gameId){string path=SessionPath(gameId);if(!File.Exists(path))return null;if(new FileInfo(path).Length>128*1024*1024)throw new InvalidDataException("Suivi personnalisé trop volumineux.");var s=JsonSerializer.Deserialize<CustomSessionState>(File.ReadAllText(path),ProjectStore.Json)??throw new InvalidDataException("Suivi personnalisé vide.");s.Validate();if(s.GameId!=gameId)throw new InvalidDataException("Le suivi appartient à une autre partie.");return s;}

    public void Start(CustomGame game,ProjectStore store,bool resume=false){

        lock(gate){

            if(IsRunning)throw new InvalidOperationException("Fermer le jeu avant de changer de partie.");

            ProjectStore.ValidateId(game.Id);

            var previous=Read(game.Id);
            if(previous is {ProcessId:>0}){
                try{using var old=Process.GetProcessById(previous.ProcessId);if(!old.HasExited && Path.GetFileNameWithoutExtension(old.MainModule?.FileName??"")=="DinoTale")throw new InvalidOperationException("Cette partie tourne encore ; ferme sa fenêtre avant de la reprendre.");}catch(ArgumentException){}
            }
            if(resume){State=Read(game.Id)??throw new InvalidOperationException("Aucune partie à reprendre.");if(State.Index>=State.Levels.Count)throw new InvalidOperationException("Cette partie est déjà terminée.");State.Phase="paused";
                string pending=Path.Combine(Path.GetDirectoryName(SessionPath(game.Id))!,"custom-run-result.json");
                if(State.ExpectedToken.Length>0 && File.Exists(pending) && new FileInfo(pending).Length<8192){
                    var result=JsonSerializer.Deserialize<CustomLevelResult>(File.ReadAllText(pending),ProjectStore.Json);
                    if(result!=null){State.Phase="running";State.Commit(result);State.Phase=State.Index==State.Levels.Count?"completed":"paused";Persist();}
                }
                if(State.Index>=State.Levels.Count){Changed?.Invoke();return;}
            }

            else{

                if(game.LevelIds.Count is <1 or >64)throw new InvalidOperationException("Ajouter entre 1 et 64 niveaux à la partie.");

                var known=store.Levels().ToDictionary(l=>l.Id);var levels=game.LevelIds.Select(id=>known.TryGetValue(id,out var l)?l:throw new InvalidDataException("Un niveau de la partie est introuvable.")).ToList();

                foreach(var l in levels)rom.ExportPlacements(l,true);

                var old=Read(game.Id);if(old!=null){string history=Path.Combine(Path.GetDirectoryName(SessionPath(game.Id))!,"history");Directory.CreateDirectory(history);File.WriteAllText(Path.Combine(history,old.RunId+".json"),JsonSerializer.Serialize(old,ProjectStore.Json));}

                State=new(){GameId=game.Id,GameName=game.Name,Levels=JsonSerializer.Deserialize<List<LevelProject>>(JsonSerializer.Serialize(levels,ProjectStore.Json),ProjectStore.Json)!};

            }

            StartNext();

        }

    }

    void StartNext(){

        if(DiagnosticArguments!=null && !Path.GetFullPath(root).Contains("validation",StringComparison.OrdinalIgnoreCase))throw new InvalidOperationException("Les diagnostics exigent un profil de validation.");
        var s=State!;s.ExpectedToken=Guid.NewGuid().ToString("N");s.Phase="running";Persist();

        string profile=runner.Prepare(s.GameId,rom.ExportPlacements(s.Levels[s.Index],true),game:true);

        string resultPath=Path.Combine(profile,"custom-run-result.json");if(File.Exists(resultPath))File.Move(resultPath,resultPath+".previous",true);

        string token=s.ExpectedToken;var process=runner.LaunchPrepared(profile,token,s.Index>0,s.PreferredColor,DiagnosticArguments?.Invoke(s.Index));

        s.ProcessId=process.Id;Persist();
        process.Exited+=(sender,e)=>Exited(process,profile,token);process.EnableRaisingEvents=true;Changed?.Invoke();

    }

    void Exited(Process process,string profile,string token){

        try{

            lock(gate){

                if(State==null || State.ExpectedToken!=token)return;

                State.ProcessId=0;bool advance=false;

                string path=Path.Combine(profile,"custom-run-result.json");

                if(process.ExitCode==0 && File.Exists(path) && new FileInfo(path).Length<8192){

                    var result=JsonSerializer.Deserialize<CustomLevelResult>(File.ReadAllText(path),ProjectStore.Json);

                    if(result!=null && State.Commit(result))advance=result.Advance && State.Phase!="completed";

                }

                if(State.Phase=="running")State.Phase="paused";Persist();Changed?.Invoke();if(advance)StartNext();

            }

        }catch(Exception e){lock(gate){if(State!=null){State.Phase=State.Index==State.Levels.Count?"completed":"paused";try{Persist();}catch{}}}Error?.Invoke(e.Message);}

    }

}

