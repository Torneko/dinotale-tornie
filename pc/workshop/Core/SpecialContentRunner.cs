using System.Diagnostics;

using System.Security.Cryptography;

using System.Text.Json;

namespace YoshiWorkshop;



public sealed class SpecialContentRunner(string root) {

    public Process? Running {get;private set;}

    public bool IsRunning=>Running is {HasExited:false};

    static string Hash(string path)=>Convert.ToHexStringLower(SHA256.HashData(File.ReadAllBytes(path)));

    public static string EngineFileName=>OperatingSystem.IsLinux()?"DinoTale":"DinoTale.exe";

    public string RuntimeDirectory {

        get {

            if(File.Exists(Path.Combine(root,"portable-package.json")))return Path.Combine(root,"build/native-special");

            foreach(string name in new[]{"native-special-0.15.0-verified","native-special-0.15.0","native-special-0.14.0-verified","native-special-0.14.0","native-special-0.13.0-verified","native-special-0.13.0","native-special-0.12.5-verified","native-special-0.7.4","native-special-0.7.1","native-special"}){

                string path=Path.Combine(root,"build",name);if(File.Exists(Path.Combine(path,"engine-build.json")) && File.Exists(Path.Combine(path,EngineFileName)))return path;

            }

            return Path.Combine(root,"build/native-special");

        }

    }

    static bool mval(JsonElement m,out int version){version=m.TryGetProperty("customLevelVersion",out var v)?v.GetInt32():0;return version>0;}
    public bool Available=>File.Exists(Path.Combine(RuntimeDirectory,"engine-build.json"));

    public string OriginalRom {

        get {

            string portable=Path.Combine(root,"profiles/content-personal/yoshisstory.n64.us.z64");

            return File.Exists(portable)?portable:Path.GetFullPath(Path.Combine(root,"../work/yoshis-story-decomp/baseroms/us/baserom.z64"));

        }

    }

    public void Verify(){

        using var manifest=JsonDocument.Parse(File.ReadAllText(Path.Combine(RuntimeDirectory,"engine-build.json")));var m=manifest.RootElement;

        foreach(var f in new[]{(EngineFileName,"exeSha256"),("workshop-content.bin","contentSha256")})if(Hash(Path.Combine(RuntimeDirectory,f.Item1))!=m.GetProperty(f.Item2).GetString())throw new InvalidDataException("Le moteur ne correspond plus à sa version vérifiée.");

    }

    public string Prepare(string id,byte[] placements,bool demo=false,bool game=false){

        if(IsRunning)throw new InvalidOperationException("Fermer le jeu avant de lancer un autre niveau.");

        if(!demo)ProjectStore.ValidateId(id);Verify();

        if(placements.Length<16)throw new InvalidDataException("Fichier de placements incomplet.");

        using(var manifest=JsonDocument.Parse(File.ReadAllText(Path.Combine(RuntimeDirectory,"engine-build.json")))){

            if(placements[7]=='3' && (!mval(manifest.RootElement,out int version3) || version3<3))throw new InvalidDataException("Cette création à plusieurs salles nécessite le moteur 0.14.0.");
            if((placements[7]=='2' || System.Buffers.Binary.BinaryPrimitives.ReadUInt32BigEndian(placements.AsSpan(8))!=43) && (!manifest.RootElement.TryGetProperty("customLevelVersion",out var version) || version.GetInt32()<2))throw new InvalidDataException("Cette création nécessite le moteur 0.13.0 ou plus récent.");

        }

        if(Hash(OriginalRom)!=VanillaRom.ExpectedSha256)throw new InvalidDataException("La ROM originale USA est requise.");

        string profile=Path.Combine(root,"profiles",demo?"special-content-demo":(game?"custom-game-":"custom-content-")+id);Directory.CreateDirectory(profile);

        if(demo){

            string target=Path.Combine(profile,"saves/yoshisstory.n64.us.bin"),fixture=Path.Combine(root,"../work/experiments/extension-v9-2/test-tous-yoshis.eep");

            if(Hash(fixture)!=ContentPortTest.FixtureSha256)throw new InvalidDataException("Sauvegarde de démonstration incorrecte.");Directory.CreateDirectory(Path.GetDirectoryName(target)!);if(!File.Exists(target))File.Copy(fixture,target);

        }else SpecialSaveSeed.Initialize(root,profile);

        foreach(string name in new[]{"graphics.json","controls.json","general.json","sound.json","book-menu.json"}){

            string source=Path.Combine(root,"profiles/content-personal",name),target=Path.Combine(profile,name);if(!File.Exists(target) && File.Exists(source))File.Copy(source,target);

        }

        if(!File.Exists(Path.Combine(profile,"sound.json")))File.WriteAllText(Path.Combine(profile,"sound.json"),"{\"main_volume\":10}");

        File.Copy(OriginalRom,Path.Combine(profile,"yoshisstory.n64.us.z64"),true);File.WriteAllText(Path.Combine(profile,"portable.txt"),"");File.WriteAllBytes(Path.Combine(profile,"level.yslevel"),placements);

        foreach(var source in Directory.GetFiles(Path.Combine(RuntimeDirectory,"assets"),"*",SearchOption.AllDirectories)){

            string target=Path.Combine(profile,"assets",Path.GetRelativePath(Path.Combine(RuntimeDirectory,"assets"),source));Directory.CreateDirectory(Path.GetDirectoryName(target)!);File.Copy(source,target,true);

        }

        File.WriteAllText(Path.Combine(profile,"content-profile.json"),JsonSerializer.Serialize(new{purpose=demo?"Six special fruit demo":game?"Custom game":"Custom level",testUnlocks=demo,newFruitGameplay=true,newMaskassGameplay=placements[7]=='2'||placements[7]=='3',sourceZone=System.Buffers.Binary.BinaryPrimitives.ReadUInt32BigEndian(placements.AsSpan(8))},ProjectStore.Json));return profile;

    }

    public Process LaunchPrepared(string profile,string? token=null,bool continuation=false,int preferred=-1,IEnumerable<string>? diagnosticArgs=null){

        if(IsRunning)throw new InvalidOperationException("Une session est déjà ouverte.");

        string allowed=Path.GetFullPath(Path.Combine(root,"profiles"))+Path.DirectorySeparatorChar;

        if(!Path.GetFullPath(profile).StartsWith(allowed,StringComparison.OrdinalIgnoreCase))throw new InvalidDataException("Profil de session invalide.");

        var start=new ProcessStartInfo(Path.Combine(RuntimeDirectory,EngineFileName)){WorkingDirectory=profile,UseShellExecute=false};

        foreach(var arg in new[]{"--workshop-main-menu","--workshop-start-mode","1","--workshop-level",Path.Combine(profile,"level.yslevel")})start.ArgumentList.Add(arg);

        if(token!=null){ProjectStore.ValidateId(token);start.ArgumentList.Add("--workshop-custom-token");start.ArgumentList.Add(token);}

        if(continuation)start.ArgumentList.Add("--workshop-custom-continue");

        if(preferred>=0){start.ArgumentList.Add("--workshop-preferred-color");start.ArgumentList.Add(preferred.ToString());}

        if(diagnosticArgs!=null)foreach(var arg in diagnosticArgs)start.ArgumentList.Add(arg);

        Running?.Dispose();Running=Process.Start(start)??throw new InvalidOperationException("Le moteur n’a pas démarré.");return Running;

    }

    public void Launch(string id,byte[] placements,bool demo=false){string profile=Prepare(id,placements,demo);LaunchPrepared(profile,Guid.NewGuid().ToString("N"));}

    public void LaunchDemo()=>Launch("demo",File.ReadAllBytes(Path.Combine(root,"build/special-port/six-fruits-demo.yslevel")),true);

}



public static class SpecialSaveSeed {

    public static bool Initialize(string root,string profile){

        string target=Path.Combine(profile,"saves","yoshisstory.n64.us.bin");

        if(File.Exists(target))return false;

        string personal=Path.Combine(root,"profiles","content-personal","saves","yoshisstory.n64.us.bin");

        if(!File.Exists(personal))return ContentSaveSeed.Initialize(root,profile);

        byte[] bytes=File.ReadAllBytes(personal);

        if(bytes.Length!=2048 || !bytes.AsSpan(0,1024).SequenceEqual(bytes.AsSpan(1024)) || !bytes.AsSpan(1020,4).SequenceEqual(new byte[]{0x81,0x31,0x75,0x39}))

            throw new InvalidDataException("Fermer le jeu pour copier une sauvegarde personnelle complète.");

        Directory.CreateDirectory(Path.GetDirectoryName(target)!);

        // These ledgers are part of the same progression as the EEPROM.

        // Copy only when creating a profile; never replace a played custom save.

        foreach(string name in new[]{"pc-achievements.json","pc-collection.json","pc-hybrid-records.json","pc-hybrid-finds.json"}){

            string ledger=Path.Combine(Path.GetDirectoryName(Path.GetDirectoryName(personal)!)!,name);

            if(File.Exists(ledger))File.Copy(ledger,Path.Combine(profile,name),false);

        }

        using var output=new FileStream(target,FileMode.CreateNew,FileAccess.Write,FileShare.None);output.Write(bytes);output.Flush(true);return true;

    }

}

