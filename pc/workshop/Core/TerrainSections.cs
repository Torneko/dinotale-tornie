using System.Collections.ObjectModel;
namespace YoshiWorkshop;

public readonly record struct TerrainSectionCell(ushort Tile, ushort Collision);

// The editor clipboard owns its values. Later painting or undo never rewrites it.
public sealed class TerrainSection {
    public int SourceRoom {get;}
    public ushort ResourceId {get;}
    public int Width {get;}
    public int Height {get;}
    public int TileWidth {get;}
    public int TileHeight {get;}
    public int Count => Cells.Count;
    public IReadOnlyList<TerrainSectionCell> Cells {get;}
    readonly byte[] pixels;
    readonly uint[] palette;
    readonly ushort[] animationFrames;
    readonly HashSet<ushort> collisionMasks;
    internal TerrainSection(NativeTerrainMap map, int room, int width, int height, TerrainSectionCell[] cells) {
        SourceRoom=room; ResourceId=map.ResourceId; Width=width; Height=height;
        TileWidth=map.TileWidth; TileHeight=map.TileHeight;
        Cells=new ReadOnlyCollection<TerrainSectionCell>((TerrainSectionCell[])cells.Clone());
        pixels=(byte[])map.Pixels.Clone(); palette=(uint[])map.Palette.Clone();
        animationFrames=(ushort[])map.AnimationFrames.Clone();
        collisionMasks=map.Collisions.Append((ushort)0).ToHashSet();
    }
    internal bool Compatible(NativeTerrainMap target) => ResourceId==target.ResourceId &&
        TileWidth==target.TileWidth && TileHeight==target.TileHeight &&
        pixels.AsSpan().SequenceEqual(target.Pixels) && palette.AsSpan().SequenceEqual(target.Palette) &&
        animationFrames.AsSpan().SequenceEqual(target.AnimationFrames);
    internal bool HasCollision(ushort mask) => collisionMasks.Contains(mask);
}

public static class TerrainSections {
    public const int MaxCells=4096;
    static void Rectangle(NativeTerrainMap map, int x, int y, int width, int height) {
        if(map.TileWidth!=16 || map.TileHeight!=16 || width<1 || height<1 ||
            (long)width*height>MaxCells || x<0 || y<0 ||
            (long)x+width>map.Width/map.TileWidth || (long)y+height>map.Height/map.TileHeight)
            throw new InvalidDataException("Choisir une section de 1 à 4096 tuiles, entièrement dans le terrain de la salle.");
    }
    public static TerrainSection Capture(NativeTerrainMap map, int sourceRoom, int x, int y, int width, int height) {
        Rectangle(map,x,y,width,height);
        var cells=new TerrainSectionCell[width*height];
        for(int row=0;row<height;row++)for(int column=0;column<width;column++) {
            if(!map.TryCell(x+column,y+row,out int index))
                throw new InvalidDataException("La sélection traverse une zone sans terrain éditable. Aucune section copiée.");
            ushort tile=map.RawTiles[index];
            if(!map.ValidTile(tile))throw new InvalidDataException("La section contient une tuile incompatible avec la salle.");
            cells[row*width+column]=new(tile,map.Collisions[index]);
        }
        return new(map,sourceRoom,width,height,cells);
    }
    // Preparation is atomic and never changes the target. Call SetTerrainCells only after success.
    // Repeated native pages may make several visual cells refer to the same stored cell.
    public static Dictionary<int,(ushort Tile,ushort Collision)> PreparePaste(
        TerrainSection section, NativeTerrainMap target, int targetRoom, int x, int y, bool applyCollision) {
        if(section.SourceRoom!=targetRoom || !section.Compatible(target))
            throw new InvalidDataException("Cette section se colle uniquement dans sa salle d’origine, avec le même terrain.");
        Rectangle(target,x,y,section.Width,section.Height);
        var result=new Dictionary<int,(ushort Tile,ushort Collision)>();
        for(int row=0;row<section.Height;row++)for(int column=0;column<section.Width;column++) {
            if(!target.TryCell(x+column,y+row,out int index))
                throw new InvalidDataException("La section dépasse une zone de terrain éditable. Rien n’a été collé.");
            var source=section.Cells[row*section.Width+column];
            if(!target.ValidTile(source.Tile) || !section.HasCollision(source.Collision))
                throw new InvalidDataException("La tuile ou la collision de la section est incompatible avec la salle.");
            var value=(Tile:source.Tile,Collision:applyCollision?source.Collision:target.Collisions[index]);
            if(result.TryGetValue(index,out var previous) && previous!=value)
                throw new InvalidDataException("Cette zone partage ses tuiles et ne peut pas recevoir ce motif. Rien n’a été collé.");
            result[index]=value;
        }
        return result;
    }
}
