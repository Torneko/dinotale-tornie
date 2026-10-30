namespace YoshiWorkshop;
// Generated from native passage records and Miss Warp destinations.
public static class NativeRooms {
    static readonly Dictionary<int,int[]> rooms=new(){
        [43]=[43,81],
        [51]=[51,85,148],
        [13]=[13,104,119,160],
        [18]=[18,90,91,92,152],
        [46]=[46,120,149,153,171],
        [19]=[19,93,155],
        [16]=[16,106,151],
        [23]=[23,107,108,109,110,111],
        [52]=[52,82,83,84,174],
        [40]=[40,102,103,156],
        [32]=[32,124,125,126,127,128,129,130,131,170],
        [14]=[14,105],
        [17]=[17,112,113,114,115,116,117],
        [22]=[22,97,150],
        [12]=[12,86,145,147],
        [27]=[27,118,161],
        [21]=[21,94,95,96,146],
        [30]=[30,121,154,166,172,173],
        [31]=[31,122,123],
        [28]=[28,101,158],
        [15]=[15,87,88,89,159],
        [35]=[35,141,142,143,144,157],
        [34]=[34,132,133,134,135,136,137,138,139,140],
        [24]=[24,98,99,100,175,176]
    };
    public static IReadOnlyList<int> For(int root)=>rooms.TryGetValue(root,out var r)?r:throw new InvalidDataException("Niveau source inconnu.");
    public static bool Contains(int root,int room)=>rooms.TryGetValue(root,out var r) && Array.IndexOf(r,room)>=0;
    public static bool Known(int room)=>rooms.Values.Any(r=>Array.IndexOf(r,room)>=0);
}
