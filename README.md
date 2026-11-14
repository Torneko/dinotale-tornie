# DinoTale Tornie

Un projet PC dérivé de [DinoTale](https://github.com/wootbeer/dinotale), imaginé par Tornie pour enrichir Yoshi’s Story avec de nouvelles découvertes, des variantes de modes, une progression et un Atelier de créations.

**Bêta 0.35.5 : Windows x64 et Linux x64 expérimental.** Les ajouts solo et l’Atelier sont disponibles pour les essais. La coop et le mode Boss restent en développement et incomplets. Les nouvelles campagnes et les interactions de jeu demandent encore des essais humains.

[Guide français](https://torneko.github.io/dinotale-tornie/) · [English guide](https://torneko.github.io/dinotale-tornie/index-en.html) · [Téléchargements / Downloads](https://github.com/Torneko/dinotale-tornie/releases/tag/v0.35.5-beta.4)

## Jouer / Play

Extraire le paquet complet dans un dossier autorisant l’enregistrement. Sous Windows, ouvrir **DinoTale.exe**. Sous Linux, lancer **Jouer.sh** ; **Atelier.sh** ouvre l’éditeur dans le navigateur local. Aucune installation séparée de .NET n’est nécessaire.

Au premier lancement, sélectionner une ROM originale USA de Yoshi’s Story (.z64, .v64 ou .n64). L’import local vérifie le fichier et prépare le contenu sans modifier la ROM choisie. Les archives ne contiennent aucune ROM et commencent avec une progression vierge, les monnaies à zéro et un volume de **3 %**.

Extract the complete package into a writable folder. On Windows, open **DinoTale.exe**. On Linux, run **Jouer.sh**; **Atelier.sh** opens the editor in a local browser. No separate .NET installation is required.

On first launch, choose an original USA Yoshi’s Story ROM (.z64, .v64 or .n64). Local import validates it and prepares content without changing the selected ROM. The archives contain no ROM and start with fresh progression, zero currency and **3% volume**.

## Essais et progression / Testing and progress

Linux est une version expérimentale destinée aux PC 64 bits, avec Linux Mint 22.3 comme cible. La compilation native, l’Atelier et les vérifications automatiques du paquet passent sur Linux. Les essais en jeu sur Mint restent à faire. Des retours positifs sur deux PC Linux concernaient l’ancienne version publique 0.34.0, et ne constituent pas une validation de cette nouvelle bêta.

Linux is experimental, targeting 64-bit PCs including Linux Mint 22.3. Native builds, Workshop APIs and automated package checks pass on Linux. Mint gameplay testing remains pending. Earlier positive reports from two Linux PCs concerned public version 0.34.0 and do not validate this new beta.

Avant une mise à jour, fermer le jeu et conserver une copie complète de l’ancien dossier. Transférer tout `pc/profiles/content-personal` pour préserver sauvegarde, journaux, records et monnaies, ainsi que `pc/content` pour les créations. Si un profil Bonus séparé existe, le conserver également. Le profil vierge d’une archive ne doit pas remplacer une progression existante.

Before updating, close the game and keep a complete backup of the previous folder. Transfer the entire `pc/profiles/content-personal` folder to preserve saves, journals, records and currency, and retain `pc/content` for creations. Keep any separate Bonus profile as well. Never replace existing progress with the fresh profile supplied in an archive.

## Nouveautés / Changes

- Bêta 4 : effacement volontaire de sauvegarde depuis les Options, avec confirmation obligatoire et avertissement irréversible ; correctifs ciblés de reprise. / Beta 4: optional save erasure in Options, with mandatory confirmation and an irreversible-action warning; focused recovery fixes.

- Bêta 3 : corrections des reprises, des couleurs et de certains passages. Les détails restent dans le guide facultatif. / Beta 3: recovery, colour and passage fixes, detailed in the optional guide.

- Bêta 2 : sélection plus discrète, import des vignettes Bonus et corrections ciblées de reprises et de terrain. / Beta 2: a quieter selection screen, Bonus thumbnail imports and focused recovery and arena fixes.
- Nouvelles découvertes et récompenses pour les modes solo. / Additional discoveries and rewards for solo modes.
- Menus qui préservent les surprises et suivis de progression enrichis. / Menus that preserve surprises and expanded progression tracking.
- Atelier avec davantage d’objets, de fruits, de passages et de possibilités de création. / Workshop improvements for objects, fruit, passages and level creation.
- Correctifs de navigation et de reprise, avec les limites des modes en développement indiquées dans le guide. / Navigation and retry fixes, with development-mode limits documented in the guide.

Le guide propose une section détaillée avec avertissement avant de révéler les nouveautés et les conditions de déblocage. / The guide has an explicit spoiler opt-in before detailed discoveries and unlock conditions are shown.

## Sources

[Build instructions](BUILDING.md) · [Notes 0.35.5-beta.4](docs/RELEASE-0.35.5-beta.4.md) · [Credits](CREDITS.md)

The engine and Workshop use version 0.35.5 on both platforms. Package version 0.35.5-beta.4 identifies the public testing distribution; it is not a stable-gameplay certification. Platform-specific verification and pending checks are recorded with the release.

`pc/engine` contains the native additions. `pc/workshop` contains the .NET editor and local ROM importer. `pc/dinotale` retains upstream sources and local changes, with pinned libraries. `pc/recompile` and `pc/distribution` contain source metadata and incremental patches. Complete generated game C, RSP output, original/prepared game images, personal data and private build caches remain outside the public tree. The source archive exports only its tagged source tree, with dependency pins; it does not contain Git history or encrypted build inputs.

The source code follows GNU GPL version 3 or later, with third-party notices preserved. Yoshi’s Story and its characters belong to Nintendo; this independent fan project is not affiliated with Nintendo. The license does not grant rights to original game content.
