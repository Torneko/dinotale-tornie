# DinoTale Tornie

Un projet PC dérivé de [DinoTale](https://github.com/wootbeer/dinotale), imaginé par Tornie pour enrichir Yoshi’s Story : seize couleurs à débloquer, fruits et Maskass hybrides, campagnes, progression, boutiques, succès et Atelier de créations.

**Préversion 0.34.0-beta.1 : Windows x64 et candidat Linux x64 expérimental.** Les ajouts solo et l’Atelier sont disponibles ; la coop et le Boss Battle restent incomplets et différés. Story difficile est intégré mais son accès public reste verrouillé. Les essais humains complets des nouvelles campagnes restent nécessaires.

**Linux : le paquet, l’import et l’Atelier ont été vérifiés ; le jeu sur Linux Mint 22.3 n’est pas validé.** Les essais graphiques WSL avec rendu logiciel n’ont pas atteint de niveau jouable. Ce candidat ne doit pas être présenté comme une version Linux stable.

[Guide français](https://torneko.github.io/dinotale-tornie/) · [English guide](https://torneko.github.io/dinotale-tornie/index-en.html) · [Téléchargements / Downloads](https://github.com/Torneko/dinotale-tornie/releases/tag/v0.34.0-beta.1)

## Jouer / Play

Extraire le paquet complet dans un dossier autorisant l’enregistrement. Sous Windows, ouvrir **DinoTale.exe**. Sous Linux, lancer **Jouer.sh** ; **Atelier.sh** ouvre l’éditeur dans le navigateur local. L’Atelier est autonome : aucune installation de .NET n’est nécessaire.

Au premier lancement, sélectionner une ROM originale USA de Yoshi’s Story (.z64, .v64 ou .n64). L’import local vérifie le fichier et prépare le contenu sans modifier la ROM choisie. Aucune ROM n’est incluse ni téléchargée. Les archives publiques commencent avec une progression vide et le volume à **3 %**.

Extract the complete package into a writable folder. On Windows, open **DinoTale.exe**. On Linux, run **Jouer.sh**; **Atelier.sh** opens the editor in a local browser. The Workshop is standalone, with no .NET installation required.

On first launch, choose an original USA Yoshi’s Story ROM (.z64, .v64 or .n64). Local import validates it and prepares content without changing the selected ROM. No ROM is included or downloaded. Public archives start with fresh progression and **3% volume**.

**Linux is an experimental candidate:** package, import and Workshop checks passed, but gameplay on Linux Mint 22.3 remains unverified. Software-rendered WSL graphics tests did not reach a playable level. Co-op and Boss Battle remain incomplete; public Hard Story access is still locked.

Back up the entire `pc/profiles/content-personal` folder, including its journals, and keep creations in `pc/content`. Classic Story and Hybrid Story have separate runs and records while purchases and owned colours are shared. Close the game and Workshop before updating; do not overwrite a personal profile with the fresh profile supplied in an archive.

## Nouveautés / What changed

- **Seize couleurs / Sixteen colours:** six classiques, blanc/noir, six hybrides, or/argent. Les métaux demandent les 24 Trial hybrides terminés, puis un œuf trouvé et ramené. / Gold and silver require all 24 Hybrid Trial clears, followed by finding an egg and finishing the level.
- **Story hybride / Hybrid Story:** campagne et records séparés, contenu hybride à 75 %, achats et Yoshis possédés partagés. / Separate campaign and records, 75% hybrid content, shared purchases and owned Yoshis.
- **Atelier / Workshop:** 24 niveaux, salles, terrain et collisions, groupes, musique, passages, points d’entrée et dix couleurs d’œufs dans les salles compatibles. / 24 levels, rooms, terrain and collisions, groups, music, passages, entry points and ten egg colours in compatible rooms.
- **Musique du livre / Book music:** musique originale dès le premier livre et à son retour. / Original music from the first book display and when returning to it.
- **Story difficile / Hard Story:** intégré, mais verrouillé tant que les conditions des 16 Yoshis et des victoires Boss correspondantes ne peuvent pas être remplies. / Integrated, but locked while the sixteen-Yoshi and corresponding Boss-win requirements cannot yet be satisfied.

## Sources

[Build instructions](BUILDING.md) · [Notes 0.34.0-beta.1](docs/RELEASE-0.34.0-beta.1.md) · [Credits](CREDITS.md)

The prerelease groups the unchanged Windows solo engine 0.33.0 and its updated standalone Workshop with the Linux engine 0.34.0 and standalone Workshop. Detailed qualification and remaining limits are listed in the release notes; a shared archive version does not imply identical validation on both platforms.

`pc/engine` contains the native additions. `pc/workshop` contains the .NET editor and ROM importer. `pc/dinotale` retains the upstream sources and local changes; its libraries are pinned submodules. `pc/recompile` and `pc/distribution` contain metadata and a delta patch: generated game C and game images stay local. `website` is the bilingual static guide, published by GitHub Pages.

The source code follows GNU GPL version 3 or later, with existing third-party notices preserved. Yoshi’s Story and its characters belong to Nintendo; this fan project is independent and not affiliated with Nintendo. The license does not grant rights to original game content.
