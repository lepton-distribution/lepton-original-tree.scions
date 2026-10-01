# MIGRATION-STATUS — Lepton, IAR/Windows → GCC/Linux

Source de vérité du chantier (ORCHESTRATION §1 et §6). Prérempli le 2026-09-30 à partir des
décisions de l'auteur et de relevés faits sur l'arbre réel ; à tenir à jour à chaque session.

## Avancement

| Étape | Statut | Date | Notes |
|---|---|---|---|
| 0 — Arbre des sources (scion) | TERMINÉ | 2026-09-30 | validé par l'utilisateur le 2026-09-30 ; rootstock `~/lepton`, trunk `trunk/`, clone `master` `055fc60` ; plan sur `migration/etape-0` ; handoff `handoff/etape-0.md` |
| 1 — Inventaire | TERMINÉ | 2026-09-30 | validé par l'utilisateur le 2026-09-30 ; branche `migration/etape-1` fusionnée ; handoff `handoff/etape-1.md` ; Graphify (optionnel) non fait |
| 2 — Build CMake, noyau statique, mklepton | TERMINÉ | 2026-09-30 | validé par l'utilisateur le 2026-09-30 ; branche `migration/etape-2` fusionnée ; `ctest -L host` 5/5 ; handoff `handoff/etape-2.md` |
| 3a — Noyau dynamique QEMU, UART | TERMINÉ | 2026-09-30 | validé par l'utilisateur le 2026-09-30 : paliers 1-6 verts (4 tracé et archivé), hard-float (preset principal) et soft-float ; E3 corrigé ; banc KAL T0-T8 + T1F/T4F/T6F/T7F verts ; `ci/run.sh` vert ; handoff final `handoff/etape-3a.md`, journal `validation-qemu-mps2-an386.md` |
| 3b — Noyau dynamique QEMU, Ethernet | TERMINÉ | 2026-09-30 | validé par l'utilisateur le 2026-09-30 ; palier 7 vert (pilote LAN9118, lwIP 2.0.1, ping et `ftpd` depuis l'hôte, `ctest -L net`, hard et soft) ; `ci/run.sh` vert ; complément tâche 1 (`compiler.h`, `lepton_irq.h`, amorce `transform_iar.py`) fait le 2026-09-30 ; **étape 3 validée par l'utilisateur le 2026-09-30**, branche `migration/etape-3` fusionnée ; handoff `handoff/etape-3.md` |
| 4 — Portage C, KAL | EN COURS | 2026-10-01 | par module (tableau ci-dessous) ; session 4.0 (outillage, ligne de base) validée par l'utilisateur le 2026-10-01, branche `migration/etape-4-outillage` fusionnée : `transform_iar.py` (7 règles), `mass_compile.sh` 243/348, `audit_isa_ifdef.py` ; aucun source transformé ; handoff `handoff/etape-4-outillage.md` ; module `kernel/core` (hors KAL) fait le 2026-10-01 sur `migration/etape-4-kernel-core`, validé par l'utilisateur le 2026-10-01 et fusionné, handoff `handoff/etape-4-kernel-core.md` ; module KAL fait le 2026-10-01 sur `migration/etape-4-kal`, validé par l'utilisateur le 2026-10-01 et fusionné, handoff `handoff/etape-4-kal.md` ; module KAL-2 (directives ISA/cœur de `kernel/core`) fait le 2026-10-01 sur `migration/etape-4-kal-2`, **en attente de validation utilisateur**, handoff `handoff/etape-4-kal-2.md` |
| 5 — NUCLEO-F439ZI | À FAIRE | | |
| 6 — Généralisation, CI, retrait IAR | À FAIRE | | par carte |
| 7 — Backend FreeRTOS | À FAIRE | | |
| Annexe RISC-V | REPORTÉ | 2026-09-30 | exigences d'architecture vérifiées aux étapes 2, 4, 6 |

## Modules de l'étape 4

| Répertoire | Transformé | mass_compile | audit_iar | Notes |
|---|---|---|---|---|
| ordre proposé (4.0) | | | | `kernel/core` hors KAL → KAL → `kernel/dev` → `kernel/fs` → `kernel/net` → `lib` → `sbin`, `bin`, `tauon-basic` → `tools/mklepton` |
| `kernel/core` (hors KAL) | oui (2026-10-01) | **50/50** (47 avant) | Lepton 7 (tous dans `kal.h`), tiers 38 | règles `garde-cible-gelee`, `garde-iar-arm`, `garde-compilateur`, `prototype-static` ; CCM et `modem_core.c` à la main ; gcc -E identique ; 24 copies `legacy/` ; reste 25 directives ISA/cœur de cibles actives → session KAL |
| KAL (`kernel/core/kal.h`) | oui (2026-10-01) | `kernel/core` 50/50 | `kal.h` 7 → 0 (`kernel/core` Lepton 0) | `kal.h` dispatcher sans condition (2137 → 71 l.) ; `kal/arch/{armv7m,host}`, `kal/backend/{embos,static,freertos}`, `kal/contrat.h` ; `kal_split.py` ; gcc -E identique, code objet identique ; ISA/cœur hors arch 68 → 51 |
| KAL-2 (directives ISA/cœur de `kernel/core`) | oui (2026-10-01) | `kernel/core` 50/50 | inchangé | `axes_kernel_core.py` (statique, gelee, inutilise) ; `kal/arch/<isa>/kal_arch_conf.h` ; `__KERNEL_CPU_NAME`, `__KERNEL_STACK_SIZE` par `cmake/cpu` ; code objet identique ; ISA/cœur hors arch 51 → 21 (`kernel/core` 0 ; restent `fs` 9, `lib/libc` 5, `sbin` 4, `tauon-basic` 3, à leurs modules) |
| `kernel/dev` | non | 65/154 | Lepton 15, tiers 16 | 71 échecs `Legacy/stm32_hal_legacy.h` (casse, HAL tiers) ; `__packed` STM32F4 |
| `kernel/fs` | non | 23/26 | Lepton 0, tiers 4 | `strtok_r`, `__weak` FatFs, `f_close` |
| `kernel/net` | non | 38/38 | 0 | |
| `lib` | non | 27/27 | 0 | |
| `sbin` | non | 32/35 | 0 | `atof`, `toupper` non déclarés |
| `bin` | non | 3/8 | 0 | `perror`, `tolower`, `strerror` non déclarés ; `libc_accept` ; M16C `test2.c` |
| `sys/user/tauon-basic` | non | 7/9 | Lepton 32 | `dlmalloc.c` variante DLIB IAR (manuel) |
| `tools/mklepton` | non | 1/1 (hôte) | Lepton 4 | `#pragma memory` M16C dans des chaînes de génération |

## Matrice du banc KAL (T0-T11)

| Cœur | embOS | FreeRTOS |
|---|---|---|
| m4 (`mps2-an386`) | hard-float (preset principal) : T0-T8 et variantes FPU T1F, T4F, T6F, T7F vertes ; soft-float : T0-T8 verts (2026-09-30) ; T0 inclut désormais réseau et `ftpd` lancés par le `.init` ; test `IRQ` (sections critiques `lepton_irq.h`) vert hard et soft | |
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
| 2026-09-30 | Étape 3 — palier hard-float : preset principal `qemu-mps2-an386-embos` en hard-float (`libosT7VHLSP.a`) ; preset `qemu-mps2-an386-embos-soft` conservé dans `ci/run.sh` (chemin sans FPU jusqu'aux cœurs M3/M0 de l'étape 6). |
| 2026-09-30 | Étape 3 — banc KAL T8 en hard-float : « aucun état FPU ne survit » = aucun contexte FPU hérité (FPCA = 0, cadre de départ de base, FPSCR par défaut) ; contenu résiduel de S16-S31 journalisé, non exigé (amendement de `BANC-TEST-KAL-QEMU.md`). |
| 2026-09-30 | Sécurité — registres FPU résiduels lisibles entre tâches et entre images (constaté par T8) : pas d'effacement tant que Lepton n'isole pas la mémoire ; à traiter avec toute évolution utilisant la MPU (dette, ci-dessous). |
| 2026-09-30 | Étape 3b — pile réseau du socle (et de la F439) : lwIP 2.0.1 de l'arbre et couches `lwip_core` ; choisie par la carte (`LEPTON_NET_STACK`, `USE_LWIP`). |
| 2026-09-30 | Étape 3b — palier 7 sans privilège : tap créé dans un espace de noms utilisateur et réseau (`unshare --user --map-root-user --net`) par le test ; échec explicite si indisponible. |
| 2026-09-30 | Étape 3b — frontière libc : `strdup` (tas Lepton) et `strerror` (numérotation errno Lepton) ajoutés à `lib/libc` ; les versions newlib sont exclues pour l'applicatif (tas et numérotation errno différents). |
| 2026-10-01 | Étape 4 — D1a : code tiers vendored (CMSIS, HAL/driverlib ST, FatFs, yaffs…) non transformé ; ses IAR-ismes (58) justifiés dans `residuel-etape4.md` ; le critère « zéro IAR-isme » porte sur le code Lepton actif (`audit_iar.py`, colonne « code Lepton »). |
| 2026-10-01 | Étape 4 — D2a : branches gelées (eCos, ARM7/9, M16C IAR, Win32) des fichiers actifs extraites à contenu constant (copie d'origine sous `legacy/`, `kal/legacy/` pour le KAL), classées gelées, supprimées à l'étape 6 ; pas de suppression à l'étape 4. |
| 2026-10-01 | Étape 4 — D3a : extension de D2a aux branches des cibles gelées Win32, ARM7/ARM9, M16C, eCos (macros CPU_*, valeurs de cœur et de puce) et des compilateurs non GCC (Keil, Visual C) : retirées avec copie d'origine sous `legacy/` ; gardes GCC levées (branches héritées de la simulation Linux non corrigées). |
| 2026-10-01 | Étape 4, module KAL-2 : périmètre `kernel/core` seul (les 21 autres directives à leurs modules) ; D3a étendu à la simulation Linux (`CPU_GNU32` hors noyau statique) ; réglages d'ISA dans `kal/arch/<isa>/kal_arch_conf.h`, réglages de cœur en définitions de `cmake/cpu/<cœur>.cmake`. |
| 2026-10-01 | Étape 4, module KAL : décomposition sous `kernel/core/kal/{arch,backend}/`, `kal.h` dispatcher par chemins d'inclusion (`LEPTON_KAL_ARCH_DIR`, `LEPTON_KAL_BACKEND_DIR`) ; copies d'origine sous `legacy/` ; branche FreeRTOS extraite telle quelle (ses `#if` de cœur jusqu'à l'étape 7). |
| 2026-10-01 | Étape 4 — plan de la session 4.0 (outillage et ligne de base, sans transformation de source) approuvé ; un module par session ensuite. |
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
| QEMU | 10.0.13 | `mps2-an385/386/500`, `microbit` présents ; référence du pilote LAN9118 et de son câblage : sources QEMU v10.0.0 (`hw/net/lan9118.c`, `hw/arm/mps2.c`) |
| lwIP | 2.0.1 | dans l'arbre (`kernel/net/lwip`), portage `ports/arm` (embOS) |
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
| IAR-ismes (sévérité `iar`) | **actif 109** au 2026-10-01 (Lepton 51, tiers 58 ; après le module KAL) ; 116 après `kernel/core` ; 128 à la fin de l'étape 3 (147 avant le complément, 148 à l'étape 1) ; chaîne minimale : 31 dont 12 CMSIS (tiers) et 19 résiduels (`residuel-etape3.md`) ; différé 935 ; gelé 578 ; hors-projet 224 | `audit_iar.py --summary` |
| Sources dérivées | `mps2-an386` : 217 f / 74 323 l ; NUCLEO-F439ZI : 353 f (base Olimex P407) | `perimetre.md` |
| Écart embOS 5.18.3.1 IAR → 5.20.0.0 GCC | actif : 189 occurrences, 25 fichiers ; 7 écarts (E1 bloquant : branche embOS de `kal.h` réservée IAR/Keil) | `embos-iar-vs-gcc.md` |
| Graphe | CFC principale de 15 composants (376 symboles) : core, core-segger, vfs, net, libc, fs | `dependances.md` |

## Blocages et dette

- Étape 3, tâche 1 : non livrée en 3a (constat 3b), faite en session de complément le 2026-09-30.
  Reste pour l'étape 4 : `transform_iar.py` réécrit une condition simplifiée sur une seule ligne
  (perte de la mise en forme sur plusieurs lignes) ; résiduels de la chaîne (M16C et en-têtes AT91 :
  étape 6 ; pragma CCM F4 : étape 5 ; branches Keil/win32 : catégorie « autre »).
- 3b : `ARG_LEN_MAX` = 64 octets (`kernel/core/process.h`) tronque **sans message** la ligne de
  commande d'un processus (cause de l'échec d'`ifconfig` au démarrage) : signaler l'erreur ou
  dimensionner par la carte (`__KERNEL_ARG_LEN_MAX`) — étape 4.
- 3b : `LWIP_PROVIDE_ERRNO 1 //lepton` (`ports/arm/lwipopts.h`) : `lwip/errno.h` redéfinit les `E*` en
  numérotation Linux dans `kernel_net_core_socket.c`, `lib/libc/net/socket.c`, `inet_addr.c`
  (228 avertissements) ; errno des erreurs de socket vu par les applications possiblement faux.
  Code identique sous IAR (latent) — étape 4.
- 3b : `ifconfig` lit `if_config.if_flags` non initialisé sans `addif` ; `ftpd` : `LIST` sans argument
  seulement, sortie silencieuse (statut 0) si `socket`/`bind`/`listen` échoue.
- 3b : pilote LAN9118 écrit d'après le modèle QEMU (aucune fiche SMSC dans l'arbre) : QEMU seulement.

- **Sécurité, à reprendre avec toute évolution utilisant la MPU** (décision 2026-09-30) : le banc de
  registres FPU (S0-S31, FPSCR) est physique et partagé ; embOS ne l'efface ni à la création de tâche
  ni à la commutation (lazy stacking), et Lepton ne l'efface pas à l'`exec`. Une tâche ou une nouvelle
  image peut donc lire les valeurs flottantes, ou des entiers que GCC range dans les registres S en
  hard-float, laissées par une autre tâche (constaté par T8 : S16-S31 de l'ancienne image lisibles
  après `exec`). Sans conséquence supplémentaire aujourd'hui (aucune isolation mémoire : tout
  processus lit déjà toute la RAM, y compris les contextes sauvegardés sur les piles). Le jour où la
  MPU isole les processus : effacer S0-S31 et FPSCR à l'`exec` (dans `kal.h`, après la création de la
  tâche), évaluer l'effacement à la commutation entre processus (hors code embOS : crochet de
  commutation, ou désactivation du lazy stacking et effacement dans le KAL), étendre T8 pour exiger
  l'effacement, et traiter de même les piles libérées (contextes sauvegardés en RAM).

- Étape 3 : `RTOS.h` : avertissement `struct _reent` (type newlib absent en freestanding), sans effet constaté. (`malloc.c` : section atomique rétablie sous GCC, corrigé le 2026-09-30.)
- Étape 3 : E4 `OS_MakeTaskReady(OS_TASK*)` déclarée par Lepton (HYPOTHÈSE À VALIDER, non documentée par Segger) ; mode embOS SP au lieu de DP (retour à DP possible depuis le verrou en sémaphore, à vérifier) ; `__KERNEL_UCORE_EMBOS` posé par CMake et par les `user_kernel_mkconf.h` des cartes existantes (double définition compatible).
- Étape 2 : `vfs.c` (I_LINK) transmet un `va_list` par argument variadique (non portable, cause du `-m32`) → passer `va_list*` à l'étape 4 ; `__kernel_set_errno` en mode statique n'enregistre rien ; backend `core-static` = 3ᵉ copie de l'amorçage (`_kernel_warmup_*`) ; `kernelconf.h` : `kernel_mkconf.h` des branches GCC croisées encore en chemin fixe (étape 3).
- Étape 4 (4.0) : HAL ST inclut `Legacy/stm32_hal_legacy.h`, répertoire `inc/legacy` (casse Windows,
  code tiers) : 71 fichiers de `kernel/dev` en échec, à résoudre côté build au module `kernel/dev`.
- Étape 4 (4.0) : `ctest` lancé hors `ci/run.sh` peut écrire `trunk/tests/__pycache__` (exporter
  `PYTHONDONTWRITEBYTECODE=1`) ; lien greffé vers un `.pyc` ignoré de `scion/tests/__pycache__`
  (nettoyage par `graft-clean` : décision).
- Étape 4 : sources en Latin-1 : l'outil Edit les réécrit en UTF-8 et altère les accents existants
  (éditer par script, `encoding="latin-1"`) ; branches `defined(__GNUC__)` héritées de la simulation
  Linux levées sans correction (`dirent.c`, `stat.c`, `kernel_pthread.h`, `select.h`, `kernelconf.h`).
- Étape 4 (KAL) : `perimetre.csv` (étape 1) ne couvre aucun fichier créé par la migration (`kal/`,
  `startup_armv7m.c`, `embos_main.c`…) : `audit_iar.py` et `audit_isa_ifdef.py` ne les voient pas
  (`kal/` audité à part : conforme). Compléter ou régénérer le périmètre avant le bilan de l'étape 4.
- Étape 4 (KAL-2) : le profil du noyau n'est plus forcé à « minimal » sur Cortex-M3 (valeur du
  `kernel_mkconf.h`, repli minimal) ; `kal/arch/armv6m/kal_arch_conf.h` (valeurs M0 IAR) : HYPOTHÈSE À
  VALIDER, étape 6.
- Étape 4 (KAL) : tests compilant hors `lepton_options` (ex. `host.ufs_format_arm`) : reprendre
  `LEPTON_KAL_ARCH_DIR`/`LEPTON_KAL_BACKEND_DIR` ; `kal/arch/armv6m` à créer à l'étape 6.
- Code Segger embOS IAR déjà versionné sous `src/kernel/core/ucore/embOS*` (licence Segger) : à considérer avant tout push.
- Pièges à reprendre : `__compiler_directive__packed` = `__lepton_packed` depuis le complément de l'étape 3 (3 usages, `flash.h` : compactage à vérifier à l'étape 5) ; `int64_t` = `long` dans `etypes.h` ; `#if` ISA/cœur hors arch : 21 (12 fichiers, hors `kernel/core`) au 2026-10-01 après KAL-2 (51 après KAL, 68 avant ; `audit_isa_ifdef.py` ; 161/34 à l'étape 1, autre méthode).
