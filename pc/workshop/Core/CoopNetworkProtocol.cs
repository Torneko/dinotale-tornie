using System.Buffers.Binary;
using System.Net.Sockets;
using System.Text.Json;
namespace YoshiWorkshop;

public sealed record CoopNetworkOptions(int HandshakeTimeoutMilliseconds=5000,int ReadTimeoutMilliseconds=30000,int WriteTimeoutMilliseconds=5000,int HeartbeatMilliseconds=5000) {
    public void Validate(){if(HandshakeTimeoutMilliseconds is <50 or >300000 || ReadTimeoutMilliseconds is <100 or >300000 || WriteTimeoutMilliseconds is <50 or >300000 || HeartbeatMilliseconds is <50 or >60000 || HeartbeatMilliseconds>=ReadTimeoutMilliseconds)throw new InvalidDataException("Invalid cooperative transport timeout.");}
}
public sealed class CoopProtocolException(string message):IOException(message);
internal sealed class CoopPacket {
    public required int ProtocolVersion {get;init;}
    public required string Kind {get;init;}
    public required JsonElement Body {get;init;}
    public static CoopPacket Create<T>(string kind,T body)=>new(){ProtocolVersion=CoopSessionIdentity.ProtocolVersion,Kind=kind,Body=JsonSerializer.SerializeToElement(body,CoopJson.Options)};
    public T Decode<T>()=>Body.Deserialize<T>(CoopJson.Options)??throw new CoopProtocolException("Empty cooperative payload.");
}
internal sealed record CoopHello(CoopSessionIdentity Identity,int YoshiColor);
internal sealed record CoopReady(CoopSessionIdentity Identity,bool Ready);
internal sealed record CoopColor(int YoshiColor);
internal sealed record CoopInput(CoopInputFrame Input);
internal sealed record CoopError(string Code,string Message);
internal sealed record CoopEmpty;
internal sealed class CoopNetworkPeer: IAsyncDisposable {
    public const int MaxFrameBytes=65536;
    readonly TcpClient client;readonly NetworkStream stream;readonly CoopNetworkOptions options;
    readonly SemaphoreSlim sendGate=new(1,1);int disposed;
    public CoopNetworkPeer(TcpClient client,CoopNetworkOptions options){this.client=client;this.options=options;client.NoDelay=true;stream=client.GetStream();}
    public async Task SendAsync<T>(string kind,T body,CancellationToken cancellationToken){
        ObjectDisposedException.ThrowIf(Volatile.Read(ref disposed)!=0,this);var data=JsonSerializer.SerializeToUtf8Bytes(CoopPacket.Create(kind,body),CoopJson.Options);if(data.Length is <1 or >MaxFrameBytes)throw new CoopProtocolException("Cooperative frame exceeds64KiB.");
        using var timeout=CancellationTokenSource.CreateLinkedTokenSource(cancellationToken);timeout.CancelAfter(options.WriteTimeoutMilliseconds);await sendGate.WaitAsync(timeout.Token).ConfigureAwait(false);
        try{byte[] header=new byte[4];BinaryPrimitives.WriteInt32BigEndian(header,data.Length);await stream.WriteAsync(header,timeout.Token).ConfigureAwait(false);await stream.WriteAsync(data,timeout.Token).ConfigureAwait(false);}finally{sendGate.Release();}
    }
    public async Task<CoopPacket> ReadAsync(CancellationToken cancellationToken,bool handshake=false){
        ObjectDisposedException.ThrowIf(Volatile.Read(ref disposed)!=0,this);using var timeout=CancellationTokenSource.CreateLinkedTokenSource(cancellationToken);timeout.CancelAfter(handshake?options.HandshakeTimeoutMilliseconds:options.ReadTimeoutMilliseconds);byte[] header=new byte[4];await stream.ReadExactlyAsync(header,timeout.Token).ConfigureAwait(false);int length=BinaryPrimitives.ReadInt32BigEndian(header);if(length is <1 or >MaxFrameBytes)throw new CoopProtocolException("Cooperative frame length outside1..65536.");byte[] data=new byte[length];await stream.ReadExactlyAsync(data,timeout.Token).ConfigureAwait(false);
        CoopPacket packet;try{packet=JsonSerializer.Deserialize<CoopPacket>(data,CoopJson.Options)??throw new CoopProtocolException("Empty cooperative frame.");}catch(JsonException e){throw new CoopProtocolException("Invalid cooperative JSON: "+e.Message);}
        if(packet.ProtocolVersion!=CoopSessionIdentity.ProtocolVersion || string.IsNullOrEmpty(packet.Kind) || packet.Kind.Length>32 || packet.Kind.Any(c=>!char.IsAsciiLetter(c) && c!='-') || packet.Body.ValueKind!=JsonValueKind.Object)throw new CoopProtocolException("Unsupported cooperative envelope.");return packet;
    }
    public ValueTask DisposeAsync(){if(Interlocked.Exchange(ref disposed,1)==0){try{client.Client.Shutdown(SocketShutdown.Both);}catch(SocketException){}catch(ObjectDisposedException){}stream.Dispose();client.Dispose();}return ValueTask.CompletedTask;}
}
