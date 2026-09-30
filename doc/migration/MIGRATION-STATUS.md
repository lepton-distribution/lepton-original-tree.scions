# MIGRATION-STATUS — Lepton, IAR/Windows → GCC/Linux

Source de vérité du chantier (ORCHESTRATION §1 et §6). Prérempli le 2026-09-30 à partir des
décisions de l'auteur et de relevés faits sur l'arbre réel ; à tenir à jour à chaque session.

## Avancement

| Étape | Statut | Date | Notes |
|---|---|---|---|
| 0 — Arbre des sources (scion) | TERMINÉ | 2026-09-30 | validé par l'utilisateur le 2026-09-30 ; rootstock `~/lepton`, trunk `trunk/`, clone `master` `055fc60` ; plan sur `migration/etape-0` ; handoff `handoff/etape-0.md` |
| 1 — Inventaire | TERMINÉ | 2026-09-30 | validé par l'utilisateur le 2026-09-30 ; branche `migration/etape-1` fusionnée ; handoff `handoff/etape-1.md` ; Graphify (optionnel) non fait |
| 2 — Build CMake, noyau statique, mklepton | TERMINÉ | 2026-09-30 | validé par l'utilisateur le 2026-09-30 ; branche `migration/etape-2` fusionnée ; `ctest -L host` 5/5 ; handoff `handoff/etape-2.md` |
| 3a — Noyau dynamique QEMU, UART | EN COURS | 2026-09-30 | paliers 1-3, 5 (KAL) et 6 verts, 4 partiel ; banc KAL T0-T8 vert (M4 soft-float) ; `ci/run.sh` complet vert ; reste palier hard-float (E3) ; reprise : `handoff/etape-3a.md`, journal `validation-qemu-mps2-an386.md` |
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
| m4 (`mps2-an386`) | T0-T8 verts en soft-float (2026-09-30) ; hard-float à faire | |
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
| 2026-09-30 | Licence embOS : SEGGER Friendly License, usage évaluation / non commercial (redistribution interdite, SFL §1b) ; réévaluation avant tout usage produit. |
| 2026-09-30 | Paquet embOS-Classic V5.20.0.0 Cortex-M GCC hors git, dans le rootstock : `~/lepton/third_party/embos/cortexm-gcc/5.20.0.0/` (copie de `archives/embOS`), exporté par `scripts/lepton-env.sh` en `LEPTON_EMBOS_ROOT`. |
| 2026-09-30 | Code gelé : proposition `code-gele.md` acceptée, sauf Cortex-M7 Atmel SAMV71/SAME70 maintenu en différé (référence M7). Gelé : 765 fichiers / 158 061 lignes. |
| 2026-09-30 | Oracle mklepton sans binaire : `libkernel.so` i386 absente, copie historique hors arbre non utilisée ; référence = sorties versionnées (`mklepton-ref.md`) ; image UFS validée par exécution sous QEMU (étape 3). Portage natif de mklepton maintenu (pas de repli). |
| 2026-09-30 | Étape 2 — stockage de l'image UFS du noyau statique : nouveau pilote bloc sur fichier hôte portable (`kernel/dev/arch/host/`, POSIX `pread`/`pwrite`), successeur de `dev_linux_fileflash` gelé. |
| 2026-09-30 | Étape 2 — sorties de mklepton : option `--output-dir` (`$LEPTON_BUILD/<preset>/generated/`) prioritaire sur les `dest_path` ; chemins d'entrée des mkconf rendus relatifs par script. |
| 2026-09-30 | Étape 2 — second pilote logiciel du noyau statique : `dev_part` (doublon `dev_null` du guide, liste alphabétique). |
| 2026-09-30 | Étape 2 — format UFS : structures écrites identiques x86_64 / i386 / arm-none-eabi (probe réel) ; assertions statiques des deux côtés. |
| 2026-09-30 | Étape 2 — noyau statique hôte compilé en 32 bits (`-m32`, ILP32 comme ARM) : le noyau transmet des `va_list` par argument variadique (`vfs.c`, `I_LINK`), impossible avec l'ABI x86_64. Paquets `gcc-multilib`, `libexpat1-dev:i386`. |
| 2026-09-30 | Étape 3 — premier palier embOS en soft-float (`libosT7L<mode>.a`, `-mfloat-abi=soft`, équivalent de l'IAR actuel) ; hard-float (`libosT7VHL`, trame FPU E3) en palier suivant, avant la fin de l'étape 3. |
| 2026-09-30 | Étape 3 — frontière libc : newlib-nano pour le noyau et le démarrage ; API POSIX applicative = `lib/libc` Lepton (préfixée) ; exports de `lib/libc` en conflit avec newlib exclus. |
| 2026-09-30 | Étape 3 — démarrage et RTOSInit écrits pour Lepton d'après l'API documentée (UM01001/UM01039), versionnés ; du paquet Segger, seuls `RTOS.h` et `libos*.a` (hors git). |
| 2026-09-30 | Étape 3 — contenu de `bin` : tests POSIX T9-T11 du banc KAL (pseudo-binaires lancés depuis `lsh`). |
| 2026-09-30 | Étape 3 — constat : avec embOS/Cortex-M, l'appel système est un événement embOS vers la tâche noyau (pas de SVC) ; aucun assembleur Lepton à traduire hormis démarrage et vecteurs (écart à ETAPE-3 tâche 2). |
| 2026-09-30 | Étape 3 — embOS lié en mode SP en Debug (comme IAR) plutôt que DP ; puis verrou des appels système refondu en sémaphore (`core-segger/kernel_syscall_lock.c`) : embOS 5.20 n'accepte pas qu'un mutex soit rendu par une autre tâche que son propriétaire (DP : erreur ; SP : état incohérent et blocage). |
| 2026-09-30 | Étape 3 — banc KAL : T2 et T8 alignés sur Lepton (amendement de `BANC-TEST-KAL-QEMU.md`) : pas de redémarrage depuis le contexte de départ (embOS 5.20 : routine et trampoline `OS_StartTask` au-dessus du cadre ; Lepton ne s'en sert que comme référence de pile du vfork, `exec` crée une nouvelle tâche). |
| 2026-09-30 | Étape 2 — critère mklepton reformulé (oracle sans binaire) : C généré structurellement conforme à `mklepton-ref.md`, deux exécutions identiques octet à octet, image UFS relue par le test hôte puis montée sous QEMU (étape 3). |

## Décisions ouvertes (ORCHESTRATION §4)

| Étape | Décision |
|---|---|
| 5 | Niveau d'optimisation final ; BSP embOS de base `ST/STM32F429_STM32F429ZI_Nucleo` (proposition étape 1 : vecteur CRYP et RAM à adapter). |
| 6 | Modèle exact de la Discovery F7 ; cartes M3 et M0+ ; suppression des fichiers IAR (tag `legacy-iar`). |
| 7 | Devenir du backend embOS. |

## Versions épinglées

| Élément | Version | Référence |
|---|---|---|
| `scion` | 0.5.0.1 | tag `0.5.0.1` = `54dc319`, `lepton-distribution/seed.scions` ; installé (pipx) depuis le clone local `e0adb2c`, code identique au tag (décision 2026-09-30) |
| Seed | `original-tree` | `083c30b`, `lepton-distribution/lepton-seed.scions` |
| Arbre Lepton | `master` | `055fc602f32f` (clone du rootstock, 2026-09-30 ; identique au relevé du 2026-09-29), `lepton-distribution/lepton-original-tree.scions` (tag `version-4.9.0.2` présent ; la branche `main` ne contient qu'un commit initial vide) |
| QEMU | ≥ 8.2 | cartes mémoire relevées sur 8.2.2 |
| embOS | embOS-Classic V5.20.0.0 Cortex-M GCC | `$LEPTON_EMBOS_ROOT` (hors git) ; pas de paquet RISC-V dans `archives/` |
| `gcc-multilib`, `libexpat1-dev:i386` | Debian 13 | noyau statique `-m32`, mklepton (étape 2) |
| `gcc-arm-none-eabi` | 14.2.1 (`15:14.2.rel1-1`), newlib 4.5.0.20241231 (nano inclus) | Debian 13, relevé 2026-09-30 |
| CMake / Ninja | 3.31.6 / 1.12.1 | Debian 13 |
| QEMU | 10.0.13 | `mps2-an385/386/500`, `microbit` présents |
| OpenOCD / gdb-multiarch | 0.12.0 / 16.3 | règles udev `60-openocd.rules` |
| cloc / coccinelle | 2.04 / 1.3 | |

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

## Métriques de l'étape 1 (2026-09-30)

| Mesure | Valeur | Source |
|---|---|---|
| Volumétrie (cloc, code) | actif 1074 f / 314 773 l ; différé 1511 / 409 081 ; gelé 765 / 158 061 ; hors-projet 762 / 192 635 | `perimetre.md` |
| IAR-ismes (sévérité `iar`) | **actif 147** au 2026-09-30 (étape 3 ; 148 à l'étape 1) ; différé 935 ; gelé 578 ; hors-projet 224 | `audit_iar.py --summary` |
| Sources dérivées | `mps2-an386` : 217 f / 74 323 l ; NUCLEO-F439ZI : 353 f (base Olimex P407) | `perimetre.md` |
| Écart embOS 5.18.3.1 IAR → 5.20.0.0 GCC | actif : 189 occurrences, 25 fichiers ; 7 écarts (E1 bloquant : branche embOS de `kal.h` réservée IAR/Keil) | `embos-iar-vs-gcc.md` |
| Graphe | CFC principale de 15 composants (376 symboles) : core, core-segger, vfs, net, libc, fs | `dependances.md` |

## Blocages et dette

- Étape 3 : `RTOS.h` : avertissement `struct _reent` (type newlib absent en freestanding), sans effet constaté. (`malloc.c` : section atomique rétablie sous GCC, corrigé le 2026-09-30.)
- Étape 3 : E4 `OS_MakeTaskReady(OS_TASK*)` déclarée par Lepton (HYPOTHÈSE À VALIDER, non documentée par Segger) ; mode embOS SP au lieu de DP (retour à DP possible depuis le verrou en sémaphore, à vérifier) ; `__KERNEL_UCORE_EMBOS` posé par CMake et par les `user_kernel_mkconf.h` des cartes existantes (double définition compatible).
- Étape 2 : `vfs.c` (I_LINK) transmet un `va_list` par argument variadique (non portable, cause du `-m32`) → passer `va_list*` à l'étape 4 ; `__kernel_set_errno` en mode statique n'enregistre rien ; backend `core-static` = 3ᵉ copie de l'amorçage (`_kernel_warmup_*`) ; `kernelconf.h` : `kernel_mkconf.h` des branches GCC croisées encore en chemin fixe (étape 3).
- Code Segger embOS IAR déjà versionné sous `src/kernel/core/ucore/embOS*` (licence Segger) : à considérer avant tout push.
- Pièges à reprendre : `__compiler_directive__packed` vide sous GCC (3 usages actifs) ; `kal.h` ligne ~1089 précédence `|| cortexM7` ; `int64_t` = `long` dans `etypes.h` ; 161 `#if` ISA/cœur hors arch (34 fichiers).
