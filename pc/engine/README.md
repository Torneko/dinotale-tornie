# Intégration moteur de l’atelier PC

Les sources DinoTale 0.1.6 restent figées au commit du fichier
`../upstream-lock.json`. Les modifications dans ce dépôt imbriqué sont
limitées à son point d’entrée et à l’inclusion CMake conditionnelle de ce
dossier. Le reste du code de l’atelier est ici, hors des dépendances.

`Construire-Moteur.ps1` utilise les outils C++ Visual Studio existants et
LLVM 20.1.8 extrait localement. L’archive LLVM est vérifiée par SHA256
dans `../tools-lock.json` ; aucun installateur n’est exécuté. La compilation
produit `../build/native/DinoTale.exe`, ses DLL, ses ressources et un
manifeste portant son empreinte. Les sources recompilées à partir de la
ROM restent privées et ne doivent pas être redistribuées.

## Fichier de placements

`--workshop-level <fichier.yslevel>` charge des placements indépendants.
Le format YSLEVEL1 contient : signature ASCII de 8 octets, zone source
uint32 big-endian (43), nombre d’objets uint32 big-endian, puis objets de
12 octets (ID uint16, paramètre natif uint16 conservé, X et Y float32).

Le chargeur refuse les longueurs incohérentes, coordonnées non finies ou
hors limites, types d’objets absents du 1-1, sources non prises en charge
et nombres de points de départ différents de un. Il accepte 1 à 2000
placements ; les limites pratiques de densité du moteur restent à mesurer.

Le runtime vérifie toujours la ROM originale USA avec son empreinte
habituelle. Après cette vérification et avant l’entrée du jeu, le callback
crée les données du niveau en mémoire et actualise sa seule entrée dans
la table déjà copiée en RDRAM. Aucun code du jeu n’est remplacé par le
fichier de niveau. Le fichier ROM et l’exécutable sur disque restent intacts.
Le résultat est comparé octet par octet à l’export indépendant de l’éditeur.

`WorkshopDataChecks` vérifie un niveau valide, six fichiers invalides et
la conservation des instructions principales et de tous les overlays.
Ces contrôles de données ne remplacent pas un essai du niveau en jeu.

Le prototype reste en solo dans l’emplacement Story 1-1. Le chargeur ne
crée pas encore de menu dans le moteur, d’enchaînement de niveaux,
de deuxième joueur, de réseau ou de nouvelles ressources.

## Licence

Cette intégration avec DinoTale est sous GPL-3.0, comme sa base.
Consulter `../dinotale/COPYING` et les licences de ses dépendances. Les
ROM et sauvegardes restent hors de tout paquet distribuable.
