using System.Buffers.Binary;
using System.Security.Cryptography;
using System.Text.Json;
using YoshiWorkshop;
string pc=Path.GetFullPath(args[0]),outdir=Path.Combine(pc,"build/editor-0160");Directory.CreateDirectory(outdir);var rom=new VanillaRom(new SpecialContentRunner(pc).OriginalRom);int checkedMaps=0,animated=0,sharedPages=0;
foreach(int root in VanillaRom.SourceZones)foreach(int room in NativeRooms.For(root))foreach(var m in rom.ReadTerrain(room)){
 sharedPages+=m.Pages.Count(v=>v!=255)-m.Pages.Where(v=>v!=255).Distinct().Count();checkedMaps++;if(m.RawTiles.Length!=m.Tiles.Length)throw new Exception("Raw tile data missing");for(int i=0;i<m.Tiles.Length;i++)if(m.Resolve(m.RawTiles[i])!=m.Tiles[i])throw new Exception("Native animation lost");animated+=m.RawTiles.Count(v=>v>=32768 && v!=65535);
}
var project=rom.CloneLevel(43);var first=rom.ReadTerrain(43).Single();var secondary=rom.ReadTerrain(81).Single();byte[] before=rom.ExportPlacements(project);if(before[7]!='3')throw new Exception("Legacy export changed");
var originals=project.AllRooms().ToDictionary(r=>r.SourceZone,r=>rom.ReadTerrain(r.SourceZone).Single());
int a=first.CellIndex(8,12),b=first.CellIndex(9,12),c=secondary.CellIndex(4,12);
int source=Array.FindIndex(first.Collisions,v=>v!=0);ushort tile=first.RawTiles[source],collision=first.Collisions[source];
if(!rom.SetTerrainCells(project,43,first.ResourceId,new Dictionary<int,(ushort,ushort)>{{a,(tile,collision)},{b,(65535,0)}}))throw new Exception("Painting did not edit");
rom.SetTerrainCells(project,81,secondary.ResourceId,new Dictionary<int,(ushort,ushort)>{{c,(65535,0)}});
if(project.Schema!=3 || project.Terrain.Count==0)throw new Exception("Terrain schema not upgraded");rom.ValidateTerrain(project);
var preview=first.Preview(project.Terrain);if(preview.RawTiles[a]!=tile || preview.Collisions[a]!=collision || first.RawTiles[a]==tile && first.Collisions[a]==collision)throw new Exception("Preview or source isolation failed");
if(rom.SetTerrainCells(project,43,first.ResourceId,new Dictionary<int,(ushort,ushort)>{{a,(tile,collision)}}))throw new Exception("No-op created an edit");
var store=new ProjectStore(Path.Combine(outdir,"validation-core/content"));store.Save(project);var restored=store.Levels().Single(l=>l.Id==project.Id);if(JsonSerializer.Serialize(project,ProjectStore.Json)!=JsonSerializer.Serialize(restored,ProjectStore.Json))throw new Exception("Terrain persistence failed");
var duplicate=store.Duplicate(restored);duplicate.Terrain[0].Tile=65535;if(project.Terrain[0].Tile==65535)throw new Exception("Duplicate shares terrain edits");
int rejected=0;void Reject(Action action){try{action();}catch(InvalidDataException){rejected++;return;}throw new Exception("Invalid terrain accepted");}
Reject(()=>rom.SetTerrainCells(project,43,first.ResourceId,new Dictionary<int,(ushort,ushort)>{{-1,(0,0)}}));Reject(()=>rom.SetTerrainCells(project,43,first.ResourceId,new Dictionary<int,(ushort,ushort)>{{65535,(0,0)}}));Reject(()=>rom.SetTerrainCells(project,43,first.ResourceId,new Dictionary<int,(ushort,ushort)>{{a,(32767,0)}}));Reject(()=>rom.SetTerrainCells(project,43,first.ResourceId,new Dictionary<int,(ushort,ushort)>{{a,(tile,0xBEEF)}}));
var invalid=JsonSerializer.Deserialize<LevelProject>(JsonSerializer.Serialize(project,ProjectStore.Json),ProjectStore.Json)!;invalid.Schema=2;Reject(()=>ProjectStore.Validate(invalid));invalid.Schema=3;invalid.Terrain.Add(invalid.Terrain[0]);Reject(()=>ProjectStore.Validate(invalid));
byte[] payload=rom.ExportPlacements(restored),data=rom.BuildDataOnly(restored);if(payload[7]!='4' || data.Length<=16777216 || BinaryPrimitives.ReadInt32BigEndian(data.AsSpan(0xA79D4+60))!=16777216)throw new Exception("V4 terrain content missing");
File.WriteAllBytes(Path.Combine(outdir,"terrain.yslevel"),payload);File.WriteAllBytes(Path.Combine(outdir,"terrain-csharp.z64"),data);
var untouched=rom.CloneLevel(43);if(!before.AsSpan().SequenceEqual(rom.ExportPlacements(untouched)))throw new Exception("Legacy V3 bytes changed");
var rootOnly=rom.CloneFirstLevel();if(rom.ExportPlacements(rootOnly)[7]!='1')throw new Exception("Legacy V1 changed");rootOnly.Objects.First(o=>ObjectCatalog.IsMaskass(o.ObjectId)).HybridVariant=2;if(rom.ExportPlacements(rootOnly,true)[7]!='2')throw new Exception("Legacy V2 changed");
// Return each edited cell to its original value: no permanent delta remains.
foreach(var room in restored.AllRooms()){var map=originals[room.SourceZone];rom.SetTerrainCells(restored,room.SourceZone,map.ResourceId,room.Terrain.ToDictionary(t=>t.Cell,t=>(map.RawTiles[t.Cell],map.Collisions[t.Cell])));}
if(restored.AllRooms().Any(r=>r.Terrain.Count!=0) || !before.AsSpan().SequenceEqual(rom.ExportPlacements(restored)))throw new Exception("Restoring terrain changed legacy export");
for(int y=18;y<32;y++)Console.WriteLine($"row {y}: "+string.Concat(Enumerable.Range(0,40).Select(x=>first.CollisionAt(first.WorldX+x*16+8,first.WorldY+y*16+8)!=0?'#':'.')));
// A raised strip provides a native gameplay check against the untouched level.
var baseline=rom.CloneLevel(43);File.WriteAllBytes(Path.Combine(outdir,"physics-baseline.yslevel"),rom.ExportPlacements(baseline));
var strip=rom.CloneLevel(43);int solid=Enumerable.Range(0,first.RawTiles.Length).First(i=>first.Collisions[i]!=0 && first.RawTiles[i]!=65535 && Enumerable.Range(0,256).Any(n=>(first.Palette[first.Pixels[first.Resolve(first.RawTiles[i])*256+n]]>>24)!=0));
var platform=Enumerable.Range(4,25).ToDictionary(x=>first.CellIndex(x,24),x=>(first.RawTiles[solid],first.Collisions[solid]));rom.SetTerrainCells(strip,43,first.ResourceId,platform);File.WriteAllBytes(Path.Combine(outdir,"physics-raised.yslevel"),rom.ExportPlacements(strip));File.WriteAllBytes(Path.Combine(outdir,"physics-raised.z64"),rom.BuildDataOnly(strip));
void Words(string name,ushort[] words){byte[] bytes=new byte[words.Length*2];for(int i=0;i<words.Length;i++)BinaryPrimitives.WriteUInt16BigEndian(bytes.AsSpan(i*2),words[i]);File.WriteAllBytes(Path.Combine(outdir,name),bytes);}
Words("baseline-43-tiles.bin",first.RawTiles);Words("baseline-43-collision.bin",first.Collisions);Words("raised-43-tiles.bin",first.Preview(strip.Terrain).RawTiles);Words("raised-43-collision.bin",first.Preview(strip.Terrain).Collisions);
Console.WriteLine($"Platform brush: tile={first.RawTiles[solid]} collision={first.Collisions[solid]:X4}; shared native pages={sharedPages}");
var result=new{maps=checkedMaps,sharedPages,animatedCellsPreserved=animated,multiRoomTerrain=true,previewSourceUnchanged=true,persistenceAndDuplicateIsolation=true,invalidCasesRejected=rejected,legacyV1V2V3Unchanged=true,returnToOriginalRemovesDeltas=true,bank15DataOnly=true,payloadBytes=payload.Length,contentBytes=data.Length,primaryCell=a,primaryTile=tile,primaryCollision=collision,originalPrimaryTile=first.RawTiles[a],originalPrimaryCollision=first.Collisions[a],worldX=first.WorldX,worldY=first.WorldY,sha256=Convert.ToHexStringLower(SHA256.HashData(data))};File.WriteAllText(Path.Combine(outdir,"terrain-core.json"),JsonSerializer.Serialize(result,ProjectStore.Json));Console.WriteLine(JsonSerializer.Serialize(result));

string matrix=Path.Combine(outdir,"matrix");Directory.CreateDirectory(matrix);var hashes=new Dictionary<string,string>();int matrixRooms=0;
foreach(int root in VanillaRom.SourceZones){var candidate=rom.CloneLevel(root);foreach(var room in candidate.AllRooms()){var map=rom.ReadTerrain(room.SourceZone).Single();int cell=TerrainEditing.ReferencedCells(map).First(i=>map.RawTiles[i]!=65535 || map.Collisions[i]!=0);rom.SetTerrainCells(candidate,room.SourceZone,map.ResourceId,new Dictionary<int,(ushort,ushort)>{{cell,(65535,0)}});matrixRooms++;}string name="level-"+root;File.WriteAllBytes(Path.Combine(matrix,name+".yslevel"),rom.ExportPlacements(candidate));hashes[name]=Convert.ToHexStringLower(SHA256.HashData(rom.BuildDataOnly(candidate)));}
File.WriteAllText(Path.Combine(matrix,"expected-sha256.json"),JsonSerializer.Serialize(hashes,ProjectStore.Json));Console.WriteLine($"Native terrain fixtures: {hashes.Count} levels / {matrixRooms} rooms.");
