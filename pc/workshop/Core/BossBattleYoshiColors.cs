namespace YoshiWorkshop;

// Persistent colour IDs follow the native game, not the old Atelier picker
// order. Keep the legacy conversion explicit so existing preparations retain
// the Yoshi the player actually selected when schema1 used that picker order.
public static class BossBattleYoshiColors {
    static readonly string[] names=["Vert","Rouge","Jaune","Bleu","Bleu pâle","Rose","Noir","Blanc","Orange","Violet","Turquoise","Lime-céleri","Brun","Vert foncé"];
    static readonly int[] legacyToNative=[0,5,4,2,3,1,7,6,8,9,10,11,12,13];
    public const int Count=14;
    public static string FrenchName(int nativeId)=>nativeId is >=0 and <Count?names[nativeId]:throw new InvalidDataException("Identifiant de Yoshi invalide.");
    public static int FromLegacyPicker(int legacyId)=>legacyId is >=0 and <Count?legacyToNative[legacyId]:throw new InvalidDataException("Ancienne sélection de Yoshi invalide.");
}
