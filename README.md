# DinoTale Tornie

Un projet PC dérivé de [DinoTale](https://github.com/wootbeer/dinotale), imaginé par Tornie pour enrichir Yoshi’s Story : six Yoshis hybrides, fruits et Maskass assortis, progression, boutique, succès et Atelier de créations.

**Bêta Windows 0.15.0-beta.1.** Les essais automatiques ne remplacent pas une partie complète. La coop jouable, le défi de huit boss et l’édition du terrain restent à développer. Linux est expérimental : aucune version Linux validée n’est annoncée.

[Guide français](https://torneko.github.io/dinotale-tornie/) · [English guide](https://torneko.github.io/dinotale-tornie/index-en.html) · [Téléchargements / Downloads](https://github.com/Torneko/dinotale-tornie/releases)

## Jouer / Play

Extraire le paquet Windows complet puis lancer **DinoTale.exe**. Au premier démarrage, sélectionner sa propre ROM originale USA de Yoshi’s Story (.z64, .v64 ou .n64). L’import est local et vérifie le fichier. Aucune ROM n’est incluse ni téléchargée. Les archives publiques commencent avec une progression vide et le volume à 10 %.

Extract the complete Windows package and run **DinoTale.exe**. On first launch, choose your own original USA Yoshi’s Story ROM (.z64, .v64 or .n64). Import is local and validates the file. No ROM is included or downloaded. Public packages start with fresh progression and 10% volume.

Back up the entire `pc/profiles/content-personal` folder. Keep creations in `pc/content`. Do not replace a personal profile with the fresh profile in an update.

## Sources

[Build instructions](BUILDING.md) · [Beta notes](docs/RELEASE-0.15.0-beta.1.md) · [Credits](CREDITS.md)

`pc/engine` contains the native additions. `pc/workshop` contains the .NET editor and ROM importer. `pc/dinotale` retains the upstream sources and local changes; its libraries are pinned submodules. `pc/recompile` and `pc/distribution` contain metadata and a delta patch: generated game C and game images stay local. `website` is the bilingual static guide, published by GitHub Pages.

The source code follows GNU GPL version 3 or later, with existing third-party notices preserved. Yoshi’s Story and its characters belong to Nintendo; this fan project is independent and not affiliated with Nintendo. The license does not grant rights to original game content.
