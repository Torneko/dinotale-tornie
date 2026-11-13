using System.Diagnostics;
using System.Net;
using System.Reflection;
using System.Security.Cryptography;
using System.Text.Json;
using YoshiWorkshop;

if(args.Length==0){Console.Error.WriteLine("DinoTale Atelier: specify the portable pc folder.");Environment.ExitCode=2;return;}
string root=Path.TrimEndingDirectorySeparator(Path.GetFullPath(args[0]));
bool play=args.Contains("--play"),noBrowser=args.Contains("--no-browser");
var gate=new object();var store=new ProjectStore(Path.Combine(root,"content"));var runner=new SpecialContentRunner(root);
VanillaRom? rom=null;CustomGameSession? sessions=null;string sessionError="",startupError="",sessionSnapshot="";
VanillaRom Rom()=>rom??=new VanillaRom(runner.OriginalRom);
CustomGameSession Sessions(){if(sessions==null){sessions=new(root,runner,Rom());sessions.Changed+=()=>Volatile.Write(ref sessionSnapshot,JsonSerializer.Serialize(sessions.State,ProjectStore.Json));sessions.Error+=e=>sessionError=e;}return sessions;}
CustomSessionState? SessionSnapshot(string id){ProjectStore.ValidateId(id);string snapshot=Volatile.Read(ref sessionSnapshot);if(snapshot.Length>0){var state=JsonSerializer.Deserialize<CustomSessionState>(snapshot,ProjectStore.Json);if(state?.GameId==id)return state;}return Sessions().Read(id);}
string sessionFile=Path.Combine(root,"build/atelier-linux-session.json");
string token=Convert.ToHexString(RandomNumberGenerator.GetBytes(32));
bool SetupRequired()=>!File.Exists(runner.OriginalRom)||PublicRomInstall.Needed(root);
void EnsureNoGame(){
    if(runner.IsRunning)throw new InvalidOperationException("Fermer la partie avant d’en lancer une autre. / Close the current game first.");
    foreach(var process in Process.GetProcessesByName("DinoTale")){using(process){try{string file=Path.GetFullPath(process.MainModule?.FileName??"");if(file.StartsWith(root+Path.DirectorySeparatorChar,OperatingSystem.IsWindows()?StringComparison.OrdinalIgnoreCase:StringComparison.Ordinal))throw new InvalidOperationException("Une fenêtre DinoTale est déjà ouverte. Ferme-la avant de lancer un autre essai. / A DinoTale window is already open; close it before starting another test.");}catch(System.ComponentModel.Win32Exception){}catch(ArgumentException){}catch(InvalidOperationException e)when(e.Message.StartsWith("Process",StringComparison.Ordinal)){} }}
}
void MainGame(){
    if(SetupRequired())throw new InvalidOperationException("Sélectionner d’abord la ROM originale USA. / Select the original USA ROM first.");EnsureNoGame();runner.Verify();
    string profile=Path.Combine(root,"profiles/content-personal");Directory.CreateDirectory(profile);if(!File.Exists(Path.Combine(profile,"portable.txt")))File.WriteAllText(Path.Combine(profile,"portable.txt"),"");if(!File.Exists(Path.Combine(profile,"sound.json")))File.WriteAllText(Path.Combine(profile,"sound.json"),"{\"main_volume\":3}");
    foreach(string source in Directory.GetFiles(Path.Combine(runner.RuntimeDirectory,"assets"),"*",SearchOption.AllDirectories)){string target=Path.Combine(profile,"assets",Path.GetRelativePath(Path.Combine(runner.RuntimeDirectory,"assets"),source));Directory.CreateDirectory(Path.GetDirectoryName(target)!);File.Copy(source,target,true);}
    var start=new ProcessStartInfo(Path.Combine(runner.RuntimeDirectory,SpecialContentRunner.EngineFileName)){WorkingDirectory=profile,UseShellExecute=false};start.ArgumentList.Add("--workshop-main-menu");_ = Process.Start(start)??throw new InvalidOperationException("DinoTale n’a pas démarré. / DinoTale did not start.");
}
void Browser(string address){if(noBrowser)return;try{var open=new ProcessStartInfo("xdg-open"){UseShellExecute=false};open.Environment.Remove("LD_LIBRARY_PATH");open.Environment.Remove("DOTNET_BUNDLE_EXTRACT_BASE_DIR");open.ArgumentList.Add(address);Process.Start(open);}catch(Exception e){Console.Error.WriteLine("Navigateur / Browser: "+e.Message);}}
// Reuse only an authenticated live loopback service belonging to this root.
if(File.Exists(sessionFile)&&new FileInfo(sessionFile).Length<8192){try{
    using var prior=JsonDocument.Parse(File.ReadAllText(sessionFile));var p=prior.RootElement;string address=p.GetProperty("url").GetString()!,secret=p.GetProperty("token").GetString()!;
    if(p.GetProperty("root").GetString()==root&&Uri.TryCreate(address,UriKind.Absolute,out var uri)&&uri.Scheme=="http"&&IPAddress.TryParse(uri.Host,out var ip)&&IPAddress.IsLoopback(ip)&&secret.Length==64&&secret.All(Uri.IsHexDigit)){
        using var handler=new HttpClientHandler{AllowAutoRedirect=false};using var client=new HttpClient(handler){Timeout=TimeSpan.FromSeconds(1)};client.DefaultRequestHeaders.Add("X-Atelier-Token",secret);using var response=await client.GetAsync(address+"/api/ping");if(response.IsSuccessStatusCode){using var ping=JsonDocument.Parse(await response.Content.ReadAsStringAsync());if(ping.RootElement.GetProperty("root").GetString()==root&&ping.RootElement.GetProperty("pid").GetInt32()==p.GetProperty("pid").GetInt32()){Browser(address+"/#"+secret+(args.Contains("--custom-games")?"/games":play?"/play":""));Console.WriteLine(JsonSerializer.Serialize(new{ready=true,reused=true,url=address,token=secret,root}));return;}}
    }
}catch(Exception e)when(e is IOException or JsonException or HttpRequestException or TaskCanceledException or InvalidOperationException or KeyNotFoundException){} }
if(play&&!SetupRequired()){try{MainGame();return;}catch(Exception e){startupError=e.Message;}}
var builder=WebApplication.CreateSlimBuilder();builder.Logging.ClearProviders();builder.WebHost.ConfigureKestrel(options=>{options.Listen(IPAddress.Loopback,0);options.Limits.MaxRequestBodySize=32*1024*1024;});var app=builder.Build();
IResult Json(object value)=>Results.Json(value,ProjectStore.Json);
LevelProject Level(string id)=>store.Levels().SingleOrDefault(l=>l.Id==id)??throw new InvalidDataException("Niveau introuvable.");
DateTime lastSeen=DateTime.UtcNow;
void Room(int source,int room){if(!NativeRooms.Contains(source,room))throw new InvalidDataException("Salle étrangère au niveau.");}
object Catalog(int room)=>Rom().Palette(room,true).Select(id=>{var o=ObjectCatalog.Get(id);return new{id,name=o.Name,category=o.Category,description=o.Description,identified=o.Identified,maskass=ObjectCatalog.IsMaskass(id)};});
static int Int(JsonElement j,string name,int fallback=0)=>j.TryGetProperty(name,out var p)&&p.ValueKind!=JsonValueKind.Null?p.GetInt32():fallback;
static float? Float(JsonElement j,string name)=>j.TryGetProperty(name,out var p)&&p.ValueKind!=JsonValueKind.Null?p.GetSingle():null;
static string Text(JsonElement j,string name,string fallback="")=>j.TryGetProperty(name,out var p)&&p.ValueKind==JsonValueKind.String?p.GetString()??fallback:fallback;
static LevelProject Draft(JsonElement j)=>j.GetProperty("level").Deserialize<LevelProject>(ProjectStore.Json)??throw new InvalidDataException("Niveau requis.");
app.Use(async(context,next)=>{
    if(context.Connection.RemoteIpAddress==null||!IPAddress.IsLoopback(context.Connection.RemoteIpAddress)){context.Response.StatusCode=403;return;}
    if(context.Request.Path.StartsWithSegments("/api")&&context.Request.Headers["X-Atelier-Token"]!=token){context.Response.StatusCode=403;return;}
    try{await next();}catch(Exception e){if(context.Response.HasStarted)throw;context.Response.StatusCode=400;await context.Response.WriteAsJsonAsync(new{error=e.Message});}
});
app.MapGet("/",()=>{using var stream=Assembly.GetExecutingAssembly().GetManifestResourceStream("WorkshopLinux.index.html")!;using var reader=new StreamReader(stream);return Results.Content(reader.ReadToEnd(),"text/html; charset=utf-8");});
app.MapGet("/api/ping",()=>{lastSeen=DateTime.UtcNow;return Json(new{ok=true,pid=Environment.ProcessId,root});});app.MapPost("/api/ping",()=>{lastSeen=DateTime.UtcNow;return Json(new{ok=true});});
app.MapGet("/api/state",()=>{lock(gate){lastSeen=DateTime.UtcNow;bool setup=SetupRequired();return Json(new{editor0340=true,editor0355=true,editorSourceVersion="0.35.5",soloYoshiColors=18,editorSpecialEggCount=12,editorElementalPlacements=true,unknownYoshiMenuIdentitiesHidden=true,knownYoshiMask18=KnownYoshis0355.ReadProfile(Path.Combine(root,"profiles","content-personal")).KnownMask18,setupRequired=setup,importAvailable=PublicRomInstall.Available(root),playRequested=play,error=startupError,levels=store.Levels(),games=store.Games(),running=runner.IsRunning,sessionError,templates=setup?Array.Empty<LevelTemplate>():Rom().Templates,music=RoomMusic.Tracks,catalog=setup?Array.Empty<object>():Catalog(43)});}});
app.MapPost("/api/import",async(HttpRequest request)=>{
    if(request.ContentLength is not 16777216)throw new InvalidDataException("Une ROM USA de 16 Mio est requise (.z64, .v64, .n64). / A 16 MiB USA ROM is required.");
    string temporary=Path.Combine(Path.GetTempPath(),"dinotale-import-"+Guid.NewGuid().ToString("N")+".rom");try{await using(var output=new FileStream(temporary,FileMode.CreateNew,FileAccess.Write,FileShare.None)){await request.Body.CopyToAsync(output);}if(new FileInfo(temporary).Length!=16777216)throw new InvalidDataException("ROM tronquée.");lock(gate){if(runner.IsRunning)throw new InvalidOperationException("Fermer le jeu avant l’import.");PublicRomInstall.Install(root,temporary);rom=null;startupError="";return Json(new{ok=true});}}finally{if(File.Exists(temporary))File.Delete(temporary);}
});
app.MapPost("/api/play",()=>{lock(gate){MainGame();_=Task.Run(async()=>{await Task.Delay(250);app.Lifetime.StopApplication();});return Json(new{ok=true});}});
app.MapGet("/api/room/{source:int}/{room:int}",(int source,int room)=>{lock(gate){Room(source,room);var maps=Rom().ReadTerrain(room);var native=Rom().ReadRoom(room);return Json(new{room,catalog=Catalog(room),nativeMusic=Rom().ReadRoomMusic(room),maps=maps.Select(m=>new{m.ResourceId,m.Width,m.Height,m.TileWidth,m.TileHeight,m.PageHeight,m.WorldX,m.WorldY,m.Pages,m.LoadedPages,m.Pixels,m.Palette,m.Tiles,m.RawTiles,m.AnimationFrames,m.Collisions}),passages=native.Objects.Where(p=>Rom().Passage(p.ObjectId)!=null).Select(p=>new{p.Id,p.ObjectId,editable=Rom().CanEditPassage(p),native=Rom().Passage(p.ObjectId)}),specialRooms=NativeRooms.For(source).Select(r=>new{room=r,eggs=SpecialEditorObjects.Compatible(Rom().SourceObjects(r)),specials=SpecialEditorObjects.NativeSpecials.Where(Rom().SourceObjects(r).Contains)})});}});
app.MapPost("/api/clone",async(HttpRequest request)=>{using var input=await JsonDocument.ParseAsync(request.Body);lock(gate){string id=Text(input.RootElement,"id");var level=id.Length==0?Rom().CloneLevel(Int(input.RootElement,"source",43),!input.RootElement.TryGetProperty("allRooms",out var p)||p.GetBoolean()):store.Duplicate(Level(id));store.Save(level);return Json(level);}});
app.MapPost("/api/level",async(HttpRequest request)=>{var level=await request.ReadFromJsonAsync<LevelProject>(ProjectStore.Json)??throw new InvalidDataException("Niveau requis.");lock(gate){Rom().ExportPlacements(level,true);store.Save(level);return Json(level);}});
app.MapGet("/api/placement/{id:int}",(int id,int? room)=>{lock(gate)return Json(Rom().CreatePlacement(checked((ushort)id),room??43));});
app.MapPost("/api/edit/{operation}",async(string operation,HttpRequest request)=>{using var document=await JsonDocument.ParseAsync(request.Body);lock(gate){var j=document.RootElement;var level=Draft(j);ProjectStore.Validate(level);int room=Int(j,"room",level.SourceZone);Room(level.SourceZone,room);var objects=level.ObjectsIn(room);
    switch(operation){
        case "inspect":{RoomEntry? entry=null;string reason="";try{entry=Rom().EffectiveEntry(level,room);}catch(InvalidDataException e){reason=e.Message;}var passages=objects.Where(p=>Rom().Passage(p.ObjectId)!=null).Select(p=>{NativePassage? effective=null;try{effective=Rom().EffectivePassage(level,room,p);}catch(InvalidDataException){}return new{p.Id,p.ObjectId,editable=Rom().CanEditPassage(p),effective};}).ToArray();return Json(new{level,entry,reason,passages});}
        case "include":Rom().IncludeOriginalRooms(level);break;
        case "music":Rom().SetRoomMusic(level,room,j.TryGetProperty("music",out var m)&&m.ValueKind!=JsonValueKind.Null?m.GetInt32():null);break;
        case "entry":Rom().SetRoomEntry(level,room,Float(j,"x"),Float(j,"y"));break;
        case "passage":Rom().SetPassage(level,room,Text(j,"id"),j.TryGetProperty("destination",out var d)&&d.ValueKind!=JsonValueKind.Null?d.GetInt32():null,Float(j,"x"),Float(j,"y"));break;
        case "group":{
            var ids=j.GetProperty("ids").Deserialize<string[]>(ProjectStore.Json)??[];var chosen=ids.Select(id=>objects.Single(p=>p.Id==id)).ToArray();string action=Text(j,"action");if(action=="move")ObjectGroups.Translate(chosen,Float(j,"dx")??0,Float(j,"dy")??0);else if(action=="remove"){ObjectGroups.Copy(chosen,level.SourceZone,room);objects.RemoveAll(p=>ids.Contains(p.Id));}else if(action=="duplicate"){var clipboard=ObjectGroups.Copy(chosen,level.SourceZone,room);var center=ObjectGroups.Center(chosen);objects.AddRange(ObjectGroups.PreparePaste(Rom(),level,room,clipboard,center.X+16,center.Y+16));}else throw new InvalidDataException("Action de groupe inconnue.");break;
        }
        case "paste":{var clip=j.GetProperty("clipboard");var source=clip.GetProperty("items").Deserialize<Placement[]>(ProjectStore.Json)??[];var clipboard=ObjectGroups.Copy(source,Int(clip,"root"),Int(clip,"room"));objects.AddRange(ObjectGroups.PreparePaste(Rom(),level,room,clipboard,Float(j,"x")??0,Float(j,"y")??0));break;}
        case "terrain":{
            ushort resource=checked((ushort)Int(j,"resource"));var changes=j.GetProperty("cells").EnumerateArray().ToDictionary(c=>Int(c,"cell"),c=>(Tile:checked((ushort)Int(c,"tile")),Collision:checked((ushort)Int(c,"collision"))));Rom().SetTerrainCells(level,room,resource,changes);break;
        }
        case "section":{
            var clip=j.GetProperty("clipboard");var source=clip.GetProperty("level").Deserialize<LevelProject>(ProjectStore.Json)??throw new InvalidDataException("Section absente.");Rom().ExportPlacements(source,true);int sourceRoom=Int(clip,"room");ushort resource=checked((ushort)Int(clip,"resource"));var map=Rom().ReadTerrain(sourceRoom).Single(m=>m.ResourceId==resource).Preview(source.AllRooms().Single(r=>r.SourceZone==sourceRoom).Terrain);var section=TerrainSections.Capture(map,sourceRoom,Int(clip,"x"),Int(clip,"y"),Int(clip,"width"),Int(clip,"height"));var target=Rom().ReadTerrain(room).Single(m=>m.ResourceId==resource).Preview(level.AllRooms().Single(r=>r.SourceZone==room).Terrain);bool collision=!j.TryGetProperty("collision",out var c)||c.GetBoolean();var values=TerrainSections.PreparePaste(section,target,room,Int(j,"x"),Int(j,"y"),collision);Rom().SetTerrainCells(level,room,resource,values);break;
        }
        default:throw new InvalidDataException("Modification inconnue.");
    }Rom().ExportPlacements(level,true);return Json(new{level});
}});
app.MapGet("/api/icon/{id}",(string id)=>{string path=store.IconPath(id);return File.Exists(path)?Results.File(path,"image/png"):Results.NotFound();});
app.MapPost("/api/icon/{id}",async(string id,HttpRequest request)=>{var level=Level(id);if(request.ContentLength is >1048576)throw new InvalidDataException("Icône trop volumineuse.");using var stream=new MemoryStream();await request.Body.CopyToAsync(stream);byte[] bytes=stream.ToArray();if(bytes.Length is <24 or >1048576||!bytes.AsSpan(0,8).SequenceEqual(new byte[]{137,80,78,71,13,10,26,10}))throw new InvalidDataException("Image PNG requise (1 Mio maximum).");lock(gate){File.WriteAllBytes(store.IconPath(id),bytes);level.Icon=id+".png";store.Save(level);return Json(new{ok=true});}});
app.MapPost("/api/game",async(HttpRequest request)=>{var game=await request.ReadFromJsonAsync<CustomGame>(ProjectStore.Json)??throw new InvalidDataException("Partie requise.");lock(gate){store.Save(game);return Json(game);}});
app.MapGet("/api/session/{id}",(string id)=>{lock(gate)return Json(new{state=SessionSnapshot(id),running=runner.IsRunning,error=sessionError});});
app.MapPost("/api/game/start/{id}",async(string id,HttpRequest request)=>{using var input=await JsonDocument.ParseAsync(request.Body);lock(gate){EnsureNoGame();var game=store.Games().Single(g=>g.Id==id);bool resume=input.RootElement.TryGetProperty("resume",out var r)&&r.GetBoolean();Sessions().Start(game,store,resume);sessionError="";return Json(new{ok=true});}});
app.MapPost("/api/launch/{id}",(string id)=>{lock(gate){EnsureNoGame();runner.Launch(id,Rom().ExportPlacements(Level(id),true));return Json(new{ok=true});}});
app.MapPost("/api/prepare/{id}",(string id)=>{lock(gate)return Json(new{profile=runner.Prepare(id,Rom().ExportPlacements(Level(id),true))});});
app.MapPost("/api/quit",()=>{if(runner.IsRunning)throw new InvalidOperationException("Fermer le test avant l’Atelier pour conserver le suivi. / Close the game before closing the Workshop.");app.Lifetime.StopApplication();return Json(new{ok=true});});
await app.StartAsync();string url=app.Urls.Single();Directory.CreateDirectory(Path.Combine(root,"build"));await File.WriteAllTextAsync(sessionFile,JsonSerializer.Serialize(new{pid=Environment.ProcessId,url,token,root},ProjectStore.Json));if(OperatingSystem.IsLinux())File.SetUnixFileMode(sessionFile,UnixFileMode.UserRead|UnixFileMode.UserWrite);Console.WriteLine(JsonSerializer.Serialize(new{ready=true,pid=Environment.ProcessId,url,token,root}));Console.Out.Flush();Browser(url+"/#"+token+(args.Contains("--custom-games")?"/games":play?"/play":""));
_=Task.Run(async()=>{while(!app.Lifetime.ApplicationStopping.IsCancellationRequested){await Task.Delay(15000);if(!runner.IsRunning&&DateTime.UtcNow-lastSeen>TimeSpan.FromMinutes(15)){app.Lifetime.StopApplication();break;}}});await app.WaitForShutdownAsync();
