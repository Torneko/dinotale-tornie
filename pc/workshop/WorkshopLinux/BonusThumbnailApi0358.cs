using System.Text.Json;
using YoshiWorkshop;

internal static class BonusThumbnailApi0358 {
    const int MaxBytes=1024*1024;
    static string Language(string root) {
        try {
            string path=Path.Combine(root,"profiles","content-personal","book-menu.json");
            using var stream=File.OpenRead(path);if(stream.Length>4096)return "fr";
            using var json=JsonDocument.Parse(stream);
            return json.RootElement.ValueKind==JsonValueKind.Object&&json.RootElement.TryGetProperty("language",out var language)&&language.ValueKind==JsonValueKind.String&&language.GetString()=="en"?"en":"fr";
        }catch(Exception e)when(e is IOException or JsonException or UnauthorizedAccessException){return "fr";}
    }
    internal static void Map(WebApplication app,string root,object gate) {
        var thumbnails=new BonusThumbnailStore(root);
        object Slot(int bonus) {
            try {bool available=thumbnails.Read(bonus)!=null;return new{bonus,available,status=available?"ready":"empty"};}
            catch(Exception e)when(e is IOException or InvalidDataException or UnauthorizedAccessException){return new{bonus,available=false,status="unavailable"};}
        }
        app.MapGet("/api/bonus-thumbnails",()=>{
            lock(gate) return Results.Json(new{language=Language(root),slots=Enumerable.Range(1,6).Select(Slot).ToArray()});
        });
        app.MapGet("/api/bonus-thumbnails/{bonus:int}",(int bonus,HttpContext context)=>{
            byte[]? png;lock(gate) png=thumbnails.Read(bonus);
            context.Response.Headers.CacheControl="no-store";
            return png is null?Results.NotFound():Results.File(png,"image/png");
        });
        app.MapPost("/api/bonus-thumbnails/{bonus:int}",async(int bonus,HttpRequest request)=>{
            if(request.ContentLength is >MaxBytes)throw new InvalidDataException("Capture trop volumineuse. / Screenshot is too large.");
            using var output=new MemoryStream();byte[] buffer=new byte[16384];
            while(true) {
                int count=await request.Body.ReadAsync(buffer,request.HttpContext.RequestAborted);
                if(count==0)break;
                if(output.Length+count>MaxBytes)throw new InvalidDataException("Capture trop volumineuse. / Screenshot is too large.");
                output.Write(buffer,0,count);
            }
            lock(gate) thumbnails.Import(bonus,output.ToArray());
            return Results.Json(new {ok=true,bonus});
        });
    }
}
