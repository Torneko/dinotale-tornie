using System.Security.Cryptography;
using System.Text.Json;
using YoshiWorkshop;
string pc=Path.GetFullPath(args[0]);var rom=new VanillaRom(new SpecialContentRunner(pc).OriginalRom);var reports=new List<object>();int total=0,covered=0;var failures=new List<string>();
foreach(int root in VanillaRom.SourceZones)foreach(int room in NativeRooms.For(root)){
 total++;try{var terrain=rom.ReadTerrain(room);if(terrain.Count>0)covered++;foreach(var map in terrain){if(map.Tiles.Length!=map.Collisions.Length || map.Pages.Any(v=>v!=255 && v>=map.LoadedPages))throw new Exception("Invalid map indexing");}
 reports.Add(new{root,room,maps=terrain.Select(m=>new{m.ResourceId,m.Width,m.Height,tiles=m.Tiles.Length,colliding=m.Collisions.Count(v=>v!=0)}),passages=rom.ReadRoom(room).Objects.Select(p=>rom.Passage(p.ObjectId)).Where(p=>p!=null).ToList()});}
 catch(Exception e){failures.Add($"{room}: {e.Message}");}
}
// Literal, overlapping-copy, truncation and invalid-distance codec cases.
byte[] sample=new byte[39];"CMPR"u8.CopyTo(sample);"SMSR00"u8.CopyTo(sample.AsSpan(16));System.Buffers.Binary.BinaryPrimitives.WriteInt32BigEndian(sample.AsSpan(24),6);System.Buffers.Binary.BinaryPrimitives.WriteInt32BigEndian(sample.AsSpan(28),4);sample[32]=0xE0;sample[35]=2;"abc"u8.CopyTo(sample.AsSpan(36));if(!SmsrBlock.Decode(sample,6).AsSpan().SequenceEqual("abcabc"u8))throw new Exception("SMSR copy failed");
void RejectBlock(byte[] input){try{SmsrBlock.Decode(input,6);}catch(InvalidDataException){return;}throw new Exception("Malformed terrain accepted");}RejectBlock(sample[..37]);var invalid=(byte[])sample.Clone();invalid[32]=0;RejectBlock(invalid);invalid=(byte[])sample.Clone();invalid[0]=0;RejectBlock(invalid);
var first=rom.ReadTerrain(43).Single();if(first.CellIndex(0,16)!=416 || first.CellIndex(0,1)!=832)throw new Exception("Wrong native vertical page order");
if(rom.Passage(0x4208) is not {Destination:81,ArrivalX:0,ArrivalY:240})throw new Exception("Wrong 1-1 pot destination");
string outdir=Path.Combine(pc,"build/editor-0150");Directory.CreateDirectory(outdir);File.WriteAllBytes(Path.Combine(outdir,"terrain43.argb"),System.Runtime.InteropServices.MemoryMarshal.AsBytes<int>(first.Rasterize()).ToArray());
var result=new{total,covered,failures,smsrLiteralAndCopyVerified=true,malformedBlocksRejected=true,nativeFirstPageOrderVerified=true,potDestinationVerified=true,previewDoesNotAlterLevelFormat=true,rooms=reports};File.WriteAllText(Path.Combine(outdir,"terrain-core.json"),JsonSerializer.Serialize(result,ProjectStore.Json));Console.WriteLine($"Terrain: {covered}/{total} rooms, {failures.Count} errors");foreach(var e in failures)Console.WriteLine(e);if(failures.Count>0)throw new Exception("Unsupported terrain data");
