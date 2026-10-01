# Transformation des IAR-ismes — rapport

Généré par `tools/migration/transform_iar.py` — ne pas éditer à la main.  
Commande : `transform_iar.py --perimetre-actif doc/migration/perimetre.csv --tiers-from-audit doc/migration/audit-iar.csv --report doc/migration/residuel-etape4.md -q`

Résumé : 6 fichier(s) modifié(s), 17 occurrence(s) automatique(s), 0 résiduelle(s).

## Résiduels (0)

| Fichier | Ligne | Règle | Motif | Détail |
|---|---:|---|---|---|

## Automatiques (17)

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

