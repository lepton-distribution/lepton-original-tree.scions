# Transformation des IAR-ismes — rapport

Généré par `tools/migration/transform_iar.py` — ne pas éditer à la main.  
Commande : `transform_iar.py --perimetre-actif doc/migration/perimetre.csv --tiers-from-audit doc/migration/audit-iar.csv --report doc/migration/residuel-etape4.md -q`

Résumé : 15 fichier(s) modifié(s), 40 occurrence(s) automatique(s), 34 résiduelle(s).

## Résiduels (34)

| Fichier | Ligne | Règle | Motif | Détail |
|---|---:|---|---|---|
| `sys/root/src/kernel/core/kal.h` | 945 | header-iar | en-tête IAR ioat91m55800.h : équivalent CMSIS/newlib ou branche gelée à traiter à la main | `#include <ioat91m55800.h>` |
| `sys/root/src/kernel/core/kal.h` | 949 | header-iar | en-tête IAR ioat91sam7x256.h : équivalent CMSIS/newlib ou branche gelée à traiter à la main | `#include <ioat91sam7x256.h>` |
| `sys/root/src/kernel/core/kal.h` | 953 | header-iar | en-tête IAR atmel/ioat91sam9261.h : équivalent CMSIS/newlib ou branche gelée à traiter à la main | `#include <atmel/ioat91sam9261.h>` |
| `sys/root/src/kernel/core/kal.h` | 1466 | header-iar | en-tête IAR ioat91m55800.h : équivalent CMSIS/newlib ou branche gelée à traiter à la main | `#include <ioat91m55800.h>` |
| `sys/root/src/kernel/core/kal.h` | 1470 | header-iar | en-tête IAR ioat91sam7x256.h : équivalent CMSIS/newlib ou branche gelée à traiter à la main | `#include <ioat91sam7x256.h>` |
| `sys/root/src/kernel/core/kal.h` | 1474 | header-iar | en-tête IAR atmel/ioat91sam9261.h : équivalent CMSIS/newlib ou branche gelée à traiter à la main | `#include <atmel/ioat91sam9261.h>` |
| `sys/root/src/kernel/core/kernelconf.h` | 277 | garde-iar-arm | branche IAR avec _Pragma/#pragma (placement, étape 5) | `(__tauon_compiler__==__compiler_iar_arm__)` |
| `sys/root/src/kernel/core/kernelconf.h` | 271 | mot-cle-iar | __packed dans une directive (définition de compatibilité ?) | `#define __compiler_directive__packed __packed` |
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

## Automatiques (40)

| Fichier | Ligne | Règle | Motif | Détail |
|---|---:|---|---|---|
| `sys/root/src/bin/test2.c` | 1 | garde-iar-gelee | copie à l'identique (code gelé) | `legacy/sys/root/src/bin/test2.c` |
| `sys/root/src/bin/test2.c` | 1665 | garde-iar-gelee | branche IAR M16C retirée | `defined (__IAR_SYSTEMS_ICC)` |
| `sys/root/src/kernel/core/core-segger/kernel.c` | 1 | garde-iar-gelee | copie à l'identique (code gelé) | `legacy/sys/root/src/kernel/core/core-segger/kernel.c` |
| `sys/root/src/kernel/core/core-segger/kernel.c` | 93 | garde-iar-gelee | branche IAR M16C retirée | `( (__tauon_compiler__==__compiler_iar_m16c__))` |
| `sys/root/src/kernel/core/core-segger/kernel_elfloader.c` | 658 | garde-iar-arm | branche IAR ARM retirée | `(__tauon_compiler__==__compiler_iar_arm__)` |
| `sys/root/src/kernel/core/interrupt.h` | 1 | garde-iar-gelee | copie à l'identique (code gelé) | `legacy/sys/root/src/kernel/core/interrupt.h` |
| `sys/root/src/kernel/core/interrupt.h` | 125 | garde-iar-gelee | branche IAR M16C retirée | `(__tauon_compiler__==__compiler_iar_m16c__)` |
| `sys/root/src/kernel/core/interrupt.h` | 130 | garde-iar-gelee | #else toujours pris | `` |
| `sys/root/src/kernel/core/kal.h` | 1 | garde-iar-gelee | copie à l'identique (code gelé) | `legacy/sys/root/src/kernel/core/kal.h` |
| `sys/root/src/kernel/core/kal.h` | 912 | garde-iar-gelee | branche IAR M16C retirée | `( defined(__IAR_SYSTEMS_ICC) && defined (__KERNEL_UCORE_EMBOS) && defined(CPU_M16C62))` |
| `sys/root/src/kernel/core/kernelconf.h` | 1 | garde-iar-gelee | copie à l'identique (code gelé) | `legacy/sys/root/src/kernel/core/kernelconf.h` |
| `sys/root/src/kernel/core/kernelconf.h` | 75 | garde-iar-gelee | branche IAR M16C retirée | `defined(__IAR_SYSTEMS_ICC)` |
| `sys/root/src/kernel/core/kernelconf.h` | 198 | garde-iar-gelee | branche IAR M16C retirée | `(__tauon_compiler__==__compiler_iar_m16c__)` |
| `sys/root/src/kernel/core/kernelconf.h` | 224 | garde-iar-gelee | branche IAR M16C retirée | `(__tauon_compiler__==__compiler_iar_m16c__)` |
| `sys/root/src/kernel/core/kernelconf.h` | 276 | garde-iar-gelee | branche IAR M16C retirée | `(__tauon_compiler__==__compiler_iar_m16c__)` |
| `sys/root/src/kernel/core/system.h` | 1 | garde-iar-gelee | copie à l'identique (code gelé) | `legacy/sys/root/src/kernel/core/system.h` |
| `sys/root/src/kernel/core/system.h` | 43 | garde-iar-gelee | branche IAR M16C retirée | `( defined(__IAR_SYSTEMS_ICC) && defined (__KERNEL_UCORE_EMBOS) && defined(CPU_M16C62))` |
| `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/dev_stm32f4xx/dev_stm32f4xx_cubemx_eth.c` | 54 | garde-iar-arm | branche IAR retirée (data_alignment doublé par l'alignement GCC) | `defined ( __ICCARM__ ) /*!< IAR Compiler */` |
| `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/dev_stm32f4xx/dev_stm32f4xx_cubemx_eth.c` | 59 | garde-iar-arm | branche IAR retirée (data_alignment doublé par l'alignement GCC) | `defined ( __ICCARM__ ) /*!< IAR Compiler */` |
| `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/dev_stm32f4xx/dev_stm32f4xx_cubemx_eth.c` | 64 | garde-iar-arm | branche IAR retirée (data_alignment doublé par l'alignement GCC) | `defined ( __ICCARM__ ) /*!< IAR Compiler */` |
| `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/dev_stm32f4xx/dev_stm32f4xx_cubemx_eth.c` | 69 | garde-iar-arm | branche IAR retirée (data_alignment doublé par l'alignement GCC) | `defined ( __ICCARM__ ) /*!< IAR Compiler */` |
| `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/gpio.c` | 37 | mot-cle-iar | __packed union → union __lepton_packed | `typedef __packed union` |
| `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/gpio.c` | 40 | mot-cle-iar | __packed struct → struct __lepton_packed | `__packed struct` |
| `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/gpio.c` | 31 | mot-cle-iar | inclusion de compiler.h ajoutée | `#include "kernel/core/compiler.h"` |
| `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/gpio.h` | 73 | mot-cle-iar | __packed struct → struct __lepton_packed | `typedef __packed struct` |
| `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/gpio.h` | 32 | mot-cle-iar | inclusion de compiler.h ajoutée (après la garde) | `#include "kernel/core/compiler.h"` |
| `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/spi.h` | 36 | mot-cle-iar | __packed struct → struct __lepton_packed | `typedef __packed struct` |
| `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/spi.h` | 34 | mot-cle-iar | inclusion de compiler.h ajoutée | `#include "kernel/core/compiler.h"` |
| `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/types.h` | 58 | mot-cle-iar | __packed union → union __lepton_packed | `typedef __packed union` |
| `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/types.h` | 32 | mot-cle-iar | inclusion de compiler.h ajoutée (après la garde) | `#include "kernel/core/compiler.h"` |
| `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/uart.h` | 34 | mot-cle-iar | __packed struct → struct __lepton_packed | `typedef __packed struct` |
| `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/uart.h` | 58 | mot-cle-iar | __packed struct → struct __lepton_packed | `typedef __packed struct` |
| `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/uart.h` | 31 | mot-cle-iar | inclusion de compiler.h ajoutée (après la garde) | `#include "kernel/core/compiler.h"` |
| `sys/user/tauon-basic/src/bin/free/dlmalloc.c` | 1438 | garde-iar-arm | condition simplifiée | `!defined(__FreeBSD__) && !defined(__OpenBSD__) && !defined(__NetBSD__) &&     !defined(__IAR_SYSTEMS_ICC__) → !defined(__FreeBSD__) && !defined(__OpenBSD__) && !defined(__NetBSD__)` |
| `sys/user/tauon-basic/src/bin/free/dlmalloc.c` | 3124 | garde-iar-arm | branche IAR ARM retirée | `__IAR_SYSTEMS_ICC__` |
| `sys/user/tauon-basic/src/bin/free/dlmalloc.c` | 3500 | garde-iar-arm | branche IAR ARM retirée | `defined(__IAR_SYSTEMS_ICC__) && !_DLIB_FULL_LOCALE_SUPPORT` |
| `sys/user/tauon-basic/src/bin/free/dlmalloc.c` | 3504 | garde-iar-arm | #else toujours pris | `` |
| `sys/user/tauon-basic/src/bin/free/dlmalloc.c` | 4087 | garde-iar-arm | branche IAR ARM retirée | `defined(__IAR_SYSTEMS_ICC__)` |
| `sys/user/tauon-basic/src/bin/sdramtest/sdramtest_main.c` | 20 | pragma-iar | _Pragma location → __lepton_section | `EXT_RAM` |
| `sys/user/tauon-basic/src/bin/sdramtest/sdramtest_main.c` | 4 | pragma-iar | inclusion de compiler.h ajoutée | `#include "kernel/core/compiler.h"` |

## Code tiers non modifié — justifiés (58)

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
| `sys/root/src/kernel/fs/fatfs/fatfscore.c` | 70 | garde_iar | __ICCARM__ |
| `sys/root/src/kernel/fs/fatfs/fatfscore.c` | 78 | garde_iar | __ICCARM__ |
| `sys/root/src/kernel/fs/yaffs/core/yportenv.h` | 25 | garde_iar | __compiler_iar_arm__ |

