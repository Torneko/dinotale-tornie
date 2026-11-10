namespace YoshiWorkshop;
public static class SpecialEditorObjects {
    public static bool IsEgg(ushort id)=>id is >=0x4F30 and <=0x4F39;
    public static bool NativeEgg(ushort id)=>id is 0x4692 or 0x4693;
    public static IEnumerable<ushort> Eggs=>Enumerable.Range(0x4F30,10).Select(i=>(ushort)i);
    public static int Color(ushort id)=>IsEgg(id)?6+id-0x4F30:throw new InvalidDataException("Œuf spécial inconnu.");
    public static ushort Alias(IEnumerable<ushort> objects)=>objects.Where(NativeEgg).DefaultIfEmpty((ushort)0).Min();
    public static bool Compatible(IEnumerable<ushort> objects)=>Alias(objects)!=0;
    public static readonly ushort[] NativeSpecials=[0x46AA,0x46AE,0x45E3,0x45E5,0x45B0];
    public static readonly string[] Names=["blanc","noir","orange","violet","turquoise","lime-céleri","brun","vert foncé","or","argent"];
    public static ObjectInfo EggInfo(int color)=>new((ushort)(0x4F30+color),"Œuf de Yoshi "+Names[color]+" en bulle","Œufs de Yoshis","Objet de cette création uniquement. Disponible dans les salles possédant déjà une bulle d’œuf native ; n’accorde pas de déblocage dans la progression personnelle.");
}
