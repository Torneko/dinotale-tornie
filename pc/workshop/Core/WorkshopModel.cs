using System.Buffers.Binary;

using System.Diagnostics;

using System.IO.Hashing;

using System.Security.Cryptography;

using System.Text.Json;



namespace YoshiWorkshop;



public sealed class Placement {

    public string Id { get; set; } = Guid.NewGuid().ToString("N");

    public ushort ObjectId { get; set; }

    public ushort Layer { get; set; }

    public float X { get; set; }

    public float Y { get; set; }

    public int? HybridVariant { get; set; }

    public PassageOverride? Passage { get; set; }

    public Placement Copy() => new() { ObjectId=ObjectId, Layer=Layer, X=X, Y=Y, HybridVariant=HybridVariant, Passage=Passage is null?null:Passage with{} };

}

public sealed class LevelProject {

    public int Schema { get; set; } = 1;

    public string Id { get; set; } = Guid.NewGuid().ToString("N");

    public string Name { get; set; } = "Mon niveau";

    public int SourceZone { get; set; } = 43;

    public string SourceSha256 { get; set; } = VanillaRom.ExpectedSha256;

    public string Icon { get; set; } = "";

    public int? MusicId { get; set; }

    public RoomEntry? Entry { get; set; }

    public List<Placement> Objects { get; set; } = [];

    public List<RoomProject> Rooms { get; set; } = [];

    public List<TerrainCellChange> Terrain { get; set; } = [];

}

public sealed class CoopRules {

    public bool LocalTwoControllers { get; set; } = true;

    public bool HostedNetwork { get; set; } = true;

    public bool PlayerInteractions { get; set; }

    public bool SharedFruitCounter => true;

    public bool EggsHitTeammate => false;

    public bool EggsBlockedByTeammate => false;

    public int FruitTarget { get; set; } = 30;

}

public sealed class CustomGame {

    public int Schema { get; set; } = 1;

    public string Id { get; set; } = Guid.NewGuid().ToString("N");

    public string Name { get; set; } = "Ma partie";

    public List<string> LevelIds { get; set; } = [];

    public CoopRules Coop { get; set; } = new();

}

// One authoritative collection event per world entity. Both players contribute

// to the same counter; re-delivery of a network event cannot count a fruit twice.

public sealed class TeamProgress {

    readonly HashSet<string> collected = [];

    readonly HashSet<string> defeated = [];

    public int FruitCount => collected.Count;

    public int DefeatedCount => defeated.Count;

    public bool Collect(string entityId) => collected.Add(entityId);

    public bool Defeat(string entityId) => defeated.Add(entityId);

    public static bool PlayersCollide(CoopRules rules) => rules.PlayerInteractions;

    public static bool EggCollidesWithTeammate(CoopRules rules) => false;

}



public sealed class ProjectStore(string root) {

    public string Root { get; } = Path.GetFullPath(root);

    public static readonly JsonSerializerOptions Json = new() { WriteIndented=true, PropertyNameCaseInsensitive=true };

    public string LevelDir => Path.Combine(Root,"levels");

    public string GameDir => Path.Combine(Root,"games");

    public string LevelPath(string id) { ValidateId(id); return Path.Combine(LevelDir,id+".json"); }

    public string IconPath(string id) { ValidateId(id); return Path.Combine(LevelDir,id+".png"); }

    public static void ValidateId(string id) {

        if (id.Length!=32 || id.Any(c => !Uri.IsHexDigit(c))) throw new InvalidDataException("Identifiant de projet invalide.");

    }

    static void AtomicSave<T>(string path,T value) {

        Directory.CreateDirectory(Path.GetDirectoryName(path)!);

        string temporary=path+".tmp";

        File.WriteAllText(temporary,JsonSerializer.Serialize(value,Json));

        File.Move(temporary,path,true);

    }

    public void Save(LevelProject level) { Validate(level); AtomicSave(LevelPath(level.Id),level); }

    public void Save(CustomGame game) {

        ValidateId(game.Id);

        if (string.IsNullOrWhiteSpace(game.Name) || game.Name.Length>100) throw new InvalidDataException("Nom de partie requis (100 caractères maximum).");

        if(game.LevelIds.Count>64)throw new InvalidDataException("Une partie peut contenir au maximum 64 niveaux.");

        foreach (string id in game.LevelIds) { ValidateId(id); if (!File.Exists(LevelPath(id))) throw new InvalidDataException("Un niveau de la partie est introuvable."); }

        if (game.Coop.FruitTarget<1 || game.Coop.FruitTarget>999) throw new InvalidDataException("Objectif de fruits invalide.");

        AtomicSave(Path.Combine(GameDir,game.Id+".json"),game);

    }

    public List<LevelProject> Levels() => ReadAll<LevelProject>(LevelDir);

    public List<CustomGame> Games() => ReadAll<CustomGame>(GameDir);

    static List<T> ReadAll<T>(string directory) {

        if (!Directory.Exists(directory)) return [];

        return Directory.GetFiles(directory,"*.json").Order().Select(p=>JsonSerializer.Deserialize<T>(File.ReadAllText(p),Json)!).ToList();

    }

    public LevelProject Duplicate(LevelProject source) {

        var copy=JsonSerializer.Deserialize<LevelProject>(JsonSerializer.Serialize(source,Json),Json)!;

        const string copySuffix=" \u2014 copie";

        int prefixLength=Math.Min(source.Name.Length,100-copySuffix.Length);

        if(prefixLength>0 && char.IsHighSurrogate(source.Name[prefixLength-1]))prefixLength--;

        copy.Id=Guid.NewGuid().ToString("N"); copy.Name=source.Name[..prefixLength]+copySuffix; copy.Icon="";

        foreach (var room in copy.AllRooms())foreach(var item in room.Objects)item.Id=Guid.NewGuid().ToString("N");

        return copy;

    }

    public static void Validate(LevelProject level) {
        ValidateId(level.Id);
        if(level.Schema is not (1 or 2 or 3 or 4 or 5) || level.SourceSha256!=VanillaRom.ExpectedSha256 || VanillaRom.SourceStage(level.SourceZone)<0 || (level.Schema==1 && level.Rooms.Count!=0))throw new InvalidDataException("Format ou source de niveau invalide.");
        if(string.IsNullOrWhiteSpace(level.Name) || level.Name.Length>100)throw new InvalidDataException("Nom de niveau requis (100 caractères maximum).");
        var rooms=level.AllRooms().ToList();
        if(rooms.Count>10 || rooms.Select(r=>r.SourceZone).Distinct().Count()!=rooms.Count || rooms.Any(r=>!NativeRooms.Contains(level.SourceZone,r.SourceZone)))throw new InvalidDataException("Une salle n’appartient pas à ce niveau ou apparaît plusieurs fois.");
        if(rooms.Sum(r=>r.Objects.Count)>6000)throw new InvalidDataException("Le niveau dépasse 6000 objets au total.");
        TerrainEditing.ValidateStructure(level);RoomMusic.Validate(level);NavigationEditing.ValidateShape(level);
        var keys=new HashSet<string>();
        foreach(var room in rooms){
            if(room.Objects.Count is <1 or >RoomProject.MaxObjects || room.Objects.Count(p=>p.ObjectId==0x4001)!=1)throw new InvalidDataException("Chaque salle doit garder exactement un départ de Yoshi et contenir entre 1 et 149 objets.");
            if(room.Objects.Where(p=>p.HybridVariant.HasValue).GroupBy(p=>(p.ObjectId,p.Layer,BitConverter.SingleToInt32Bits(p.X),BitConverter.SingleToInt32Bits(p.Y))).Any(g=>g.Select(p=>p.HybridVariant).Distinct().Count()>1))throw new InvalidDataException("Deux Maskass superposés ne peuvent pas avoir des couleurs différentes.");
            foreach(var p in room.Objects){ValidateId(p.Id);if(!keys.Add(p.Id))throw new InvalidDataException("Deux objets partagent le même identifiant.");
                if(p.HybridVariant is int color && (color<0 || color>5 || !ObjectCatalog.IsMaskass(p.ObjectId)))throw new InvalidDataException("Couleur hybride incompatible avec cet objet.");
                if(!float.IsFinite(p.X) || !float.IsFinite(p.Y) || Math.Abs(p.X)>100000 || Math.Abs(p.Y)>100000)throw new InvalidDataException("Coordonnées hors limites.");
            }
        }
    }

}



public sealed record LevelTemplate(int SourceZone,string Code,int Objects,bool Supported,string Detail) {

    public override string ToString()=>$"{Code} · {NativeRooms.For(SourceZone).Count} salles{(Supported?"":" · indisponible")}";

}

public sealed partial class VanillaRom {

    public const string ExpectedSha256="e8a63388c38f8f0bea37e9b13a97f0898ac5496e08ca2028afb8db9a866e0ce9";

    public const ulong ExpectedXxh3=0xf1d8a6737dda5b82;

    readonly byte[] data;

    readonly int[] banks;

    readonly Dictionary<int,LevelProject> originals=[];

    public static readonly int[] SourceZones=[43,51,13,18,46,19,16,23,52,40,32,14,17,22,12,27,21,30,31,28,15,35,34,24];
    public static int SourceStage(int zone)=>Array.IndexOf(SourceZones,zone);
    public static string SourceCode(int zone)=>SourceStage(zone) is int i && i>=0?$"{i/4+1}-{i%4+1}":$"Salle {zone}";

    public static string Label(ushort id)=>ObjectCatalog.Get(id).DisplayName;

    public static readonly Dictionary<ushort,string> Labels=ObjectCatalog.All.ToDictionary(p=>p.Key,p=>p.Value.Name);

    public VanillaRom(string path){

        data=File.ReadAllBytes(path);

        if(Convert.ToHexStringLower(SHA256.HashData(data))!=ExpectedSha256 || XxHash3.HashToUInt64(data)!=ExpectedXxh3)throw new InvalidDataException("ROM originale USA attendue.");

        banks=Enumerable.Range(0,16).Select(i=>(int)U32(0xA79D4+4*i)).ToArray();

    }

    uint U32(int p)=>BinaryPrimitives.ReadUInt32BigEndian(data.AsSpan(p,4));

    ushort U16(int p)=>BinaryPrimitives.ReadUInt16BigEndian(data.AsSpan(p,2));

    int Bank(uint pointer){int offset=checked(banks[(int)(pointer>>24)&15]+(int)(pointer&0xFFFFFF));if((pointer>>28)!=0 || offset<0 || offset>=data.Length)throw new InvalidDataException("Pointeur de niveau invalide.");return offset;}

    LevelProject Original(int zone){

        if(SourceStage(zone)<0)throw new InvalidDataException("Source de niveau invalide.");

        if(originals.TryGetValue(zone,out var cached))return cached;

        int header=Bank(U32(0xA84A0+zone*8+4)),meta=Bank(U32(Bank(U32(header+0x5C)))),start=Bank(U32(meta+4)),count=U16(meta);

        if(count is <1 or >2000 || start+count*12>data.Length)throw new InvalidDataException("Placements vanilla invalides.");

        var level=new LevelProject{Name="Mon "+SourceCode(zone),SourceZone=zone};

        for(int n=0;n<count;n++){int p=start+n*12;level.Objects.Add(new(){ObjectId=U16(p),Layer=U16(p+2),X=BitConverter.Int32BitsToSingle((int)U32(p+4)),Y=BitConverter.Int32BitsToSingle((int)U32(p+8))});}

        ProjectStore.Validate(level);originals.Add(zone,level);return level;

    }

    public IReadOnlyList<LevelTemplate> Templates=>SourceZones.Select(zone=>{

        try{var l=Original(zone);return new LevelTemplate(zone,SourceCode(zone),l.Objects.Count,true,"Objets modifiables de la salle de départ ; terrain et autres salles d’origine conservés.");}

        catch(InvalidDataException e){return new LevelTemplate(zone,SourceCode(zone),0,false,e.Message);}

    }).ToArray();

    public LevelProject CloneFirstLevel()=>CloneLevel(43,false);

    public LevelProject CloneLevel(int zone,bool allRooms=true){var source=Original(zone);var copy=System.Text.Json.JsonSerializer.Deserialize<LevelProject>(System.Text.Json.JsonSerializer.Serialize(source,ProjectStore.Json),ProjectStore.Json)!;copy.Id=Guid.NewGuid().ToString("N");foreach(var o in copy.Objects)o.Id=Guid.NewGuid().ToString("N");if(allRooms)IncludeOriginalRooms(copy);return copy;}

    public IReadOnlySet<ushort> SourceObjects(int zone){var allowed=ReadRoom(zone).Objects.Select(o=>o.ObjectId).ToHashSet();if(CanPlaceBlackFlyGuy(zone))allowed.Add(0x45E3);return allowed;}

    public IEnumerable<ushort> Palette(int zone,bool special){

        var ids=SourceObjects(zone).Where(id=>id<0x8000 && id!=0x4001 && (special || id!=0x45E3));

        if(special){if(SpecialEditorObjects.Compatible(SourceObjects(zone)))ids=ids.Concat(SpecialEditorObjects.Eggs);ids=ids.Concat(ObjectCatalog.SpecialFruits);if(SourceObjects(zone).Any(ObjectCatalog.IsFruitBubble))ids=ids.Concat(ObjectCatalog.SpecialBubbles);if(ReadRoom(zone).Objects.Any(p=>ObjectCatalog.IsMaskass(p.ObjectId)))ids=ids.Concat(ObjectCatalog.SpecialMaskass);}

        return ids.Distinct().OrderBy(id=>ObjectCatalog.Get(id).Name);

    }

    public Placement CreatePlacement(ushort id,int sourceZone=43){

        var objects=ReadRoom(sourceZone).Objects;bool hybrid=ObjectCatalog.IsSpecialMaskass(id);

        var template=id==0x45E3 && CanPlaceBlackFlyGuy(sourceZone)?objects.First(p=>p.ObjectId==0x45E5):SpecialEditorObjects.IsEgg(id)?objects.FirstOrDefault(p=>p.ObjectId==SpecialEditorObjects.Alias(objects.Select(o=>o.ObjectId))):hybrid?objects.FirstOrDefault(p=>p.ObjectId==0x403A)??objects.FirstOrDefault(p=>ObjectCatalog.IsMaskass(p.ObjectId)):

            ObjectCatalog.IsSpecialBubble(id)?objects.Where(p=>ObjectCatalog.IsFruitBubble(p.ObjectId)).OrderBy(p=>p.ObjectId).FirstOrDefault():
            ObjectCatalog.IsSpecialFruit(id)?objects.FirstOrDefault(p=>p.ObjectId is >=0x4030 and <=0x4034):objects.FirstOrDefault(p=>p.ObjectId==id);

        if(template==null)throw new InvalidDataException($"L’objet {ObjectCatalog.Get(id).Name} ne possède pas de modèle compatible dans {SourceCode(sourceZone)}.");

        var p=template.Copy();if(hybrid)p.HybridVariant=id-0x4F10;else p.ObjectId=id;return p;

    }

    void ValidateResources(LevelProject level,bool special){
        ProjectStore.Validate(level);
        ValidateTerrain(level);
        ValidateNavigation(level);
        if(!special && NavigationEditing.HasEdits(level))throw new InvalidDataException("Les passages et entrées modifiés nécessitent le moteur personnalisé.");
        foreach(var room in level.AllRooms()){var original=ReadRoom(room.SourceZone).Objects.Single(p=>p.ObjectId==0x4001);var spawn=room.Objects.Single(p=>p.ObjectId==0x4001);
            if(spawn.Layer!=original.Layer || (room.Entry is null && (spawn.X!=original.X || spawn.Y!=original.Y)) || (room.Entry is not null && (spawn.X!=room.Entry.X || spawn.Y!=room.Entry.Y)))throw new InvalidDataException("Conserve le point d’entrée d’origine de chaque salle. Le bouton de rétablissement permet de le récupérer sans toucher aux autres objets.");var allowed=SourceObjects(room.SourceZone);
            var nativeEggAlias=SpecialEditorObjects.Alias(allowed);var nativeEggLayer=ReadRoom(room.SourceZone).Objects.FirstOrDefault(p=>p.ObjectId==nativeEggAlias)?.Layer;
            if(room.Objects.Any(p=>SpecialEditorObjects.IsEgg(p.ObjectId) && (!nativeEggLayer.HasValue || p.Layer!=nativeEggLayer.Value)))throw new InvalidDataException("Conserver les paramètres natifs de la bulle d’œuf.");
            if(!special && room.Objects.Any(p=>p.ObjectId==0x45E3))throw new InvalidDataException("Le Maskass noir seul nécessite le moteur personnalisé.");
            if(room.Objects.Where(p=>p.ObjectId is 0x45E3 or 0x45E5).GroupBy(p=>(p.Layer,p.X,p.Y)).Any(g=>g.Count()>1 && g.Any(p=>p.ObjectId==0x45E3)))throw new InvalidDataException("Les Maskass noirs seuls et les blocs à piquants doivent être séparés.");
            var eggGroups=room.Objects.Where(p=>SpecialEditorObjects.IsEgg(p.ObjectId) || SpecialEditorObjects.NativeEgg(p.ObjectId)).GroupBy(p=>(p.Layer,p.X,p.Y));
            if(eggGroups.Any(g=>g.Count()>1 && g.Any(p=>SpecialEditorObjects.IsEgg(p.ObjectId))))throw new InvalidDataException("Les bulles d’œufs spéciales doivent être séparées les unes des autres.");
            var bubbleGroups=room.Objects.Where(p=>ObjectCatalog.IsSpecialBubble(p.ObjectId) || ObjectCatalog.IsFruitBubble(p.ObjectId)).GroupBy(p=>(p.Layer,p.X,p.Y));
            if(bubbleGroups.Any(g=>g.Any(p=>ObjectCatalog.IsSpecialBubble(p.ObjectId)) && (g.Select(p=>p.ObjectId).Distinct().Count()>1)))throw new InvalidDataException("Les bulles superposées doivent contenir le même fruit.");
            if(room.Objects.Any(p=>(!allowed.Contains(p.ObjectId) && !(special && (ObjectCatalog.IsSpecialFruit(p.ObjectId) || SpecialEditorObjects.IsEgg(p.ObjectId) && SpecialEditorObjects.Compatible(allowed) || ObjectCatalog.IsSpecialBubble(p.ObjectId) && allowed.Any(ObjectCatalog.IsFruitBubble)))) || (!special && p.HybridVariant.HasValue)))throw new InvalidDataException("Un objet n’est pas compatible avec les ressources de sa salle.");
        }
    }

    public byte[] BuildDataOnly(LevelProject level,bool specialContent=false){ValidateResources(level,specialContent);return BuildRooms(level);}
    public byte[] ExportPlacements(LevelProject level,bool specialContent=false){ValidateResources(level,specialContent);return ExportRooms(level);}

}



public static class NativeSaveSeed {

    // A new level starts from the imported progress once. Existing PC progress

    // always wins on subsequent launches, including if a new import is made.

    public static bool Initialize(string pcRoot,string profile) {

        string target=Path.Combine(profile,"saves","yoshisstory.n64.us.bin");

        string seed=Path.Combine(pcRoot,"save-imports","default.bin");

        if(File.Exists(target) || !File.Exists(seed))return false;

        byte[] bytes=File.ReadAllBytes(seed);

        using var report=JsonDocument.Parse(File.ReadAllText(Path.Combine(pcRoot,"save-imports","current-import.json")));

        string hash=Convert.ToHexString(SHA256.HashData(bytes)).ToLowerInvariant();

        if(bytes.Length!=2048 || report.RootElement.GetProperty("convertedSha256").GetString()!=hash)

            throw new InvalidDataException("La copie de progression importée ne correspond plus à son rapport.");

        Directory.CreateDirectory(Path.GetDirectoryName(target)!);

        using var output=new FileStream(target,FileMode.CreateNew,FileAccess.Write,FileShare.None);

        output.Write(bytes);output.Flush(true);return true;

    }

}



public sealed class NativeRunner(string pcRoot) {

    public string Root { get; }=Path.GetFullPath(pcRoot);

    public Process? Running { get; private set; }

    public bool IsRunning => Running is {HasExited:false};

    public string Prepare(byte[] rom,string profileId,byte[]? placements=null) {

        if (IsRunning) throw new InvalidOperationException("Fermer le jeu avant de lancer un autre niveau.");

        if (profileId!="vanilla") ProjectStore.ValidateId(profileId);

        if(Convert.ToHexString(SHA256.HashData(rom)).ToLowerInvariant()!=VanillaRom.ExpectedSha256)throw new InvalidDataException("Le moteur attend la ROM originale USA et des placements séparés.");

        string runtime=Path.Combine(Root,"build","native");

        using var engineManifest=JsonDocument.Parse(File.ReadAllText(Path.Combine(runtime,"engine-build.json")));

        string exePath=Path.Combine(runtime,"DinoTale.exe");

        string exeHash=Convert.ToHexString(SHA256.HashData(File.ReadAllBytes(exePath))).ToLowerInvariant();

        if(exeHash!=engineManifest.RootElement.GetProperty("exeSha256").GetString())throw new InvalidDataException("Recompiler le moteur : son empreinte ne correspond plus au rapport de compilation.");

        string profile=Path.Combine(Root,"profiles",profileId);Directory.CreateDirectory(profile);

        NativeSaveSeed.Initialize(Root,profile);

        File.WriteAllText(Path.Combine(profile,"portable.txt"),"");

        File.WriteAllBytes(Path.Combine(profile,"yoshisstory.n64.us.z64"),rom);

        if(placements!=null)File.WriteAllBytes(Path.Combine(profile,"level.yslevel"),placements);

        foreach (string source in Directory.GetFiles(Path.Combine(runtime,"assets"),"*",SearchOption.AllDirectories)) {

            string target=Path.Combine(profile,"assets",Path.GetRelativePath(Path.Combine(runtime,"assets"),source));

            Directory.CreateDirectory(Path.GetDirectoryName(target)!);File.Copy(source,target,true);

        }

        File.WriteAllText(Path.Combine(profile,"launch-manifest.json"),JsonSerializer.Serialize(new {romSha256=VanillaRom.ExpectedSha256,exeSha256=exeHash,sourceBuiltEngine=true,placements=placements!=null?"level.yslevel":null,placementsSha256=placements!=null?Convert.ToHexString(SHA256.HashData(placements)).ToLowerInvariant():null,dataOnly=true},ProjectStore.Json));

        return profile;

    }

    public void Launch(byte[] rom,string profileId,byte[]? placements=null) {

        string profile=Prepare(rom,profileId,placements);

        var start=new ProcessStartInfo(Path.Combine(Root,"build","native","DinoTale.exe")) {WorkingDirectory=profile,UseShellExecute=false};

        start.ArgumentList.Add("--game");start.ArgumentList.Add("yoshisstory");

        if(placements!=null){start.ArgumentList.Add("--workshop-level");start.ArgumentList.Add(Path.Combine(profile,"level.yslevel"));}

        Running=Process.Start(start);

        // The custom payload is a separate, code-free file; the ROM and

        // executable are never patched for an editor test.

    }

}

