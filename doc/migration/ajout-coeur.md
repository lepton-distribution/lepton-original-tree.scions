# Ajout d'un cœur, d'une famille, d'une carte ou d'un micro-noyau

Procédure de l'étape 2 (architecture de build à quatre axes). Principe : **ajouter une valeur =
ajouter des fichiers** aux emplacements ci-dessous, sans modifier le code commun (au plus une ligne
d'enregistrement, par exemple un preset). Chemins relatifs à la racine du trunk ; les sources Lepton
sont sous `sys/root/src/` (abrégé `src/`). Tout fichier est créé dans le clone
(`depots/lepton/original/master/scion/…`), puis `scion graft`.

## 1. Les quatre axes

| Axe | Variable de cache | Fichier CMake | Code source |
|---|---|---|---|
| ISA / famille | `LEPTON_ISA` : `host`, `armv7m`, `armv6m` (→ `rv32`) | `cmake/isa/<isa>.cmake` ; compilateur : `cmake/toolchains/<toolchain>.cmake` | `src/kernel/core/arch/<famille>/`, `src/kernel/dev/arch/<famille>/` ; KAL : `src/kernel/core/kal/arch/<isa>/kal_arch.h` (`LEPTON_KAL_ARCH_DIR`) |
| Cœur | `LEPTON_CPU` : `cortex-m0plus`, `cortex-m3`, `cortex-m4f`, `cortex-m7` | `cmake/cpu/<cœur>.cmake` (flags `-mcpu/-mfpu/-mfloat-abi`, famille de bibliothèque embOS) | aucun, sauf spécificité de cœur (FPU, cache) sous `arch/<famille>/` |
| Carte | `LEPTON_BOARD` : `qemu-mps2-an386`, `nucleo-f439zi` | `cmake/boards/<carte>.cmake` (BSP, mkconf, mémoire) | `ld/mem_<carte>.ld` ; pilotes `src/kernel/dev/arch/<famille>/…` et `src/kernel/dev/bsp/<carte>/` |
| Micro-noyau | `LEPTON_KAL_BACKEND` : `static`, `embos` (→ `freertos`) | `cmake/kal/<backend>.cmake` | `src/kernel/core/core-<backend>/` ; KAL : `src/kernel/core/kal/backend/<backend>/kal_backend.h` (`LEPTON_KAL_BACKEND_DIR`) |

Règles : aucun flag compilateur hors de `cmake/` ; aucune adresse de carte hors de `cmake/boards/`,
`ld/mem_*` et du BSP ; sélection par ces variables (et les macros qu'y posent les fichiers d'axe),
jamais par les macros du compilateur.

Macros historiques conservées et posées **uniquement** par les fichiers d'axe :
`CPU_GNU32` + `__tauon_cpu_device__` (isa `host`), `CPU_CORTEXM` (isa `armv7m`/`armv6m`),
`USE_KERNEL_STATIC` (kal `static`), `__KERNEL_UCORE_EMBOS` + `OS_LIBMODE_*` (kal `embos`).

## 2. Ajouter un cœur (même famille)

1. `cmake/cpu/<cœur>.cmake` : `cpu_flags` (compilation **et** édition de liens),
   `LEPTON_EMBOS_LIB_FAMILY` (table variante ↔ flags : `embos-inventaire.md` §3),
   `LEPTON_FLOAT_ABI` (`hard` ou `soft` : variantes FPU du banc KAL), et les
   définitions propres au cœur : `__KERNEL_CPU_NAME` (nom vu par `uname`),
   `__KERNEL_STACK_SIZE` (pile du thread noyau, `core-segger/kernel.c`) et
   `__tauon_cpu_core__` ; le CMSIS-Core du cœur s'il manque au CMSIS 3.20 de l'ISA (M7 :
   `ucore/cmsis-5/CMSIS/Core/Include`). Propriétés de la puce qui varient pour un même cœur :
   variable de cache réglée par le preset de la carte (ex. `LEPTON_M7_FPU` = `dp` ou `sp`), que
   le fichier de carte contrôle.
2. Preset dans `CMakePresets.json` (ligne d'enregistrement).
3. Si le cœur change le contexte sauvegardé (FPU, trame étendue) : code sous `src/kernel/core/arch/<famille>/`
   ou `src/kernel/core/kal/arch/<isa>/`, jamais de `#if` de cœur dans le code commun.

## 3. Ajouter une famille / ISA

1. `cmake/toolchains/<toolchain>.cmake` si le compilateur change (ex. `riscv64-unknown-elf`).
2. `cmake/isa/<isa>.cmake` : options communes, macro de famille, `LEPTON_ISA_ARCH_DIR`.
3. `src/kernel/core/arch/<famille>/` : démarrage, appel système, commutation (ce qu'embOS ou
   FreeRTOS ne fournissent pas) ; `src/kernel/dev/arch/<famille>/` : pilotes communs de la famille.
4. `src/kernel/core/kal/arch/<isa>/kal_arch.h` : primitives du KAL propres à l'ISA (`__va_list_copy`,
   tick de l'ordonnanceur `__stop_sched`/`__restart_sched`, bits d'EXC_RETURN…), sans type de
   micro-noyau ; `cmake/isa/<isa>.cmake` pose `LEPTON_KAL_ARCH_DIR` et l'ajoute aux chemins
   d'inclusion (`kal.h` inclut `kal_arch.h` sans condition) ; `kal_arch_conf.h` dans le même
   répertoire : configuration du noyau dépendant de l'ISA (`__KERNEL_COMPILER_SUPPORT_TYPE`,
   `__KERNEL_CPU_ARCH`, signaux temps réel, verrous de fichiers), incluse par `kernelconf.h`.
5. `cmake/cpu/<cœur>.cmake` pour chaque cœur (§2).
6. Intégration matérielle du micro-noyau propre à l'ISA (tick, priorités) :
   `src/kernel/core/core-<backend>/arch/<isa>/` (embOS : `embos_init_hw.c`, choisi par
   `LEPTON_ISA` dans `cmake/kal/embos.cmake` ; `embos_main.c` commun Cortex-M sous `arch/armv7m`).
   Étape 6 (ARMv6-M) : priorité de SysTick selon les bits de priorité implémentés.
7. Banc KAL : registres sauvegardés en assembleur de l'ISA sous `tests/kal/arch/<isa>/`
   (branche de `tests/kal/CMakeLists.txt`) ; `kal_bench.c` (C commun Cortex-M) reste sous
   `tests/kal/arch/armv7m/`. Démarrage de la famille : `src/kernel/core/arch/<famille>/startup_<isa>.c`
   (`LEPTON_ISA_STARTUP_SOURCES`), une table de vecteurs par profil (ARMv6-M : 32 IRQ au plus).

## 4. Ajouter une carte

1. `cmake/boards/<carte>.cmake` : contrôle de `LEPTON_CPU`, `LEPTON_BOARD_MEMORY_LD`,
   `LEPTON_BSP_NAME`, `LEPTON_BSP_SOURCES` (bibliothèque `lepton_bsp_<carte>`, jamais dans
   `lepton_dev`), `LEPTON_BOARD_MKCONF` (+ `LEPTON_BOARD_MKCONF_TARGET`),
   `LEPTON_BOARD_UNAME_MACHINE` et la définition `__KERNEL_CPU_DEVICE_NAME` qui en découle
   (nom vu par `uname`) : une carte nouvelle n'ajoute **rien** à la table
   `__tauon_cpu_device__` de `kernelconf.h` (cartes antérieures seulement, étape 6).
   Machines voisines : un BSP commun et un en-tête d'adresses par machine (ex.
   `bsp/qemu_mps2/<machine>/qemu_mps2_machine.h`, choisi par le chemin d'inclusion).
   Paquet de périphériques qui exige un CMSIS-Core plus récent que celui de l'ISA (ex. DFP
   Microchip SAMD21 : CMSIS-Core 5) : chemin `ucore/cmsis-5/CMSIS/Core/Include` ajouté par le
   fichier de carte en tête (`BEFORE`). Petite RAM : pile principale réduite dans
   `ld/mem_<carte>.ld` (`__main_stack_size__`), tables du noyau par le mkconf et le
   `user_kernel_mkconf.h` de la carte (profil `minimal`, `__KERNEL_RTFS_*`,
   `__KERNEL_STDIO_PRINTF_BUFSIZ`) ; `openfiles` ≥ 12 (8 ne suffisent pas à `initd`).
   Variante de bibliothèque du micro-noyau propre à la puce : `LEPTON_EMBOS_LIB_VARIANT`
   (ex. `_837070`, Cortex-M7 r0p1, avec `USE_ERRATUM_837070=1`). Paquet constructeur (HAL,
   CMSIS Device) : code tiers non modifié, configuration (`*_hal_conf.h`) dans le BSP.
   Puce dont le cœur n'a pas de FPU alors que l'axe cœur en prévoit une (Cortex-M4 du
   STM32WL55) : pas de fichier cœur nouveau, la carte impose l'ABI sans FPU
   (`LEPTON_CPU=cortex-m4f` et `LEPTON_FLOAT_ABI=soft`, posés par le preset, contrôlés par
   `cmake/boards/<carte>.cmake`). Plusieurs cartes identiques sur le banc (ex. radio entre deux
   NUCLEO-WL55JC1) : la carte génère sa cfg OpenOCD dans le répertoire de build, avec
   `adapter serial` tiré d'une variable de cache (`LEPTON_BOARD_STLINK_SERIAL`), puis source
   `debug/openocd-<carte>.cfg` ; `LEPTON_BOARD_OPENOCD_CFG` désigne la cfg générée.
   Gestionnaires d'interruption nommés à la CMSIS dans un pilote repris (ex.
   `USART2_IRQHandler`) : reliés par le BSP aux `IRQ<n>_Handler` de la table générique, et
   priorités NVIC posées par le BSP (`SystemInit`) si le pilote ne les règle pas.
   Paquet constructeur dont un fichier compilé définit des fonctions faibles (ex. `HAL_GetTick`,
   `HAL_Delay` de `stm32wlxx_hal.c`) : leur remplaçant fort va dans `LEPTON_FIRMWARE_SOURCES`
   (objet de l'exécutable), sinon l'éditeur de liens garde la version faible et ne tire pas le
   membre de bibliothèque. Pilote qui attend (`HAL_Delay`) à son chargement : les pilotes se
   chargent avant `OS_Start`, interruptions masquées, tick figé ; attente sur un compteur
   matériel (DWT du Cortex-M3/M4/M7), pas sur le tick.
   Test entre deux cartes identiques (radio) : enregistré par le fichier de carte
   (`add_test`, label `board;radio`), même image flashée sur la carte paire désignée par des
   variables de cache (sonde, console) ; aucun ajout au CMake commun.
2. `ld/mem_<carte>.ld` : bloc `MEMORY` seul.
3. mkconf de l'application : chemins relatifs au trunk (`tools/migration/mkconf_relpaths.py`) ;
   généré par `lepton_generate()` (`cmake/mklepton.cmake`) dans `$LEPTON_BUILD/<preset>/generated/`.
4. Preset `<carte>-<backend>`.

## 5. Ajouter un micro-noyau

1. `cmake/kal/<backend>.cmake` : chemins du paquet (hors git si licence), macros, bibliothèque,
   `LEPTON_KAL_SOURCES`.
2. `src/kernel/core/core-<backend>/`, et `src/kernel/core/kal/backend/<backend>/kal_backend.h` :
   types et macros du contrat (`kal/contrat.h`), en s'appuyant sur les primitives de `kal_arch.h` ;
   `cmake/kal/<backend>.cmake` pose `LEPTON_KAL_BACKEND_DIR` et l'ajoute aux chemins d'inclusion.
3. Banc KAL (`BANC-TEST-KAL-QEMU.md`) sur chaque cœur.

## 6. Bibliothèques et cycles (décisions de l'étape 2)

| Bibliothèque | Contenu |
|---|---|
| `lepton_core` | `src/kernel/core/*.c` communs ; sans les fichiers d'API système en `static` (`kernel.cmake`) |
| `lepton_kal_<backend>` | `core-<backend>/` (+ configuration fixe en `static`) |
| `lepton_dev` | pilotes logiciels `dev_cpufs`, `dev_head`, `dev_null`, `dev_part`, `dev_proc`, `dev_tty` |
| `lepton_bsp_<carte>` | pilotes matériels ; hôte : disque et horloge (`src/kernel/dev/arch/host/`) + partie POSIX séparée |
| `lepton_vfs`, `lepton_fs_rootfs`, `lepton_fs_ufs` | un système de fichiers par bibliothèque |
| `lepton_libc` | `src/lib/libc` (cibles : liste à l'étape 3) ; hôte : adaptateur glibc `arch/host/libc_host.c` |
| `lepton_kernel` | interface : groupe `$<LINK_GROUP:RESCAN,…>` des précédentes |

- Cycle principal (`dependances.md`, CFC 1 : core ↔ kal ↔ vfs ↔ fs ↔ dev ↔ libc) : **groupe
  d'édition de liens**, pas de fusion ; les bibliothèques restent séparées pour que l'axe
  micro-noyau et l'axe carte se remplacent indépendamment.
- Fusion documentée à venir (étape 3) : `kernel/core/usb` avec `kernel/usb` (CFC 2).
- `lepton_options` (options communes) est liée en PRIVATE : les options freestanding du noyau ne
  se propagent pas aux outils hôte (mklepton, compilé avec la glibc).

## 7. Noyau statique hôte (preset `host`)

`LEPTON_ISA=host`, `LEPTON_KAL_BACKEND=static` ; ILP32 (`-m32`, décision 2026-09-30),
freestanding (`-nostdinc`, déclarations libc de `src/kernel/core/include/libc/`, communes à toutes les ISA),
configuration fixe `src/kernel/core/arch/host/static/`, backend `src/kernel/core/core-static/`.
Tests `ctest --preset host` (label `host`).
