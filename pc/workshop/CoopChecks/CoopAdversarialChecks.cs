using System.Buffers.Binary;
using System.Diagnostics;
using System.Net;
using System.Net.Sockets;
using System.Text;
using System.Text.Json;
using YoshiWorkshop;

internal static class CoopAdversarialChecks {
    static void Need(bool value,string message){if(!value)throw new Exception(message);}
    static async Task Until(Func<bool> condition){var timer=Stopwatch.StartNew();while(!condition()){if(timer.Elapsed>TimeSpan.FromSeconds(5))throw new TimeoutException("Adversarial coop check stalled.");await Task.Delay(10);}}
    static async Task Write(NetworkStream stream,string kind,object body){var data=JsonSerializer.SerializeToUtf8Bytes(new{ProtocolVersion=1,Kind=kind,Body=body},CoopJson.Options);await WriteRaw(stream,data);}
    static async Task WriteRaw(NetworkStream stream,byte[] data){byte[] header=new byte[4];BinaryPrimitives.WriteInt32BigEndian(header,data.Length);await stream.WriteAsync(header);await stream.WriteAsync(data);}
    static async Task<JsonDocument> Read(NetworkStream stream){using var cancel=new CancellationTokenSource(4000);byte[] header=new byte[4];await stream.ReadExactlyAsync(header,cancel.Token);int length=BinaryPrimitives.ReadInt32BigEndian(header);Need(length is >0 and <=65536,"Server response exceeded frame bound.");byte[] body=new byte[length];await stream.ReadExactlyAsync(body,cancel.Token);return JsonDocument.Parse(body);}
    public static async Task<IReadOnlyList<string>> RunAsync(CoopSessionIdentity identity){
        var done=new List<string>();var options=new CoopNetworkOptions(1500,5000,2000,250);
        // A real malicious loopback host replays an entity and a stale snapshot,
        // then supplies an invalid three-player snapshot. No native world runs.
        var listener=new TcpListener(IPAddress.Loopback,0);listener.Start();int port=((IPEndPoint)listener.LocalEndpoint).Port;var release=new TaskCompletionSource(TaskCreationOptions.RunContinuationsAsynchronously);
        var model=new CoopSession(identity);model.SetReady(0,true,identity);model.SetReady(1,true,identity);model.StartGame();var initial=model.Snapshot;
        var server=Task.Run(async()=>{using var raw=await listener.AcceptTcpClientAsync();var stream=raw.GetStream();using var hello=await Read(stream);await Write(stream,"welcome",initial);await release.Task;
            var e=new CoopAuthoritativeEvent(initial.Revision+1,CoopEntityKind.Fruit,"replayed/fruit/1",0,1,0);await Write(stream,"entity",e);await Write(stream,"entity",e);await Write(stream,"snapshot",initial with{Revision=initial.Revision+2,FruitCount=1});await Write(stream,"snapshot",initial);
            await Task.Delay(75);await Write(stream,"snapshot",initial with{Revision=initial.Revision+3,Players=initial.Players.Concat(new[]{initial.Players[0] with{Slot=2}}).ToArray()});await Task.Delay(100);
        });
        try{await using var client=await CoopNetworkClient.ConnectAsync("127.0.0.1",port,identity,options:options);int events=0;bool statusObserved=false,statusSawDisconnected=false;client.AuthoritativeEventReceived+=_=>Interlocked.Increment(ref events);client.StatusChanged+=_=>{statusSawDisconnected=!client.IsConnected;statusObserved=true;};release.SetResult();await Until(()=>!client.IsConnected && statusObserved);Need(events==1,"Repeated host entity event was applied twice.");Need(client.Snapshot.FruitCount==1 && client.Snapshot.Revision==initial.Revision+2,"Stale/invalid snapshot rewound accepted state.");Need(statusSawDisconnected,"Disconnected signal fired before IsConnected=false.");await server;done.Add("Actual host-event replay dedup; stale snapshot rejection; three-slot snapshot rejected; disconnect signal ordering");}finally{listener.Stop();}
        await using(var host=CoopNetworkHost.Start(IPAddress.Loopback,0,identity,options:options)){
            using var raw=new TcpClient();await raw.ConnectAsync(IPAddress.Loopback,host.Port);var stream=raw.GetStream();await Write(stream,"hello",new{Identity=identity,YoshiColor=1});using(var welcome=await Read(stream))Need(welcome.RootElement.GetProperty("Kind").GetString()=="welcome","Raw client did not join.");await host.SetHostReadyAsync(true);await Write(stream,"ready",new{Identity=identity,Ready=true});await Until(()=>host.Snapshot.BothReady);await host.StartGameAsync();
            // Guest cannot choose a slot in its input command.
            await Write(stream,"input",new{Slot=0,Input=new CoopInputFrame(1,0x8000,0,0)});bool sawError=false;for(int i=0;i<8 && !sawError;i++){using var packet=await Read(stream);sawError=packet.RootElement.GetProperty("Kind").GetString()=="error";}await Until(()=>!host.Snapshot.Players[1].Connected);Need(sawError && host.Snapshot.Players[0].LastInputSequence==0 && host.Snapshot.Players[1].LastInputSequence==0,"Guest spoofed a host input slot.");done.Add("Actual running guest cannot spoof the host slot or unknown input fields");
        }
        var shortOptions=new CoopNetworkOptions(300,2000,1000,100);
        await using(var host=CoopNetworkHost.Start(IPAddress.Loopback,0,identity,options:shortOptions)){
            // Exactly64KiB is accepted by the framing layer and rejected by
            // the typed hello schema, proving that the limit is not off by one.
            string prefix="{\"ProtocolVersion\":1,\"Kind\":\"hello\",\"Body\":{\"Unexpected\":\"",suffix="\"}}";var exact=Encoding.UTF8.GetBytes(prefix+new string('x',65536-prefix.Length-suffix.Length)+suffix);Need(exact.Length==65536,"Bad exact-limit fixture.");
            using(var raw=new TcpClient()){await raw.ConnectAsync(IPAddress.Loopback,host.Port);var stream=raw.GetStream();await WriteRaw(stream,exact);using var error=await Read(stream);Need(error.RootElement.GetProperty("Kind").GetString()=="error" && host.LastError?.Contains("Unexpected",StringComparison.Ordinal)==true,"Exact-limit JSON bypassed typed schema or was rejected by the wrong bound.");}
            string nested="{\"ProtocolVersion\":1,\"Kind\":\"hello\",\"Body\":{\"nested\":"+new string('[',30)+"0"+new string(']',30)+"}}";
            using(var raw=new TcpClient()){await raw.ConnectAsync(IPAddress.Loopback,host.Port);var stream=raw.GetStream();await WriteRaw(stream,Encoding.UTF8.GetBytes(nested));using var error=await Read(stream);Need(error.RootElement.GetProperty("Kind").GetString()=="error","Deep JSON was accepted.");}
            using(var raw=new TcpClient()){await raw.ConnectAsync(IPAddress.Loopback,host.Port);var stream=raw.GetStream();byte[] header=new byte[4];BinaryPrimitives.WriteInt32BigEndian(header,100);await stream.WriteAsync(header);await stream.WriteAsync(new byte[]{(byte)'{'});raw.Client.Shutdown(SocketShutdown.Send);using var error=await Read(stream);Need(error.RootElement.GetProperty("Kind").GetString()=="error","Truncated body was accepted.");}
            using(var raw=new TcpClient()){await raw.ConnectAsync(IPAddress.Loopback,host.Port);var stream=raw.GetStream();await stream.WriteAsync(new byte[]{0,0});using var error=await Read(stream);Need(error.RootElement.GetProperty("Kind").GetString()=="error","Partial-prefix timeout was not rejected.");}
            await using var valid=await CoopNetworkClient.ConnectAsync("127.0.0.1",host.Port,identity,options:options);Need(valid.IsConnected,"Listener failed after malformed peers.");done.Add("Exact64KiB framing, depth bound, truncated body and partial-prefix timeout; listener survives");
        }
        // Cancellation during the welcome wait must close the socket, not
        // leave an orphan connection after the canceled UI action returns.
        var silent=new TcpListener(IPAddress.Loopback,0);silent.Start();port=((IPEndPoint)silent.LocalEndpoint).Port;
        var helloReceived=new TaskCompletionSource(TaskCreationOptions.RunContinuationsAsynchronously);var accepted=Task.Run(async()=>{using var raw=await silent.AcceptTcpClientAsync();using var hello=await Read(raw.GetStream());helloReceived.SetResult();using var deadline=new CancellationTokenSource(3000);byte[] one=new byte[1];return await raw.GetStream().ReadAsync(one,deadline.Token);});
        try{using var cancel=new CancellationTokenSource();var connecting=CoopNetworkClient.ConnectAsync("127.0.0.1",port,identity,cancellationToken:cancel.Token,options:options);await helloReceived.Task.WaitAsync(TimeSpan.FromSeconds(3));cancel.Cancel();bool canceled=false;try{await using var client=await connecting;}catch(OperationCanceledException){canceled=true;}Need(canceled && await accepted.WaitAsync(TimeSpan.FromSeconds(3))==0,"Canceled welcome wait leaked a socket.");done.Add("Cancellation during an actual pending handshake closes its loopback socket");}finally{silent.Stop();}
        return done;
    }
}
