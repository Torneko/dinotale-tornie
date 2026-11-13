# DinoTale Tornie 0.35.5-beta.1 — Windows / Linux

Version publique destinée aux essais. Le moteur et l’Atelier sont en 0.35.5 sur les deux plateformes ; la distribution est identifiée par 0.35.5-beta.1.

## Français

Extraire l’archive complète dans un nouveau dossier. Sous Windows, ouvrir **DinoTale.exe** ; sous Linux, lancer **Jouer.sh**. Au premier lancement, sélectionner une ROM originale USA de Yoshi’s Story (.z64, .v64 ou .n64). L’import reste local et préserve le fichier choisi. Les deux archives commencent avec une sauvegarde vierge, les monnaies à zéro et un volume de 3 %. Aucune ROM ni progression personnelle n’est incluse.

Cette bêta enrichit les découvertes et les récompenses des modes solo, le suivi de progression et les possibilités de l’Atelier. L’affichage des découvertes préserve les surprises. Les détails et conditions de déblocage se trouvent dans la section du guide explicitement réservée aux spoilers.

Le correctif R2 rétablit le démarrage du premier combat du Boss Battle et la reprise des boss à rejouer après un checkpoint, y compris après plusieurs morts. Le titre des défis est calculé depuis le catalogue réel de Yoshis. **La coop et le Boss Battle restent incomplets et en développement** : ces correctifs ciblés ne constituent pas une validation complète de ces modes.

Les contrôles Windows vérifient le paquet portable, les trois formats de ROM, les refus d’entrées invalides, l’import idempotent, les chemins déplacés avec espaces et accents, et les liaisons du lanceur sans lancer une vraie partie. Des contrôles natifs CPU couvrent le cache du premier combat et les reprises. Sous Linux, la compilation native, les services de persistance, le lanceur Bonus et les API de l’Atelier ont leurs propres preuves. Le paquet est contrôlé sur Linux puis inspecté indépendamment sous Windows sans l’exécuter. **Les essais humains en jeu sur Linux Mint 22.3 restent à faire.** Les retours positifs antérieurs sur deux PC Linux concernaient 0.34.0.

Le programme audio Linux conserve l’identité des sources auparavant qualifiées sur Windows et Linux ; aucune nouvelle exécution du rendu audio avec ROM sur GitHub n’est annoncée. Les pilotes graphiques et bibliothèques de bureau proviennent du système Linux.

Avant toute migration, fermer le jeu et sauvegarder tout l’ancien dossier. Conserver l’intégralité de `pc/profiles/content-personal`, les créations dans `pc/content` et tout profil Bonus séparé. Extraire cette bêta ailleurs puis transférer les données ; ne pas écraser une progression avec le profil vierge de l’archive ni recopier d’anciens exécutables.

## English

Public testing beta. Extract the complete archive into a new folder. On Windows, open **DinoTale.exe**; on Linux, run **Jouer.sh**. First launch asks for an original USA Yoshi’s Story ROM (.z64, .v64 or .n64). Import stays local and preserves the selected file. Both archives start with a fresh save, zero currency and 3% volume. No ROM or personal progress is included.

This beta expands solo discoveries and rewards, progression tracking and Workshop creation options. Discovery presentation preserves surprises; detailed additions and unlock conditions are behind the guide’s explicit spoiler opt-in.

The R2 fix restores the first Boss Battle encounter and checkpoint replay of previously defeated bosses, including repeated deaths. The challenge title follows the actual Yoshi catalogue. **Co-op and Boss Battle remain unfinished development features**; these targeted fixes do not certify those entire modes.

Windows checks cover the portable archive, three ROM byte orders, invalid-input refusal, idempotent import, relocated Unicode paths and fake-child launcher routing without running the actual game. Native CPU fixtures exercise the first encounter’s cache and checkpoint retries. Linux has its own native build, persistence, Bonus-launcher and Workshop API checks. Its archive is checked on Linux and independently inspected on Windows without execution. **Human gameplay testing on Linux Mint 22.3 remains pending.** Earlier positive reports from two Linux PCs concerned 0.34.0.

The Linux audio worker is bound to previously qualified Windows/Linux source identities; no fresh ROM-backed audio render on GitHub is claimed. Linux graphics drivers and desktop libraries remain system dependencies.

Before migrating, close the game and back up the entire previous folder. Retain all of `pc/profiles/content-personal`, creations in `pc/content` and any separate Bonus profile. Extract the beta elsewhere and transfer data; do not overwrite progress with the supplied fresh profile or copy older executables into the new package.

## Sources and verification

The source ZIP exports the tagged public source tree and exact dependency pins. It contains no full generated game C/RSP, original/prepared game image, personal profile, transfer key, ciphertext or Git history. Local reconstruction uses the original USA ROM, the published BPS delta, verified 219-file hook manifests and pinned N64Recomp/RSPRecomp tools. See [BUILDING.md](../BUILDING.md) and the release’s validation/source manifests.

Native compilation and final source commits are recorded separately. Later documentation and website changes do not change the canonical production inputs of the reviewed binaries. `SHA256SUMS.txt` covers all four downloadable archives.

[Guide FR](https://torneko.github.io/dinotale-tornie/) · [English guide](https://torneko.github.io/dinotale-tornie/index-en.html) · [Source tag](https://github.com/Torneko/dinotale-tornie/tree/v0.35.5-beta.1)
