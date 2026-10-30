namespace YoshiWorkshop;

public sealed record ObjectInfo(ushort Id,string Name,string Category,string Description,bool Identified=true) {
    public string DisplayName=>$"{Name} · {Id:X4}";
}

public static class ObjectCatalog {
    public static readonly IReadOnlyDictionary<ushort,ObjectInfo> All=new ObjectInfo[] {
        new(0x4001,"Départ de Yoshi","Repères","Point d’apparition du joueur. En conserver un seul."),
        new(0x4030,"Pomme","Fruits","Fruit posé dans le niveau."),
        new(0x4031,"Banane","Fruits","Fruit posé dans le niveau."),
        new(0x4032,"Pastèque","Fruits","Fruit posé dans le niveau."),
        new(0x4033,"Raisin","Fruits","Fruit posé dans le niveau."),
        new(0x4034,"Melon","Fruits","Melon posé dans le niveau ; conservé par le tirage des fruits hybrides."),
        new(0x4035,"Cœur","Objets","Cœur souriant qui déclenche le bonus d’invincibilité ; ne compte pas parmi les 30 fruits."),
        new(0x4F00,"Pêche","Fruits","Fruit orange : +4 points et +4 de vie au Yoshi orange ; favori du rouge, du rose et du jaune (+2)."),
        new(0x4F01,"Mûre","Fruits","Fruit violet : +4 points et +4 de vie au Yoshi violet ; favori du rouge, du rose, du bleu et du bleu pâle (+2)."),
        new(0x4F02,"Bleuet","Fruits","Fruit turquoise : +4 points et +4 de vie au Yoshi turquoise ; favori du bleu, du bleu pâle et du vert (+2)."),
        new(0x4F03,"Poire verte","Fruits","Fruit lime-céleri : +4 points et +4 de vie au Yoshi lime-céleri ; favori du jaune et du vert (+2)."),
        new(0x4F04,"Prune","Fruits","Fruit brun : +4 points et +4 de vie au Yoshi brun ; favori du rouge, du rose et du vert (+2)."),
        new(0x4F05,"Kiwi","Fruits","Fruit vert foncé : +4 points et +4 de vie au Yoshi vert foncé ; favori du bleu, du bleu pâle et du jaune (+2)."),
        new(0x4F10,"Maskass orange","Maskass hybrides","Couleur orange ; variante native compatible avec la source, pirouettes et récompenses conservées."),
        new(0x4F11,"Maskass violet","Maskass hybrides","Couleur violet ; variante native compatible avec la source, pirouettes et récompenses conservées."),
        new(0x4F12,"Maskass turquoise","Maskass hybrides","Couleur turquoise ; variante native compatible avec la source, pirouettes et récompenses conservées."),
        new(0x4F13,"Maskass lime-céleri","Maskass hybrides","Couleur lime-céleri ; variante native compatible avec la source, pirouettes et récompenses conservées."),
        new(0x4F14,"Maskass brun","Maskass hybrides","Couleur brun ; variante native compatible avec la source, pirouettes et récompenses conservées."),
        new(0x4F15,"Maskass vert foncé","Maskass hybrides","Couleur vert foncé ; variante native compatible avec la source, pirouettes et récompenses conservées."),
        new(0x403A,"Maskass","Ennemis","Maskass ordinaire. Sa couleur peut changer après une pirouette."),
        new(0x406F,"Pièce","Objets","Pièce collectable ; le paramètre natif distingue les exemplaires."),
        new(0x4086,"Bloc d’œufs","Objets","Bloc qui fournit des œufs."),
        new(0x40B8,"Boule surprise","Objets","Boule contenant une surprise."),
        new(0x40C3,"Barrière","Décor","Élément de clôture du niveau."),
        new(0x4147,"Drapeau de départ","Repères","Repère de départ du défi de course."),
        new(0x414A,"Drapeau d’arrivée","Repères","Repère d’arrivée du défi de course."),
        new(0x414F,"Déclencheur invisible 414F","Technique","Objet sans sprite du groupe 80530490. Fonction précise à confirmer.",false),
        new(0x417F,"Maskass caché","Ennemis","Variante de Maskass caché."),
        new(0x4199,"Maskass avec fruit — variante A","Ennemis","Variante de placement du Maskass porteur de fruit."),
        new(0x419C,"Maskass avec fruit — variante B","Ennemis","Deuxième variante de placement du Maskass porteur de fruit."),
        new(0x41D1,"Pomme en bulle","Fruits","Bulle contenant une pomme."),
        new(0x41D2,"Banane en bulle","Fruits","Bulle contenant une banane."),
        new(0x41D3,"Pastèque en bulle","Fruits","Bulle contenant une pastèque."),
        new(0x41D4,"Raisin en bulle","Fruits","Bulle contenant du raisin."),
        new(0x41D5,"Melon en bulle","Fruits","Bulle contenant un melon."),
        new(0x41E8,"Bifurcation vers la droite","Passages","Repère de bifurcation du niveau."),
        new(0x41EA,"Bifurcation vers la gauche","Passages","Repère de bifurcation du niveau."),
        new(0x4208,"Pot de transport","Passages","Pot de passage ; conserver ses paramètres de destination."),
        new(0x44CA,"Effet sphérique bleu 44CA","Technique","Sprite bleu identifié visuellement. Rôle de l’acteur à confirmer.",false),
        new(0x4507,"Miss Warp — point 1","Passages","Premier point de téléportation."),
        new(0x4508,"Miss Warp — point 2","Passages","Deuxième point de téléportation."),
        new(0x4536,"Pak E. Derm","Ennemis","Éléphant qui bloque le passage."),
        new(0x45B6,"Piment","Fruits","Piment ; +2 de vie pour les nouveaux Yoshis dans les règles 9.2."),
        new(0x45BC,"Melon caché","Fruits","Melon révélé par son déclencheur associé."),
        new(0x45BD,"Déclencheur du melon caché","Repères","Déclenche l’apparition du melon caché."),
        new(0x45F4,"Boîte à conseils","Objets","Boîte affichant un conseil."),
        new(0x4684,"Bloc surprise ?","Objets","Bloc brun à point d’interrogation, identifié par son sprite."),
        new(0x4686,"Fleur","Objets","Fleur du niveau."),
        new(0x46A8,"Pastèque en bulle — variante B","Fruits","Autre variante native de la bulle à pastèque."),
        new(0x4732,"Déclencheur invisible 4732","Technique","Alias sans sprite du groupe 80530490. Fonction précise à confirmer.",false),
        new(0x473D,"Pièce secrète","Objets","Variante de pièce cachée."),
        new(0x8067,"Terrain du 1-1","Décor","Tuiles et collisions héritées du niveau original."),
        new(0x80B8,"Décor du 1-1 — couche B8","Décor","Couche de décor composée de tuiles."),
        new(0x80B9,"Décor du 1-1 — couche B9","Décor","Autre couche de décor composée de tuiles.")
    }.ToDictionary(x=>x.Id);
    public static ObjectInfo Get(ushort id)=>All.TryGetValue(id,out var item)?item:new(id,$"Objet à identifier {id:X4}","Technique","Fonction encore non identifiée.",false);
    public static readonly HashSet<ushort> MaskassIds=[0x403a,0x4040,0x4043,0x4044,0x4045,0x410d,0x417e,0x417f,0x4180,0x4181,0x4182,0x4183,0x4184,0x4185,0x4186,0x4187,0x4188,0x4189,0x418a,0x418b,0x418c,0x418d,0x4198,0x4199,0x419a,0x419b,0x419c,0x419d,0x450e,0x4547,0x4548,0x4549,0x454a,0x454b,0x454c,0x454d,0x454e,0x45b3,0x45b4,0x45b5,0x45b7,0x464e,0x4696,0x4697,0x4698,0x46db,0x46dc,0x46dd,0x46de,0x46df,0x46e0];
    public static bool IsMaskass(ushort id)=>MaskassIds.Contains(id);
    public static bool IsSpecialMaskass(ushort id)=>id is >=0x4F10 and <=0x4F15;
    public static IEnumerable<ushort> SpecialMaskass=>Enumerable.Range(0x4F10,6).Select(id=>(ushort)id);
    public static ushort PaletteId(Placement p)=>p.HybridVariant is int c?(ushort)(0x4F10+c):p.ObjectId;
    public static bool IsSpecialFruit(ushort id)=>id is >=0x4F00 and <=0x4F05;
    public static IEnumerable<ushort> SpecialFruits=>Enumerable.Range(0x4F00,6).Select(id=>(ushort)id);
}
