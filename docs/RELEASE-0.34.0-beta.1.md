# 0.34.0-beta.1

## Français

Préversion Windows x64 et candidat Linux x64 expérimental, avec progression vierge et volume à 3 %. Aucune ROM, sauvegarde personnelle, monnaie, création privée ou cache de musique n’est inclus. L’import original USA (.z64, .v64, .n64) reste local et conserve le fichier choisi.

### Ajouts depuis la première bêta publique

- Seize couleurs à débloquer : six classiques, blanc/noir, six hybrides, or et argent. Les œufs or/argent deviennent admissibles après les 24 Trial hybrides réellement terminés, puis doivent être ramenés en terminant un niveau. Les réussites hybrides valides déjà enregistrées sont conservées.
- Or/argent en solo et dans les créations compatibles ; œufs ajoutés au catalogue, dix couleurs au total. Placer un œuf dans l’éditeur n’accorde pas de déblocage permanent à la partie normale.
- Story hybride : campagne, vies et records distincts du Story classique ; achats et couleurs possédées partagés. Fruits hybrides à 75 % et un nouvel œuf hybride trouvable après chaque monde terminé. L’achat du mode hybride et les 24 Trial classiques restent requis.
- Musique originale dès le premier affichage du livre et au retour au menu, avec arrêt lors de la reprise du jeu. Le cache audio est produit localement ; aucun extrait musical n’est distribué.
- Atelier enrichi : copies des 24 niveaux avec leurs salles, terrain et collisions éditables, peinture/remplissage/sections, groupes d’objets, musique par salle, passages et coordonnées d’entrée. Fruits hybrides en bulles, variantes de Maskass et œufs spéciaux restent soumis aux ressources compatibles de la salle.
- Parties personnalisées : création d’une liste de niveaux, enchaînement à 30 fruits et reprise. Profils de test distincts de la progression personnelle.
- Atelier Linux autonome avec import local de ROM et lancement par `Jouer.sh` ; `Atelier.sh` ouvre son interface dans le navigateur local. Aucun .NET, Python ou compilateur n’est requis chez le joueur. L’Atelier Windows est également autonome.

### Statut des modes

Story difficile est intégré, avec partie séparée, départ Vert, recrutement d’œufs et suivi cumulatif des Yoshis recrutés gardés en vie jusqu’à Bowser. **Son accès public reste verrouillé** : les seize couleurs et une victoire Boss Battle avec chacune sont nécessaires, mais la condition Boss à seize couleurs n’est pas encore disponible. Aucun succès ni droit fictif n’est accordé pour la contourner.

La coop et le Boss Battle contiennent des prototypes mais restent **incomplets et différés**. La coopération entre deux PC n’est pas annoncée comme disponible. Les six niveaux bonus, l’arbre final des déblocages, les essais humains complets et la traduction intégrale du jeu natif restent à poursuivre.

### Plateformes et vérifications

Le paquet Windows utilise le moteur solo **0.33.0 inchangé**, accompagné de l’Atelier autonome actualisé. Les essais natifs antérieurs portent sur des parcours ciblés ; l’acquisition naturelle de tous les œufs, les campagnes complètes et la stabilité longue ne sont pas certifiées par cette préversion.

Le candidat Linux utilise le moteur **0.34.0**, compilé sous Ubuntu 22.04/glibc 2.35, avec Atelier autonome. Les contrôles de paquet, dépendances, import, progression vierge et API de l’Atelier passent. Les tests de l’enchaînement Core emploient un petit processus simulé et ne constituent pas une preuve de jeu natif. Les essais graphiques sous WSL avec rendu logiciel **n’ont pas atteint de niveau jouable**. Un vrai PC Linux Mint 22.3 avec pilote graphique adapté reste à tester ; le candidat Linux n’est pas une version stable ni une validation de jouabilité Mint.

Avant toute mise à jour, fermer le jeu et l’Atelier puis sauvegarder tout `pc/profiles/content-personal` ainsi que `pc/content`. Les journaux du profil font partie de la progression ; ne pas les remplacer par ceux du paquet vierge.

## English

Windows x64 prerelease and experimental Linux x64 candidate, with fresh progression and 3% volume. No ROM, personal save, currency, private creation or music cache is included. Import of an original USA ROM (.z64, .v64, .n64) remains local and leaves the selected file unchanged.

### Additions since the first public beta

- Sixteen unlockable colours: six originals, white/black, six hybrids, gold and silver. Gold/silver eggs become eligible after all 24 Hybrid Trial levels have actually been cleared, then require finding the egg and finishing a level. Previously recorded valid hybrid clears are retained.
- Gold/silver in solo play and compatible creations, with ten egg colours in the editor catalogue. Placing an egg in a creation grants no permanent unlock in ordinary progression.
- Hybrid Story: its own campaign, lives and records, separate from Classic Story, with shared purchases and owned colours. It uses 75% hybrid fruit and makes one new hybrid egg searchable after each cleared world. Hybrid mode purchase and all 24 classic Trial clears are still required.
- Original music from the first book display and when returning to the menu, stopping when gameplay resumes. Its audio cache is generated locally; no music recording is distributed.
- Expanded Workshop: copies of all 24 levels and their rooms, editable terrain and collisions, painting/fill/sections, object groups, per-room music, passages and entry coordinates. Hybrid fruit bubbles, Shy Guy variants and special eggs remain subject to compatible room resources.
- Custom games: ordered level lists, thirty-fruit level chaining and resume. Test profiles stay separate from personal progression.
- Standalone Linux Workshop with local ROM import and launch through `Jouer.sh`; `Atelier.sh` opens its local browser interface. No .NET, Python or compiler is required for players. The Windows Workshop is also standalone.

### Mode status

Hard Story is integrated with a separate run, Green-only start, egg recruitment and cumulative tracking of recruited Yoshis kept alive until Bowser. **Public access remains locked:** all sixteen colours and a Boss Battle victory with each are required, but sixteen-colour Boss completion is not available yet. No synthetic achievement or entitlement bypass is granted.

Co-op and Boss Battle contain prototypes but remain **incomplete and deferred**. Hosted play between two PCs is not announced as available. The six bonus levels, final unlock tree, full human playthroughs and complete translation of native game text remain pending.

### Platforms and verification

The Windows package retains the **unchanged solo engine 0.33.0**, alongside the updated standalone Workshop. Prior native tests cover targeted paths; this prerelease does not certify natural acquisition of every egg, complete campaigns or long-term stability.

The Linux candidate uses engine **0.34.0**, built on Ubuntu 22.04/glibc 2.35, with the standalone Workshop. Package, dependency, import, fresh-progression and Workshop API checks pass. Core chaining tests use an explicit small fake process and do not prove native gameplay. Software-rendered WSL graphics tests **did not reach a playable level**. A real Linux Mint 22.3 PC with a suitable graphics driver still needs testing; the Linux candidate is neither a stable release nor a Mint gameplay certification.

Before updating, close the game and Workshop and back up all of `pc/profiles/content-personal` and `pc/content`. Profile journals are part of progression; do not replace them with the archive’s fresh profile.
