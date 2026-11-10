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

            if(OperatingSystem.IsLinux())foreach(string name in new[]{"native-linux-0.34.0","native-linux-0340","native-solo-0.34.0","linux-runtime-clang15-0.34.0","linux-runtime-clang15-0.12.4"}){
                string path=Path.Combine(root,"build",name);if(File.Exists(Path.Combine(path,"engine-build.json")) && File.Exists(Path.Combine(path,EngineFileName)))return path;
            }

            foreach(string name in new[]{"native-solo-0.31.1-verified","native-solo-0.31.1","native-special-0.21.0-verified","native-special-0.21.0","native-special-0.20.0-verified","native-special-0.20.0","native-special-0.19.0-verified","native-special-0.19.0","native-special-0.18.0-verified","native-special-0.18.0","native-special-0.17.0-verified","native-special-0.17.0","native-special-0.16.0-verified","native-special-0.16.0","native-special-0.15.0-verified","native-special-0.15.0","native-special-0.14.0-verified","native-special-0.14.0","native-special-0.13.0-verified","native-special-0.13.0","native-special-0.12.5-verified","native-special-0.7.4","native-special-0.7.1","native-special"}){

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

    static int PayloadFeatures(byte[] b){
        if(b.Length<16 || !b.AsSpan(0,7).SequenceEqual("YSLEVEL"u8) || b[7]<'1' || b[7]>'6')throw new InvalidDataException("Format de niveau personnalisé invalide.");
        int p=16;bool multi=b[7]>='3';int found=0;int rooms=multi?checked((int)System.Buffers.Binary.BinaryPrimitives.ReadUInt32BigEndian(b.AsSpan(12))):1;
        if(rooms is <1 or >10)throw new InvalidDataException("Nombre de salles invalide.");
        for(int r=0;r<rooms;r++){
            int header=multi?(b[7]=='6'?32:b[7]=='5'?16:b[7]=='4'?12:8):0;
            if(p>b.Length-header)throw new InvalidDataException("Entête de salle incomplet.");
            int Word(int offset)=>System.Buffers.Binary.BinaryPrimitives.ReadInt32BigEndian(b.AsSpan(p+offset));
            int count=multi?Word(4):System.Buffers.Binary.BinaryPrimitives.ReadInt32BigEndian(b.AsSpan(12));int terrain=b[7]>='4'?Word(8):0;int routes=b[7]=='6'?Word(28):0;
            if(b[7]>='5' && Word(12)!=-1 && !RoomMusic.Valid(Word(12)))throw new InvalidDataException("Musique de salle invalide.");
            if(b[7]=='6'){
                int flags=Word(16);if(flags is not (0 or 1) || flags==0 && (Word(20)!=0 || Word(24)!=0) || flags==1 && (!NavigationEditing.ValidCoordinate(BitConverter.Int32BitsToSingle(Word(20))) || !NavigationEditing.ValidCoordinate(BitConverter.Int32BitsToSingle(Word(24)))))throw new InvalidDataException("Point d’entrée invalide.");found|=8;
            }
            p+=header;int stride=b[7]=='1'?12:16;
            if(count is <1 or >149 || terrain is <0 or >65536 || routes<0 || routes>count || p+(long)count*stride+(long)terrain*8+(long)routes*20>b.Length)throw new InvalidDataException("Placements incomplets.");
            for(int i=0;i<count;i++){ushort id=System.Buffers.Binary.BinaryPrimitives.ReadUInt16BigEndian(b.AsSpan(p+i*stride));if(ObjectCatalog.IsSpecialBubble(id))found|=1;if(SpecialEditorObjects.IsEgg(id))found|=2;if(id is 0x4F38 or 0x4F39)found|=16;if(id==0x45E3)found|=4;}
            p+=count*stride+terrain*8;var seen=new HashSet<int>();
            for(int i=0;i<routes;i++){
                int index=Word(0),flags=Word(8);if(index<0 || index>=count || !seen.Add(index) || flags is not (0 or 1) || flags==0 && (Word(12)!=0 || Word(16)!=0) || flags==1 && (!NavigationEditing.ValidCoordinate(BitConverter.Int32BitsToSingle(Word(12))) || !NavigationEditing.ValidCoordinate(BitConverter.Int32BitsToSingle(Word(16)))))throw new InvalidDataException("Arrivée de passage invalide.");p+=20;
            }
        }
        if(p!=b.Length)throw new InvalidDataException("Données de niveau supplémentaires inattendues.");return found;
    }

    public string Prepare(string id,byte[] placements,bool demo=false,bool game=false){

        if(IsRunning)throw new InvalidOperationException("Fermer le jeu avant de lancer un autre niveau.");

        if(!demo)ProjectStore.ValidateId(id);Verify();

        if(placements.Length<16)throw new InvalidDataException("Fichier de placements incomplet.");
        int features=PayloadFeatures(placements);

        using(var manifest=JsonDocument.Parse(File.ReadAllText(Path.Combine(RuntimeDirectory,"engine-build.json")))){

            if(placements[7]=='6' && (!mval(manifest.RootElement,out int version6) || version6<6 || !manifest.RootElement.TryGetProperty("navigationEditor",out var navigation) || navigation.ValueKind!=JsonValueKind.True))throw new InvalidDataException("Les passages et les points d’entrée nécessitent le moteur 0.21.0 ou plus récent.");
            if(placements[7]=='5' && (!mval(manifest.RootElement,out int version5) || version5<5))throw new InvalidDataException("Le choix de musique nécessite le moteur 0.20.0 ou plus récent.");
            if((features&4)!=0 && (!manifest.RootElement.TryGetProperty("nativeBlackCarrierEditor",out var carriers) || carriers.ValueKind!=JsonValueKind.True))throw new InvalidDataException("Le Maskass volant noir seul nécessite le moteur 0.20.0 ou plus récent.");
            if((features&2)!=0 && (!manifest.RootElement.TryGetProperty("editorSpecialEggs",out var eggs) || eggs.ValueKind!=JsonValueKind.True))throw new InvalidDataException("Les œufs spéciaux nécessitent le moteur 0.20.0 ou plus récent.");
            if((features&16)!=0)RequireMetallicPlacements(manifest.RootElement);
            if((features&1)!=0 && (!manifest.RootElement.TryGetProperty("specialFruitBubbles",out var bubbles) || bubbles.ValueKind!=JsonValueKind.True))throw new InvalidDataException("Les fruits spéciaux en bulle nécessitent le moteur 0.17.0 ou plus récent.");
            if(placements[7]>='4' && (!mval(manifest.RootElement,out int version4) || version4<4))throw new InvalidDataException("Le terrain modifié nécessite le moteur 0.16.0 ou plus récent.");
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

        if(!File.Exists(Path.Combine(profile,"sound.json")))File.WriteAllText(Path.Combine(profile,"sound.json"),"{\"main_volume\":3}");

        File.Copy(OriginalRom,Path.Combine(profile,"yoshisstory.n64.us.z64"),true);File.WriteAllText(Path.Combine(profile,"portable.txt"),"");File.WriteAllBytes(Path.Combine(profile,"level.yslevel"),placements);

        foreach(var source in Directory.GetFiles(Path.Combine(RuntimeDirectory,"assets"),"*",SearchOption.AllDirectories)){

            string target=Path.Combine(profile,"assets",Path.GetRelativePath(Path.Combine(RuntimeDirectory,"assets"),source));Directory.CreateDirectory(Path.GetDirectoryName(target)!);File.Copy(source,target,true);

        }

        File.WriteAllText(Path.Combine(profile,"content-profile.json"),JsonSerializer.Serialize(new{purpose=demo?"Six special fruit demo":game?"Custom game":"Custom level",testUnlocks=demo,newFruitGameplay=true,newMaskassGameplay=placements[7]>='2' && placements[7]<='6',sourceZone=System.Buffers.Binary.BinaryPrimitives.ReadUInt32BigEndian(placements.AsSpan(8))},ProjectStore.Json));return profile;

    }

    static void RequireMetallicPlacements(JsonElement manifest){
        if(!manifest.TryGetProperty("editorMetallicPlacements",out var supported)||supported.ValueKind!=JsonValueKind.True)throw new InvalidDataException("Les œufs et les Yoshis or/argent personnalisés nécessitent le moteur 0.31.1 ou plus récent.");
    }

    // Pure launch construction: capability/argument checks can be reviewed and
    // tested without starting a game. LaunchPrepared owns the actual process.
    public ProcessStartInfo PreparedStartInfo(string profile,string? token=null,bool continuation=false,int preferred=-1,IEnumerable<string>? diagnosticArgs=null){

        if(IsRunning)throw new InvalidOperationException("Une session est déjà ouverte.");
        if(preferred is <-1 or >15)throw new InvalidDataException("Couleur de Yoshi personnalisée invalide.");
        if(preferred>=14){using var manifest=JsonDocument.Parse(File.ReadAllText(Path.Combine(RuntimeDirectory,"engine-build.json")));RequireMetallicPlacements(manifest.RootElement);}

        string allowed=Path.GetFullPath(Path.Combine(root,"profiles"))+Path.DirectorySeparatorChar;

        if(!Path.GetFullPath(profile).StartsWith(allowed,OperatingSystem.IsWindows()?StringComparison.OrdinalIgnoreCase:StringComparison.Ordinal))throw new InvalidDataException("Profil de session invalide.");

        var start=new ProcessStartInfo(Path.Combine(RuntimeDirectory,EngineFileName)){WorkingDirectory=profile,UseShellExecute=false};

        foreach(var arg in new[]{"--workshop-main-menu","--workshop-start-mode","1","--workshop-level",Path.Combine(profile,"level.yslevel")})start.ArgumentList.Add(arg);

        if(token!=null){ProjectStore.ValidateId(token);start.ArgumentList.Add("--workshop-custom-token");start.ArgumentList.Add(token);}

        if(continuation)start.ArgumentList.Add("--workshop-custom-continue");

        if(preferred>=0){start.ArgumentList.Add("--workshop-preferred-color");start.ArgumentList.Add(preferred.ToString());}

        if(diagnosticArgs!=null)foreach(var arg in diagnosticArgs)start.ArgumentList.Add(arg);

        return start;
    }

    public Process LaunchPrepared(string profile,string? token=null,bool continuation=false,int preferred=-1,IEnumerable<string>? diagnosticArgs=null){
        var start=PreparedStartInfo(profile,token,continuation,preferred,diagnosticArgs);
        Running?.Dispose();Running=Process.Start(start)??throw new InvalidOperationException("Le moteur n’a pas démarré.");return Running;

    }

    public void Launch(string id,byte[] placements,bool demo=false){string profile=Prepare(id,placements,demo);LaunchPrepared(profile,Guid.NewGuid().ToString("N"));}

    public void LaunchDemo()=>Launch("demo",File.ReadAllBytes(Path.Combine(root,"build/special-port/six-fruits-demo.yslevel")),true);

}



public static class SpecialSaveSeed {
    static readonly string[] NewJournals=["pc-metal-yoshis.json","pc-hybrid-completion.json"];
    static bool Natural(JsonElement root,string name,uint maximum,out uint value){value=0;return root.TryGetProperty(name,out var property)&&property.ValueKind==JsonValueKind.Number&&property.TryGetUInt32(out value)&&value<=maximum;}
    static void ValidateJournal(string name,byte[] bytes){
        try{
            using var json=JsonDocument.Parse(bytes,new JsonDocumentOptions{MaxDepth=8});var root=json.RootElement;
            if(root.ValueKind!=JsonValueKind.Object)throw new InvalidDataException();var properties=root.EnumerateObject().ToArray();
            if(properties.Select(p=>p.Name).Distinct(StringComparer.Ordinal).Count()!=properties.Length||!Natural(root,"schema",1,out uint schema)||schema!=1)throw new InvalidDataException();
            if(name=="pc-metal-yoshis.json"){
                if(properties.Length!=3||!Natural(root,"foundMask",3,out uint found)||!Natural(root,"storyAliveMask",3,out uint alive)||(alive&~found)!=0)throw new InvalidDataException();
            }else{
                if(properties.Length!=5||!Natural(root,"observedMask",0xFFFFFF,out _)||!Natural(root,"legacyMask",0xFFFFFF,out uint legacy)||!root.TryGetProperty("legacyChecked",out var check)||check.ValueKind is not (JsonValueKind.True or JsonValueKind.False)||!root.TryGetProperty("legacyChecksum",out var checksum))throw new InvalidDataException();
                bool hasChecksum=checksum.ValueKind==JsonValueKind.Number&&checksum.TryGetUInt32(out _);
                if(checksum.ValueKind!=JsonValueKind.Null&&!hasChecksum||!check.GetBoolean()&&(legacy!=0||hasChecksum)||legacy!=0&&!hasChecksum)throw new InvalidDataException();
            }
        }catch(Exception error)when(error is JsonException or InvalidDataException){throw new InvalidDataException("Journal "+name+" invalide ; fichier conservé.",error);}
    }
    static void CopyMissingNewJournals(string personalDirectory,string profile){
        var pending=new List<(string Target,byte[] Bytes)>();
        foreach(string name in NewJournals){
            string source=Path.Combine(personalDirectory,name),target=Path.Combine(profile,name);
            // Existing local state always wins, even when corrupt; never turn
            // this additive migration into a repair or a progress reset.
            if(File.Exists(target)||Directory.Exists(target)||!File.Exists(source))continue;
            using var input=new FileStream(source,FileMode.Open,FileAccess.Read,FileShare.Read|FileShare.Delete);byte[] buffer=new byte[4097];int count=0,read;
            while(count<buffer.Length&&(read=input.Read(buffer,count,buffer.Length-count))>0)count+=read;
            if(count>4096)throw new InvalidDataException("Journal "+name+" trop volumineux ; fichier conservé.");
            byte[] bytes=buffer[..count];ValidateJournal(name,bytes);pending.Add((target,bytes));
        }
        // Validate every candidate before adding either journal. Copy the exact
        // validated bytes, not a second read that could race a source update.
        if(pending.Count==0)return;Directory.CreateDirectory(profile);
        foreach(var item in pending){
            FileStream output;
            try{output=new FileStream(item.Target,FileMode.CreateNew,FileAccess.Write,FileShare.None);}
            catch(IOException)when(File.Exists(item.Target)||Directory.Exists(item.Target)){continue;/* Another creator won; keep its local state. */}
            using(output){output.Write(item.Bytes);output.Flush(true);}
        }
    }
    public static bool Initialize(string root,string profile){
        string target=Path.Combine(profile,"saves","yoshisstory.n64.us.bin");
        string personalDirectory=Path.Combine(root,"profiles","content-personal");
        if(File.Exists(target)){CopyMissingNewJournals(personalDirectory,profile);return false;}
        string personal=Path.Combine(personalDirectory,"saves","yoshisstory.n64.us.bin");
        if(!File.Exists(personal))return ContentSaveSeed.Initialize(root,profile);
        byte[] bytes=File.ReadAllBytes(personal);
        if(bytes.Length!=2048||!bytes.AsSpan(0,1024).SequenceEqual(bytes.AsSpan(1024))||!bytes.AsSpan(1020,4).SequenceEqual(new byte[]{0x81,0x31,0x75,0x39}))throw new InvalidDataException("Fermer le jeu pour copier une sauvegarde personnelle complète.");
        Directory.CreateDirectory(Path.GetDirectoryName(target)!);
        CopyMissingNewJournals(personalDirectory,profile);
        // The original journals follow the first EEPROM seed only. The two
        // newer sidecars above also migrate additively into played profiles.
        foreach(string name in new[]{"pc-achievements.json","pc-collection.json","pc-hybrid-records.json","pc-hybrid-finds.json"}){
            string ledger=Path.Combine(personalDirectory,name);if(File.Exists(ledger))File.Copy(ledger,Path.Combine(profile,name),false);
        }
        using var output=new FileStream(target,FileMode.CreateNew,FileAccess.Write,FileShare.None);output.Write(bytes);output.Flush(true);return true;
    }
}
