# Audit des IAR-ismes — étape 1, tâche 3

Généré par `tools/migration/audit_iar.py` le 2026-09-30 — ne pas éditer à la main.  
Commande : `python3 tools/migration/audit_iar.py` ; trunk lu : `/home/lepton-user/lepton/trunk`.  
Détail ligne à ligne : `doc/migration/audit-iar.csv`.

**Ventilation : perimetre.csv (doc/migration/perimetre.csv) + repli heuristique pour les fichiers non couverts.**

## Métrique

> **Total IAR-ismes périmètre actif (sévérité `iar`) : 148** — code Lepton : 90, code tiers vendored dans l'arbre (CMSIS, HAL ST, embOS IAR, lwIP…) : 58.

Métrique décroissante des étapes 3-4 (`--summary` pour la seule relever). Sévérités : `iar` = à traiter (comptée) ; `autre` = Keil/MSVC, à retirer ; `a_verifier` = portable ou toléré par GCC (CMSIS, `#pragma pack/weak`, garde `__GNUC__`, définition de compatibilité) ; `info` = inventaire (projets IAR, asm GNU). Ensemble `hors_projet` (perimetre.csv) : fichier déclaré par aucun projet ni inclus, hors métrique.

## Totaux sévérité × ensemble (occurrences)

| sévérité | actif | differe | gele | hors_projet | total |
|---|---:|---:|---:|---:|---:|
| iar | 148 | 935 | 578 | 224 | 1885 |
| autre | 165 | 65 | 620 | 414 | 1264 |
| a_verifier | 1518 | 733 | 555 | 195 | 3001 |
| info | 43 | 74 | 51 | 14 | 182 |

## Catégorie × ensemble (sévérité `iar`)

| catégorie | actif | differe | gele | hors_projet | total |
|---|---:|---:|---:|---:|---:|
| garde_iar | 74 | 706 | 103 | 163 | 1046 |
| symbole_iar | 20 | 14 | 203 | 5 | 242 |
| pragma | 25 | 71 | 51 | 27 | 174 |
| mot_cle | 12 | 86 | 51 | 0 | 149 |
| intrinsic | 0 | 26 | 87 | 2 | 115 |
| header | 12 | 14 | 66 | 1 | 93 |
| asm_iar | 1 | 15 | 12 | 25 | 53 |
| modele_mklepton | 4 | 0 | 4 | 0 | 8 |
| placement_@ | 0 | 3 | 1 | 1 | 5 |

## Catégorie × ensemble (toutes sévérités hors `iar`)

| catégorie | sévérité | actif | differe | gele | hors_projet |
|---|---|---:|---:|---:|---:|
| abstraction_lepton | a_verifier | 3 | 0 | 0 | 0 |
| asm_inconnu | a_verifier | 0 | 0 | 3 | 0 |
| asm_multi | a_verifier | 0 | 0 | 4 | 0 |
| garde_compilateur | a_verifier | 22 | 1 | 6 | 0 |
| garde_gcc | a_verifier | 104 | 52 | 63 | 51 |
| intrinsic_cmsis | a_verifier | 895 | 241 | 374 | 32 |
| mot_cle | a_verifier | 284 | 265 | 0 | 32 |
| pragma | a_verifier | 66 | 121 | 65 | 80 |
| symbole_dlib_io | a_verifier | 0 | 2 | 6 | 0 |
| xml_mklepton | a_verifier | 144 | 51 | 34 | 0 |
| asm_armcc | autre | 0 | 2 | 1 | 16 |
| garde_autre | autre | 150 | 59 | 173 | 186 |
| pragma_autre | autre | 15 | 4 | 446 | 212 |
| asm_gnu | info | 0 | 2 | 3 | 14 |
| fichier_iar | info | 43 | 72 | 48 | 0 |

## Motifs du périmètre actif (sévérité `iar`, top 40)

| catégorie | motif | occurrences |
|---|---|---:|
| garde_iar | `__ICCARM__` | 41 |
| garde_iar | `__compiler_iar_arm__` | 17 |
| mot_cle | `__packed` | 9 |
| garde_iar | `__IAR_SYSTEMS_ICC__` | 8 |
| pragma | `#pragma data_alignment` | 8 |
| pragma | `#pragma system_include` | 7 |
| garde_iar | `__compiler_iar_m16c__` | 5 |
| pragma | `_Pragma optimize` | 4 |
| modele_mklepton | `#pragma memory` | 4 |
| garde_iar | `__IAR_SYSTEMS_ICC` | 3 |
| header | `cmsis_iar.h` | 3 |
| symbole_iar | `__iar_dlmallinfo` | 3 |
| header | `ioat91m55800.h` | 2 |
| header | `ioat91sam7x256.h` | 2 |
| header | `atmel/ioat91sam9261.h` | 2 |
| pragma | `_Pragma location` | 2 |
| mot_cle | `__ramfunc` | 2 |
| header | `yvals.h` | 2 |
| pragma | `_Pragma section` | 1 |
| asm_iar | `IAR — démarrage, vecteurs` | 1 |
| mot_cle | `__weak` | 1 |
| symbole_iar | `__iar_Locksyslock` | 1 |
| symbole_iar | `__iar_Unlocksyslock` | 1 |
| symbole_iar | `__iar_dlcalloc` | 1 |
| symbole_iar | `__iar_dlfree` | 1 |
| symbole_iar | `__iar_dlmalloc` | 1 |
| symbole_iar | `__iar_dlmemalign` | 1 |
| symbole_iar | `__iar_dlrealloc` | 1 |
| symbole_iar | `__iar_dlvalloc` | 1 |
| symbole_iar | `__iar_dlpvalloc` | 1 |
| symbole_iar | `__iar_dlmallopt` | 1 |
| symbole_iar | `__iar_dlmalloc_trim` | 1 |
| symbole_iar | `__iar_dlmalloc_stats` | 1 |
| symbole_iar | `__iar_dlmalloc_usable_size` | 1 |
| symbole_iar | `__iar_dlmalloc_footprint` | 1 |
| symbole_iar | `__iar_dlmalloc_max_footprint` | 1 |
| symbole_iar | `__iar_dlindependent_calloc` | 1 |
| symbole_iar | `__iar_dlindependent_comalloc` | 1 |
| pragma | `#pragma diag_suppress` | 1 |
| header | `ysizet.h` | 1 |

## Répertoires du périmètre actif (sévérité `iar`, top 30)

| répertoire | origine | occurrences |
|---|---|---:|
| `sys/root/src/kernel/core/ucore/cmsis/CMSIS` | tiers | 37 |
| `sys/user/tauon-basic/src/bin/free` | lepton | 31 |
| `sys/root/src/kernel/core` | lepton | 30 |
| `sys/root/src/kernel/dev/arch/cortexm/stm32f4xx` | tiers | 30 |
| `sys/root/src/kernel/core/core-segger` | lepton | 7 |
| `tools/mklepton/src` | lepton | 4 |
| `sys/root/src/kernel/fs/fatfs` | tiers | 2 |
| `sys/root/src/lib/libc/stdio` | lepton | 2 |
| `sys/root/src/kernel/core/ucore/cmsis/Device` | tiers | 1 |
| `sys/root/src/kernel/dev/arch/cmsis/dev_cmsis_itm` | tiers | 1 |
| `sys/root/src/kernel/fs/fatfs/core` | tiers | 1 |
| `sys/root/src/kernel/fs/yaffs/core` | tiers | 1 |
| `sys/user/tauon-basic/src/bin/sdramtest` | lepton | 1 |

## Répertoires différé / gelé (sévérité `iar`, top 15 chacun)

**differe**

| répertoire | occurrences |
|---|---:|
| `sys/root/src/kernel/core/ucore/cmsis/Device` | 345 |
| `sys/root/src/kernel/dev/arch/at91/softpack-lib` | 344 |
| `sys/root/src/kernel/core/ucore/freeRTOS_9-0-0/source` | 61 |
| `sys/root/src/kernel/core/ucore/freeRTOS_8-0-0/source` | 60 |
| `sys/root/src/kernel/usb/stm32f4-usb-core/stm32_usb_device_library/Class` | 28 |
| `sys/root/src/kernel/dev/arch/cortexm/stm32f1xx` | 25 |
| `sys/root/src/kernel/dev/arch/at91/asf` | 24 |
| `sys/root/src/kernel/core/ucore/cmsis-5/Device` | 11 |
| `sys/root/src/kernel/dev/arch/cortexm/stellaris` | 11 |
| `sys/root/src/kernel/core/core-freertos` | 8 |
| `sys/root/src/kernel/usb/stm32f4-usb-core/core` | 8 |
| `sys/root/src/kernel/dev/arch/cortexm/stm32wlxx` | 6 |
| `sys/root/src/kernel/core/usb/stm32_usb_core` | 2 |
| `sys/root/src/kernel/dev/bsp/samd20xplained_pro/dev_samd20xplained_pro_board` | 1 |
| `sys/root/src/kernel/usb/stm32f4-usb-core/stm32_usb_device_library/Core` | 1 |

**gele**

| répertoire | occurrences |
|---|---:|
| `sys/root/src/kernel/core/ucore/embOSCXM4_518/arch` | 67 |
| `sys/root/src/kernel/core/ucore/embOSCXM3_384/arch` | 62 |
| `sys/root/src/kernel/core/ucore/embOSARM7-9_388/arch` | 48 |
| `sys/root/src/kernel/core/ucore/embOSARM7_360/arch` | 48 |
| `sys/root/src/kernel/core/ucore/embOSCXM4_440/arch` | 46 |
| `sys/root/src/kernel/core/ucore/embOSCXM4_386/arch` | 38 |
| `sys/root/src/kernel/core/ucore/embOSCXM7_430/Inc` | 38 |
| `sys/root/src/kernel/core/ucore/embOSCXM4_440/Inc` | 37 |
| `sys/root/src/kernel/core/ucore/embOSCXM7_430/arch` | 35 |
| `sys/root/src/kernel/core/ucore/embOSCXM4_518/src` | 27 |
| `sys/root/src/kernel/dev/arch/at91/at91lib` | 27 |
| `sys/root/src/kernel/dev/arch/arm9/at91sam9261` | 25 |
| `sys/root/src/kernel/core/ucore/embOSCXM4_518/inc` | 19 |
| `sys/root/src/kernel/core/ucore/embOSARM7-9_388/inc` | 16 |
| `sys/root/src/kernel/core/ucore/embOSCXM3_384/inc` | 9 |

## Fichiers assembleur (98)

Syntaxe déduite des directives (IAR : `MODULE`/`RSEG`/`SECTION x:CODE`/`DC32`/`PUBWEAK` ; ARMASM : `AREA`/`PRESERVE8`/`DCD` ; GNU : `.section`/`.global`/…) ; rôle déduit du nom et des symboles.

| syntaxe | actif | differe | gele | hors_projet |
|---|---:|---:|---:|---:|
| ARMASM (Keil) | 0 | 2 | 1 | 16 |
| GNU | 0 | 2 | 3 | 14 |
| IAR | 1 | 15 | 12 | 25 |
| indéterminée | 0 | 0 | 3 | 0 |
| multi-assembleur (gardes) | 0 | 0 | 4 | 0 |

| fichier | syntaxe | rôle | ensemble | origine |
|---|---|---|---|---|
| `sys/root/src/kernel/core/ucore/cmsis/Device/st/stm32f4xx/startup/iar/startup_stm32f4xx.s` | IAR | démarrage, vecteurs | actif | tiers |
| `sys/root/src/kernel/core/ucore/cmsis/Device/ARM/ARMCM3/Source/Templates/IAR/startup_ARMCM3.s` | IAR | démarrage, vecteurs | differe | tiers |
| `sys/root/src/kernel/core/ucore/cmsis/Device/st/stm32f2xx/startup/iar/startup_stm32f2xx.s` | IAR | démarrage, vecteurs | differe | tiers |
| `sys/root/src/kernel/core/ucore/cmsis/Device/st/stm32wlxx/startup/iar/startup_stm32wl55xx_cm4.s` | IAR | démarrage, vecteurs | differe | tiers |
| `sys/root/src/kernel/core/ucore/freeRTOS_8-0-0/source/arch/arm9/at91sam9261/board_cstartup.S` | GNU | démarrage, init. horloges/mémoire | differe | tiers |
| `sys/root/src/kernel/core/ucore/freeRTOS_8-0-0/source/arch/arm9/at91sam9261/board_cstartup_iar.s` | IAR | démarrage, init. horloges/mémoire | differe | tiers |
| `sys/root/src/kernel/core/ucore/freeRTOS_8-0-0/source/arch/arm9/at91sam9261/board_cstartup_iar_sam9xe.s` | IAR | démarrage, init. horloges/mémoire | differe | tiers |
| `sys/root/src/kernel/core/ucore/freeRTOS_8-0-0/source/arch/arm9/at91sam9261/board_cstartup_keil.s` | ARMASM (Keil) | démarrage, vecteurs, init. horloges/mémoire | differe | tiers |
| `sys/root/src/kernel/core/ucore/freeRTOS_8-0-0/source/portable/IAR/ARM_CM0/portasm.s` | IAR | commutation de contexte | differe | tiers |
| `sys/root/src/kernel/core/ucore/freeRTOS_8-0-0/source/portable/IAR/ARM_CM3/portasm.s` | IAR | commutation de contexte | differe | tiers |
| `sys/root/src/kernel/core/ucore/freeRTOS_8-0-0/source/portable/IAR/ARM_CM4F/portasm.s` | IAR | commutation de contexte | differe | tiers |
| `sys/root/src/kernel/core/ucore/freeRTOS_8-0-0/source/portable/IAR/AtmelSAM9XE/portasm.s79` | IAR | commutation de contexte | differe | tiers |
| `sys/root/src/kernel/core/ucore/freeRTOS_9-0-0/source/arch/arm9/at91sam9261/board_cstartup.S` | GNU | démarrage, init. horloges/mémoire | differe | tiers |
| `sys/root/src/kernel/core/ucore/freeRTOS_9-0-0/source/arch/arm9/at91sam9261/board_cstartup_iar.s` | IAR | démarrage, init. horloges/mémoire | differe | tiers |
| `sys/root/src/kernel/core/ucore/freeRTOS_9-0-0/source/arch/arm9/at91sam9261/board_cstartup_iar_sam9xe.s` | IAR | démarrage, init. horloges/mémoire | differe | tiers |
| `sys/root/src/kernel/core/ucore/freeRTOS_9-0-0/source/arch/arm9/at91sam9261/board_cstartup_keil.s` | ARMASM (Keil) | démarrage, vecteurs, init. horloges/mémoire | differe | tiers |
| `sys/root/src/kernel/core/ucore/freeRTOS_9-0-0/source/portable/IAR/ARM_CM0/portasm.s` | IAR | commutation de contexte | differe | tiers |
| `sys/root/src/kernel/core/ucore/freeRTOS_9-0-0/source/portable/IAR/ARM_CM3/portasm.s` | IAR | commutation de contexte | differe | tiers |
| `sys/root/src/kernel/core/ucore/freeRTOS_9-0-0/source/portable/IAR/ARM_CM4F/portasm.s` | IAR | commutation de contexte | differe | tiers |
| `sys/root/src/kernel/core/ucore/freeRTOS_9-0-0/source/portable/IAR/ARM_CM7/r0p1/portasm.s` | IAR | commutation de contexte | differe | tiers |
| `sys/root/src/kernel/core/ucore/embOSARM7-9_388/arch/cpu_at91sam9261/AT91SAM9261_Startup.s` | IAR | démarrage, vecteurs, init. horloges/mémoire | gele | tiers |
| `sys/root/src/kernel/core/ucore/embOSARM7_360/arch/cpu_at91m55800/AT91M55_CStartup_V4.s79` | IAR | démarrage, init. horloges/mémoire | gele | tiers |
| `sys/root/src/kernel/core/ucore/embOSARM7_360/arch/cpu_at91m55800/AT91M55_CStartup_V4_bootloader.s79` | IAR | démarrage, bootloader, init. horloges/mémoire | gele | tiers |
| `sys/root/src/kernel/core/ucore/embOSARM7_360/arch/cpu_at91m55800/AT91M55_CStartup_V4_isit.s79` | IAR | démarrage, init. horloges/mémoire | gele | tiers |
| `sys/root/src/kernel/core/ucore/embOSARM7_360/arch/cpu_at91sam7se512/AT91SAM7S_Cstartup_V4.s79` | IAR | démarrage, init. horloges/mémoire | gele | tiers |
| `sys/root/src/kernel/core/ucore/embOSARM7_360/arch/cpu_at91sam7x/Startup.s79` | IAR | démarrage | gele | tiers |
| `sys/root/src/kernel/core/ucore/embOSARM7_360/arch/cpu_at91sam9261/AT91SAM9261_Cstartup_V4.s79` | IAR | démarrage, init. horloges/mémoire | gele | tiers |
| `sys/root/src/kernel/core/ucore/embOSCXM4_386/arch/cmsis/DeviceSupport/startup_Device.s` | IAR | démarrage, vecteurs | gele | tiers |
| `sys/root/src/kernel/core/ucore/embOSCXM4_440/arch/cmsis/cpu/HardFaultHandler.S` | multi-assembleur (gardes) | HardFault | gele | tiers |
| `sys/root/src/kernel/core/ucore/embOSCXM4_518/arch/cmsis/cpu/HardFaultHandler.S` | multi-assembleur (gardes) | HardFault | gele | tiers |
| `sys/root/src/kernel/core/ucore/embOSCXM4_518/arch/cmsis/cpu/SEGGER_RTT_ASM_ARMv7M.S` | multi-assembleur (gardes) | démarrage, SEGGER RTT | gele | tiers |
| `sys/root/src/kernel/core/ucore/embOSCXM7_430/arch/cmsis/startup/HardFaultHandler.S` | multi-assembleur (gardes) | HardFault | gele | tiers |
| `sys/root/src/kernel/dev/arch/at91/at91lib/boards/at91sam9261-ek/board_cstartup.S` | GNU | démarrage, init. horloges/mémoire | gele | tiers |
| `sys/root/src/kernel/dev/arch/at91/at91lib/boards/at91sam9261-ek/board_cstartup_iar.s` | IAR | démarrage, init. horloges/mémoire | gele | tiers |
| `sys/root/src/kernel/dev/arch/at91/at91lib/boards/at91sam9261-ek/board_cstartup_iar_sam9xe.s` | IAR | démarrage, init. horloges/mémoire | gele | tiers |
| `sys/root/src/kernel/dev/arch/at91/at91lib/boards/at91sam9261-ek/board_cstartup_keil.s` | ARMASM (Keil) | démarrage, vecteurs, init. horloges/mémoire | gele | tiers |
| `sys/root/src/kernel/dev/arch/at91/at91lib/boards/at91sam9xe-ek/board_cstartup_iar.s` | IAR | démarrage, init. horloges/mémoire | gele | tiers |
| `sys/root/src/kernel/dev/arch/at91/at91lib/boards/at91sam9xe-ek/board_cstartup_iar_sam9xe.s` | IAR | démarrage, init. horloges/mémoire | gele | tiers |
| `sys/root/src/kernel/dev/arch/gnu32/dummy_linux_syscall.S` | GNU | stub d'appels système | gele | lepton |
| `sys/user/tauon_sampleapp/hal/board_atmel_at91sam9261-ek/lib/install/include/cyg/hal/arch.inc` | GNU | include asm | gele | tiers |
| `sys/user/tauon_sampleapp/hal/board_atmel_at91sam9261-ek/lib/install/include/cyg/infra/cyg_type.inc` | indéterminée | include asm | gele | tiers |
| `sys/user/tauon_sampleapp/hal/board_freescale_twrk60n512/lib/install/include/cyg/hal/variant.inc` | indéterminée | include asm | gele | tiers |
| `sys/user/tauon_sampleapp/hal/board_freescale_twrk60n512/lib/install/include/cyg/infra/cyg_type.inc` | indéterminée | include asm | gele | tiers |
| `sys/root/src/kernel/core/ucore/cmsis-5/Device/ARM/ARMCM0/Source/ARM/startup_ARMCM0.s` | ARMASM (Keil) | démarrage, vecteurs | hors_projet | tiers |
| `sys/root/src/kernel/core/ucore/cmsis-5/Device/ARM/ARMCM0/Source/GCC/startup_ARMCM0.S` | GNU | démarrage, vecteurs | hors_projet | tiers |
| `sys/root/src/kernel/core/ucore/cmsis-5/Device/ARM/ARMCM0/Source/IAR/startup_ARMCM0.s` | IAR | démarrage, vecteurs | hors_projet | tiers |
| `sys/root/src/kernel/core/ucore/cmsis-5/Device/ARM/ARMCM0plus/Source/ARM/startup_ARMCM0plus.s` | ARMASM (Keil) | démarrage, vecteurs | hors_projet | tiers |
| `sys/root/src/kernel/core/ucore/cmsis-5/Device/ARM/ARMCM0plus/Source/GCC/startup_ARMCM0plus.S` | GNU | démarrage, vecteurs | hors_projet | tiers |
| `sys/root/src/kernel/core/ucore/cmsis-5/Device/ARM/ARMCM0plus/Source/IAR/startup_ARMCM0plus.s` | IAR | démarrage, vecteurs | hors_projet | tiers |
| `sys/root/src/kernel/core/ucore/cmsis-5/Device/ARM/ARMCM23/Source/ARM/startup_ARMCM23.s` | ARMASM (Keil) | démarrage, vecteurs | hors_projet | tiers |
| `sys/root/src/kernel/core/ucore/cmsis-5/Device/ARM/ARMCM23/Source/GCC/startup_ARMCM23.S` | GNU | démarrage, vecteurs | hors_projet | tiers |
| `sys/root/src/kernel/core/ucore/cmsis-5/Device/ARM/ARMCM23/Source/IAR/startup_ARMCM23.s` | IAR | démarrage, vecteurs | hors_projet | tiers |
| `sys/root/src/kernel/core/ucore/cmsis-5/Device/ARM/ARMCM3/Source/ARM/startup_ARMCM3.s` | ARMASM (Keil) | démarrage, vecteurs | hors_projet | tiers |
| `sys/root/src/kernel/core/ucore/cmsis-5/Device/ARM/ARMCM3/Source/GCC/startup_ARMCM3.S` | GNU | démarrage, vecteurs | hors_projet | tiers |
| `sys/root/src/kernel/core/ucore/cmsis-5/Device/ARM/ARMCM3/Source/IAR/startup_ARMCM3.s` | IAR | démarrage, vecteurs | hors_projet | tiers |
| `sys/root/src/kernel/core/ucore/cmsis-5/Device/ARM/ARMCM33/Source/ARM/startup_ARMCM33.s` | ARMASM (Keil) | démarrage, vecteurs | hors_projet | tiers |
| `sys/root/src/kernel/core/ucore/cmsis-5/Device/ARM/ARMCM33/Source/GCC/startup_ARMCM33.S` | GNU | démarrage, vecteurs | hors_projet | tiers |
| `sys/root/src/kernel/core/ucore/cmsis-5/Device/ARM/ARMCM33/Source/IAR/startup_ARMCM33.s` | IAR | démarrage, vecteurs | hors_projet | tiers |
| `sys/root/src/kernel/core/ucore/cmsis-5/Device/ARM/ARMCM4/Source/ARM/startup_ARMCM4.s` | ARMASM (Keil) | démarrage, vecteurs | hors_projet | tiers |
| `sys/root/src/kernel/core/ucore/cmsis-5/Device/ARM/ARMCM4/Source/GCC/startup_ARMCM4.S` | GNU | démarrage, vecteurs | hors_projet | tiers |
| `sys/root/src/kernel/core/ucore/cmsis-5/Device/ARM/ARMCM4/Source/IAR/startup_ARMCM4.s` | IAR | démarrage, vecteurs | hors_projet | tiers |
| `sys/root/src/kernel/core/ucore/cmsis-5/Device/ARM/ARMCM7/Source/ARM/startup_ARMCM7.s` | ARMASM (Keil) | démarrage, vecteurs | hors_projet | tiers |
| `sys/root/src/kernel/core/ucore/cmsis-5/Device/ARM/ARMCM7/Source/GCC/startup_ARMCM7.S` | GNU | démarrage, vecteurs | hors_projet | tiers |
| `sys/root/src/kernel/core/ucore/cmsis-5/Device/ARM/ARMCM7/Source/IAR/startup_ARMCM7.s` | IAR | démarrage, vecteurs | hors_projet | tiers |
| `sys/root/src/kernel/core/ucore/cmsis-5/Device/ARM/ARMSC000/Source/ARM/startup_ARMSC000.s` | ARMASM (Keil) | démarrage, vecteurs | hors_projet | tiers |
| `sys/root/src/kernel/core/ucore/cmsis-5/Device/ARM/ARMSC000/Source/GCC/startup_ARMSC000.S` | GNU | démarrage, vecteurs | hors_projet | tiers |
| `sys/root/src/kernel/core/ucore/cmsis-5/Device/ARM/ARMSC000/Source/IAR/startup_ARMSC000.s` | IAR | démarrage, vecteurs | hors_projet | tiers |
| `sys/root/src/kernel/core/ucore/cmsis-5/Device/ARM/ARMSC300/Source/ARM/startup_ARMSC300.s` | ARMASM (Keil) | démarrage, vecteurs | hors_projet | tiers |
| `sys/root/src/kernel/core/ucore/cmsis-5/Device/ARM/ARMSC300/Source/GCC/startup_ARMSC300.S` | GNU | démarrage, vecteurs | hors_projet | tiers |
| `sys/root/src/kernel/core/ucore/cmsis-5/Device/ARM/ARMSC300/Source/IAR/startup_ARMSC300.s` | IAR | démarrage, vecteurs | hors_projet | tiers |
| `sys/root/src/kernel/core/ucore/cmsis-5/Device/ARM/ARMv8MBL/Source/ARM/startup_ARMv8MBL.s` | ARMASM (Keil) | démarrage, vecteurs | hors_projet | tiers |
| `sys/root/src/kernel/core/ucore/cmsis-5/Device/ARM/ARMv8MBL/Source/GCC/startup_ARMv8MBL.S` | GNU | démarrage, vecteurs | hors_projet | tiers |
| `sys/root/src/kernel/core/ucore/cmsis-5/Device/ARM/ARMv8MBL/Source/IAR/startup_ARMv8MBL.s` | IAR | démarrage, vecteurs | hors_projet | tiers |
| `sys/root/src/kernel/core/ucore/cmsis-5/Device/ARM/ARMv8MML/Source/ARM/startup_ARMv8MML.s` | ARMASM (Keil) | démarrage, vecteurs | hors_projet | tiers |
| `sys/root/src/kernel/core/ucore/cmsis-5/Device/ARM/ARMv8MML/Source/GCC/startup_ARMv8MML.S` | GNU | démarrage, vecteurs | hors_projet | tiers |
| `sys/root/src/kernel/core/ucore/cmsis-5/Device/ARM/ARMv8MML/Source/IAR/startup_ARMv8MML.s` | IAR | démarrage, vecteurs | hors_projet | tiers |
| `sys/root/src/kernel/core/ucore/cmsis/Device/ARM/ARMCM0/Source/Templates/ARM/startup_ARMCM0.s` | ARMASM (Keil) | démarrage, vecteurs | hors_projet | tiers |
| `sys/root/src/kernel/core/ucore/cmsis/Device/ARM/ARMCM0/Source/Templates/GCC/startup_ARMCM0.s` | GNU | démarrage, vecteurs | hors_projet | tiers |
| `sys/root/src/kernel/core/ucore/cmsis/Device/ARM/ARMCM0/Source/Templates/IAR/startup_ARMCM0.s` | IAR | démarrage, vecteurs | hors_projet | tiers |
| `sys/root/src/kernel/core/ucore/cmsis/Device/ARM/ARMCM3/Source/Templates/ARM/startup_ARMCM3.s` | ARMASM (Keil) | démarrage, vecteurs | hors_projet | tiers |
| `sys/root/src/kernel/core/ucore/cmsis/Device/ARM/ARMCM3/Source/Templates/GCC/startup_ARMCM3.s` | GNU | démarrage, vecteurs | hors_projet | tiers |
| `sys/root/src/kernel/core/ucore/cmsis/Device/ARM/ARMCM4/Source/Templates/ARM/startup_ARMCM4.s` | ARMASM (Keil) | démarrage, vecteurs | hors_projet | tiers |
| `sys/root/src/kernel/core/ucore/cmsis/Device/ARM/ARMCM4/Source/Templates/GCC/startup_ARMCM4.s` | GNU | démarrage, vecteurs | hors_projet | tiers |
| `sys/root/src/kernel/core/ucore/cmsis/Device/ARM/ARMCM4/Source/Templates/IAR/startup_ARMCM4.s` | IAR | démarrage, vecteurs | hors_projet | tiers |
| `sys/root/src/kernel/core/ucore/cmsis/Device/st/stm32f2xx/startup/arm/startup_stm32f2xx.s` | ARMASM (Keil) | démarrage, vecteurs | hors_projet | tiers |
| `sys/root/src/kernel/core/ucore/cmsis/Device/st/stm32f4xx/startup/iar/startup_stm32f401xc.s` | IAR | démarrage, vecteurs | hors_projet | tiers |
| `sys/root/src/kernel/core/ucore/cmsis/Device/st/stm32f4xx/startup/iar/startup_stm32f401xe.s` | IAR | démarrage, vecteurs | hors_projet | tiers |
| `sys/root/src/kernel/core/ucore/cmsis/Device/st/stm32f4xx/startup/iar/startup_stm32f405xx.s` | IAR | démarrage, vecteurs | hors_projet | tiers |
| `sys/root/src/kernel/core/ucore/cmsis/Device/st/stm32f4xx/startup/iar/startup_stm32f407xx.s` | IAR | démarrage, vecteurs | hors_projet | tiers |
| `sys/root/src/kernel/core/ucore/cmsis/Device/st/stm32f4xx/startup/iar/startup_stm32f411xe.s` | IAR | démarrage, vecteurs | hors_projet | tiers |
| `sys/root/src/kernel/core/ucore/cmsis/Device/st/stm32f4xx/startup/iar/startup_stm32f415xx.s` | IAR | démarrage, vecteurs | hors_projet | tiers |
| `sys/root/src/kernel/core/ucore/cmsis/Device/st/stm32f4xx/startup/iar/startup_stm32f417xx.s` | IAR | démarrage, vecteurs | hors_projet | tiers |
| `sys/root/src/kernel/core/ucore/cmsis/Device/st/stm32f4xx/startup/iar/startup_stm32f427xx.s` | IAR | démarrage, vecteurs | hors_projet | tiers |
| `sys/root/src/kernel/core/ucore/cmsis/Device/st/stm32f4xx/startup/iar/startup_stm32f429xx.s` | IAR | démarrage, vecteurs | hors_projet | tiers |
| `sys/root/src/kernel/core/ucore/cmsis/Device/st/stm32f4xx/startup/iar/startup_stm32f437xx.s` | IAR | démarrage, vecteurs | hors_projet | tiers |
| `sys/root/src/kernel/core/ucore/cmsis/Device/st/stm32f4xx/startup/iar/startup_stm32f439xx.s` | IAR | démarrage, vecteurs | hors_projet | tiers |
| `sys/root/src/kernel/core/ucore/cmsis/Device/st/stm32f4xx/startup/iar/startup_stm32f446xx.s` | IAR | démarrage, vecteurs | hors_projet | tiers |
| `sys/root/src/kernel/core/ucore/cmsis/Device/st/stm32f4xx/startup/keil/startup_stm32f4xx.s` | ARMASM (Keil) | démarrage, vecteurs | hors_projet | tiers |

## Fichiers de projet IAR (inventaire, sévérité `info`)

| type | actif | differe | gele | hors_projet |
|---|---:|---:|---:|---:|
| fichier .ewp | 21 | 19 | 6 | 0 |
| fichier .eww | 8 | 5 | 5 | 0 |
| fichier .icf | 9 | 28 | 15 | 0 |
| fichier .mac | 1 | 4 | 12 | 0 |
| fichier .xcl | 4 | 16 | 10 | 0 |

## Points notables

- `__compiler_directive__packed` (abstraction Lepton, `kernel/core/kernelconf.h`) : vaut `__packed` sous IAR/Keil et **rien sous GCC** — les structures concernées perdent l'attribut packed : 3 usages (catégorie `abstraction_lepton`, dont 3 actifs). Point à traiter à l'étape 4 (`compiler.h`).
- Sélection du compilateur par `__tauon_compiler__` (`kernelconf.h`, valeurs `__compiler_iar_arm__`, `__compiler_gnuc__`, `__compiler_keil_arm__`…) : 31 gardes `garde_iar` sur `__compiler_iar_*`, 1015 sur `__ICCARM__`/`__IAR_SYSTEMS_*` (tous ensembles).
- `_Pragma` dans des macros (ex. `CORTEXM4_CCM_RAM` : `section`/`location` pour la CCM du STM32F4) : 21 occurrences détectées via `_Pragma("…")`.
- Modèles émis par mklepton (`tools/mklepton/src`) : 8 IAR-ismes dans les chaînes générées (catégorie `modele_mklepton`, `#pragma memory=constseg` M16C) ; XML `mkconf*` : 229 chemins Windows absolus (`c:/tauon/…`, sévérité `a_verifier`), 0 références IAR.
- Fichiers générés par mklepton présents dans l'arbre (colonne `genere`) : 5 fichiers (`sys/root/src/kernel/core/arch/win32/bin_mkconf.c`, `sys/root/src/kernel/core/arch/win32/dev_dskimg.c`, `sys/root/src/kernel/core/arch/win32/dev_dskimg.h`, `sys/root/src/kernel/core/arch/win32/dev_mkconf.c`, `sys/root/src/kernel/core/arch/win32/kernel_mkconf.h`) ; 0 occurrences toutes sévérités, dont 0 `iar`.
- Code tiers : 58 des 148 IAR-ismes actifs sont dans du code vendored (CMSIS/HAL fournissent déjà des branches GCC ; les copies embOS IAR de `ucore/` sont remplacées par le port GCC Segger, non traduites).

## Limites et faux positifs connus

- Pas d'évaluation du préprocesseur : les blocs `#if 0` et les branches IAR déjà gardées sont comptés (une branche `#if defined(__ICCARM__)` compte la garde et son contenu).
- Commentaires, chaînes et doxygen (`@note`, `@brief`) exclus par analyse lexicale ; `placement_@` = tout `@` restant dans le code C hors directive préprocesseur.
- `__weak`/`__packed`/`__noreturn` sous `cmsis`/`cubemx_hal_driver` classés `a_verifier` (macros CMSIS/HAL portables) ; ailleurs comptés `iar` même si une macro de compatibilité les rend portables.
- `__get_*`/`__set_*`/`__DSB`… (`intrinsic_cmsis`) : fournis par CMSIS pour GCC, `a_verifier` ; `__enable_interrupt`, `__get_interrupt_state`, `__section_begin`… : IAR seul, `iar`.
- Syntaxe asm déduite par comptage de directives : un fichier IAR très court ou multi-assembleur peut être mal classé (voir la colonne `motif`).
- Asm en ligne (`asm("…")`, `__asm`) non audité : syntaxe proche entre IAR et GCC, contraintes d'opérandes à vérifier à la compilation de masse (étape 4).
- Ventilation : 396 occurrences portent sur des fichiers absents de perimetre.csv (XML mkconf, projets IAR, asm `.s79`…) et sont ventilées par l'heuristique de chemin (colonne `ventilation`).

## Fichiers analysés

| genre | actif | differe | gele | hors_projet |
|---|---:|---:|---:|---:|
| c | 1073 | 1493 | 850 | 707 |
| asm | 1 | 19 | 23 | 55 |
| projet | 43 | 72 | 48 | 0 |
| xml | 15 | 9 | 2 | 0 |
