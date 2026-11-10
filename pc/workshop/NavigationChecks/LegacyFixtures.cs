using System.Security.Cryptography;
using System.Text.Json;
using YoshiWorkshop;
internal static class LegacyFixtures {
 internal static List<(string Name,LevelProject Level)> Create(VanillaRom rom){
  var all=new List<(string,LevelProject)>();var one=rom.CloneFirstLevel();all.Add(("v1",one));
  var two=rom.CloneFirstLevel();two.Objects.First(p=>ObjectCatalog.IsMaskass(p.ObjectId)).HybridVariant=2;all.Add(("v2",two));
  all.Add(("v3",rom.CloneLevel(43)));
  void Paint(LevelProject level){var map=rom.ReadTerrain(43).Single();int cell=TerrainEditing.ReferencedCells(map).First(c=>map.RawTiles[c]!=65535);rom.SetTerrainCells(level,43,map.ResourceId,new Dictionary<int,(ushort,ushort)>{{cell,(65535,0)}});}
  var four=rom.CloneLevel(43);Paint(four);all.Add(("v4",four));
  var five=rom.CloneLevel(43);rom.SetRoomMusic(five,43,2);rom.SetRoomMusic(five,81,8);all.Add(("v5-music",five));
  var mixed=rom.CloneLevel(43);Paint(mixed);rom.SetRoomMusic(mixed,43,2);mixed.Objects.First(p=>ObjectCatalog.IsMaskass(p.ObjectId)).HybridVariant=3;all.Add(("v5-mixed",mixed));return all;
 }
 internal static void Emit(VanillaRom rom,string dir){Directory.CreateDirectory(dir);var hashes=new Dictionary<string,object>();foreach(var (name,level) in Create(rom)){var payload=rom.ExportPlacements(level,true);var image=rom.BuildDataOnly(level,true);File.WriteAllBytes(Path.Combine(dir,name+".yslevel"),payload);File.WriteAllBytes(Path.Combine(dir,name+".z64"),image);hashes[name]=new{payloadSha256=Convert.ToHexStringLower(SHA256.HashData(payload)),imageSha256=Convert.ToHexStringLower(SHA256.HashData(image)),version=(char)payload[7]};}File.WriteAllText(Path.Combine(dir,"manifest.json"),JsonSerializer.Serialize(hashes,ProjectStore.Json));Console.WriteLine("Old pre-021 Core produced six literal payload/image baselines.");}
}
