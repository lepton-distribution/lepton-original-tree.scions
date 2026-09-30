# Étape 4 — Portage du code C sur tout le périmètre actif, décomposition du KAL

## Contexte

L'étape 3 a porté la chaîne minimale qui démarre `lsh` sur QEMU. Cette étape étend le portage à
tout le périmètre actif (étape 1 : noyau, VFS, POSIX, réseau, binaires, BSP STM32F4) et décompose
le KAL selon les axes de l'étape 2. IAR n'étant plus supporté, le code devient **GCC seul** : aucune
branche IAR n'est conservée. La validation est continue : compilation de masse, puis exécution sur
QEMU (`mps2-an386`) après chaque lot. Le volume (centaines de kLOC) impose une transformation
outillée, jamais une édition fichier par fichier.

## Prérequis

- Étape 3 : socle QEMU vert, `compiler.h`, `transform_iar.py` amorcés, banc KAL T0-T8.
- `audit-iar.md` et périmètre actif de l'étape 1.

## Tâches

### 1. Compléter `compiler.h` et CMSIS

| Macro Lepton | GCC |
|---|---|
| `__lepton_no_init` | `__attribute__((section(".noinit")))` |
| `__lepton_used` | `__attribute__((used))` (ex-`__root`) |
| `__lepton_weak` | `__attribute__((weak))` |
| `__lepton_packed` | `__attribute__((packed))` |
| `__lepton_align(n)` | `__attribute__((aligned(n)))` |
| `__lepton_section(s)` | `__attribute__((section(s)))` |
| `__lepton_ramfunc` | `__attribute__((section(".ramfunc"), long_call))` |
| `__lepton_noreturn` | `__attribute__((noreturn))` |

Conserver la couche de macros (elle isole le code d'un changement futur de compilateur ou d'ISA) ;
intrinsics IAR → CMSIS-Core.

### 2. Transformation de masse

- Exécutée **dans le clone** `depots/lepton/original/master/scion/…`, jamais dans le trunk (`sed -i`
  et les outils qui écrivent par renommage remplacent les liens) ; après chaque lot : `scion graft`,
  puis `find "$LEPTON_TRUNK" -type f ! -name .scion.grafted.list` vide.
- `tools/migration/transform_iar.py` ou patches Coccinelle : une règle par IAR-isme, activable
  individuellement ; chaque occurrence classée « automatique » ou « résiduelle »
  (`doc/migration/residuel-etape4.md`) ; seules les résiduelles sont traitées à la main.
- Répertoire par répertoire, **un commit par (répertoire × règle)** ; corrections manuelles en
  commits séparés et relus.
- Attention sémantique : `__packed` IAR sur un pointeur (accès non aligné) ; bitfields des formats
  binaires (UFS, FAT, trames réseau) ; `#pragma optimize` à supprimer sauf nécessité démontrée.

### 3. Décomposition du KAL

```
src/kernel/core/kal/
  kal.h                     dispatcher : inclut arch/<famille>/ et backend/<rtos>/ selon LEPTON_*
  arch/armv7m/              contexte, sections critiques, variantes cœur (fpu : m4f, m7 ; cache : m7)
  arch/armv6m/              M0/M0+
  backend/embos/            TCB et API embOS
```

- Emplacement exact aligné sur l'arborescence réelle et sur `ajout-coeur.md` (étape 2).
- `arch/` = ce qui dépend du processeur ; `backend/` = ce qui dépend du micro-noyau ; une macro qui
  dépend des deux vit dans `backend/` et s'appuie sur des primitives d'`arch/`.
- Une nouvelle famille (RISC-V) ne demande qu'un répertoire `arch/rv32/` et une ligne du dispatcher.
- Décomposition à contenu constant, un commit par fichier extrait ; critère : sortie `gcc -E`
  identique avant/après sur des fichiers témoins, banc KAL T0-T8 vert.

### 4. Tableau de bord

- `tools/migration/mass_compile.sh` : `arm-none-eabi-gcc -c` sur tout le périmètre actif ;
  métriques : fichiers OK / total, histogramme des erreurs (l'erreur la plus fréquente désigne la
  prochaine règle).
- Après chaque répertoire : `ctest -L smoke` et `ctest -L kal` sur `mps2-an386`.
- Métriques reportées par module dans `MIGRATION-STATUS.md`.

## Critères de validation

- [ ] `audit_iar.py` : zéro IAR-isme dans le C du périmètre actif.
- [ ] `mass_compile.sh` : 100 % du périmètre actif.
- [ ] Transformations rejouables (scripts commités) ; `residuel-etape4.md` vidé ou justifié.
- [ ] KAL décomposé selon les axes ; `grep` : aucun `#ifdef` d'ISA ou de cœur hors `kal/arch/` et
      répertoires d'architecture.
- [ ] Socle QEMU toujours vert (paliers de l'étape 3, banc T0-T8).

## Pièges connus

- Ne pas corriger en passant des comportements douteux (aliasing, unions) : `dette-technique.md`.
- Sur Cortex-M, les gestionnaires d'interruption sont des fonctions C ordinaires : pas d'attribut
  `interrupt`.
- `.noinit` doit exister dans `common-cortexm.ld`.

## À la fin de l'étape

`MIGRATION-STATUS.md` : comptes finaux, dette ; `handoff/etape-4.md` (ou par module).
