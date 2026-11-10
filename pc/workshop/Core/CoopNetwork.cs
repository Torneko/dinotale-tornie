using System.Net;
using System.Net.Sockets;
using System.Text.Json;
namespace YoshiWorkshop;

// Explicit TCP endpoint only: no relay, discovery, port forwarding or firewall
// changes. Events run on background tasks; UI subscribers marshal their own UI.
public sealed class CoopNetworkHost:IAsyncDisposable {
    readonly TcpListener listener;readonly IPEndPoint endpoint;readonly CoopNetworkOptions options;readonly CoopSession session;
    readonly CancellationTokenSource lifetime=new();readonly SemaphoreSlim operations=new(1,1);
    readonly TaskCompletionSource<CoopSessionSnapshot> guestReady=new(TaskCreationOptions.RunContinuationsAsynchronously);
    CoopNetworkPeer? peer;Task acceptTask=Task.CompletedTask,guestTask=Task.CompletedTask,heartbeatTask=Task.CompletedTask;int disposed;
    public event Action<CoopSessionSnapshot>? SnapshotChanged;
    public event Action<string>? StatusChanged;
    public string? LastError {get;private set;}
    public int Port=>endpoint.Port;
    public IPEndPoint Endpoint=>new(endpoint.Address,endpoint.Port);
    public CoopSessionSnapshot Snapshot=>session.Snapshot;
    public CoopSessionIdentity Identity=>session.Identity;
    CoopNetworkHost(IPAddress address,int port,CoopSessionIdentity identity,int color,CoopNetworkOptions options){options.Validate();if(port is <0 or >65535)throw new InvalidDataException("Invalid listening port.");this.options=options;session=new(identity,CoopSessionMode.Host,color);listener=new(address,port);listener.Start(2);endpoint=(IPEndPoint)listener.LocalEndpoint;acceptTask=AcceptLoopAsync();}
    public static CoopNetworkHost Start(IPAddress bindAddress,int port,CoopSessionIdentity identity,int hostColor=0,CoopNetworkOptions? options=null)=>new(bindAddress,port,identity,hostColor,options??new());
    static void Notify<T>(Action<T>? handler,T value){if(handler is null)return;foreach(Action<T> action in handler.GetInvocationList())try{action(value);}catch{/*A UI observer cannot tear down authority.*/}}
    void Status(string value){LastError=value;Notify(StatusChanged,value);}
    public Task<CoopSessionSnapshot> WaitForGuestAsync(CancellationToken token=default)=>guestReady.Task.WaitAsync(token);
    async Task AcceptLoopAsync(){
        while(!lifetime.IsCancellationRequested){CoopNetworkPeer? candidate=null;try{
            var client=await listener.AcceptTcpClientAsync(lifetime.Token).ConfigureAwait(false);candidate=new(client,options);
            if(peer is not null){await candidate.SendAsync("error",new CoopError("full","Two-player lobby is full."),lifetime.Token).ConfigureAwait(false);await candidate.DisposeAsync();continue;}
            var helloPacket=await candidate.ReadAsync(lifetime.Token,true).ConfigureAwait(false);if(helloPacket.Kind!="hello")throw new CoopProtocolException("Expected cooperative hello.");var hello=helloPacket.Decode<CoopHello>();if(hello.Identity is null)throw new CoopProtocolException("Missing content identity.");Identity.RequireCompatible(hello.Identity);
            await operations.WaitAsync(lifetime.Token).ConfigureAwait(false);try{session.AttachGuest(hello.Identity,hello.YoshiColor);peer=candidate;await candidate.SendAsync("welcome",Snapshot,lifetime.Token).ConfigureAwait(false);}finally{operations.Release();}
            var established=candidate;candidate=null;LastError=null;guestReady.TrySetResult(Snapshot);Notify(SnapshotChanged,Snapshot);guestTask=GuestLoopAsync(established);heartbeatTask=HeartbeatAsync(established);
        }catch(OperationCanceledException) when(lifetime.IsCancellationRequested){break;}catch(Exception e) when(e is IOException or SocketException or OperationCanceledException or JsonException or InvalidDataException or InvalidOperationException){
            if(lifetime.IsCancellationRequested)break;Status(e.Message);if(candidate is not null){try{await candidate.SendAsync("error",new CoopError("incompatible",e.Message),lifetime.Token).ConfigureAwait(false);}catch(Exception){}await candidate.DisposeAsync();if(ReferenceEquals(peer,candidate)){peer=null;session.DetachGuest();}}
        }finally{if(candidate is not null)await candidate.DisposeAsync();}}
    }
    async Task GuestLoopAsync(CoopNetworkPeer established){
        try{while(!lifetime.IsCancellationRequested){var packet=await established.ReadAsync(lifetime.Token).ConfigureAwait(false);if(packet.Kind=="pong")continue;if(packet.Kind=="ping"){await established.SendAsync("pong",new CoopEmpty(),lifetime.Token).ConfigureAwait(false);continue;}
            await operations.WaitAsync(lifetime.Token).ConfigureAwait(false);try{
                switch(packet.Kind){
                    case "ready":var ready=packet.Decode<CoopReady>();if(ready.Identity is null)throw new CoopProtocolException("Missing ready identity.");session.SetReady(1,ready.Ready,ready.Identity);break;
                    case "color":session.SetColor(1,packet.Decode<CoopColor>().YoshiColor);break;
                    case "input":var input=packet.Decode<CoopInput>().Input??throw new CoopProtocolException("Missing input.");if(!session.TryInput(1,input))throw new CoopProtocolException("Stale or excessive input sequence.");break;
                    default:throw new CoopProtocolException("Guest command is not authorized: "+packet.Kind);
                }
                await established.SendAsync("snapshot",Snapshot,lifetime.Token).ConfigureAwait(false);
            }finally{operations.Release();}Notify(SnapshotChanged,Snapshot);
        }}catch(Exception e) when(e is IOException or SocketException or OperationCanceledException or JsonException or InvalidDataException or InvalidOperationException){if(!lifetime.IsCancellationRequested){Status(e.Message);try{await established.SendAsync("error",new CoopError("protocol",e.Message),lifetime.Token).ConfigureAwait(false);}catch(Exception){}}}
        finally{await established.DisposeAsync();await operations.WaitAsync().ConfigureAwait(false);try{if(ReferenceEquals(peer,established)){peer=null;session.DetachGuest();}}finally{operations.Release();}Notify(SnapshotChanged,Snapshot);}
    }
    async Task HeartbeatAsync(CoopNetworkPeer established){try{using var timer=new PeriodicTimer(TimeSpan.FromMilliseconds(options.HeartbeatMilliseconds));while(await timer.WaitForNextTickAsync(lifetime.Token).ConfigureAwait(false)){if(!ReferenceEquals(peer,established))return;await established.SendAsync("ping",new CoopEmpty(),lifetime.Token).ConfigureAwait(false);}}catch(Exception e) when(e is IOException or SocketException or OperationCanceledException or ObjectDisposedException){if(!lifetime.IsCancellationRequested && ReferenceEquals(peer,established)){Status(e.Message);await established.DisposeAsync();}}}
    async Task PublishLockedAsync(CancellationToken token){if(peer is {} guest)await guest.SendAsync("snapshot",Snapshot,token).ConfigureAwait(false);}
    async Task<CoopSessionSnapshot> ChangeAsync(Action change,CancellationToken token){ObjectDisposedException.ThrowIf(disposed!=0,this);using var linked=CancellationTokenSource.CreateLinkedTokenSource(token,lifetime.Token);await operations.WaitAsync(linked.Token).ConfigureAwait(false);try{change();await PublishLockedAsync(linked.Token).ConfigureAwait(false);}finally{operations.Release();}var snapshot=Snapshot;Notify(SnapshotChanged,snapshot);return snapshot;}
    public Task<CoopSessionSnapshot> SetHostReadyAsync(bool ready,CancellationToken token=default)=>ChangeAsync(()=>session.SetReady(0,ready,Identity),token);
    public Task<CoopSessionSnapshot> SetHostColorAsync(int color,CancellationToken token=default)=>ChangeAsync(()=>session.SetColor(0,color),token);
    public Task<CoopSessionSnapshot> StartGameAsync(CancellationToken token=default)=>ChangeAsync(()=>{if(!session.StartGame())throw new InvalidOperationException("Both compatible players must be ready.");},token);
    public Task<CoopSessionSnapshot> SendHostInputAsync(CoopInputFrame input,CancellationToken token=default)=>ChangeAsync(()=>{if(!session.TryInput(0,input))throw new InvalidDataException("Invalid host input sequence.");},token);
    public Task<CoopSessionSnapshot> SetHealthAsync(int slot,int health,CancellationToken token=default)=>ChangeAsync(()=>session.SetHealth(slot,health),token);
    public Task<CoopSessionSnapshot> AdvanceAsync(TimeSpan elapsed,CancellationToken token=default)=>ChangeAsync(()=>session.Advance(elapsed),token);
    public Task<CoopSessionSnapshot> CompleteScenarioAsync(CancellationToken token=default)=>ChangeAsync(()=>session.CompleteScenario(),token);
    public Task<CoopSessionSnapshot> PublishSnapshotAsync(CancellationToken token=default)=>ChangeAsync(()=>{},token);
    public void RegisterEntities(IEnumerable<CoopWorldEntity> entities)=>session.RegisterEntities(entities);
    async Task<CoopAuthoritativeEvent?> EntityAsync(int slot,string id,bool fruit,CancellationToken token){ObjectDisposedException.ThrowIf(disposed!=0,this);using var linked=CancellationTokenSource.CreateLinkedTokenSource(token,lifetime.Token);await operations.WaitAsync(linked.Token).ConfigureAwait(false);CoopAuthoritativeEvent? result;try{result=fruit?session.CollectFruit(slot,id):session.DefeatEnemy(slot,id);if(result is not null){if(peer is {} guest)await guest.SendAsync("entity",result,linked.Token).ConfigureAwait(false);await PublishLockedAsync(linked.Token).ConfigureAwait(false);}}finally{operations.Release();}if(result is not null)Notify(SnapshotChanged,Snapshot);return result;}
    public Task<CoopAuthoritativeEvent?> CollectFruitAsync(int slot,string stableId,CancellationToken token=default)=>EntityAsync(slot,stableId,true,token);
    public Task<CoopAuthoritativeEvent?> DefeatEnemyAsync(int slot,string stableId,CancellationToken token=default)=>EntityAsync(slot,stableId,false,token);
    public async ValueTask DisposeAsync(){if(Interlocked.Exchange(ref disposed,1)!=0)return;lifetime.Cancel();listener.Stop();if(peer is {} guest)await guest.DisposeAsync();guestReady.TrySetCanceled();try{await Task.WhenAll(acceptTask,guestTask,heartbeatTask).WaitAsync(TimeSpan.FromSeconds(5)).ConfigureAwait(false);}catch(Exception e) when(e is IOException or SocketException or OperationCanceledException or TimeoutException){}lifetime.Dispose();}
}

public sealed class CoopNetworkClient:IAsyncDisposable {
    readonly CoopNetworkPeer peer;readonly CoopSessionIdentity identity;readonly CancellationTokenSource lifetime=new();readonly HashSet<string> seenEntities=new(StringComparer.Ordinal);
    CoopSessionSnapshot snapshot;Task receiveTask=Task.CompletedTask;int connected=1,disposed;
    public event Action<CoopSessionSnapshot>? SnapshotChanged;
    public event Action<CoopAuthoritativeEvent>? AuthoritativeEventReceived;
    public event Action<string>? StatusChanged;
    public string? LastError {get;private set;}
    public bool IsConnected=>Volatile.Read(ref connected)!=0;
    public CoopSessionSnapshot Snapshot=>Volatile.Read(ref snapshot);
    CoopNetworkClient(CoopNetworkPeer peer,CoopSessionIdentity identity,CoopSessionSnapshot snapshot){this.peer=peer;this.identity=identity;this.snapshot=snapshot;receiveTask=ReceiveLoopAsync();}
    static void Notify<T>(Action<T>? handler,T value){if(handler is null)return;foreach(Action<T> action in handler.GetInvocationList())try{action(value);}catch{}}
    public static async Task<CoopNetworkClient> ConnectAsync(string ip,int port,CoopSessionIdentity identity,int guestColor=1,CancellationToken cancellationToken=default,CoopNetworkOptions? options=null){
        identity.Validate();if(!IPAddress.TryParse(ip,out var address) || port is <1 or >65535 || guestColor is <0 or >13)throw new InvalidDataException("Use an explicit valid IP, port and Yoshi color.");var transport=options??new();transport.Validate();var tcp=new TcpClient(address.AddressFamily);CoopNetworkPeer? peer=null;
        try{using var timeout=CancellationTokenSource.CreateLinkedTokenSource(cancellationToken);timeout.CancelAfter(transport.HandshakeTimeoutMilliseconds);await tcp.ConnectAsync(address,port,timeout.Token).ConfigureAwait(false);peer=new(tcp,transport);await peer.SendAsync("hello",new CoopHello(identity,guestColor),timeout.Token).ConfigureAwait(false);var welcome=await peer.ReadAsync(timeout.Token,true).ConfigureAwait(false);if(welcome.Kind=="error")throw new CoopProtocolException(welcome.Decode<CoopError>().Message);if(welcome.Kind!="welcome")throw new CoopProtocolException("Expected cooperative welcome.");var snapshot=welcome.Decode<CoopSessionSnapshot>();snapshot.Validate();identity.RequireCompatible(snapshot.Identity);return new(peer,identity,snapshot);}catch{if(peer is not null)await peer.DisposeAsync();else tcp.Dispose();throw;}
    }
    async Task ReceiveLoopAsync(){string? failure=null;try{while(!lifetime.IsCancellationRequested){var packet=await peer.ReadAsync(lifetime.Token).ConfigureAwait(false);switch(packet.Kind){
        case "ping":await peer.SendAsync("pong",new CoopEmpty(),lifetime.Token).ConfigureAwait(false);break;
        case "pong":break;
        case "snapshot":var value=packet.Decode<CoopSessionSnapshot>();value.Validate();identity.RequireCompatible(value.Identity);if(value.SessionId!=Snapshot.SessionId)throw new CoopProtocolException("Session ID changed.");if(value.Revision>=Snapshot.Revision){Volatile.Write(ref snapshot,value);Notify(SnapshotChanged,value);}break;
        case "entity":var entity=packet.Decode<CoopAuthoritativeEvent>();CoopWorldEntity.ValidateId(entity.StableId);if(entity.PlayerSlot is <0 or >1 || !Enum.IsDefined(entity.Kind) || entity.Revision<0 || entity.FruitCount is <0 or >CoopSession.MaxEntities || entity.DefeatedCount is <0 or >CoopSession.MaxEntities)throw new CoopProtocolException("Invalid authoritative world event.");if(!seenEntities.Contains(entity.StableId)){if(seenEntities.Count>=CoopSession.MaxEntities)throw new CoopProtocolException("Too many authoritative entity IDs.");seenEntities.Add(entity.StableId);Notify(AuthoritativeEventReceived,entity);}break;
        case "error":throw new CoopProtocolException(packet.Decode<CoopError>().Message);
        default:throw new CoopProtocolException("Unsupported host command.");
    }}}catch(Exception e) when(e is IOException or SocketException or OperationCanceledException or JsonException or InvalidDataException or InvalidOperationException){if(!lifetime.IsCancellationRequested){LastError=e.Message;failure=e.Message;}}finally{Volatile.Write(ref connected,0);await peer.DisposeAsync();if(failure is not null)Notify(StatusChanged,failure);}}
    public Task SetReadyAsync(bool ready,CancellationToken token=default)=>SendAsync("ready",new CoopReady(identity,ready),token);
    public Task SetColorAsync(int color,CancellationToken token=default){if(color is <0 or >13)throw new InvalidDataException("Yoshi color outside0..13.");return SendAsync("color",new CoopColor(color),token);}
    public Task SendInputAsync(CoopInputFrame input,CancellationToken token=default){input.Validate();return SendAsync("input",new CoopInput(input),token);}
    async Task SendAsync<T>(string kind,T body,CancellationToken token){ObjectDisposedException.ThrowIf(disposed!=0 || !IsConnected,this);using var linked=CancellationTokenSource.CreateLinkedTokenSource(token,lifetime.Token);await peer.SendAsync(kind,body,linked.Token).ConfigureAwait(false);}
    public async ValueTask DisposeAsync(){if(Interlocked.Exchange(ref disposed,1)!=0)return;lifetime.Cancel();await peer.DisposeAsync();try{await receiveTask.WaitAsync(TimeSpan.FromSeconds(5)).ConfigureAwait(false);}catch(Exception e) when(e is IOException or SocketException or OperationCanceledException or TimeoutException){}Volatile.Write(ref connected,0);lifetime.Dispose();}
}
