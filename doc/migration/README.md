# Plan de migration Lepton : Windows/IAR → Linux (Debian)/GCC

## Objectif

Lepton est un RTOS embarqué en C (~1 MLOC) : KAL (abstraction du micro-noyau), noyau POSIX
1003.1a/1c, VFS, pilotes, réseau, et `mklepton` (générateur XML → C et constructeur du système de
fichiers embarqué en flash). Il est aujourd'hui construit par des projets IAR `.ewp` sous Windows.

État final : développement et compilation sous Linux Debian, CMake, `arm-none-eabi-gcc`.
**IAR n'est plus supporté ni utilisé** : il ne sert pas de référence de comparaison ; les `.ewp`
ne sont lus que pour connaître les listes de sources, puis supprimés. La **simulation Linux est
abandonnée** : QEMU la remplace comme banc de validation permanent.

Cibles : Cortex-M4F (base), puis M7, M3, M0/M0+. Micro-noyau : embOS (port GCC Segger), puis
FreeRTOS. RISC-V est **reporté**, mais l'architecture de sources et de compilation est conçue pour
accueillir de nouveaux cœurs (annexe). Abandonnés (code gelé) : ARM7, ARM9, M16C, simulations.

## Environnement

- Machine principale : PC x86_64 sous Debian natif, Claude Code, sonde USB directe, KVM.
  Installation : `scripts/install-debian.sh --with-debug-tools`.
- Arbre des sources composé par `scion` 0.5.0.1 (`lepton-distribution/seed.scions`, projet distinct)
  dans un rootstock : `trunk/` = liens symboliques relatifs vers le clone
  `depots/lepton/original/master` du dépôt `lepton-original-tree.scions`. On édite et on utilise git
  (commandes natives) dans le clone ; on compile depuis le trunk.
- Conteneur `ci/Dockerfile` (même `install-debian.sh`) : référence de la CI et de la reproductibilité.
- Second hôte (étapes 8 à 10, chantier ouvert le 2026-10-07) : macOS sur Mac Intel ; Debian reste
  l'hôte de référence.
- Git local uniquement : aucun push sans accord explicite de l'utilisateur (ORCHESTRATION §5) ;
  suivi par `MIGRATION-STATUS.md`, `handoff/` et `git log` sur la machine.

## Principes directeurs

1. **Valider par l'exécution, au plus tôt.** Le noyau statique (VFS, rootfs, UFS) s'exécute sur l'hôte
   dès l'étape 2 ; le noyau dynamique tourne sur QEMU dès l'étape 3 ; chaque étape suivante se
   revalide sur ce socle.
2. **Une seule variable change par étape** : l'étape 3 change le compilateur sur un matériel émulé et
   maîtrisé ; l'étape 5 change le matériel avec un compilateur validé ; l'étape 7 change le
   micro-noyau.
3. **Architecture à quatre axes** (ISA, cœur, carte, micro-noyau) : ajouter une valeur à un axe =
   ajouter des fichiers aux emplacements prévus (`doc/migration/ajout-coeur.md`), sans toucher au
   code commun.
4. **Test de fumée canonique identique partout** : démarrage jusqu'au prompt `lsh` sur le
   périphérique standard (UART), puis `uname -a` et vérification des champs
   (`tests/smoke_lsh.py`, transport série QEMU ou port réel).

## Volumétrie (~1 MLOC)

- Périmètre = fermeture de build (socle Cortex-M + BSP de la carte), pas le dépôt entier ; le reste
  est différé (autres cartes) ou gelé.
- Transformations de masse par scripts rejouables (Coccinelle, Python), exécutées dans le clone ;
  commits mécaniques séparés des commits sémantiques.
- Pilotage par métriques (IAR-ismes restants, taux de compilation) ; sessions bornées à un module.

## Étapes

| Étape | Fichier | Objet | Livrable |
|---|---|---|---|
| 0 | `ETAPE-0-arbre-sources.md` | Rootstock scion, règles d'édition, installation du plan | Arbre greffé, plan dans le dépôt |
| 1 | `ETAPE-1-inventaire.md` | Inventaire, périmètre, audit IAR, KAL, mklepton | Rapports, matrice, code gelé |
| 2 | `ETAPE-2-build-cmake.md` | CMake multi-cœurs ; noyau statique hôte (sans ordonnanceur) ; mklepton natif | `libkernel`, mklepton, image UFS, `ajout-coeur.md` |
| 3 | `ETAPE-3-socle-qemu.md` | Noyau dynamique sur QEMU `mps2-an386` : toolchain croisée, UART puis Ethernet | `lsh`, `ps`, `ls`, `uname` et réseau sous QEMU, banc KAL |
| 4 | `ETAPE-4-portage-c.md` | Portage C de masse, décomposition du KAL | Périmètre actif GCC, KAL par axes |
| 5 | `ETAPE-5-nucleo-f439zi.md` | Carte de base NUCLEO-F439ZI | Base de départ posée |
| 6 | `ETAPE-6-generalisation.md` | Olimex P407, Discovery F7, M3, M0+ ; CI ; retrait d'IAR | Mono-chaîne GCC |
| 7 | `ETAPE-7-backend-freertos.md` | Backend KAL FreeRTOS (depuis `core-freertos`) | Iso-comportement avec embOS |
| 8 | `ETAPE-8-hote-64-bits.md` | Noyau statique hôte en 64 bits : `va_list` de `I_LINK`, `size_t`, retrait de `-m32` | mklepton 64 bits, sorties identiques |
| 9 | `ETAPE-9-hote-macos.md` | Second hôte : macOS sur Mac Intel (noyau statique, mklepton, QEMU) | `install-macos.sh`, `ci/run.sh` vert sur macOS |
| 10 | `ETAPE-10-cartes-macos.md` | Validation des quatre cartes depuis macOS, embOS | `validation-macos.md` |
| — | `BANC-TEST-KAL-QEMU.md` | Tests unitaires du KAL sous QEMU (contexte, signaux, vfork/exec) | Matrice cœur × micro-noyau |
| — | `ANNEXE-nouveau-coeur-riscv.md` | RISC-V reporté ; exigences sur l'architecture | Liste de contrôle |
| — | `sources/lepton-migration-guide-step-1.md` | Guide de l'auteur (étapes 2 et 3 du plan : §1.1-1.2 et §2.1) | Source |

```
0 ──► 1 ──► 2 ──► 3 ──► 4 ──► 5 ──► 6 ──► 7 ──► 8 ──► 9 ──► 10
                  └── banc KAL (3, 4, 6, 7)
```

## Utilisation avec Claude Code

Emplacement : dépôt `lepton-original-tree.scions`, à la racine, hors de `scion/` (non greffé) —
`CLAUDE.md`, `doc/migration/` (ce plan, `MIGRATION-STATUS.md`, `handoff/`, `sources/`),
`scripts/`, `.claude/` (hook de protection du trunk, skill). L'étape 0 amorce le rootstock depuis
le paquet décompressé et y installe le plan (voir `README-PAQUET.md`) ; les sessions suivantes :

```bash
scripts/claude-lepton.sh "Lis doc/migration/ORCHESTRATION.md et poursuis la migration."
```
