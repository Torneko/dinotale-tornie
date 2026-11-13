# DinoTale Tornie 0.35.5-beta.2

Préversion publique de test du 13 novembre 2026 pour Windows x64 et Linux x64 expérimental. Le moteur et l’Atelier gardent la version 0.35.5 ; ce lot reprend les finitions récentes et le correctif ciblé Candy1. La version précédente reste disponible dans [la publication bêta 1](https://github.com/Torneko/dinotale-tornie/releases/tag/v0.35.5-beta.1).

Public testing prerelease dated 13 November 2026 for Windows x64 and experimental Linux x64. The engine and Workshop retain version 0.35.5; this package includes recent finishes and the focused Candy1 correction. The previous beta remains available separately.

## Nouveautés / Changes

- Sélection : retrait des points d’interrogation blancs des emplacements inconnus, sans révéler leurs identités ni modifier les droits. / Selection: remove white question marks from undiscovered slots while preserving identities and earned rights.
- Atelier : import de captures PNG pour les six vignettes Bonus et aperçu des Bonus accessibles ; les captures réelles ne sont pas fournies. / Workshop: import screenshots for six Bonus thumbnails and show previews for accessible courses; actual route screenshots are not included.
- Éditeur Linux : conservation de la couche lors du remplacement des fruits en bulle. / Linux editor: preserve the layer when replacing fruit bubbles.
- Combat : pot de sortie des Lèvres maintenu dans la zone accessible ; retrait des blocs dorés ajoutés aux arènes Bonbon en conservant le sol. / Combat: keep the Lips exit pot in the reachable area and remove added golden platforms from Candy arenas while retaining the native floor.
- Correction d’une interruption après la défaite d’un adversaire dans certains combats multiples. / Fix an interruption after defeating an opponent in some multi-enemy encounters.
- Libellés et descriptions de l’Atelier actualisés. Les conditions actuelles de progression sont conservées. / Workshop labels and descriptions updated. Existing progression requirements remain unchanged.

## Distribution

Les archives démarrent avec une sauvegarde vierge, aucune monnaie, aucun déblocage de test imposé et un volume de 3 %. Aucune ROM, image de jeu préparée ou progression personnelle n’est fournie. Au premier lancement, sélectionner une ROM originale USA de Yoshi’s Story ; .z64, .v64 et .n64 sont reconnus. Conserver une copie complète du profil existant avant toute mise à jour.

Archives start with fresh progression, zero currency, no forced testing unlocks and 3% volume. No ROM, prepared game image or personal progress is included. On first launch, select an original USA Yoshi’s Story ROM; .z64, .v64 and .n64 are supported. Back up the complete existing profile before updating.

- `DinoTale-Tornie-0.35.5-beta.2-Windows-x64.zip`
- `DinoTale-Tornie-0.35.5-beta.2-Linux-x64.tar.gz`
- `DinoTale-Tornie-0.35.5-beta.2-Sources.zip`
- `DinoTale-Tornie-Guide-FR-EN-0.35.5-beta.2.zip`
- `SHA256SUMS.txt`

## Vérification et limites / Verification and limits

La compilation Windows et les contrôles de code, PNG, entrées/sorties, commandes et géométrie ont réussi. Le moteur Linux et l’Atelier autonome ont été compilés sous Ubuntu 22.04 ; les tests réels de persistance et d’API locale, les contrôles de sources et l’audit du paquet vierge ont réussi. Le nouveau contrôle Linux couvre notamment 105 vérifications du stockage des images, 667 contrôles d’interface et 74 contrôles d’API sur 36 requêtes. Les preuves CPU utilisent des scènes et données synthétiques ; elles ne sont pas une partie naturelle.

Windows compilation and source, PNG, IO, control and geometry checks passed. Linux native engine and standalone Workshop were built under Ubuntu 22.04; actual persistence and local HTTP tests, source checks and fresh-package audits passed. The new Linux preflight includes 105 image-storage checks, 667 interface checks and 74 API checks over 36 requests. CPU fixtures use synthetic data and are not a natural playthrough.

La coop et le Boss Battle restent incomplets et expérimentaux. Les six parcours Bonus gardent leur aperçu séparé et leur verrou public de validation. Les essais naturels des campagnes, du dernier correctif Bonbon et de l’affichage restent à faire. Linux Mint 22.3, le rendu GPU et l’écoute physique ne sont pas certifiés. Les anciens retours Basalt/CachyOS ne valident que la 0.34.0-beta.1. L’identité des sources du moteur audio est vérifiée, sans nouveau rendu Linux utilisant la ROM originale.

Co-op and Boss Battle remain unfinished and experimental. The six Bonus routes keep their separate testing preview and public acceptance gate. Natural campaign, Candy correction and visual testing remain pending. Linux Mint 22.3, GPU rendering and physical audio playback are not certified. Earlier Basalt/CachyOS reports cover 0.34.0-beta.1 only. Music worker source identity is verified without a new Linux ROM-backed render.

Linux source build: [9c5409ab939ad4650503617dd719129347514a67](https://github.com/Torneko/dinotale-tornie/commit/9c5409ab939ad4650503617dd719129347514a67). [Build run](https://github.com/Torneko/dinotale-tornie/actions/runs/43451362324). The final documentation/tag commit can differ while preserving the compiled production inputs.
