using System.Net;
using System.Security.Cryptography;
using System.Text;
using System.Text.Json;
using System.Text.Json.Serialization;
namespace YoshiWorkshop;

public enum CoopSessionMode { Local, Host, Join }
public enum CoopSessionPhase { Lobby, Running, Completed, GameOver, Disconnected }
public enum CoopEntityKind { Fruit, Enemy }
public enum CoopCompletionMode { FruitTarget, HostSignal }

public sealed record CoopSessionRules(bool PlayerInteractions=false,int FruitTarget=30,int MaxHealth=8,int RespawnDelayMilliseconds=5000,bool AllowLocal=true,bool AllowNetwork=true,CoopCompletionMode CompletionMode=CoopCompletionMode.FruitTarget) {
    [JsonIgnore] public bool SharedFruitCounter=>true;
    [JsonIgnore] public bool EggsHitTeammate=>false;
    [JsonIgnore] public bool EggsBlockedByTeammate=>false;
    public static CoopSessionRules From(CoopRules rules,int respawnDelayMilliseconds=5000,int maxHealth=8)=>new(rules.PlayerInteractions,rules.FruitTarget,maxHealth,respawnDelayMilliseconds,rules.LocalTwoControllers,rules.HostedNetwork);
    public void Validate(){if(FruitTarget is <1 or >999 || MaxHealth is <1 or >64 || RespawnDelayMilliseconds is <1000 or >60000 || !Enum.IsDefined(CompletionMode))throw new InvalidDataException("Invalid cooperative rules.");}
}
public sealed record CoopSessionIdentity(string ContentVersion,string ContentSha256,string LevelSha256,CoopSessionRules Rules) {
    public const int ProtocolVersion=1;
    public void Validate(){if(string.IsNullOrWhiteSpace(ContentVersion) || ContentVersion.Length>64 || !IsHash(ContentSha256) || !IsHash(LevelSha256) || Rules is null)throw new InvalidDataException("Invalid cooperative content identity.");Rules.Validate();}
    public void RequireCompatible(CoopSessionIdentity other){Validate();if(other is null)throw new InvalidDataException("Missing peer content identity.");other.Validate();if(this!=other)throw new InvalidDataException("Content version, level hash or cooperative rules differ.");}
    public static bool IsHash(string? value)=>value is {Length:64} && value.All(c=>c is >= '0' and <= '9' or >= 'a' and <= 'f');
    public string Fingerprint(){Validate();return Convert.ToHexStringLower(SHA256.HashData(JsonSerializer.SerializeToUtf8Bytes(this,CoopJson.Options)));}
}
public sealed record CoopInputFrame(long Sequence,int Buttons,int StickX,int StickY) {
    public void Validate(){if(Sequence<1 || Buttons is <0 or >65535 || StickX is <-80 or >80 || StickY is <-80 or >80)throw new InvalidDataException("Input sequence/buttons/axes outside protocol bounds.");}
}
public sealed record CoopWorldEntity(string StableId,CoopEntityKind Kind) {
    public static void ValidateId(string? id){if(string.IsNullOrEmpty(id) || id.Length>128 || id.Any(c=>!char.IsAsciiLetterOrDigit(c) && c is not ('/' or ':' or '.' or '_' or '-')))throw new InvalidDataException("Invalid stable world-entity ID.");}
    public void Validate(){ValidateId(StableId);if(!Enum.IsDefined(Kind))throw new InvalidDataException("Invalid cooperative entity kind.");}
}
public sealed record CoopPlayerSnapshot(int Slot,int YoshiColor,bool Connected,bool Ready,int Health,int RespawnMillisecondsRemaining,long LastInputSequence,CoopInputFrame? Input) {
    [JsonIgnore] public bool Alive=>Health>0;
}
public sealed record CoopSessionSnapshot(string SessionId,CoopSessionIdentity Identity,CoopSessionPhase Phase,long Revision,int FruitCount,int DefeatedCount,IReadOnlyList<CoopPlayerSnapshot> Players) {
    [JsonIgnore] public bool BothReady=>Players.Count==2 && Players.All(p=>p.Connected && p.Ready);
    public void Validate(){
        if(Identity is null)throw new InvalidDataException("Missing snapshot identity.");Identity.Validate();if(SessionId is not {Length:32} || SessionId.Any(c=>!Uri.IsHexDigit(c)) || !Enum.IsDefined(Phase) || Revision<0 || FruitCount is <0 or >CoopSession.MaxEntities || DefeatedCount is <0 or >CoopSession.MaxEntities || Players is null || Players.Count!=2)throw new InvalidDataException("Invalid cooperative snapshot.");
        for(int i=0;i<2;i++){var p=Players[i];if(p is null || p.Slot!=i || p.YoshiColor is <0 or >13 || p.Health<0 || p.Health>Identity.Rules.MaxHealth || p.RespawnMillisecondsRemaining<0 || p.RespawnMillisecondsRemaining>Identity.Rules.RespawnDelayMilliseconds || p.LastInputSequence<0 || p.Ready && !p.Connected)throw new InvalidDataException("Invalid cooperative player snapshot.");if(p.Input is {} input){input.Validate();if(input.Sequence!=p.LastInputSequence)throw new InvalidDataException("Snapshot input sequence differs.");}else if(p.LastInputSequence!=0)throw new InvalidDataException("Snapshot omitted input.");}
    }
}
public sealed record CoopAuthoritativeEvent(long Revision,CoopEntityKind Kind,string StableId,int PlayerSlot,int FruitCount,int DefeatedCount);
public static class CoopJson {
    public static readonly JsonSerializerOptions Options=Create();
    static JsonSerializerOptions Create(){var options=new JsonSerializerOptions{MaxDepth=16,PropertyNameCaseInsensitive=false,UnmappedMemberHandling=JsonUnmappedMemberHandling.Disallow};options.Converters.Add(new JsonStringEnumConverter(allowIntegerValues:false));return options;}
}
public sealed record CoopSessionPreferences(int Schema=1,CoopSessionMode Mode=CoopSessionMode.Local,string HostAddress="127.0.0.1",int Port=6464,int FirstYoshiColor=0,int SecondYoshiColor=1,CoopSessionRules? Rules=null,string LastLevelId="") {
    [JsonIgnore] public CoopSessionRules EffectiveRules=>Rules??new();
    public void Validate(){if(Schema!=1 || !Enum.IsDefined(Mode) || !IPAddress.TryParse(HostAddress,out _) || Port is <1 or >65535 || FirstYoshiColor is <0 or >13 || SecondYoshiColor is <0 or >13 || LastLevelId is null || LastLevelId.Length>64 || LastLevelId.Any(c=>!char.IsAsciiLetterOrDigit(c) && c is not ('-' or '_')))throw new InvalidDataException("Invalid cooperative preferences.");EffectiveRules.Validate();}
}
public static class CoopSessionPreferencesStore {
    public const int MaxBytes=16384;
    public static CoopSessionPreferences Load(string path){if(!File.Exists(path))return new();if(new FileInfo(path).Length>MaxBytes)throw new InvalidDataException("Cooperative preferences too large; file preserved.");var prefs=JsonSerializer.Deserialize<CoopSessionPreferences>(File.ReadAllBytes(path),CoopJson.Options)??throw new InvalidDataException("Invalid cooperative preferences.");prefs.Validate();return prefs;}
    public static void Save(string path,CoopSessionPreferences prefs){prefs.Validate();var bytes=JsonSerializer.SerializeToUtf8Bytes(prefs,CoopJson.Options);if(bytes.Length>MaxBytes)throw new InvalidDataException("Cooperative preferences too large.");string full=Path.GetFullPath(path);Directory.CreateDirectory(Path.GetDirectoryName(full)!);string temp=full+"."+Guid.NewGuid().ToString("N")+".tmp";try{File.WriteAllBytes(temp,bytes);File.Move(temp,full,true);}finally{if(File.Exists(temp))File.Delete(temp);}}
}
