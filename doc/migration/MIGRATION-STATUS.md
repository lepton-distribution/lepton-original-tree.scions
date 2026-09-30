# MIGRATION-STATUS — Lepton, IAR/Windows → GCC/Linux

Source de vérité du chantier (ORCHESTRATION §1 et §6). Prérempli le 2026-09-30 à partir des
décisions de l'auteur et de relevés faits sur l'arbre réel ; à tenir à jour à chaque session.

## Avancement

| Étape | Statut | Date | Notes |
|---|---|---|---|
| 0 — Arbre des sources (scion) | TERMINÉ | 2026-09-30 | validé par l'utilisateur le 2026-09-30 ; rootstock `~/lepton`, trunk `trunk/`, clone `master` `055fc60` ; plan sur `migration/etape-0` ; handoff `handoff/etape-0.md` |
| 1 — Inventaire | À FAIRE | | |
| 2 — Build CMake, noyau statique, mklepton | À FAIRE | | |
| 3a — Noyau dynamique QEMU, UART | À FAIRE | | |
| 3b — Noyau dynamique QEMU, Ethernet | À FAIRE | | |
| 4 — Portage C, KAL | À FAIRE | | par module (tableau ci-dessous) |
| 5 — NUCLEO-F439ZI | À FAIRE | | |
| 6 — Généralisation, CI, retrait IAR | À FAIRE | | par carte |
| 7 — Backend FreeRTOS | À FAIRE | | |
| Annexe RISC-V | REPORTÉ | 2026-09-30 | exigences d'architecture vérifiées aux étapes 2, 4, 6 |

## Modules de l'étape 4

| Répertoire | Transformé | mass_compile | audit_iar | Notes |
|---|---|---|---|---|

## Matrice du banc KAL (T0-T11)

| Cœur | embOS | FreeRTOS |
|---|---|---|
| m4 (`mps2-an386`) | | |
| m7 (`mps2-an500`) | | |
| m3 (`mps2-an385`) | | |
| m0 (`microbit`) | | |

## Décisions actées

| Date | Décision |
|---|---|
| 2026-09-29 | Cibles abandonnées (code gelé) : ARM7, ARM9, M16C. |
| 2026-09-29 | Hôte de développement : PC x86_64 Debian natif ; Claude Code sur cette machine. |
| 2026-09-29 | Micro-noyau : embOS (port GCC Segger, paquets Cortex-M et RISC-V téléchargés), puis FreeRTOS. |
| 2026-09-29 | Arbre des sources par `scion` 0.5.0.1 ; `seed.scions` est un projet distinct, non modifié par la migration (défauts signalés par issue) ; jamais `scion git`, git natif uniquement. |
| 2026-09-29 | Numérotation du plan : ETAPE-0 à 7 (la réécriture de scion est un prérequis terminé). |
| 2026-09-29 | Plan installé à la racine du dépôt `lepton-original-tree.scions`, hors `scion/` (non greffé) ; premier commit sur la branche `migration/etape-0`. |
| 2026-09-29 | Test de fumée canonique : démarrage → `lsh` sur le périphérique série standard → `uname -a`. |
| 2026-09-29 | Carte de base : NUCLEO-F439ZI (la NUCLEO-F429ZI est obsolète). Cartes disponibles pour l'étape 6 : Olimex STM32-P407, Discovery F7. |
| 2026-09-30 | IAR n'est plus supporté ni utilisé ; aucune comparaison avec IAR. |
| 2026-09-30 | Simulation Linux abandonnée ; QEMU générique `mps2-an386` (UART, puis Ethernet LAN9118) la remplace, placé tôt (étape 3). |
| 2026-09-30 | RISC-V reporté ; l'architecture de sources et de compilation doit accueillir de nouveaux cœurs (annexe). |
| 2026-09-30 | Noyau statique hôte (sans ordonnanceur) pour mklepton, puis noyau dynamique sur QEMU (guide de l'auteur, `sources/`). |
| 2026-09-30 | Bibliothèques statiques par composant ; `bin`, `sbin`, `lib` hors de `kernel/`. |
| 2026-09-30 | Git local uniquement : aucun push ni opération distante sans accord explicite de l'utilisateur, demandé à chaque fois. |
| 2026-09-30 | Trunk : chemins des mkconf rendus relatifs au trunk (à l'étape 2) ; pas de `tauon` dans `$HOME`. Rootstock `~/lepton` (`/home/lepton-user/lepton`), trunk au nom par défaut `trunk/`. Claude Code lancé à la racine du clone par `scripts/claude-lepton.sh`. Fichiers de migration à la racine du clone (hors `scion/`). |
| 2026-09-30 | scion 0.5.0.1 conservé tel qu'installé par pipx depuis un clone local de `seed.scions` (`master` `e0adb2c`, 4 commits après le tag, écarts limités à `README.md`, `.gitignore`, `doc/` : code identique au tag) ; pas de réinstallation depuis le tag. |
| 2026-09-30 | `archives/embOS` du paquet (paquet Segger sous licence) non copié dans le clone ; emplacement à décider avec la licence (étape 1). |

## Décisions ouvertes (ORCHESTRATION §4)

| Étape | Décision |
|---|---|
| 1 | BSP embOS le plus proche du STM32F439 ; licence embOS (évaluation / production) et emplacement du paquet embOS (fourni hors dépôt dans `archives/embOS` du paquet de passation) ; sort du code gelé. |
| 2 | Second pilote logiciel du noyau statique (`dev_null` cité deux fois dans le guide) ; stockage de l'image UFS ; sorties de mklepton hors du trunk ; format UFS si différent entre hôte et ARM. |
| 3 | Frontière newlib / API POSIX Lepton ; contenu de `bin` (proposition : tests T9-T11). |
| 5 | Niveau d'optimisation final. |
| 6 | Modèle exact de la Discovery F7 ; cartes M3 et M0+ ; suppression des fichiers IAR (tag `legacy-iar`). |
| 7 | Devenir du backend embOS. |

## Versions épinglées

| Élément | Version | Référence |
|---|---|---|
| `scion` | 0.5.0.1 | tag `0.5.0.1` = `54dc319`, `lepton-distribution/seed.scions` ; installé (pipx) depuis le clone local `e0adb2c`, code identique au tag (décision 2026-09-30) |
| Seed | `original-tree` | `083c30b`, `lepton-distribution/lepton-seed.scions` |
| Arbre Lepton | `master` | `055fc602f32f` (clone du rootstock, 2026-09-30 ; identique au relevé du 2026-09-29), `lepton-distribution/lepton-original-tree.scions` (tag `version-4.9.0.2` présent ; la branche `main` ne contient qu'un commit initial vide) |
| QEMU | ≥ 8.2 | cartes mémoire relevées sur 8.2.2 |
| `gcc-arm-none-eabi`, embOS, CMake (≥ 3.24) | à consigner à l'étape 1 | |

## Constats vérifiés sur l'arbre (2026-09-29/30)

- Greffe : 4829 feuilles ; clone propre après greffe ; aucun lien cassé après déplacement du rootstock
  (rejoué le 2026-09-30 sur `~/lepton` : 2 scions, graft idempotent, aucun fichier régulier dans le trunk).
- `.gitignore` du clone hérité de Visual Studio/IAR : ignore `[Dd]ebug/`, `[B]in/`, `*.bin`, `*.a`,
  `[Ss]ettings/`, `bld/`… — risque pour les fichiers ajoutés par la migration (ex. `debug/`).
- `sed -i` sur un fichier du trunk remplace le lien par un fichier régulier et bloque `scion graft`
  (hook `.claude/hooks/lepton_guard.py`).
- `scion git` n'affiche pas la sortie de git (0.5.0.1) : défaut à signaler au projet `seed.scions`.
- Projets IAR : 46 `.ewp` (plusieurs générations d'EWARM, jusqu'à 8.40), 52 `.icf`, sous `sys/root/prj/iar/`.
- `kernel/core` : `kal.c`/`kal.h`, backends `core-segger`, `core-freertos`, `core-generic` ;
  `kernel/core/arch` : `cortexm`, `win32`.
- mklepton se lie à `-lkernel` (ancien noyau statique `prj/scons/arch/synthetic/x86_static`), dont la
  liste de sources référence `core-ecos` et `core/arch/synthetic/x86_static`, absents de l'arbre.
- `tools/bin/mklepton_gnu` : ELF i386 précompilé, lancé par `mklepton_gnu.sh` ; mkconf avec
  `dest_path="$(HOME)/tauon/…"` (écriture dans le trunk).
- Aucun pilote UART CMSDK ni LAN9118 dans `kernel/dev/arch` ; `ftpd` présent dans `src/bin/net/`.
- QEMU `mps2-an386` : SSRAM `0x00000000` et `0x20000000` (4 Mo chacune), UART CMSDK à
  `0x40004000`–`0x40007000` et `0x40009000`, LAN9118 à `0x40200000` (`0xA0000000` sur `mps2-an500`).

## Blocages et dette

(vide)
