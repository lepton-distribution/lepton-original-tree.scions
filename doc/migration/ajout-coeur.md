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

1. `cmake/cpu/<cœur>.cmake` : `cpu_flags` (compilation **et** édition de liens), et
   `LEPTON_EMBOS_LIB_FAMILY` (table variante ↔ flags : `embos-inventaire.md` §3).
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
   d'inclusion (`kal.h` inclut `kal_arch.h` sans condition).
5. `cmake/cpu/<cœur>.cmake` pour chaque cœur (§2).

## 4. Ajouter une carte

1. `cmake/boards/<carte>.cmake` : contrôle de `LEPTON_CPU`, `LEPTON_BOARD_MEMORY_LD`,
   `LEPTON_BSP_NAME`, `LEPTON_BSP_SOURCES` (bibliothèque `lepton_bsp_<carte>`, jamais dans
   `lepton_dev`), `LEPTON_BOARD_MKCONF` (+ `LEPTON_BOARD_MKCONF_TARGET`).
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
