# Handoff étape 2 → 3

## Réponses aux prérequis de 3
- Liste de sources `mps2-an386` : `perimetre.md` (217 f, dérivée de `tauon-kernel-cortex-m4-debug`).
- Presets : `qemu-mps2-an386-embos`, `nucleo-f439zi-embos` **configurés** (toolchain, cpu, carte,
  kal embos) mais pas construits : BSP vide, `lepton_libc` sans source, `kal.h` GCC+embOS (E1).
- mklepton natif : `$LEPTON_BUILD/host/mklepton` (`-s <trunk> -o <sortie> -t <cible> <mkconf>`) ;
  `lepton_generate()` (`cmake/mklepton.cmake`) ; preset croisé → `LEPTON_MKLEPTON`.
- Image UFS : format vérifié identique i386 / arm-none-eabi (assertions `tests/host/ufs_format.c`
  compilées des deux côtés, `ctest -L host`) ; image relue par le noyau statique (28 pseudo-binaires,
  mkconf Olimex P407) ; montage sous QEMU = critère de l'étape 3.
- `gcc-arm-none-eabi` 14.2.1, QEMU 10.0.13, embOS V5.20.0.0 (`$LEPTON_EMBOS_ROOT`, hors git).

## Décisions actées pendant 2 (MIGRATION-STATUS)
- Image UFS du noyau statique : pilote fichier hôte (`kernel/dev/arch/host/`, `.fsflash.o`).
- Sorties mklepton : `-o` dans le build ; mkconf en chemins relatifs au trunk (`mkconf_relpaths.py`).
- Second pilote logiciel : `dev_part`. Critère mklepton reformulé (oracle sans binaire).
- Noyau statique en `-m32` (va_list transmis par `...` dans `vfs.c`, I_LINK).

## Artefacts produits (manifeste, pas copie)
| Fichier | Contenu et quand le lire |
|---|---|
| `doc/migration/ajout-coeur.md` | axes, emplacements, bibliothèques, cycles ; avant tout ajout de cœur/carte |
| `scion/cmake/` (`isa`, `cpu`, `boards`, `kal`, `components`, `toolchains`, `mklepton.cmake`) | build ; `boards/qemu-mps2-an386.cmake` et `kal/embos.cmake` sont les points d'entrée de 3 |
| `scion/ld/mem_*.ld` | blocs MEMORY seuls (F439 : 192 Ko, HYPOTHÈSE À VALIDER) |
| `scion/tests/host/` | 5 tests `host` ; `mklepton_check.cmake` = critère mklepton |
| `scion/sys/root/src/kernel/core/core-static/`, `arch/host/` ; `kernel/dev/arch/host/` | noyau statique hôte |
| `tools/migration/mkconf_relpaths.py`, `lib_unresolved.sh` | rejouables |

## Écarts au plan et pièges découverts
- Arborescence CMake : `cmake/isa/`, `cmake/kal/`, `cmake/components/` ajoutés (documenté).
- Dépendances : les bibliothèques de la CFC 1 (core, kal, vfs, fs, dev, libc) sont liées en
  un groupe `LINK_GROUP:RESCAN` ; pas de `target_link_libraries` intra-groupe (circulaire).
- `kernelconf.h` : les branches GCC incluent encore `kernel/core/arch/cortexm/kernel_mkconf.h` en
  dur → à passer par chemin d'inclusion (répertoire généré), comme la branche statique.
- `core-segger/heap.c` et `kernel/core/heap.c` coexistent : doublon de symboles probable.
- libc Lepton : `lib/libc/stdlib.c` exporte `abort/div/ldiv`, `stdio` entraîne FILE + appels
  système → frontière newlib à décider (§4 étape 3).
- Défauts corrigés (commits sémantiques) : nœuds et entrées UFS non initialisés (droits aléatoires
  sur cible), macros RTC (`desc` au lieu de `__desc`), `int64_t`, `struct dirent`, stdint GCC.
- Dette : `vfs.c` I_LINK transmet un `va_list` par `...` (non portable ; étape 4, passer `va_list*`) ;
  `__kernel_set_errno` en mode statique n'écrit rien ; backend statique = 3ᵉ copie de l'amorçage.
- `grep -R` rate les fichiers ISO-8859 (vus binaires) : `grep -a`. Hook : tout `>` ou `sed -i`
  d'une commande est résolu depuis le cwd de session → éditer par Edit/Write ou scripts.
- `-mcpu` hors `cmake/` : seulement dans des fichiers hérités (SConstruct, eCos `.cdl/.ecc/.mak`,
  `tools/config/t-arm-elf`), non modifiés.

## Non transmis volontairement
- Détail des itérations de compilation (journal git, `lib_unresolved.sh` rejouable).
- Graphe Graphify : non produit (optionnel).
