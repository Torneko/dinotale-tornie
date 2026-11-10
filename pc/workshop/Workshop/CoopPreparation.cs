using System.Net;
using System.Net.Sockets;
using System.Security.Cryptography;
using System.Text;
using System.Text.Json;
using YoshiWorkshop;
namespace WorkshopApp;
public sealed partial class WorkshopWindow {
    sealed record CoopLevelChoice(string Key,string Name,int Zone,LevelProject? Saved){public override string ToString()=>Name;}
    ComboBox coopFirst=null!,coopSecond=null!;
    readonly ComboBox coopLevel=new(){DropDownStyle=ComboBoxStyle.DropDownList,Width=385,DropDownWidth=600};
    readonly CheckBox coopInteractions=new(){Text="Autoriser les interactions entre Yoshis",AutoSize=true};
    readonly NumericUpDown coopRespawn=new(){Minimum=1,Maximum=60,Width=250,Value=5};
    readonly NumericUpDown coopHostPort=new(){Minimum=0,Maximum=65535,Width=250};
    readonly NumericUpDown coopJoinPort=new(){Minimum=1,Maximum=65535,Width=250,Value=42042};
    readonly TextBox coopAddress=new(){Width=310,Text="127.0.0.1"};
    readonly Label coopState=ModeText("Aucun salon ouvert.",650,12,true);
    readonly Label coopPlayers=ModeText("Choisis les deux Yoshis et un niveau pour préparer la partie.",650);
    readonly Label coopEndpoint=ModeText("",650);
    readonly CheckBox coopReady=new(){Text="Je suis prêt dans le salon",AutoSize=true};
    FlowLayoutPanel coopSettingsPanel=null!,coopLobbyPanel=null!;Button coopLocalButton=null!,coopHostButton=null!,coopJoinButton=null!,coopStopButton=null!;
    CoopSession? coopLocal;CoopNetworkHost? coopHost;CoopNetworkClient? coopClient;CoopSessionSnapshot? coopSnapshot;
    CancellationTokenSource? coopLifetime;int coopEpoch;bool coopUpdating,coopUiClosing,coopConnecting;
    bool CoopActive=>coopLocal!=null || coopHost!=null || coopClient!=null || coopConnecting;
    void BuildCoopPreparation(){
        var page=Page("Coopération");coopFirst=YoshiPicker(modeSetup.FirstColor);coopSecond=YoshiPicker(modeSetup.SecondColor);coopRespawn.Value=modeSetup.RespawnSeconds;coopInteractions.Checked=modeSetup.PlayerInteractions;coopHostPort.Value=modeSetup.HostPort;coopJoinPort.Value=modeSetup.JoinPort;coopAddress.Text=modeSetup.PeerAddress;
        coopLocalButton=Button("Préparer deux manettes",()=>Safe(PrepareLocalCoop),315);coopHostButton=Button("Héberger un salon",()=>{_=ModeAction(StartCoopHostAsync);},315);coopJoinButton=Button("Rejoindre le salon",()=>{_=ModeAction(JoinCoopAsync);},315);coopStopButton=Button("Quitter la préparation",()=>{_=ModeAction(StopCoopAsync);},315);coopStopButton.Enabled=false;coopReady.Enabled=false;
        coopSettingsPanel=ModeColumn(ModeText("Prépare ta partie à deux",400,20,true),ModeText("Les deux Yoshis partagent les trente fruits et les ennemis du même niveau. Chacun garde sa santé.",400),ModeField("Niveau enregistré ou d’origine",coopLevel),Button("Actualiser les niveaux",()=>Safe(RefreshCoopLevels),315),ModeField("Joueur 1 — hôte ou premier joueur local",coopFirst),ModeField("Joueur 2 — invité ou second joueur local",coopSecond),coopInteractions,ModeField("Retour après une mort : délai en secondes",coopRespawn),Button("Enregistrer ces réglages",()=>Safe(SaveCoopSetup),315),ModeText("Les œufs traversent toujours le partenaire et ne le blessent jamais. Un Yoshi revient après le délai choisi si l’autre est encore en vie.",400));
        var actions=coopLobbyPanel=ModeColumn(ModeText("Les réglages et les salons sont disponibles. Le pilotage de deux Yoshis dans le jeu est encore en préparation ; le salon ne lance pas encore une partie.",650),ModeText("Même PC",650,16,true),coopLocalButton,ModeText("Deux PC",650,16,true),ModeText("L’hôte et l’invité choisissent le même niveau et les mêmes règles. Le salon vérifie leur compatibilité avant de les réunir.",650),ModeField("Port de l’hôte — 0 choisit un port libre",coopHostPort,650),coopHostButton,ModeField("Adresse IP donnée par l’hôte",coopAddress,650),ModeField("Port donné par l’hôte",coopJoinPort,650),coopJoinButton,coopReady,coopStopButton,coopState,coopEndpoint,coopPlayers);
        var split=new TableLayoutPanel{Dock=DockStyle.Fill,ColumnCount=2,RowCount=1};split.ColumnStyles.Add(new(SizeType.Absolute,445));split.ColumnStyles.Add(new(SizeType.Percent,100));split.Controls.Add(coopSettingsPanel,0,0);split.Controls.Add(actions,1,0);page.Controls.Add(split);RefreshCoopLevels();
        coopReady.CheckedChanged+=(s,e)=>{if(!coopUpdating)_=ModeAction(SetCoopReadyAsync);};
        FormClosed+=(s,e)=>{coopUiClosing=true;coopLifetime?.Cancel();_=CloseCoopResourcesAsync();};
        if(modeLoadProblem!=null)coopState.Text=modeLoadProblem;
    }
    void RefreshCoopLevels(){
        if(CoopActive)return;string key=(coopLevel.SelectedItem as CoopLevelChoice)?.Key??modeSetup.LevelKey;coopLevel.Items.Clear();
        foreach(var t in rom.Templates.Where(t=>t.Supported))coopLevel.Items.Add(new CoopLevelChoice("vanilla-"+t.SourceZone,t.Code+" — niveau d’origine",t.SourceZone,null));
        foreach(var l in store.Levels())coopLevel.Items.Add(new CoopLevelChoice(l.Id,l.Name+" — création enregistrée",l.SourceZone,l));
        coopLevel.SelectedItem=coopLevel.Items.Cast<CoopLevelChoice>().FirstOrDefault(l=>l.Key==key)??coopLevel.Items.Cast<CoopLevelChoice>().FirstOrDefault();
    }
    void SaveCoopSetup(){
        modeSetup.FirstColor=((YoshiChoice)coopFirst.SelectedItem!).Index;modeSetup.SecondColor=((YoshiChoice)coopSecond.SelectedItem!).Index;modeSetup.LevelKey=(coopLevel.SelectedItem as CoopLevelChoice)?.Key??"vanilla-43";modeSetup.PlayerInteractions=coopInteractions.Checked;modeSetup.RespawnSeconds=(int)coopRespawn.Value;modeSetup.PeerAddress=coopAddress.Text.Trim();modeSetup.HostPort=(int)coopHostPort.Value;modeSetup.JoinPort=(int)coopJoinPort.Value;
        PersistModeSetup();status.Text="Préparation coop enregistrée.";
    }
    CoopSessionIdentity BuildCoopIdentity(){
        if(coopLevel.SelectedItem is not CoopLevelChoice selected)throw new InvalidOperationException("Choisir un niveau.");specialRunner.Verify();var level=selected.Saved??rom.CloneLevel(selected.Zone);byte[] payload=rom.ExportPlacements(level,true);
        using var manifest=JsonDocument.Parse(File.ReadAllText(Path.Combine(specialRunner.RuntimeDirectory,"engine-build.json")));var m=manifest.RootElement;string content=m.GetProperty("contentSha256").GetString()!,engine=m.GetProperty("exeSha256").GetString()!;
        string digest=Convert.ToHexStringLower(SHA256.HashData(Encoding.UTF8.GetBytes(content+"|"+engine)));string levelHash=Convert.ToHexStringLower(SHA256.HashData(payload));var rules=new CoopSessionRules(PlayerInteractions:coopInteractions.Checked,RespawnDelayMilliseconds:checked((int)coopRespawn.Value*1000));return new("dinotale-coop-preparation-1",digest,levelHash,rules);
    }
    void PrepareLocalCoop(){
        if(CoopActive)throw new InvalidOperationException("Quitter la préparation actuelle avant d’en ouvrir une autre.");var identity=BuildCoopIdentity();SaveCoopSetup();coopLocal=new(identity,CoopSessionMode.Local,modeSetup.FirstColor,modeSetup.SecondColor);coopLocal.SetReady(0,true,identity);coopLocal.SetReady(1,true,identity);coopSnapshot=coopLocal.Snapshot;coopEndpoint.Text="Préparation locale : deux manettes prévues sur ce PC.";RefreshCoopStatus();
    }
    async Task StartCoopHostAsync(){
        if(CoopActive)throw new InvalidOperationException("Quitter la préparation actuelle avant d’ouvrir un salon.");var identity=BuildCoopIdentity();SaveCoopSetup();coopLifetime=new();int epoch=++coopEpoch;
        try{
            coopHost=CoopNetworkHost.Start(automatedChecks?IPAddress.Loopback:IPAddress.Any,(int)coopHostPort.Value,identity,modeSetup.FirstColor);coopHost.SnapshotChanged+=s=>PostCoop(epoch,()=>{coopSnapshot=s;RefreshCoopStatus();});coopHost.StatusChanged+=s=>PostCoop(epoch,()=>{RefreshCoopStatus();coopState.Text=FriendlyCoopNetworkError(s);});
            coopSnapshot=coopHost.Snapshot;string ips=LocalCoopAddresses();coopEndpoint.Text=$"Port du salon : {coopHost.Port}\nAdresses de ce PC : {(ips.Length>0?ips:"voir les réglages réseau du PC")}\nCommunique une adresse et ce port à ton partenaire.";RefreshCoopStatus();coopLobbyPanel.ScrollControlIntoView(coopEndpoint);
        }catch{if(epoch==coopEpoch){await CloseCoopResourcesAsync();RefreshCoopStatus();}throw;}

    }
    async Task JoinCoopAsync(){
        if(CoopActive)throw new InvalidOperationException("Quitter la préparation actuelle avant de rejoindre un salon.");var identity=BuildCoopIdentity();if(!IPAddress.TryParse(coopAddress.Text.Trim(),out _))throw new InvalidDataException("Indique une adresse IP valide pour rejoindre un salon.");SaveCoopSetup();coopConnecting=true;coopLifetime=new();int epoch=++coopEpoch;RefreshCoopStatus();
        try{var connected=await CoopNetworkClient.ConnectAsync(modeSetup.PeerAddress,modeSetup.JoinPort,identity,modeSetup.SecondColor,coopLifetime.Token);if(epoch!=coopEpoch){await connected.DisposeAsync();return;}coopClient=connected;coopClient.SnapshotChanged+=s=>PostCoop(epoch,()=>{coopSnapshot=s;RefreshCoopStatus();});coopClient.StatusChanged+=s=>PostCoop(epoch,()=>{RefreshCoopStatus();coopState.Text=FriendlyCoopNetworkError(s);});coopSnapshot=coopClient.Snapshot;coopEndpoint.Text=$"Salon rejoint : {modeSetup.PeerAddress}:{modeSetup.JoinPort}";coopLobbyPanel.ScrollControlIntoView(coopEndpoint);}
        catch{if(epoch==coopEpoch){await CloseCoopResourcesAsync();RefreshCoopStatus();}throw;}
        finally{if(epoch==coopEpoch){coopConnecting=false;RefreshCoopStatus();}}
    }
    async Task SetCoopReadyAsync(){
        int epoch=coopEpoch;bool ready=coopReady.Checked;var h=coopHost;var c=coopClient;var token=coopLifetime?.Token??default;if(h!=null){var snapshot=await h.SetHostReadyAsync(ready,token);if(epoch==coopEpoch)coopSnapshot=snapshot;}else if(c!=null)await c.SetReadyAsync(ready,token);if(epoch==coopEpoch)RefreshCoopStatus();
    }
    async Task CloseCoopResourcesAsync(){
        ++coopEpoch;var oldLifetime=coopLifetime;var h=coopHost;var c=coopClient;
        coopConnecting=false;coopHost=null;coopClient=null;coopLifetime=null;coopLocal=null;coopSnapshot=null;oldLifetime?.Cancel();
        try{if(c!=null)await c.DisposeAsync();}finally{try{if(h!=null)await h.DisposeAsync();}finally{oldLifetime?.Dispose();}}
    }
    async Task StopCoopAsync(){int expected=coopEpoch+1;await CloseCoopResourcesAsync();if(expected!=coopEpoch)return;coopEndpoint.Text="";RefreshCoopStatus();}
    void PostCoop(int epoch,Action action){if(coopUiClosing || epoch!=coopEpoch || IsDisposed || !IsHandleCreated)return;try{BeginInvoke(()=>{if(!coopUiClosing && epoch==coopEpoch && !IsDisposed)action();});}catch(InvalidOperationException){}}
    void RefreshCoopStatus(){
        if(coopUiClosing)return;bool active=CoopActive;coopSettingsPanel.Enabled=!active;coopHostPort.Enabled=coopAddress.Enabled=coopJoinPort.Enabled=!active;coopLocalButton.Enabled=coopHostButton.Enabled=coopJoinButton.Enabled=!active;coopStopButton.Enabled=active;
        coopUpdating=true;try{coopReady.Enabled=coopHost!=null || coopClient is {IsConnected:true};int slot=coopClient!=null?1:0;coopReady.Checked=coopSnapshot?.Players[slot].Ready??false;}finally{coopUpdating=false;}
        if(coopConnecting){coopState.Text="Connexion au salon…";return;}
        if(coopClient is {IsConnected:false}){coopState.Text="Le salon a été interrompu. Quitte la préparation pour te reconnecter.";coopReady.Enabled=false;return;}
        if(coopSnapshot==null){coopState.Text="Aucun salon ouvert.";coopPlayers.Text="La préparation enregistrée sera retrouvée à la prochaine ouverture.";return;}
        coopState.Text=coopLocal!=null?"Préparation locale enregistrée.":coopSnapshot.BothReady?"Les deux joueurs sont prêts dans le salon.":coopHost!=null?"Salon hébergé — en attente du partenaire ou de sa confirmation.":"Salon rejoint — choisis quand tu es prêt.";
        coopPlayers.Text=string.Join("\n",coopSnapshot.Players.Select(p=>$"Joueur {p.Slot+1} · {ModeYoshis[p.YoshiColor].Name} · {(p.Connected?(p.Ready?"prêt":"connecté"):"en attente")}"));
    }
    static string FriendlyCoopNetworkError(string message)=>message.Contains("differ",StringComparison.OrdinalIgnoreCase)?"Connexion refusée : le niveau, la version ou les règles diffèrent.":message.Contains("timeout",StringComparison.OrdinalIgnoreCase)?"Le partenaire ne répond plus. Quitter le salon permet d’en ouvrir un nouveau.":"Le salon a été interrompu : "+message;
    static string LocalCoopAddresses(){try{return string.Join(" / ",System.Net.NetworkInformation.NetworkInterface.GetAllNetworkInterfaces().Where(n=>n.OperationalStatus==System.Net.NetworkInformation.OperationalStatus.Up).SelectMany(n=>n.GetIPProperties().UnicastAddresses).Select(a=>a.Address).Where(a=>a.AddressFamily==AddressFamily.InterNetwork && !IPAddress.IsLoopback(a)).Select(a=>a.ToString()).Distinct());}catch(System.Net.NetworkInformation.NetworkInformationException){return "";}}

}
