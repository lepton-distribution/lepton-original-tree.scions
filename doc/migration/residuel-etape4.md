# Transformation des IAR-ismes — rapport

Généré par `tools/migration/transform_iar.py` — ne pas éditer à la main.  
Commande : `transform_iar.py --perimetre-actif doc/migration/perimetre.csv --tiers-from-audit doc/migration/audit-iar.csv --report doc/migration/residuel-etape4.md -q`

Résumé : 9 fichier(s) modifié(s), 62 occurrence(s) automatique(s), 26 résiduelle(s).

## Résiduels (26)

| Fichier | Ligne | Règle | Motif | Détail |
|---|---:|---|---|---|
| `sys/user/tauon-basic/src/bin/free/dlmalloc.c` | 2 | garde-iar-arm | branche IAR avec _Pragma/#pragma (placement, étape 5) | `defined(__IAR_SYSTEMS_ICC__) && _DLIB_INCLUDE_DLMALLOC_ALTERNATIVE` |
| `sys/user/tauon-basic/src/bin/free/dlmalloc.c` | 4 | garde-iar-arm | branche IAR avec _Pragma/#pragma (placement, étape 5) | `defined(__IAR_SYSTEMS_ICC__)` |
| `sys/user/tauon-basic/src/bin/free/dlmalloc.c` | 3 | header-iar | en-tête IAR yvals.h : équivalent CMSIS/newlib ou branche gelée à traiter à la main | `#include <yvals.h>` |
| `sys/user/tauon-basic/src/bin/free/dlmalloc.c` | 72 | header-iar | en-tête IAR yvals.h : équivalent CMSIS/newlib ou branche gelée à traiter à la main | `#include <yvals.h>` |
| `sys/user/tauon-basic/src/bin/free/dlmalloc.c` | 73 | header-iar | en-tête IAR ysizet.h : équivalent CMSIS/newlib ou branche gelée à traiter à la main | `#include <ysizet.h>` |
| `sys/user/tauon-basic/src/bin/free/dlmalloc.c` | 42 | symbole-iar | symbole IAR __iar_Locksyslock (bibliothèque DLIB / éditeur de liens) : équivalent newlib, Lepton ou .ld à choisir | `#define PREACTION(M)  (__iar_Locksyslock(_LOCK_MALLOC), 0)` |
| `sys/user/tauon-basic/src/bin/free/dlmalloc.c` | 43 | symbole-iar | symbole IAR __iar_Unlocksyslock (bibliothèque DLIB / éditeur de liens) : équivalent newlib, Lepton ou .ld à choisir | `#define POSTACTION(M) __iar_Unlocksyslock(_LOCK_MALLOC)` |
| `sys/user/tauon-basic/src/bin/free/dlmalloc.c` | 51 | symbole-iar | symbole IAR __iar_dlcalloc (bibliothèque DLIB / éditeur de liens) : équivalent newlib, Lepton ou .ld à choisir | `#define dlcalloc               __iar_dlcalloc` |
| `sys/user/tauon-basic/src/bin/free/dlmalloc.c` | 52 | symbole-iar | symbole IAR __iar_dlfree (bibliothèque DLIB / éditeur de liens) : équivalent newlib, Lepton ou .ld à choisir | `#define dlfree                 __iar_dlfree` |
| `sys/user/tauon-basic/src/bin/free/dlmalloc.c` | 53 | symbole-iar | symbole IAR __iar_dlmalloc (bibliothèque DLIB / éditeur de liens) : équivalent newlib, Lepton ou .ld à choisir | `#define dlmalloc               __iar_dlmalloc` |
| `sys/user/tauon-basic/src/bin/free/dlmalloc.c` | 54 | symbole-iar | symbole IAR __iar_dlmemalign (bibliothèque DLIB / éditeur de liens) : équivalent newlib, Lepton ou .ld à choisir | `#define dlmemalign             __iar_dlmemalign` |
| `sys/user/tauon-basic/src/bin/free/dlmalloc.c` | 55 | symbole-iar | symbole IAR __iar_dlrealloc (bibliothèque DLIB / éditeur de liens) : équivalent newlib, Lepton ou .ld à choisir | `#define dlrealloc              __iar_dlrealloc` |
| `sys/user/tauon-basic/src/bin/free/dlmalloc.c` | 56 | symbole-iar | symbole IAR __iar_dlvalloc (bibliothèque DLIB / éditeur de liens) : équivalent newlib, Lepton ou .ld à choisir | `#define dlvalloc               __iar_dlvalloc` |
| `sys/user/tauon-basic/src/bin/free/dlmalloc.c` | 57 | symbole-iar | symbole IAR __iar_dlpvalloc (bibliothèque DLIB / éditeur de liens) : équivalent newlib, Lepton ou .ld à choisir | `#define dlpvalloc              __iar_dlpvalloc` |
| `sys/user/tauon-basic/src/bin/free/dlmalloc.c` | 58 | symbole-iar | symbole IAR __iar_dlmallinfo (bibliothèque DLIB / éditeur de liens) : équivalent newlib, Lepton ou .ld à choisir | `#define dlmallinfo             __iar_dlmallinfo` |
| `sys/user/tauon-basic/src/bin/free/dlmalloc.c` | 59 | symbole-iar | symbole IAR __iar_dlmallopt (bibliothèque DLIB / éditeur de liens) : équivalent newlib, Lepton ou .ld à choisir | `#define dlmallopt              __iar_dlmallopt` |
| `sys/user/tauon-basic/src/bin/free/dlmalloc.c` | 60 | symbole-iar | symbole IAR __iar_dlmalloc_trim (bibliothèque DLIB / éditeur de liens) : équivalent newlib, Lepton ou .ld à choisir | `#define dlmalloc_trim          __iar_dlmalloc_trim` |
| `sys/user/tauon-basic/src/bin/free/dlmalloc.c` | 61 | symbole-iar | symbole IAR __iar_dlmalloc_stats (bibliothèque DLIB / éditeur de liens) : équivalent newlib, Lepton ou .ld à choisir | `#define dlmalloc_stats         __iar_dlmalloc_stats` |
| `sys/user/tauon-basic/src/bin/free/dlmalloc.c` | 62 | symbole-iar | symbole IAR __iar_dlmalloc_usable_size (bibliothèque DLIB / éditeur de liens) : équivalent newlib, Lepton ou .ld à choisir | `#define dlmalloc_usable_size   __iar_dlmalloc_usable_size` |
| `sys/user/tauon-basic/src/bin/free/dlmalloc.c` | 63 | symbole-iar | symbole IAR __iar_dlmalloc_footprint (bibliothèque DLIB / éditeur de liens) : équivalent newlib, Lepton ou .ld à choisir | `#define dlmalloc_footprint     __iar_dlmalloc_footprint` |
| `sys/user/tauon-basic/src/bin/free/dlmalloc.c` | 64 | symbole-iar | symbole IAR __iar_dlmalloc_max_footprint (bibliothèque DLIB / éditeur de liens) : équivalent newlib, Lepton ou .ld à choisir | `#define dlmalloc_max_footprint __iar_dlmalloc_max_footprint` |
| `sys/user/tauon-basic/src/bin/free/dlmalloc.c` | 65 | symbole-iar | symbole IAR __iar_dlindependent_calloc (bibliothèque DLIB / éditeur de liens) : équivalent newlib, Lepton ou .ld à choisir | `#define dlindependent_calloc   __iar_dlindependent_calloc` |
| `sys/user/tauon-basic/src/bin/free/dlmalloc.c` | 66 | symbole-iar | symbole IAR __iar_dlindependent_comalloc (bibliothèque DLIB / éditeur de liens) : équivalent newlib, Lepton ou .ld à choisir | `#define dlindependent_comalloc __iar_dlindependent_comalloc` |
| `sys/user/tauon-basic/src/bin/free/free_main.c` | 59 | symbole-iar | symbole IAR __iar_dlmallinfo (bibliothèque DLIB / éditeur de liens) : équivalent newlib, Lepton ou .ld à choisir | `extern struct mallinfo __iar_dlmallinfo();` |
| `sys/user/tauon-basic/src/bin/free/free_main.c` | 81 | symbole-iar | symbole IAR __iar_dlmallinfo (bibliothèque DLIB / éditeur de liens) : équivalent newlib, Lepton ou .ld à choisir | `info = __iar_dlmallinfo();` |
| `sys/user/tauon-basic/src/bin/sdramtest/sdramtest_main.c` | 20 | pragma-iar | section EXT_RAM absente des scripts ld/*.ld (carte, étape 5) | `#define EXT_RAM_REGION      _Pragma("location = \"EXT_RAM\"")` |

## Automatiques (62)

| Fichier | Ligne | Règle | Motif | Détail |
|---|---:|---|---|---|
| `sys/root/src/kernel/core/core-segger/fork.c` | 1 | garde-cible-gelee | copie à l'identique (code gelé) | `legacy/sys/root/src/kernel/core/core-segger/fork.c` |
| `sys/root/src/kernel/core/core-segger/fork.c` | 59 | garde-cible-gelee | branche cible gelée retirée | `defined(WIN32) && defined(LEPTON_CHKESP)` |
| `sys/root/src/kernel/core/interrupt.h` | 1 | garde-cible-gelee | copie à l'identique (code gelé) | `legacy/sys/root/src/kernel/core/interrupt.h` |
| `sys/root/src/kernel/core/interrupt.h` | 255 | garde-cible-gelee | branche cible gelée retirée | `defined(WIN32)` |
| `sys/root/src/kernel/core/interrupt.h` | 260 | garde-cible-gelee | #else toujours pris | `` |
| `sys/root/src/kernel/core/kernel_compiler.h` | 1 | garde-cible-gelee | copie à l'identique (code gelé) | `legacy/sys/root/src/kernel/core/kernel_compiler.h` |
| `sys/root/src/kernel/core/kernel_compiler.h` | 50 | garde-cible-gelee | branche cible gelée retirée | `defined (WIN32)` |
| `sys/root/src/kernel/core/kernel_pthread.h` | 1 | garde-cible-gelee | copie à l'identique (code gelé) | `legacy/sys/root/src/kernel/core/kernel_pthread.h` |
| `sys/root/src/kernel/core/kernel_pthread.h` | 266 | garde-cible-gelee | branche cible gelée retirée | `defined (WIN32) && defined(_DEBUG)` |
| `sys/root/src/kernel/core/kernel_pthread.h` | 268 | garde-cible-gelee | #else toujours pris | `` |
| `sys/root/src/kernel/core/kernelconf.h` | 1 | garde-cible-gelee | copie à l'identique (code gelé) | `legacy/sys/root/src/kernel/core/kernelconf.h` |
| `sys/root/src/kernel/core/kernelconf.h` | 71 | garde-cible-gelee | branche cible gelée retirée | `defined(WIN32)` |
| `sys/root/src/kernel/core/kernelconf.h` | 73 | garde-cible-gelee | #else toujours pris | `` |
| `sys/root/src/kernel/core/kernelconf.h` | 82 | garde-cible-gelee | branche cible gelée retirée | `defined(WIN32)` |
| `sys/root/src/kernel/core/kernelconf.h` | 84 | garde-cible-gelee | #else toujours pris | `` |
| `sys/root/src/kernel/dev/arch/all/flash/dev_ftl/dev_ftl.c` | 1 | garde-cible-gelee | copie à l'identique (code gelé) | `legacy/sys/root/src/kernel/dev/arch/all/flash/dev_ftl/dev_ftl.c` |
| `sys/root/src/kernel/dev/arch/all/flash/dev_ftl/dev_ftl.c` | 52 | garde-cible-gelee | branche cible gelée retirée | `defined(WIN32)` |
| `sys/user/tauon-basic/src/bin/dhrystone/timers.c` | 1 | garde-cible-gelee | copie à l'identique (code gelé) | `legacy/sys/user/tauon-basic/src/bin/dhrystone/timers.c` |
| `sys/user/tauon-basic/src/bin/dhrystone/timers.c` | 435 | garde-cible-gelee | branche cible gelée retirée | `defined(WIN32)` |
| `sys/user/tauon-basic/src/bin/free/dlmalloc.c` | 1 | garde-cible-gelee | copie à l'identique (code gelé) | `legacy/sys/user/tauon-basic/src/bin/free/dlmalloc.c` |
| `sys/user/tauon-basic/src/bin/free/dlmalloc.c` | 585 | garde-cible-gelee | garde toujours vraie levée | `!defined(WIN32)` |
| `sys/user/tauon-basic/src/bin/free/dlmalloc.c` | 594 | garde-cible-gelee | branche cible gelée retirée | `defined(WIN32)` |
| `sys/user/tauon-basic/src/bin/free/dlmalloc.c` | 713 | garde-cible-gelee | condition simplifiée | `(MORECORE_CONTIGUOUS \|\| defined(WIN32)) → MORECORE_CONTIGUOUS` |
| `sys/user/tauon-basic/src/bin/free/dlmalloc.c` | 1384 | garde-cible-gelee | branche cible gelée retirée | `defined(WIN32)` |
| `sys/user/tauon-basic/src/bin/free/dlmalloc.c` | 1446 | garde-cible-gelee | garde toujours vraie levée | `!defined(WIN32)` |
| `sys/user/tauon-basic/src/bin/free/dlmalloc.c` | 1451 | garde-cible-gelee | branche morte (précédente toujours vraie) | `` |
| `sys/user/tauon-basic/src/bin/free/dlmalloc.c` | 1489 | garde-cible-gelee | garde toujours vraie levée | `!defined(WIN32)` |
| `sys/user/tauon-basic/src/bin/free/dlmalloc.c` | 1503 | garde-cible-gelee | branche cible gelée retirée | `defined(WIN32)` |
| `sys/user/tauon-basic/src/bin/free/dlmalloc.c` | 1505 | garde-cible-gelee | #else toujours pris | `` |
| `sys/user/tauon-basic/src/bin/free/dlmalloc.c` | 1581 | garde-cible-gelee | garde toujours vraie levée | `!defined(WIN32)` |
| `sys/user/tauon-basic/src/bin/free/dlmalloc.c` | 1605 | garde-cible-gelee | branche morte (précédente toujours vraie) | `/* WIN32 */` |
| `sys/user/tauon-basic/src/bin/free/dlmalloc.c` | 1645 | garde-cible-gelee | garde toujours vraie levée | `!defined(WIN32)` |
| `sys/user/tauon-basic/src/bin/free/dlmalloc.c` | 1751 | garde-cible-gelee | garde toujours vraie levée | `!defined(WIN32)` |
| `sys/user/tauon-basic/src/bin/free/dlmalloc.c` | 1851 | garde-cible-gelee | branche morte (précédente toujours vraie) | `/* WIN32 */` |
| `sys/user/tauon-basic/src/bin/free/dlmalloc.c` | 1921 | garde-cible-gelee | garde toujours vraie levée | `!defined(WIN32)` |
| `sys/user/tauon-basic/src/bin/free/dlmalloc.c` | 1953 | garde-cible-gelee | branche morte (précédente toujours vraie) | `/* WIN32 */` |
| `sys/user/tauon-basic/src/bin/free/dlmalloc.c` | 2637 | garde-cible-gelee | branche cible gelée retirée | `defined(WIN32)` |
| `sys/user/tauon-basic/src/bin/free/dlmalloc.c` | 2639 | garde-cible-gelee | #else toujours pris | `` |
| `sys/user/tauon-basic/src/bin/free/dlmalloc.c` | 3065 | garde-cible-gelee | garde toujours vraie levée | `!defined(WIN32)` |
| `sys/user/tauon-basic/src/bin/free/dlmalloc.c` | 3068 | garde-cible-gelee | branche morte (précédente toujours vraie) | `/* WIN32 */` |
| `sys/user/tauon-basic/src/bin/free/dlmalloc.c` | 3122 | garde-cible-gelee | branche cible gelée retirée | `defined(WIN32)` |
| `sys/user/tauon-basic/src/bin/free/dlmalloc.c` | 1412 | garde-iar-arm | condition simplifiée | `!defined(__FreeBSD__) && !defined(__OpenBSD__) && !defined(__NetBSD__) &&     !defined(__IAR_SYSTEMS_ICC__) → !defined(__FreeBSD__) && !defined(__OpenBSD__) && !defined(__NetBSD__)` |
| `sys/user/tauon-basic/src/bin/free/dlmalloc.c` | 2917 | garde-iar-arm | branche IAR ARM retirée | `__IAR_SYSTEMS_ICC__` |
| `sys/user/tauon-basic/src/bin/free/dlmalloc.c` | 2919 | garde-iar-arm | #else toujours pris | `` |
| `sys/user/tauon-basic/src/bin/free/dlmalloc.c` | 3293 | garde-iar-arm | branche IAR ARM retirée | `defined(__IAR_SYSTEMS_ICC__) && !_DLIB_FULL_LOCALE_SUPPORT` |
| `sys/user/tauon-basic/src/bin/free/dlmalloc.c` | 3297 | garde-iar-arm | #else toujours pris | `` |
| `sys/user/tauon-basic/src/bin/free/dlmalloc.c` | 3880 | garde-iar-arm | branche IAR ARM retirée | `defined(__IAR_SYSTEMS_ICC__)` |
| `sys/user/tauon-basic/src/bin/free/dlmalloc.c` | 1 | garde-compilateur | copie à l'identique (code gelé) | `legacy/sys/user/tauon-basic/src/bin/free/dlmalloc.c` |
| `sys/user/tauon-basic/src/bin/free/dlmalloc.c` | 609 | garde-compilateur | condition simplifiée | `(defined(__GNUC__) && ((defined(__i386__) \|\| defined(__x86_64__)))) \|\| (defined(_MSC_VER) && _MSC_VER>=1310) → (defined(__i386__) \|\| defined(__x86_64__))` |
| `sys/user/tauon-basic/src/bin/free/dlmalloc.c` | 802 | garde-compilateur | garde toujours vraie levée | `defined(__GNUC__)` |
| `sys/user/tauon-basic/src/bin/free/dlmalloc.c` | 804 | garde-compilateur | branche morte (précédente toujours vraie) | `defined(_MSC_VER)` |
| `sys/user/tauon-basic/src/bin/free/dlmalloc.c` | 809 | garde-compilateur | garde toujours vraie levée | `defined(__GNUC__)` |
| `sys/user/tauon-basic/src/bin/free/dlmalloc.c` | 811 | garde-compilateur | branche morte (précédente toujours vraie) | `defined(_MSC_VER)` |
| `sys/user/tauon-basic/src/bin/free/dlmalloc.c` | 813 | garde-compilateur | branche morte (précédente toujours vraie) | `` |
| `sys/user/tauon-basic/src/bin/free/dlmalloc.c` | 1426 | garde-compilateur | branche compilateur retirée | `defined(_MSC_VER) && _MSC_VER>=1300` |
| `sys/user/tauon-basic/src/bin/free/dlmalloc.c` | 2596 | garde-compilateur | condition simplifiée | `defined(__GNUC__) && (defined(__i386__) \|\| defined(__x86_64__)) → (defined(__i386__) \|\| defined(__x86_64__))` |
| `sys/user/tauon-basic/src/bin/free/dlmalloc.c` | 2625 | garde-compilateur | branche compilateur retirée | `defined(_MSC_VER) && _MSC_VER>=1300` |
| `sys/user/tauon-basic/src/bin/free/dlmalloc.c` | 2700 | garde-compilateur | condition simplifiée | `defined(__GNUC__) && (defined(__i386__) \|\| defined(__x86_64__)) → (defined(__i386__) \|\| defined(__x86_64__))` |
| `sys/user/tauon-basic/src/bin/free/dlmalloc.c` | 2716 | garde-compilateur | branche compilateur retirée | `defined(_MSC_VER) && _MSC_VER>=1300` |
| `sys/user/tauon-basic/src/bin/free/dlmalloc.c` | 2797 | garde-compilateur | condition simplifiée | `defined(__GNUC__) && __GNUC__ >= 3 → __GNUC__ >= 3` |
| `sys/user/tauon-basic/src/bin/sdramtest/sdramtest_main.c` | 20 | pragma-iar | _Pragma location → __lepton_section | `EXT_RAM` |
| `sys/user/tauon-basic/src/bin/sdramtest/sdramtest_main.c` | 4 | pragma-iar | inclusion de compiler.h ajoutée | `#include "kernel/core/compiler.h"` |

## Code tiers non modifié — justifiés (56)

Décision D1a (2026-10-01) : le code vendored (CMSIS, HAL/driverlib ST, FatFs, yaffs…) n'est pas transformé ; ses branches IAR sont inactives sous GCC (en-têtes multi-compilateurs du fournisseur). Source : `audit-iar.csv`.

| Fichier | Ligne | Catégorie | Motif |
|---|---:|---|---|
| `sys/root/src/kernel/core/ucore/cmsis/CMSIS/Include/arm_math.h` | 380 | garde_iar | __ICCARM__ |
| `sys/root/src/kernel/core/ucore/cmsis/CMSIS/Include/arm_math.h` | 488 | garde_iar | __ICCARM__ |
| `sys/root/src/kernel/core/ucore/cmsis/CMSIS/Include/arm_math.h` | 7283 | garde_iar | __ICCARM__ |
| `sys/root/src/kernel/core/ucore/cmsis/CMSIS/Include/arm_math.h` | 7298 | pragma | _Pragma optimize |
| `sys/root/src/kernel/core/ucore/cmsis/CMSIS/Include/arm_math.h` | 7305 | pragma | _Pragma optimize |
| `sys/root/src/kernel/core/ucore/cmsis/CMSIS/Include/core_cm0.h` | 38 | garde_iar | __ICCARM__ |
| `sys/root/src/kernel/core/ucore/cmsis/CMSIS/Include/core_cm0.h` | 39 | pragma | #pragma system_include |
| `sys/root/src/kernel/core/ucore/cmsis/CMSIS/Include/core_cm0.h` | 89 | garde_iar | __ICCARM__ |
| `sys/root/src/kernel/core/ucore/cmsis/CMSIS/Include/core_cm0.h` | 125 | garde_iar | __ICCARM__ |
| `sys/root/src/kernel/core/ucore/cmsis/CMSIS/Include/core_cm0plus.h` | 38 | garde_iar | __ICCARM__ |
| `sys/root/src/kernel/core/ucore/cmsis/CMSIS/Include/core_cm0plus.h` | 39 | pragma | #pragma system_include |
| `sys/root/src/kernel/core/ucore/cmsis/CMSIS/Include/core_cm0plus.h` | 89 | garde_iar | __ICCARM__ |
| `sys/root/src/kernel/core/ucore/cmsis/CMSIS/Include/core_cm0plus.h` | 125 | garde_iar | __ICCARM__ |
| `sys/root/src/kernel/core/ucore/cmsis/CMSIS/Include/core_cm3.h` | 38 | garde_iar | __ICCARM__ |
| `sys/root/src/kernel/core/ucore/cmsis/CMSIS/Include/core_cm3.h` | 39 | pragma | #pragma system_include |
| `sys/root/src/kernel/core/ucore/cmsis/CMSIS/Include/core_cm3.h` | 89 | garde_iar | __ICCARM__ |
| `sys/root/src/kernel/core/ucore/cmsis/CMSIS/Include/core_cm3.h` | 125 | garde_iar | __ICCARM__ |
| `sys/root/src/kernel/core/ucore/cmsis/CMSIS/Include/core_cm4.h` | 38 | garde_iar | __ICCARM__ |
| `sys/root/src/kernel/core/ucore/cmsis/CMSIS/Include/core_cm4.h` | 39 | pragma | #pragma system_include |
| `sys/root/src/kernel/core/ucore/cmsis/CMSIS/Include/core_cm4.h` | 89 | garde_iar | __ICCARM__ |
| `sys/root/src/kernel/core/ucore/cmsis/CMSIS/Include/core_cm4.h` | 137 | garde_iar | __ICCARM__ |
| `sys/root/src/kernel/core/ucore/cmsis/CMSIS/Include/core_cm4_simd.h` | 38 | garde_iar | __ICCARM__ |
| `sys/root/src/kernel/core/ucore/cmsis/CMSIS/Include/core_cm4_simd.h` | 39 | pragma | #pragma system_include |
| `sys/root/src/kernel/core/ucore/cmsis/CMSIS/Include/core_cm4_simd.h` | 669 | garde_iar | __ICCARM__ |
| `sys/root/src/kernel/core/ucore/cmsis/CMSIS/Include/core_cm4_simd.h` | 671 | header | cmsis_iar.h |
| `sys/root/src/kernel/core/ucore/cmsis/CMSIS/Include/core_cmFunc.h` | 610 | garde_iar | __ICCARM__ |
| `sys/root/src/kernel/core/ucore/cmsis/CMSIS/Include/core_cmFunc.h` | 612 | header | cmsis_iar.h |
| `sys/root/src/kernel/core/ucore/cmsis/CMSIS/Include/core_cmInstr.h` | 660 | garde_iar | __ICCARM__ |
| `sys/root/src/kernel/core/ucore/cmsis/CMSIS/Include/core_cmInstr.h` | 662 | header | cmsis_iar.h |
| `sys/root/src/kernel/core/ucore/cmsis/CMSIS/Include/core_sc000.h` | 38 | garde_iar | __ICCARM__ |
| `sys/root/src/kernel/core/ucore/cmsis/CMSIS/Include/core_sc000.h` | 39 | pragma | #pragma system_include |
| `sys/root/src/kernel/core/ucore/cmsis/CMSIS/Include/core_sc000.h` | 89 | garde_iar | __ICCARM__ |
| `sys/root/src/kernel/core/ucore/cmsis/CMSIS/Include/core_sc000.h` | 125 | garde_iar | __ICCARM__ |
| `sys/root/src/kernel/core/ucore/cmsis/CMSIS/Include/core_sc300.h` | 38 | garde_iar | __ICCARM__ |
| `sys/root/src/kernel/core/ucore/cmsis/CMSIS/Include/core_sc300.h` | 39 | pragma | #pragma system_include |
| `sys/root/src/kernel/core/ucore/cmsis/CMSIS/Include/core_sc300.h` | 89 | garde_iar | __ICCARM__ |
| `sys/root/src/kernel/core/ucore/cmsis/CMSIS/Include/core_sc300.h` | 125 | garde_iar | __ICCARM__ |
| `sys/root/src/kernel/core/ucore/cmsis/Device/st/stm32f4xx/startup/iar/startup_stm32f4xx.s` | 1 | asm_iar | IAR — démarrage, vecteurs |
| `sys/root/src/kernel/dev/arch/cmsis/dev_cmsis_itm/dev_cmsis_itm.h` | 32 | garde_iar | __ICCARM__ |
| `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/inc/stm32f4xx_hal_def.h` | 152 | garde_iar | __ICCARM__ |
| `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/inc/stm32f4xx_hal_def.h` | 174 | garde_iar | __ICCARM__ |
| `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/inc/stm32f4xx_hal_def.h` | 179 | mot_cle | __ramfunc |
| `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/inc/stm32f4xx_hal_def.h` | 200 | garde_iar | __ICCARM__ |
| `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/inc/stm32f4xx_hal_def.h` | 204 | pragma | _Pragma optimize |
| `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/driverlib/stm32f4x7_eth.c` | 76 | garde_iar | __ICCARM__ |
| `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/driverlib/stm32f4x7_eth.c` | 77 | pragma | #pragma data_alignment |
| `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/driverlib/stm32f4x7_eth.c` | 79 | pragma | #pragma data_alignment |
| `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/driverlib/stm32f4x7_eth.c` | 81 | pragma | #pragma data_alignment |
| `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/driverlib/stm32f4x7_eth.c` | 83 | pragma | #pragma data_alignment |
| `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/driverlib/stm32f4xx_hal_def.h` | 152 | garde_iar | __ICCARM__ |
| `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/driverlib/stm32f4xx_hal_def.h` | 174 | garde_iar | __ICCARM__ |
| `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/driverlib/stm32f4xx_hal_def.h` | 179 | mot_cle | __ramfunc |
| `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/driverlib/stm32f4xx_hal_def.h` | 200 | garde_iar | __ICCARM__ |
| `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/driverlib/stm32f4xx_hal_def.h` | 204 | pragma | _Pragma optimize |
| `sys/root/src/kernel/fs/fatfs/core/diskio.c` | 158 | mot_cle | __weak |
| `sys/root/src/kernel/fs/yaffs/core/yportenv.h` | 25 | garde_iar | __compiler_iar_arm__ |

