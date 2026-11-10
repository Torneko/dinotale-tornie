using YoshiWorkshop;
using System.Text.Json;
using System.Security.Cryptography;
string pc=Path.GetFullPath(args[0]),outdir=Path.Combine(pc,"build/editor-0170");Directory.CreateDirectory(outdir);var rom=new VanillaRom(new SpecialContentRunner(pc).OriginalRom);var matrix=new List<object>();int supported=0,unavailable=0,rejected=0;
void Reject(Action action){try{action();}catch(InvalidDataException){rejected++;return;}throw new Exception("Invalid bubble placement accepted");}
foreach(int root in VanillaRom.SourceZones){
 var project=rom.CloneLevel(root);var originalJson=JsonSerializer.Serialize(project.AllRooms().Select(r=>r.Objects.Select(p=>new{p.ObjectId,p.Layer,p.X,p.Y}).ToArray()).ToArray());
 foreach(var room in project.AllRooms()){
  var native=room.Objects.Where(p=>ObjectCatalog.IsFruitBubble(p.ObjectId)).OrderBy(p=>p.ObjectId).FirstOrDefault();
  var palette=rom.Palette(room.SourceZone,true).ToArray();
  if(native==null){unavailable++;if(palette.Any(ObjectCatalog.IsSpecialBubble))throw new Exception("Unsupported bubble offered");Reject(()=>rom.CreatePlacement(0x4F20,room.SourceZone));continue;}
  supported++;if(palette.Count(ObjectCatalog.IsSpecialBubble)!=6 || rom.Palette(room.SourceZone,false).Any(ObjectCatalog.IsSpecialBubble))throw new Exception("Bubble palette mismatch");
  var spawn=room.Objects.Single(p=>p.ObjectId==0x4001);room.Objects.RemoveAll(p=>p.ObjectId<0x8000 && p.ObjectId!=0x4001);
  for(int i=0;i<6;i++){var b=rom.CreatePlacement((ushort)(0x4F20+i),room.SourceZone);if(b.Layer!=native.Layer || b.ObjectId!=0x4F20+i)throw new Exception("Native parameters lost");b.X=spawn.X+64+i*48;b.Y=spawn.Y-16;room.Objects.Add(b);}
 }
 var payload=rom.ExportPlacements(project,true);File.WriteAllBytes(Path.Combine(outdir,$"bubbles-{root}.yslevel"),payload);Reject(()=>rom.BuildDataOnly(project));
 var store=new ProjectStore(Path.Combine(outdir,"qa-core/content"));store.Save(project);var restored=store.Levels().Single(p=>p.Id==project.Id);if(JsonSerializer.Serialize(restored,ProjectStore.Json)!=JsonSerializer.Serialize(project,ProjectStore.Json))throw new Exception("Bubble persistence failed");
 var copy=store.Duplicate(restored);var special=copy.AllRooms().SelectMany(r=>r.Objects).First(p=>ObjectCatalog.IsSpecialBubble(p.ObjectId));special.X+=16;if(JsonSerializer.Serialize(restored,ProjectStore.Json)!=JsonSerializer.Serialize(project,ProjectStore.Json))throw new Exception("Duplicate affected original");
 foreach(var room in project.AllRooms()){
  var alias=rom.SourceObjects(room.SourceZone).Where(ObjectCatalog.IsFruitBubble).Order().FirstOrDefault();foreach(var b in room.Objects.Where(p=>ObjectCatalog.IsSpecialBubble(p.ObjectId)))b.ObjectId=alias;
 }
 var expected=rom.BuildDataOnly(project);matrix.Add(new{root,rooms=project.AllRooms().Count(),expectedSha256=Convert.ToHexStringLower(SHA256.HashData(expected))});
 if(originalJson!=JsonSerializer.Serialize(rom.CloneLevel(root).AllRooms().Select(r=>r.Objects.Select(p=>new{p.ObjectId,p.Layer,p.X,p.Y}).ToArray()).ToArray()))throw new Exception("Source mutated");
}
// Six separate actual native scenarios: one bubble, native terrain and player.
for(int i=0;i<6;i++){
 var p=rom.CloneFirstLevel();var spawn=p.Objects.Single(x=>x.ObjectId==0x4001);p.Objects.RemoveAll(x=>x.ObjectId<0x8000 && x.ObjectId!=0x4001);var b=rom.CreatePlacement((ushort)(0x4F20+i));b.X=spawn.X+80;b.Y=spawn.Y-16;p.Objects.Add(b);File.WriteAllBytes(Path.Combine(outdir,$"native-fruit-{i}.yslevel"),rom.ExportPlacements(p,true));
}
var report=new{supportedRooms=supported,unavailableRooms=unavailable,sourceRooms=113,specialTypes=6,rejected,persistence=true,duplicateIsolation=true,matrix};File.WriteAllText(Path.Combine(outdir,"bubble-core.json"),JsonSerializer.Serialize(report,ProjectStore.Json));Console.WriteLine($"Bubble editor checks passed: {supported} compatible rooms, {unavailable} guarded rooms, six fruits, persistence/duplicate isolation, {rejected} invalid paths rejected.");
