using System.Buffers.Binary;
using System.Security.Cryptography;
using System.Text.Json;
using YoshiWorkshop;

string pc=Path.GetFullPath(args[0]),outdir=Path.Combine(pc,"build/editor-0190");
Directory.CreateDirectory(outdir);
var rom=new VanillaRom(new SpecialContentRunner(pc).OriginalRom);
int rejected=0,maps=0,animationCases=0,emptyCases=0;
void Need(bool condition,string text){if(!condition)throw new Exception(text);}
void Reject(Action action){try{action();}catch(InvalidDataException){rejected++;return;}throw new Exception("Invalid terrain section operation accepted");}
string Snapshot(LevelProject level)=>JsonSerializer.Serialize(level,ProjectStore.Json);
string Hash(byte[] values)=>Convert.ToHexStringLower(SHA256.HashData(values));
byte[] Words(ushort[] values){byte[] bytes=new byte[values.Length*2];for(int i=0;i<values.Length;i++)BinaryPrimitives.WriteUInt16BigEndian(bytes.AsSpan(i*2),values[i]);return bytes;}
PointCell FirstCell(NativeTerrainMap map)=>Enumerable.Range(0,map.Height/16).SelectMany(y=>Enumerable.Range(0,map.Width/16).Select(x=>new PointCell(x,y))).First(p=>map.TryCell(p.X,p.Y,out _));

// Every original map can capture its current raw tile and paste it without changing the source.
foreach(int root in VanillaRom.SourceZones)foreach(int room in NativeRooms.For(root))foreach(var map in rom.ReadTerrain(room)){
    maps++;var position=FirstCell(map);int index=map.CellIndex(position.X,position.Y);
    ushort beforeTile=map.RawTiles[index],beforeCollision=map.Collisions[index];
    var clip=TerrainSections.Capture(map,room,position.X,position.Y,3,2);
    var prepared=TerrainSections.PreparePaste(clip,map,room,position.X,position.Y,true);
    Need(prepared.Count==6 && prepared.All(p=>p.Value==(map.RawTiles[p.Key],map.Collisions[p.Key])),"Map rectangle roundtrip changed raw tile or collision");
    Need(map.RawTiles[index]==beforeTile && map.Collisions[index]==beforeCollision,"Preparation changed source");
    int animated=Array.FindIndex(map.RawTiles,v=>v>=32768 && v!=65535);
    int empty=Array.FindIndex(map.RawTiles,v=>v==65535);
    foreach(int target in new[]{animated,empty}.Where(v=>v>=0)){
        var p=Enumerable.Range(0,map.Height/16).SelectMany(y=>Enumerable.Range(0,map.Width/16).Select(x=>new PointCell(x,y))).FirstOrDefault(p=>map.TryCell(p.X,p.Y,out int c) && c==target);
        if(p==null)continue;var single=TerrainSections.Capture(map,room,p.X,p.Y,1,1);
        Need(single.Cells[0].Tile==map.RawTiles[target],"Raw animated/empty tile changed");
        if(target==animated)animationCases++;if(target==empty)emptyCases++;
    }
}
Need(maps==113,"Not all native maps checked");

// Immutable clipboard (including empty and animated raw IDs), bounded rectangles and missing pages.
var synthetic=Map([0,1,2,2],3);
synthetic.RawTiles[0]=65535;synthetic.RawTiles[1]=32768;synthetic.RawTiles[16]=1;
synthetic.Collisions[0]=2;synthetic.Collisions[1]=3;
var snapshot=TerrainSections.Capture(synthetic,43,0,0,2,1);
synthetic.RawTiles[0]=2;synthetic.Collisions[0]=0;
Need(snapshot.Cells[0]==new TerrainSectionCell(65535,2) && snapshot.Cells[1].Tile==32768,"Clipboard is not an independent raw snapshot");
Need(snapshot.Cells is not TerrainSectionCell[],"Mutable array escaped clipboard");
try{((IList<TerrainSectionCell>)snapshot.Cells)[0]=new(3,4);throw new Exception("Clipboard list is writable");}catch(NotSupportedException){}
var paste=TerrainSections.PreparePaste(snapshot,synthetic,43,3,0,true);
Need(paste[synthetic.CellIndex(3,0)]==(65535,2) && paste[synthetic.CellIndex(4,0)].Tile==32768,"Snapshot/empty/animation paste changed values");
var visualOnly=TerrainSections.PreparePaste(snapshot,synthetic,43,3,0,false);
Need(visualOnly[synthetic.CellIndex(3,0)].Collision==synthetic.Collisions[synthetic.CellIndex(3,0)],"Visual-only paste changed destination collision");
Reject(()=>TerrainSections.Capture(synthetic,43,-1,0,1,1));
Reject(()=>TerrainSections.Capture(synthetic,43,0,0,0,1));
Reject(()=>TerrainSections.Capture(synthetic,43,int.MaxValue,0,2,1));
Reject(()=>TerrainSections.Capture(synthetic,43,0,0,65,65));
Reject(()=>TerrainSections.PreparePaste(snapshot,synthetic,81,0,0,true));
Reject(()=>TerrainSections.PreparePaste(snapshot,synthetic,43,63,0,true));
var other=Map([0,1,2,2],3,0x8068);Reject(()=>TerrainSections.PreparePaste(snapshot,other,43,0,0,true));
other=Map([0,1,2,2],3);other.Palette[0]^=1;Reject(()=>TerrainSections.PreparePaste(snapshot,other,43,0,0,true));
var missing=Map([0,255],1);Reject(()=>TerrainSections.Capture(missing,43,15,0,2,1));
var one=TerrainSections.Capture(missing,43,0,0,1,1);Reject(()=>TerrainSections.PreparePaste(one,missing,43,16,0,true));
synthetic.RawTiles[2]=32767;Reject(()=>TerrainSections.Capture(synthetic,43,2,0,1,1));synthetic.RawTiles[2]=2;
// Source spans unique native pages; target spans two visual aliases of the same page.
var aliasClip=TerrainSections.Capture(synthetic,43,0,0,17,1);
var aliasBefore=Words(synthetic.RawTiles);Reject(()=>TerrainSections.PreparePaste(aliasClip,synthetic,43,32,0,true));
Need(aliasBefore.AsSpan().SequenceEqual(Words(synthetic.RawTiles)),"Rejected aliased paste changed terrain");
var identical=TerrainSections.Capture(synthetic,43,32,0,17,1);
Need(TerrainSections.PreparePaste(identical,synthetic,43,32,0,true).Count==16,"Identical aliases should coalesce");
synthetic.RawTiles[0]=synthetic.RawTiles[16]=2;
synthetic.Collisions[0]=1;synthetic.Collisions[16]=2;
var collisionAlias=TerrainSections.Capture(synthetic,43,0,0,17,1);
Reject(()=>TerrainSections.PreparePaste(collisionAlias,synthetic,43,32,0,true));
Need(TerrainSections.PreparePaste(collisionAlias,synthetic,43,32,0,false).Count==16,"Visual-only alias paste should ignore conflicting source collision");
var maximum=Map(Enumerable.Range(0,16).Select(i=>(byte)i).ToArray(),16);
Need(TerrainSections.Capture(maximum,43,0,0,256,16).Count==TerrainSections.MaxCells,"4096-cell limit rejected its exact maximum");
Reject(()=>TerrainSections.Capture(maximum,43,0,0,257,16));

// Real multi-tile section is pasted as a raised strip near the player; authoritative ROM validation
// checks every prepared value, persistence and V4 export. Secondary room and original source stay intact.
var level=rom.CloneLevel(43);var native=rom.ReadTerrain(43).Single();
var originalTiles=Words(native.RawTiles);var originalCollision=Words(native.Collisions);
string secondaryBefore=JsonSerializer.Serialize(level.Rooms,ProjectStore.Json);
int sourceY=-1,sourceX=-1;
for(int y=23;y<40 && sourceY<0;y++)for(int x=2;x<30;x++){
    bool good=true;
    for(int col=0;col<4;col++)if(!native.TryCell(x+col,y,out int cell) || native.RawTiles[cell]==65535 || native.Collisions[cell]==0)good=false;
    if(good){sourceX=x;sourceY=y;break;}
}
Need(sourceY>=0,"No native platform section found near entry");
var section=TerrainSections.Capture(native,43,sourceX,sourceY,4,1);
int destinationX=sourceX,destinationY=sourceY-3;
var values=TerrainSections.PreparePaste(section,native,43,destinationX,destinationY,true);
Need(rom.SetTerrainCells(level,43,native.ResourceId,values),"Section paste did not alter terrain");
rom.ValidateTerrain(level);var preview=native.Preview(level.Terrain);
foreach(var pair in values)Need(preview.RawTiles[pair.Key]==pair.Value.Tile && preview.Collisions[pair.Key]==pair.Value.Collision,"Committed terrain differs from clipboard");
Need(secondaryBefore==JsonSerializer.Serialize(level.Rooms,ProjectStore.Json),"Paste changed another room");
Need(originalTiles.AsSpan().SequenceEqual(Words(native.RawTiles)) && originalCollision.AsSpan().SequenceEqual(Words(native.Collisions)),"Paste changed original ROM map");
Need(!rom.SetTerrainCells(level,43,native.ResourceId,values),"Identical section paste should be a no-op");
var store=new ProjectStore(Path.Combine(outdir,"validation-core/content"));store.Save(level);
var restored=store.Levels().Single(l=>l.Id==level.Id);Need(Snapshot(restored)==Snapshot(level),"Section edits lost after save/reload");
var clone=store.Duplicate(restored);clone.Terrain[0].Tile=65535;Need(Snapshot(restored)==Snapshot(level),"Duplicate shares section data");
byte[] payload=rom.ExportPlacements(level),data=rom.BuildDataOnly(level);Need(payload[7]=='4',"Section export is not V4");
File.WriteAllBytes(Path.Combine(outdir,"section-runtime.yslevel"),payload);
File.WriteAllBytes(Path.Combine(outdir,"section-runtime.z64"),data);
File.WriteAllBytes(Path.Combine(outdir,"section-runtime-tiles.bin"),Words(preview.RawTiles));
File.WriteAllBytes(Path.Combine(outdir,"section-runtime-collision.bin"),Words(preview.Collisions));
File.WriteAllText(Path.Combine(outdir,"section-runtime.json"),JsonSerializer.Serialize(new{
    rootZone=43,roomZone=43,resourceId=native.ResourceId,
    sourceRectangle=new{x=sourceX,y=sourceY,width=section.Width,height=section.Height},
    destinationRectangle=new{x=destinationX,y=destinationY,width=section.Width,height=section.Height},
    destinationWorld=new{x=native.WorldX+destinationX*16,y=native.WorldY+destinationY*16,width=section.Width*16,height=section.Height*16},
    sourceWorld=new{x=native.WorldX+sourceX*16,y=native.WorldY+sourceY*16,width=section.Width*16,height=section.Height*16},
    changedCells=level.Terrain.Select(c=>new{cell=c.Cell,tile=c.Tile,collision=c.Collision}).ToArray(),
    originalTilesSha256=Hash(originalTiles),originalCollisionSha256=Hash(originalCollision),
    previewTilesSha256=Hash(Words(preview.RawTiles)),previewCollisionSha256=Hash(Words(preview.Collisions)),
    payloadSha256=Hash(payload),contentSha256=Hash(data),clipboardCells=section.Count
},ProjectStore.Json));
File.WriteAllText(Path.Combine(outdir,"section-core.json"),JsonSerializer.Serialize(new{
    passed=true,maps,rawAnimationCases=animationCases,emptyTileCases=emptyCases,
    immutableSnapshot=true,readOnlyCells=true,syntheticEmptyAndAnimationPreserved=true,exactMaximumCellsAccepted=true,missingPagesRejected=true,aliasConflictsRejected=true,
    identicalAliasesCoalesced=true,collisionAliasConflictsRejected=true,visualOnlyAliasCollisionsIgnored=true,wholePastePreparedBeforeMutation=true,sameRoomAndAssetsOnly=true,
    visualOnlyPreservesCollision=true,originalMapUnchanged=true,secondaryRoomUntouched=true,
    noOp=true,persistence=true,duplicateIsolation=true,v4Export=true,rejected,
    runtimeFixture="section-runtime.yslevel",runtimeDestinationX=destinationX,runtimeDestinationY=destinationY
},ProjectStore.Json));
Console.WriteLine($"Terrain sections passed: {maps} native maps, {animationCases} animation cases, {emptyCases} empty cases, {rejected} atomic rejections. Fixture destination {destinationX},{destinationY}.");

NativeTerrainMap Map(byte[] pages,int loaded,ushort resource=0x8067){
    int width=pages.Length*256;var raw=Enumerable.Range(0,loaded*256).Select(i=>(ushort)(i%4)).ToArray();
    return new(){ResourceId=resource,Width=width,Height=256,TileWidth=16,TileHeight=16,PageHeight=256,
        Pages=pages,LoadedPages=loaded,Pixels=new byte[4*256],Palette=new uint[256],RawTiles=raw,Tiles=(ushort[])raw.Clone(),
        AnimationFrames=[1],Collisions=Enumerable.Range(0,raw.Length).Select(i=>(ushort)(i%4)).ToArray()};
}
sealed record PointCell(int X,int Y);
