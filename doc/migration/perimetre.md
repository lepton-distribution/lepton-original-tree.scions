# Périmètre de la migration (étape 1, tâche 2)

Généré par `tools/migration/build_closure.py` le 2026-09-30. Rejouer depuis la racine du clone : `python3 tools/migration/ewp_extract.py && python3 tools/migration/build_closure.py`. Détail par fichier : `perimetre.csv` ; matrice : `matrice-fichiers-projets.csv`.

**Volumétrie — méthode : cloc 2.04 (--by-file --skip-uniqueness, colonne « code ») ; 3 fichier(s) non reconnu(s) par cloc comptés par le repli interne.** Fichiers pris en compte : `.c .h .s .S .asm .s79 .cpp` de tout le trunk (4112 fichiers).

## Chiffres par ensemble

| Ensemble | Fichiers | Lignes de code |
|---|---|---|
| actif | 1074 | 314773 |
| différé | 1511 | 409081 |
| gelé | 765 | 158061 |
| hors-projet | 762 | 192635 |
| **total** | 4112 | 1074550 |

Provenance du classement : aucun = 762, projet = 1496, règle = 1518, répertoire = 336.

## Méthode

1. Chaque configuration des 46 projets reçoit un ensemble (tableau ci-dessous).
2. Fermeture par configuration : fichiers déclarés puis `#include` suivis récursivement avec les includes de la configuration ; **toutes** les directives sont suivies, conditions `#if` ignorées (sur-approximation : un en-tête inclus sous `#ifdef` d'un autre cœur tombe dans l'ensemble de la configuration) ; 41047 résolutions non abouties, cumulées sur toutes les configurations (en-têtes de la bibliothèque IAR, fichiers absents).
3. Un fichier prend, dans l'ordre : la règle de nature (qui prime), sinon le meilleur ensemble des configurations qui le déclarent ou l'incluent (actif > différé > gelé), sinon (en-tête) l'ensemble majoritaire de son répertoire, sinon une règle de répertoire, sinon `hors-projet`.

### Ensembles des configurations

| Projet | Configuration | Ensemble | Justification |
|---|---|---|---|
| `dev_nxp_nfc_pn7150` | cortex-m4-debug | différé | périphérique NFC PN7150 optionnel |
| `dev_nxp_nfc_pn7150` | cortex-m7-debug | différé | périphérique NFC PN7150 optionnel |
| `dev_at91m55800a` | lib-debug | gelé | projet ARM7/ARM9 (cible abandonnée) |
| `dev_at91sam7x` | lib-debug | gelé | projet ARM7/ARM9 (cible abandonnée) |
| `dev_at91sam9261` | lib-debug | gelé | projet ARM7/ARM9 (cible abandonnée) |
| `dev_at91sam9261_7.20` | lib-debug | gelé | projet ARM7/ARM9 (cible abandonnée) |
| `dev_at91samd20_7.20` | Debug | différé | candidat M0+ (étape 6) |
| `dev_at91samd20_7.20` | Release | différé | candidat M0+ (étape 6) |
| `dev_at91samv7x_7.80` | Debug | différé | Cortex-M7 Atmel SAMV71/SAME70, référence M7 (décision 2026-09-30) |
| `dev_at91samv7x_7.80` | Release | différé | Cortex-M7 Atmel SAMV71/SAME70, référence M7 (décision 2026-09-30) |
| `dev_at91samv7x_7.80` | samv71-freertos-debug | différé | Cortex-M7 Atmel SAMV71/SAME70, référence M7 (décision 2026-09-30) |
| `dev_lm3s_6.21` | Debug | différé | candidat M3 (étape 6) |
| `dev_stm32f1xx_6.21` | Debug | différé | candidat M3 (étape 6) |
| `dev_stm32f4xx_6.21` | Debug | différé | génération ancienne ou configuration Release (squelette) |
| `dev_stm32f4xx_7.20` | Debug | actif | pilotes STM32F4 (config. liée par les applications F4) |
| `dev_stm32f4xx_8.40` | Debug | actif | pilotes STM32F4 (config. liée par les applications F4) |
| `dev_stm32wlxx_8.40` | Debug | différé | STM32WL55 Nucleo, carte non listée (à décider) |
| `dev_stm32wlxx_8.40` | Release | différé | STM32WL55 Nucleo, carte non listée (à décider) |
| `dev_stm32wlxx_9.50` | Debug | différé | STM32WL55 Nucleo, carte non listée (à décider) |
| `dev_stm32wlxx_9.50` | Release | différé | STM32WL55 Nucleo, carte non listée (à décider) |
| `Backup of stm32f4_usb_core` | Debug | différé | USB device STM32F4, hors paliers des étapes 3-5 |
| `Backup of stm32f4_usb_core` | Release | différé | USB device STM32F4, hors paliers des étapes 3-5 |
| `stm32f4_usb_core` | Debug | différé | USB device STM32F4, hors paliers des étapes 3-5 |
| `stm32f4_usb_core` | Release | différé | USB device STM32F4, hors paliers des étapes 3-5 |
| `stm32f4_usb_core_8.40` | Debug | différé | USB device STM32F4, hors paliers des étapes 3-5 |
| `stm32f4_usb_core_8.40` | Release | différé | USB device STM32F4, hors paliers des étapes 3-5 |
| `tauon` | lib-debug | gelé | noyau EWARM 4.x, AT91SAM9261 (ARM9) |
| `tauon_6.10` | lib-debug | différé | noyau, génération ancienne ou config. lib-debug (Cortex-M3) |
| `tauon_6.21` | lib-debug | différé | noyau, génération ancienne ou config. lib-debug (Cortex-M3) |
| `tauon_6.21` | tauon-kernel-cortex-m4-debug | différé | noyau, génération ancienne ou config. lib-debug (Cortex-M3) |
| `tauon_6.21` | tauon-kernel-arm926ejs-debug | gelé | noyau, configuration ARM926EJ-S |
| `tauon_6.21` | tauon-kernel-cortex-m3-debug | différé | noyau, configuration d'un autre cœur (étape 6) |
| `tauon_6.21` | tauon-kernel-cortex-m4f-freertos-debug | différé | noyau, configuration FreeRTOS (étape 7) |
| `tauon_6.21` | tauon-kernel-arm926ejs-freertos-debug | gelé | noyau, configuration ARM926EJ-S |
| `tauon_7.20` | lib-debug | différé | noyau, génération ancienne ou config. lib-debug (Cortex-M3) |
| `tauon_7.20` | tauon-kernel-cortex-m4-debug | différé | noyau, génération ancienne ou config. lib-debug (Cortex-M3) |
| `tauon_7.20` | tauon-kernel-arm926ejs-debug | gelé | noyau, configuration ARM926EJ-S |
| `tauon_7.20` | tauon-kernel-cortex-m3-debug | différé | noyau, configuration d'un autre cœur (étape 6) |
| `tauon_7.20` | tauon-kernel-cortex-m4f-freertos-debug | différé | noyau, configuration FreeRTOS (étape 7) |
| `tauon_7.20` | tauon-kernel-arm926ejs-freertos-debug | gelé | noyau, configuration ARM926EJ-S |
| `tauon_7.20` | tauon-kernel-cortex-m0+-freertos-debug | différé | noyau, configuration FreeRTOS (étape 7) |
| `tauon_7.80` | lib-debug | différé | noyau, génération ancienne ou config. lib-debug (Cortex-M3) |
| `tauon_7.80` | tauon-kernel-cortex-m4-debug | actif | noyau, configuration Cortex-M4 embOS (socle) |
| `tauon_7.80` | tauon-kernel-arm926ejs-debug | gelé | noyau, configuration ARM926EJ-S |
| `tauon_7.80` | tauon-kernel-cortex-m3-debug | différé | noyau, configuration d'un autre cœur (étape 6) |
| `tauon_7.80` | tauon-kernel-cortex-m4f-freertos-debug | différé | noyau, configuration FreeRTOS (étape 7) |
| `tauon_7.80` | tauon-kernel-arm926ejs-freertos-debug | gelé | noyau, configuration ARM926EJ-S |
| `tauon_7.80` | tauon-kernel-cortex-m0+-freertos-debug | différé | noyau, configuration FreeRTOS (étape 7) |
| `tauon_7.80` | tauon-kernel-cortex-m4m7-freertosv9-debug | différé | noyau, configuration FreeRTOS (étape 7) |
| `tauon_7.80` | tauon-kernel-cortex-m7-debug | différé | noyau, configuration d'un autre cœur (étape 6) |
| `tauon_8.40` | lib-debug | différé | noyau, génération ancienne ou config. lib-debug (Cortex-M3) |
| `tauon_8.40` | tauon-kernel-cortex-m4-debug | actif | noyau, configuration Cortex-M4 embOS (socle) |
| `tauon_8.40` | tauon-kernel-arm926ejs-debug | gelé | noyau, configuration ARM926EJ-S |
| `tauon_8.40` | tauon-kernel-cortex-m3-debug | différé | noyau, configuration d'un autre cœur (étape 6) |
| `tauon_8.40` | tauon-kernel-cortex-m4f-freertos-debug | différé | noyau, configuration FreeRTOS (étape 7) |
| `tauon_8.40` | tauon-kernel-arm926ejs-freertos-debug | gelé | noyau, configuration ARM926EJ-S |
| `tauon_8.40` | tauon-kernel-cortex-m0+-freertos-debug | différé | noyau, configuration FreeRTOS (étape 7) |
| `tauon_8.40` | tauon-kernel-cortex-m4m7-freertosv9-debug | différé | noyau, configuration FreeRTOS (étape 7) |
| `tauon_8.40` | tauon-kernel-cortex-m7-debug | différé | noyau, configuration d'un autre cœur (étape 6) |
| `tauon_9.50` | lib-debug | différé | noyau, génération ancienne ou config. lib-debug (Cortex-M3) |
| `tauon_9.50` | tauon-kernel-cortex-m4-debug | actif | noyau, configuration Cortex-M4 embOS (socle) |
| `tauon_9.50` | tauon-kernel-arm926ejs-debug | gelé | noyau, configuration ARM926EJ-S |
| `tauon_9.50` | tauon-kernel-cortex-m3-debug | différé | noyau, configuration d'un autre cœur (étape 6) |
| `tauon_9.50` | tauon-kernel-cortex-m4f-freertos-debug | différé | noyau, configuration FreeRTOS (étape 7) |
| `tauon_9.50` | tauon-kernel-arm926ejs-freertos-debug | gelé | noyau, configuration ARM926EJ-S |
| `tauon_9.50` | tauon-kernel-cortex-m0+-freertos-debug | différé | noyau, configuration FreeRTOS (étape 7) |
| `tauon_9.50` | tauon-kernel-cortex-m4m7-freertosv9-debug | différé | noyau, configuration FreeRTOS (étape 7) |
| `tauon_9.50` | tauon-kernel-cortex-m7-debug | différé | noyau, configuration d'un autre cœur (étape 6) |
| `bsp_discovery_f4-baseboard-modem_7.40` | Debug | différé | variante STM32F4 + modem, non retenue |
| `bsp_discovery_f4-baseboard-modem_7.40` | Release | différé | variante STM32F4 + modem, non retenue |
| `bsp_discovery_f4_7.30` | Debug | actif | BSP STM32F4 |
| `bsp_discovery_f4_7.30` | Release | différé | génération ancienne ou configuration Release (squelette) |
| `bsp_olimex_p407_7.30` | Debug | actif | BSP STM32F4 |
| `bsp_olimex_p407_7.30` | Release | différé | génération ancienne ou configuration Release (squelette) |
| `bsp_samd20xplained_pro_7.30` | Debug | différé | candidat M0+ (étape 6) |
| `bsp_samd20xplained_pro_7.30` | Release | différé | candidat M0+ (étape 6) |
| `bsp_same70xplained_7.80` | Debug | différé | Cortex-M7 Atmel SAMV71/SAME70, référence M7 (décision 2026-09-30) |
| `bsp_same70xplained_7.80` | Release | différé | Cortex-M7 Atmel SAMV71/SAME70, référence M7 (décision 2026-09-30) |
| `bsp_same70xplained_7.80` | freertos-debug | différé | Cortex-M7 Atmel SAMV71/SAME70, référence M7 (décision 2026-09-30) |
| `bsp_samv71xplained_ultra_7.80` | Debug | différé | Cortex-M7 Atmel SAMV71/SAME70, référence M7 (décision 2026-09-30) |
| `bsp_samv71xplained_ultra_7.80` | Release | différé | Cortex-M7 Atmel SAMV71/SAME70, référence M7 (décision 2026-09-30) |
| `bsp_samv71xplained_ultra_7.80` | freertos-debug | différé | Cortex-M7 Atmel SAMV71/SAME70, référence M7 (décision 2026-09-30) |
| `bsp-stm32f469i-eval` | Debug | actif | BSP STM32F4 |
| `bsp-stm32f469i-eval` | Release | différé | génération ancienne ou configuration Release (squelette) |
| `bsp_stm32wl55jci_nucleo_8.40` | Debug | différé | STM32WL55 Nucleo, carte non listée (à décider) |
| `bsp_stm32wl55jci_nucleo_8.40` | Release | différé | STM32WL55 Nucleo, carte non listée (à décider) |
| `bsp_stm32wl55jci_nucleo_9.50` | Debug | différé | STM32WL55 Nucleo, carte non listée (à décider) |
| `bsp_stm32wl55jci_nucleo_9.50` | Release | différé | STM32WL55 Nucleo, carte non listée (à décider) |
| `lib-nxpnfc` | cortex-m4-debug | différé | périphérique NFC PN7150 optionnel |
| `lib-nxpnfc` | cortex-m7-debug | différé | périphérique NFC PN7150 optionnel |
| `driverlib` | Debug | différé | candidat M3 (étape 6) |
| `tauon-basic_stm32wl55jci_nucleo_8.40` | Debug | différé | STM32WL55 Nucleo, carte non listée (à décider) |
| `tauon-basic_stm32wl55jci_nucleo_8.40` | Release | différé | STM32WL55 Nucleo, carte non listée (à décider) |
| `tauon-basic_stm32wl55jci_nucleo_8.40` | freertos-debug | différé | STM32WL55 Nucleo, carte non listée (à décider) |
| `tauon-basic_stm32wl55jci_nucleo_9.50` | Debug | différé | STM32WL55 Nucleo, carte non listée (à décider) |
| `tauon-basic_stm32wl55jci_nucleo_9.50` | Release | différé | STM32WL55 Nucleo, carte non listée (à décider) |
| `tauon-basic_stm32wl55jci_nucleo_9.50` | freertos-debug | différé | STM32WL55 Nucleo, carte non listée (à décider) |
| `tauon_at91sam9261` | firmware | gelé | projet ARM7/ARM9 (cible abandonnée) |
| `tauon_at91sam9261` | firmware_injector | gelé | projet ARM7/ARM9 (cible abandonnée) |
| `tauon_at91sam9261` | firmware_flash | gelé | projet ARM7/ARM9 (cible abandonnée) |
| `tauon_at91sam9261` | freertos-firmware_injector | gelé | projet ARM7/ARM9 (cible abandonnée) |
| `tauon_at91sam9261_7.20` | firmware | gelé | projet ARM7/ARM9 (cible abandonnée) |
| `tauon_at91sam9261_7.20` | firmware_injector | gelé | projet ARM7/ARM9 (cible abandonnée) |
| `tauon_at91sam9261_7.20` | firmware_flash | gelé | projet ARM7/ARM9 (cible abandonnée) |
| `tauon_at91sam9261_7.20` | freertos-firmware_injector | gelé | projet ARM7/ARM9 (cible abandonnée) |
| `tauon-basic_at91samd20_7.20` | Debug | différé | candidat M0+ (étape 6) |
| `tauon-basic_at91samd20_7.20` | Release | différé | candidat M0+ (étape 6) |
| `tauon-basic_at91samd20_7.20` | freertos-debug | différé | candidat M0+ (étape 6) |
| `tauon-basic_at91samv71_7.80` | Debug | différé | Cortex-M7 Atmel SAMV71/SAME70, référence M7 (décision 2026-09-30) |
| `tauon-basic_at91samv71_7.80` | Release | différé | Cortex-M7 Atmel SAMV71/SAME70, référence M7 (décision 2026-09-30) |
| `tauon-basic_at91samv71_7.80` | freertos-debug | différé | Cortex-M7 Atmel SAMV71/SAME70, référence M7 (décision 2026-09-30) |
| `tauon-basic_cmsis_6.21` | Debug | différé | candidat M3 (étape 6) |
| `tauon-basic_cmsis_6.21` | Release | différé | candidat M3 (étape 6) |
| `tauon-basic_lm3s_6.21` | Debug | différé | candidat M3 (étape 6) |
| `tauon-basic_lm3s_6.21` | Release | différé | candidat M3 (étape 6) |
| `tauon-basic_stm32f4-olimex_p407_7.20` | Debug | actif | application STM32F4 embOS |
| `tauon-basic_stm32f4-olimex_p407_7.20` | Release | différé | génération ancienne ou configuration Release (squelette) |
| `tauon-basic_stm32f4-olimex_p407_7.20` | freertos-debug | différé | application STM32F4 FreeRTOS (étape 7) |
| `tauon-basic_stm32f407_discovery_7.20` | Debug | actif | application STM32F4 embOS |
| `tauon-basic_stm32f407_discovery_7.20` | Release | différé | génération ancienne ou configuration Release (squelette) |
| `tauon-basic_stm32f407_discovery_7.20` | freertos-debug | différé | application STM32F4 FreeRTOS (étape 7) |
| `tauon-basic_stm32f469i_eval_7.80` | Debug | actif | application STM32F4 embOS |
| `tauon-basic_stm32f469i_eval_7.80` | Release | différé | génération ancienne ou configuration Release (squelette) |
| `tauon-basic_stm32f469i_eval_7.80` | freertos-debug | différé | application STM32F4 FreeRTOS (étape 7) |

### Règles de nature (priment sur les projets)

- `^sys/root/src/kernel/dev/arch/(arm7|arm9)/` → **gelé** : ARM7/ARM9 (cible abandonnée)
- `^sys/root/src/kernel/dev/arch/(gnu32|win32)/` → **gelé** : simulation Linux/Windows
- `^sys/root/src/kernel/core/arch/win32/` → **gelé** : simulation Windows
- `^sys/root/src/kernel/core/(windows|windef|winnt)\.h$` → **gelé** : en-têtes Windows de la simulation
- `^sys/root/prj/vc-2010/` → **gelé** : projet Visual Studio de la simulation
- `^tools/virtual_cpu/` → **gelé** : simulation (virtual_cpu)
- `^tools/host/win32/` → **gelé** : outillage hôte Windows (WinPcap)
- `^tools/host/debian/ecos/` → **gelé** : port eCos (backend core-ecos absent de l'arbre)
- `^tools/mklepton/src/mklepton-w32\.c$` → **gelé** : variante Windows de mklepton
- `^sys/root/src/kernel/net/lwip/ports/m16c/` → **gelé** : M16C (cible abandonnée)
- `^sys/root/src/kernel/core/ucore/(embOSARM7|embOSW32)` → **gelé** : embOS IAR ARM7/ARM9/Win32
- `^sys/root/src/kernel/core/ucore/embOSCX` → **gelé** : embOS IAR Cortex-M (Segger) remplacé par le port GCC (third_party/embos) ; lecture seule (tâche 4)
- `^sys/user/tauon_sampleapp/hal/board_atmel_at91sam9261-ek/` → **gelé** : carte AT91SAM9261-EK (ARM9)
- `^(sys/root/src/kernel/dev/arch/cortexm/k60n512|sys/user/tauon_sampleapp/hal/board_freescale_twrk60n512)/` → **gelé** : Freescale K60 (TWR-K60N512), carte abandonnée, sans projet IAR (proposition)
- `^sys/root/src/kernel/(dev/arch/cortexm/at91samv7x|dev/bsp/(same70xplained|samv71xplained_ultra)|dev/arch/at91/softpack-lib)/` → **différé** : Cortex-M7 Atmel SAMV71/SAME70, référence M7 (décision 2026-09-30)
- `^sys/root/src/kernel/core/(core-freertos|ucore/freeRTOS_)` → **différé** : backend FreeRTOS (étape 7)

Règles de répertoire (fichiers non atteints par un projet) :

- `^sys/root/src/kernel/dev/arch/at91/(at91lib|dev_at91_)` → **gelé** : Atmel AT91 ARM7/ARM9
- `^sys/root/src/kernel/dev/arch/at91/asf/` → **différé** : Atmel ASF SAMD20, candidat M0+ (étape 6)
- `^sys/root/src/kernel/dev/(arch/cortexm/at91samd20|bsp/samd20xplained_pro)/` → **différé** : SAMD20, candidat M0+ (étape 6)
- `^sys/root/src/kernel/dev/arch/cortexm/(stm32f1xx|stellaris)/` → **différé** : candidat M3 (étape 6)
- `^sys/root/src/kernel/dev/(arch/cortexm/stm32wlxx|bsp/stm32wl55jci_nucleo)/` → **différé** : STM32WL55 Nucleo, carte non listée (à décider)
- `^sys/root/src/kernel/dev/bsp/discovery_f4-baseboard-modem/` → **différé** : variante STM32F4 + modem
- `^sys/root/src/(kernel/dev/arch/all/i2c/nxp-nfc|lib/lib-nxpnfc)/` → **différé** : périphérique NFC optionnel
- `^sys/root/src/kernel/(usb/stm32f4-usb-core|core/usb/stm32_usb_core)/` → **différé** : USB device STM32F4
- `^tools/mklepton/src/` → **actif** : mklepton, outil hôte porté à l'étape 2

Fichiers qu'une configuration active ou différée atteint mais qu'une règle de nature classe plus bas (à vérifier : inclusions conditionnelles probables, ou code remplacé) :

- `src/kernel/core` : 3
- `src/kernel/core/arch/win32` : 1
- `src/kernel/core/ucore/embOSCXM3_384` : 17
- `src/kernel/core/ucore/embOSCXM4_386` : 10
- `src/kernel/core/ucore/embOSCXM4_440` : 21
- `src/kernel/core/ucore/embOSCXM4_518` : 23
- `src/kernel/core/ucore/embOSCXM7_430` : 26
- `src/kernel/core/ucore/embOSW32_100` : 3
- `src/kernel/core/ucore/freeRTOS_8-0-0` : 14
- `src/kernel/dev/arch/arm7/dev_at91` : 2
- `src/kernel/dev/arch/gnu32/common` : 1
- `src/kernel/dev/arch/gnu32/dev_linux_screen` : 1

## Volumétrie par répertoire principal

| Répertoire | actif (fich. / lignes) | différé (fich. / lignes) | gelé (fich. / lignes) | hors-projet (fich. / lignes) | total lignes |
|---|---|---|---|---|---|
| `prj/vc-2010` | – | – | 1 / 7 | – | 7 |
| `src/bin` | 13 / 6756 | 1 / 156 | 1 / 396 | 25 / 4036 | 11344 |
| `src/kernel/core` | 163 / 33362 | 247 / 73193 | 196 / 73613 | 298 / 50995 | 231163 |
| `src/kernel/dev` | 349 / 200458 | 1066 / 315748 | 251 / 48825 | 59 / 17266 | 582297 |
| `src/kernel/fs` | 78 / 14910 | – | – | 36 / 39398 | 54308 |
| `src/kernel/net` | 376 / 39924 | 135 / 11202 | 7 / 289 | 315 / 78583 | 129998 |
| `src/kernel/usb` | – | 38 / 6408 | – | – | 6408 |
| `src/lib` | 46 / 6539 | 24 / 2374 | – | 2 / 167 | 9080 |
| `src/sbin` | 35 / 6836 | – | – | 4 / 486 | 7322 |
| `sys/user` | 11 / 4184 | – | 239 / 26615 | 23 / 1704 | 32503 |
| `tools/host` | – | – | 40 / 4660 | – | 4660 |
| `tools/mklepton` | 3 / 1804 | – | 1 / 1539 | – | 3343 |
| `tools/virtual_cpu` | – | – | 29 / 2117 | – | 2117 |

### Détail

| Répertoire | actif (fich. / lignes) | différé (fich. / lignes) | gelé (fich. / lignes) | hors-projet (fich. / lignes) | total lignes |
|---|---|---|---|---|---|
| `prj` | – | – | 1 / 7 | – | 7 |
| `src/bin` | 13 / 6756 | 1 / 156 | 1 / 396 | 25 / 4036 | 11344 |
| `src/kernel/core` | 102 / 8679 | – | 4 / 4769 | 3 / 339 | 13787 |
| `src/kernel/core/arch/win32` | – | – | 5 / 751 | – | 751 |
| `src/kernel/core/core-freertos` | – | 15 / 4075 | – | – | 4075 |
| `src/kernel/core/core-generic` | 1 / 113 | – | – | – | 113 |
| `src/kernel/core/core-segger` | 14 / 4316 | – | – | 1 / 12 | 4328 |
| `src/kernel/core/net` | 27 / 5501 | – | – | – | 5501 |
| `src/kernel/core/ucore/cmsis` | 19 / 14753 | 94 / 40732 | – | 31 / 7508 | 62993 |
| `src/kernel/core/ucore/cmsis-5` | – | 2 / 450 | – | 263 / 43136 | 43586 |
| `src/kernel/core/ucore/embOSARM7-9_388` | – | – | 13 / 3110 | – | 3110 |
| `src/kernel/core/ucore/embOSARM7_360` | – | – | 23 / 7065 | – | 7065 |
| `src/kernel/core/ucore/embOSCXM3_384` | – | – | 19 / 2930 | – | 2930 |
| `src/kernel/core/ucore/embOSCXM4_386` | – | – | 18 / 3625 | – | 3625 |
| `src/kernel/core/ucore/embOSCXM4_440` | – | – | 28 / 6440 | – | 6440 |
| `src/kernel/core/ucore/embOSCXM4_518` | – | – | 38 / 9126 | – | 9126 |
| `src/kernel/core/ucore/embOSCXM7_430` | – | – | 27 / 5731 | – | 5731 |
| `src/kernel/core/ucore/embOSW32_100` | – | – | 21 / 30066 | – | 30066 |
| `src/kernel/core/ucore/freeRTOS_8-0-0` | – | 64 / 12467 | – | – | 12467 |
| `src/kernel/core/ucore/freeRTOS_9-0-0` | – | 66 / 14999 | – | – | 14999 |
| `src/kernel/core/usb/stm32_usb_core` | – | 6 / 470 | – | – | 470 |
| `src/kernel/dev` | 14 / 7616 | – | – | 3 / 565 | 8181 |
| `src/kernel/dev/arch/all/debug` | 2 / 246 | – | – | 1 / 32 | 278 |
| `src/kernel/dev/arch/all/eth` | – | – | 3 / 1026 | 2 / 1240 | 2266 |
| `src/kernel/dev/arch/all/flash` | 14 / 2246 | – | – | 1 / 357 | 2603 |
| `src/kernel/dev/arch/all/i2c` | 17 / 1744 | 2 / 185 | – | 6 / 873 | 2802 |
| `src/kernel/dev/arch/all/lcd` | 5 / 897 | – | – | 2 / 720 | 1617 |
| `src/kernel/dev/arch/all/modem` | 3 / 1489 | – | – | – | 1489 |
| `src/kernel/dev/arch/all/ppp` | 14 / 1992 | – | – | – | 1992 |
| `src/kernel/dev/arch/all/sd` | – | – | – | 3 / 1028 | 1028 |
| `src/kernel/dev/arch/all/sdcard` | – | – | 5 / 804 | – | 804 |
| `src/kernel/dev/arch/all/slip` | 1 / 308 | – | – | – | 308 |
| `src/kernel/dev/arch/all/usb` | – | – | – | 16 / 3244 | 3244 |
| `src/kernel/dev/arch/arm7/at91m55800a` | – | – | 8 / 2386 | – | 2386 |
| `src/kernel/dev/arch/arm7/at91sam7se` | – | – | 22 / 7758 | – | 7758 |
| `src/kernel/dev/arch/arm7/dev_at91` | – | – | 2 / 195 | – | 195 |
| `src/kernel/dev/arch/arm9/at91sam9260` | – | – | 12 / 2063 | – | 2063 |
| `src/kernel/dev/arch/arm9/at91sam9261` | – | – | 29 / 4539 | – | 4539 |
| `src/kernel/dev/arch/at91/asf` | – | 48 / 6570 | – | – | 6570 |
| `src/kernel/dev/arch/at91/at91lib` | – | 13 / 3320 | 65 / 16362 | – | 19682 |
| `src/kernel/dev/arch/at91/dev_at91_mci` | – | – | 2 / 762 | – | 762 |
| `src/kernel/dev/arch/at91/dev_at91_rtt` | – | 2 / 87 | – | – | 87 |
| `src/kernel/dev/arch/at91/dev_at91_usbdp` | – | – | 4 / 1164 | – | 1164 |
| `src/kernel/dev/arch/at91/softpack-lib` | – | 536 / 101466 | – | – | 101466 |
| `src/kernel/dev/arch/cmsis/dev_cmsis_cpu` | 2 / 123 | – | – | – | 123 |
| `src/kernel/dev/arch/cmsis/dev_cmsis_itm` | 3 / 267 | – | – | – | 267 |
| `src/kernel/dev/arch/cortexm/at91samd20` | – | 2 / 291 | – | – | 291 |
| `src/kernel/dev/arch/cortexm/at91samv7x` | – | 13 / 1627 | – | – | 1627 |
| `src/kernel/dev/arch/cortexm/k60n512` | – | – | 24 / 3691 | – | 3691 |
| `src/kernel/dev/arch/cortexm/stellaris` | 2 / 132 | 98 / 91483 | – | – | 91615 |
| `src/kernel/dev/arch/cortexm/stm32f1xx` | – | 79 / 13695 | – | – | 13695 |
| `src/kernel/dev/arch/cortexm/stm32f4xx` | 262 / 182638 | 4 / 1112 | – | 25 / 9207 | 192957 |
| `src/kernel/dev/arch/cortexm/stm32wlxx` | – | 221 / 91128 | – | – | 91128 |
| `src/kernel/dev/arch/gnu32/common` | – | – | 3 / 146 | – | 146 |
| `src/kernel/dev/arch/gnu32/dev_linux_com0` | – | – | 4 / 446 | – | 446 |
| `src/kernel/dev/arch/gnu32/dev_linux_eth` | – | – | 2 / 193 | – | 193 |
| `src/kernel/dev/arch/gnu32/dev_linux_fileflash` | – | – | 2 / 206 | – | 206 |
| `src/kernel/dev/arch/gnu32/dev_linux_filerom` | – | – | 2 / 145 | – | 145 |
| `src/kernel/dev/arch/gnu32/dev_linux_flash` | – | – | 1 / 267 | – | 267 |
| `src/kernel/dev/arch/gnu32/dev_linux_kb` | – | – | 2 / 152 | – | 152 |
| `src/kernel/dev/arch/gnu32/dev_linux_leds` | – | – | 2 / 89 | – | 89 |
| `src/kernel/dev/arch/gnu32/dev_linux_rtc` | – | – | 2 / 135 | – | 135 |
| `src/kernel/dev/arch/gnu32/dev_linux_screen` | – | – | 4 / 444 | – | 444 |
| `src/kernel/dev/arch/gnu32/dev_linux_sdcard` | – | – | 1 / 130 | – | 130 |
| `src/kernel/dev/arch/gnu32/dev_linux_serial_0` | – | – | 2 / 323 | – | 323 |
| `src/kernel/dev/arch/gnu32/dev_linux_serial_1` | – | – | 2 / 323 | – | 323 |
| `src/kernel/dev/arch/gnu32/dev_linux_serial_pt` | – | – | 2 / 316 | – | 316 |
| `src/kernel/dev/arch/gnu32/dummy_linux_syscall.S` | – | – | 1 / 93 | – | 93 |
| `src/kernel/dev/arch/win32/dev-bsp-nu.tube` | – | – | 4 / 180 | – | 180 |
| `src/kernel/dev/arch/win32/dev_win32_board` | – | – | 1 / 287 | – | 287 |
| `src/kernel/dev/arch/win32/dev_win32_com0` | – | – | 3 / 294 | – | 294 |
| `src/kernel/dev/arch/win32/dev_win32_com1` | – | – | 3 / 640 | – | 640 |
| `src/kernel/dev/arch/win32/dev_win32_com2` | – | – | 3 / 543 | – | 543 |
| `src/kernel/dev/arch/win32/dev_win32_eth` | – | – | 3 / 341 | – | 341 |
| `src/kernel/dev/arch/win32/dev_win32_fileflash` | – | – | 4 / 207 | – | 207 |
| `src/kernel/dev/arch/win32/dev_win32_filerom` | – | – | 5 / 316 | – | 316 |
| `src/kernel/dev/arch/win32/dev_win32_flash` | – | – | 4 / 350 | – | 350 |
| `src/kernel/dev/arch/win32/dev_win32_kb` | – | – | 1 / 254 | – | 254 |
| `src/kernel/dev/arch/win32/dev_win32_lcd` | – | – | 1 / 137 | – | 137 |
| `src/kernel/dev/arch/win32/dev_win32_lcd_matrix` | – | – | 1 / 219 | – | 219 |
| `src/kernel/dev/arch/win32/dev_win32_lcd_vga` | – | – | 1 / 242 | – | 242 |
| `src/kernel/dev/arch/win32/dev_win32_rotary_switch` | – | – | 2 / 296 | – | 296 |
| `src/kernel/dev/arch/win32/dev_win32_rtc` | – | – | 3 / 153 | – | 153 |
| `src/kernel/dev/arch/win32/dev_win32_sdcard` | – | – | 4 / 208 | – | 208 |
| `src/kernel/dev/bsp/discovery_f4` | 3 / 240 | – | – | – | 240 |
| `src/kernel/dev/bsp/discovery_f4-baseboard-modem` | – | 22 / 2224 | – | – | 2224 |
| `src/kernel/dev/bsp/olimex_p407` | 4 / 296 | – | – | – | 296 |
| `src/kernel/dev/bsp/samd20xplained_pro` | – | 6 / 1069 | – | – | 1069 |
| `src/kernel/dev/bsp/same70xplained` | – | 3 / 475 | – | – | 475 |
| `src/kernel/dev/bsp/samv71xplained_ultra` | – | 5 / 308 | – | – | 308 |
| `src/kernel/dev/bsp/stm32f469i-eval` | 3 / 224 | 3 / 213 | – | – | 437 |
| `src/kernel/dev/bsp/stm32wl55jci_nucleo` | – | 9 / 495 | – | – | 495 |
| `src/kernel/fs/fat` | 11 / 3404 | – | – | – | 3404 |
| `src/kernel/fs/fatfs` | 18 / 4768 | – | – | 10 / 31018 | 35786 |
| `src/kernel/fs/kofs` | 2 / 516 | – | – | – | 516 |
| `src/kernel/fs/rootfs` | 4 / 536 | – | – | – | 536 |
| `src/kernel/fs/ufs` | 15 / 2286 | – | – | – | 2286 |
| `src/kernel/fs/vfs` | 9 / 2416 | – | – | – | 2416 |
| `src/kernel/fs/yaffs` | 19 / 984 | – | – | 26 / 8380 | 9364 |
| `src/kernel/net/lwip` | 171 / 31732 | – | 7 / 289 | 63 / 30422 | 62443 |
| `src/kernel/net/uip` | 200 / 7123 | – | – | 177 / 34672 | 41795 |
| `src/kernel/net/uip2.5` | 5 / 1069 | 135 / 11202 | – | 75 / 13489 | 25760 |
| `src/kernel/usb/stm32f4-usb-core` | – | 38 / 6408 | – | – | 6408 |
| `src/lib/lib-nxpnfc` | – | 24 / 2374 | – | – | 2374 |
| `src/lib/libc` | 38 / 5825 | – | – | 2 / 167 | 5992 |
| `src/lib/librt` | 4 / 409 | – | – | – | 409 |
| `src/lib/pthread` | 4 / 305 | – | – | – | 305 |
| `src/sbin` | 35 / 6836 | – | – | 4 / 486 | 7322 |
| `sys/user/tauon-basic` | 11 / 4184 | – | 2 / 107 | 8 / 439 | 4730 |
| `sys/user/tauon_sampleapp` | – | – | – | 15 / 1265 | 1265 |
| `sys/user/tauon_sampleapp/hal/board_atmel_at91sam9261-ek` | – | – | 113 / 16846 | – | 16846 |
| `sys/user/tauon_sampleapp/hal/board_freescale_twrk60n512` | – | – | 124 / 9662 | – | 9662 |
| `tools/host/debian` | – | – | 1 / 233 | – | 233 |
| `tools/host/win32` | – | – | 39 / 4427 | – | 4427 |
| `tools/mklepton/src` | 3 / 1804 | – | 1 / 1539 | – | 3343 |
| `tools/virtual_cpu` | – | – | 4 / 511 | – | 511 |
| `tools/virtual_cpu/hardware` | – | – | 24 / 1591 | – | 1591 |
| `tools/virtual_cpu/ui` | – | – | 1 / 15 | – | 15 |

## Liste de sources dérivée : `mps2-an386` (QEMU, Cortex-M4)

Aucun projet IAR. Dérivation : union des sources de la configuration `tauon-kernel-cortex-m4-debug` de `tauon_7.80/8.40/9.50.ewp` (listes identiques à `sbin/read.c` près, présent dans 8.40 seulement) **moins** tout fichier propre à un SoC ou une carte (`dev/arch/cortexm/`, `dev/bsp/`, `ucore/cmsis*/Device/`). Retirés : `src/kernel/dev/arch/cortexm/stellaris/drivers/dev_lm3s_cpu/dev_lm3s_cpu.c`.

- Sources retenues : **217** (74323 lignes) ; ensembles : actif = 217.
- Manquent (à écrire ou à fournir) : pilote UART CMSDK (console `lsh`), pilote Ethernet LAN9118 (aucun pilote `lan9118`/`smsc911x`/CMSDK dans l'arbre, vérifié par recherche), startup + script de liaison GCC et `RTOSInit` (paquet embOS GCC), `board`/`cpu` MPS2 (horloge SysTick), et les sorties mklepton (`kernel/core/arch/cortexm/{kernel_mkconf.h,dev_mkconf.c,bin_mkconf.c,dev_dskimg.[ch]}`).
- HYPOTHÈSE À VALIDER : les pilotes génériques `dev/arch/all/*` et `dev/arch/cmsis/*` de la configuration noyau sont conservés tels quels (ils se compilent sans SoC ; leur instanciation dépend de `dev_mkconf.c` généré).

- `src/bin/` : test2.c
- `src/bin/net/` : telnetd.c
- `src/bin/net/httpc/` : httpc.c
- `src/kernel/core/` : bin.c, cpu.c, dirent.c, env.c, fcntl.c, flock.c, kernel_io.c, kernel_mqueue.c, kernel_printk.c, kernel_ring_buffer.c, lib.c, malloc.c, pipe.c, posix_mqueue.c, select.c, stat.c, statvfs.c, sysctl.c, system.c, systime.c, time.c, timer.c, truncate.c, wait.c
- `src/kernel/core/core-generic/` : kernel_pthread_tsd.c
- `src/kernel/core/core-segger/` : core_rttimer.c, fork.c, kernel.c, kernel_clock.c, kernel_elfloader.c, kernel_object.c, kernel_pthread.c, kernel_pthread_mutex.c, kernel_sem.c, kernel_sigqueue.c, kernel_timer.c, process.c, signal.c, syscall.c
- `src/kernel/core/net/` : kernel_net_core_socket.c
- `src/kernel/core/net/lwip_core/` : ethif_core.c, lwip_core.c, lwip_core_socket.c
- `src/kernel/core/net/modem_core/` : modem_core.c, modem_core_socket.c
- `src/kernel/core/net/uip_core/` : uip_core.c, uip_core_socket.c, uip_slip.c, uip_sock.c
- `src/kernel/dev/arch/all/debug/` : dev_os_debug.c
- `src/kernel/dev/arch/all/flash/` : flash.c
- `src/kernel/dev/arch/all/flash/amd/` : dev_flash_am29dlxxxx_1.c, dev_flash_am29dlxxxx_2.c, dev_flash_am29dlxxxx_3.c, dev_flash_nor_amd.c
- `src/kernel/dev/arch/all/flash/amd/lldapi/` : lld.c, trace.c
- `src/kernel/dev/arch/all/flash/dev_ftl/` : dev_ftl.c
- `src/kernel/dev/arch/all/i2c/` : dev_eeprom_24xxx.c, dev_eeprom_24xxx_0.c
- `src/kernel/dev/arch/all/i2c/bq/` : dev_bq24161_x.c, dev_bq27510_g3_x.c
- `src/kernel/dev/arch/all/i2c/fram/` : dev_fram_24clxx.c
- `src/kernel/dev/arch/all/i2c/rtc/` : dev_rtc_nxp_pca8565.c
- `src/kernel/dev/arch/all/lcd/` : dev_oled_ssd1305.c, dev_oled_ssd1322.c
- `src/kernel/dev/arch/all/modem/simcom/` : dev_modem_simcom.c
- `src/kernel/dev/arch/all/modem/ublox/` : dev_modem_ublox_sarag3.c
- `src/kernel/dev/arch/all/ppp/dev_ppp_uip/` : ahdlc.c, dev_ppp_uip.c, ipcp.c, ipv6cp.c, lcp.c, pap.c, ppp.c
- `src/kernel/dev/arch/all/slip/` : dev_slip.c
- `src/kernel/dev/arch/cmsis/dev_cmsis_cpu/` : dev_cmsis_cpu.c
- `src/kernel/dev/arch/cmsis/dev_cmsis_itm/` : dev_cmsis_itm.c, dev_cmsis_itm_0.c
- `src/kernel/dev/dev_cpufs/` : dev_cpufs.c
- `src/kernel/dev/dev_fb/` : dev_fb.c
- `src/kernel/dev/dev_head/` : dev_head.c
- `src/kernel/dev/dev_mem/` : dev_mem.c
- `src/kernel/dev/dev_null/` : dev_null.c
- `src/kernel/dev/dev_proc/` : dev_proc.c
- `src/kernel/dev/dev_tty/` : dev_tty.c, tty_font-8x16.c, tty_font-8x8.c
- `src/kernel/fs/fat/` : fat16.c, fat16_msdos.c, fat16_vfat.c, fatcore.c, fatcore_msdos.c, fatcore_vfat.c
- `src/kernel/fs/fatfs/` : fatfs.c, fatfscore.c
- `src/kernel/fs/fatfs/core/` : diskio.c, ff.c, ff_gen_drv.c
- `src/kernel/fs/fatfs/core/drivers/` : sd_diskio.c
- `src/kernel/fs/kofs/` : kofs.c
- `src/kernel/fs/rootfs/` : rootfs.c, rootfscore.c
- `src/kernel/fs/ufs/` : ufs.c, ufscore.c, ufsdriver.c, ufsdriver_1_3.c, ufsdriver_1_4.c, ufsdriver_1_5.c, ufsx.c
- `src/kernel/fs/vfs/` : vfs.c, vfscore.c, vfsdev.c, vfskernel.c
- `src/kernel/net/lwip/api/` : api_lib.c, api_msg.c, err.c, netbuf.c, netdb.c, netifapi.c, sockets.c, tcpip.c
- `src/kernel/net/lwip/core/` : def.c, dns.c, inet_chksum.c, init.c, ip.c, mem.c, memp.c, netif.c, pbuf.c, raw.c, stats.c, sys.c, tcp.c, tcp_in.c, tcp_out.c, timeouts.c, udp.c
- `src/kernel/net/lwip/core/ipv4/` : autoip.c, dhcp.c, etharp.c, icmp.c, igmp.c, ip4.c, ip4_addr.c, ip4_frag.c
- `src/kernel/net/lwip/netif/` : ethernet.c, ethernetif.c, lowpan6.c, slipif.c
- `src/kernel/net/lwip/ports/arm/` : sys_arch.c
- `src/lib/libc/` : libc.c
- `src/lib/libc/ctype/` : ctype.c
- `src/lib/libc/misc/` : crc.c, dtostr.c, ftoa.c, itoa.c, ltostr.c, prsopt.c, strto_l.c
- `src/lib/libc/net/` : htonl.c, socket.c
- `src/lib/libc/net/inet/` : inet_addr.c
- `src/lib/libc/stdio/` : printf.c, scanf.c, stdio.c
- `src/lib/libc/string/` : string.c
- `src/lib/libc/termios/` : tcgetattr.c, tcsetattr.c, termios.c
- `src/lib/libc/unistd/` : getopt.c, io.c, unistd.c
- `src/lib/librt/` : mq.c, sem.c
- `src/lib/pthread/` : pthread.c, pthread_cond.c, pthread_mutex.c
- `src/sbin/` : btb.c, cat.c, compress.c, cp.c, date.c, df.c, ecat.c, echo.c, initd.c, kill.c, ls.c, lsh.c, mkdir.c, mkfifo.c, mkfs.c, more.c, mount.c, mv.c, od.c, ps.c, pwd.c, read.c, rm.c, rmdir.c, shutdown.c, sleep.c, stty.c, sync.c, touch.c, umount.c, uname.c, wrapr.c, xmodem.c
- `src/sbin/net/` : ifconfig.c, slipd.c

## Liste de sources dérivée : NUCLEO-F439ZI

Dérivation : socle ci-dessus + fichiers SoC de la configuration noyau + `dev_stm32f4xx_7.20.ewp` (Debug, bibliothèque liée par les applications F4) + `bsp_olimex_p407_7.30.ewp` (Debug) + application `tauon-basic_stm32f4-olimex_p407_7.20.ewp` (Debug).

HYPOTHÈSE À VALIDER : l'Olimex STM32-P407 est retenue comme projet STM32F4 le plus proche (Ethernet présent comme sur la NUCLEO-F439ZI ; BSP le plus complet : `uart_3`, `spi_3` ; même famille F40x/F41x que la base de registres F42x/F43x pour UART/ETH). La STM32F4-Discovery n'a pas d'Ethernet embarqué ; la STM32F469I-EVAL est une F469. Brochage, horloges (180 MHz) et PHY sont à reprendre pour la F439 (étape 5).

- Sources : **353** (144527 lignes existantes) ; ensembles : absent = 3, actif = 344, gelé = 6.
- Éléments `[gelé]` de la liste (embOS IAR : `RTOSInit_STM32F4x_CMSIS.c`, `OS_Error.c`, `xmtx*.c`, `JLINKMEM_Process.c`, `main.c` d'exemple et `os7m_tl__sp.a`) : à remplacer par leurs équivalents du paquet embOS GCC ; `startup_stm32f4xx.s` (syntaxe IAR) : à remplacer par un startup GCC.
- Éléments `[ABSENT]` : sorties mklepton à générer (étape 2).
- Anomalie : le noyau (`tauon-kernel-cortex-m4-debug`) compile avec les en-têtes `embOSCXM4_518/inc` puis `embOSCXM4_440/Inc`, alors que les applications F4 lient `embOSCXM4_386/Lib/os7m_tl__sp.a` et ses sources `arch/cmsis` : versions d'embOS incohérentes, à examiner en tâche 4.
- Anomalie : la configuration noyau Cortex-M4 déclare `dev/arch/cortexm/stellaris/drivers/dev_lm3s_cpu/dev_lm3s_cpu.c` (pilote CPU LM3S) ; conservé dans la liste par fidélité au projet, à retirer ou remplacer par `dev_stm32f4xx_cpu_x.c` à l'étape 5 (HYPOTHÈSE À VALIDER).

- `src/bin/net/ftpd/` : ftpd.c, ls.c
- `src/bin/net/mongoose/` : mongoose.c, mongoosed.c
- `src/bin/tst/` : tsteth.c
- `src/kernel/core/arch/cortexm/` : bin_mkconf.c [ABSENT], dev_dskimg.c [ABSENT], dev_mkconf.c [ABSENT]
- `src/kernel/core/ucore/cmsis/Device/st/stm32f4xx/` : system_stm32f4xx.c
- `src/kernel/core/ucore/cmsis/Device/st/stm32f4xx/startup/iar/` : startup_stm32f4xx.s
- `src/kernel/core/ucore/embOSCXM4_386/arch/cmsis/` : main.c [gelé]
- `src/kernel/core/ucore/embOSCXM4_386/arch/cmsis/cpu/` : JLINKMEM_Process.c [gelé], OS_Error.c [gelé], RTOSInit_STM32F4x_CMSIS.c [gelé], xmtx.c [gelé], xmtx2.c [gelé]
- `src/kernel/dev/arch/cortexm/stellaris/drivers/dev_lm3s_cpu/` : dev_lm3s_cpu.c
- `src/kernel/dev/arch/cortexm/stm32f4xx/` : eth.c, flash_if.c, gpio.c, spi.c, uart.c
- `src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/src/` : stm32f4xx_hal.c, stm32f4xx_hal_adc.c, stm32f4xx_hal_adc_ex.c, stm32f4xx_hal_can.c, stm32f4xx_hal_cec.c, stm32f4xx_hal_cortex.c, stm32f4xx_hal_crc.c, stm32f4xx_hal_cryp.c, stm32f4xx_hal_cryp_ex.c, stm32f4xx_hal_dac.c, stm32f4xx_hal_dac_ex.c, stm32f4xx_hal_dcmi.c, stm32f4xx_hal_dcmi_ex.c, stm32f4xx_hal_dma.c, stm32f4xx_hal_dma2d.c, stm32f4xx_hal_dma_ex.c, stm32f4xx_hal_eth.c, stm32f4xx_hal_flash.c, stm32f4xx_hal_flash_ex.c, stm32f4xx_hal_flash_ramfunc.c, stm32f4xx_hal_fmpi2c.c, stm32f4xx_hal_fmpi2c_ex.c, stm32f4xx_hal_gpio.c, stm32f4xx_hal_hash.c, stm32f4xx_hal_hash_ex.c, stm32f4xx_hal_hcd.c, stm32f4xx_hal_i2c.c, stm32f4xx_hal_i2c_ex.c, stm32f4xx_hal_i2s.c, stm32f4xx_hal_i2s_ex.c, stm32f4xx_hal_irda.c, stm32f4xx_hal_iwdg.c, stm32f4xx_hal_ltdc.c, stm32f4xx_hal_msp_template.c, stm32f4xx_hal_nand.c, stm32f4xx_hal_nor.c, stm32f4xx_hal_pccard.c, stm32f4xx_hal_pcd.c, stm32f4xx_hal_pcd_ex.c, stm32f4xx_hal_pwr.c, stm32f4xx_hal_pwr_ex.c, stm32f4xx_hal_qspi.c, stm32f4xx_hal_rcc.c, stm32f4xx_hal_rcc_ex.c, stm32f4xx_hal_rng.c, stm32f4xx_hal_rtc.c, stm32f4xx_hal_rtc_ex.c, stm32f4xx_hal_sai.c, stm32f4xx_hal_sai_ex.c, stm32f4xx_hal_sd.c, stm32f4xx_hal_sdram.c, stm32f4xx_hal_smartcard.c, stm32f4xx_hal_spdifrx.c, stm32f4xx_hal_spi.c, stm32f4xx_hal_sram.c, stm32f4xx_hal_tim.c, stm32f4xx_hal_tim_ex.c, stm32f4xx_hal_uart.c, stm32f4xx_hal_usart.c, stm32f4xx_hal_wwdg.c, stm32f4xx_ll_fmc.c, stm32f4xx_ll_fsmc.c, stm32f4xx_ll_rcc.c, stm32f4xx_ll_sdmmc.c, stm32f4xx_ll_usb.c
- `src/kernel/dev/arch/cortexm/stm32f4xx/dev_stm32f4xx/` : dev_stm32f4xx_cpu_x.c, dev_stm32f4xx_cubemx_eth.c, dev_stm32f4xx_dac_x.c, dev_stm32f4xx_eth.c, dev_stm32f4xx_flash.c, dev_stm32f4xx_i2c_x.c, dev_stm32f4xx_sdio.c, dev_stm32f4xx_spi_x.c, dev_stm32f4xx_uart_x.c
- `src/kernel/dev/arch/cortexm/stm32f4xx/driverlib/` : misc.c, stm32f4x7_eth.c, stm32f4xx_adc.c, stm32f4xx_can.c, stm32f4xx_crc.c, stm32f4xx_cryp_aes.c, stm32f4xx_cryp_des.c, stm32f4xx_cryp_tdes.c, stm32f4xx_dac.c, stm32f4xx_dbgmcu.c, stm32f4xx_dcmi.c, stm32f4xx_dma.c, stm32f4xx_exti.c, stm32f4xx_flash.c, stm32f4xx_gpio.c, stm32f4xx_hash_md5.c, stm32f4xx_hash_sha1.c, stm32f4xx_i2c.c, stm32f4xx_iwdg.c, stm32f4xx_pwr.c, stm32f4xx_rcc.c, stm32f4xx_rng.c, stm32f4xx_rtc.c, stm32f4xx_spi.c, stm32f4xx_syscfg.c, stm32f4xx_tim.c, stm32f4xx_usart.c, stm32f4xx_wwdg.c
- `src/kernel/dev/bsp/olimex_p407/dev_olimex_p407_board/` : dev_olimex_p407_board.c
- `src/kernel/dev/bsp/olimex_p407/dev_olimex_p407_peripherals/` : dev_olimex_p407_spi_3.c, dev_olimex_p407_uart_3.c
- `sys/user/tauon-basic/src/bin/dhrystone/` : dhry21a.c, dhry21b.c, dhrystone_main.c, timers.c
- `sys/user/tauon-basic/src/bin/free/` : dlmalloc.c, free_main.c
- `sys/user/tauon-basic/src/bin/net/cgi-bin/` : tstcgi2.c, tstpost.c
- `sys/user/tauon-basic/src/bin/sdramtest/` : sdramtest_main.c

## Fichiers hors-projet (principaux répertoires)

Non déclarés, non inclus, sans règle : bibliothèques tierces partiellement utilisées (CMSIS, HAL CubeMX, uIP/lwIP), outils et applications d'exemple. À revoir si un palier en a besoin.

- `src/kernel/core/ucore/cmsis-5` : 263 fichiers, 43136 lignes
- `src/kernel/net/uip` : 177 fichiers, 34672 lignes
- `src/kernel/fs/fatfs` : 10 fichiers, 31018 lignes
- `src/kernel/net/lwip` : 63 fichiers, 30422 lignes
- `src/kernel/net/uip2.5` : 75 fichiers, 13489 lignes
- `src/kernel/dev/arch/cortexm/stm32f4xx` : 25 fichiers, 9207 lignes
- `src/kernel/fs/yaffs` : 26 fichiers, 8380 lignes
- `src/kernel/core/ucore/cmsis` : 31 fichiers, 7508 lignes
- `src/bin` : 25 fichiers, 4036 lignes
- `src/kernel/dev/arch/all/usb` : 16 fichiers, 3244 lignes
- `sys/user/tauon_sampleapp` : 15 fichiers, 1265 lignes
- `src/kernel/dev/arch/all/eth` : 2 fichiers, 1240 lignes
- `src/kernel/dev/arch/all/sd` : 3 fichiers, 1028 lignes
- `src/kernel/dev/arch/all/i2c` : 6 fichiers, 873 lignes
- `src/kernel/dev/arch/all/lcd` : 2 fichiers, 720 lignes
- `src/kernel/dev` : 3 fichiers, 565 lignes
- `src/sbin` : 4 fichiers, 486 lignes
- `sys/user/tauon-basic` : 8 fichiers, 439 lignes
- `src/kernel/dev/arch/all/flash` : 1 fichiers, 357 lignes
- `src/kernel/core` : 3 fichiers, 339 lignes
- `src/lib/libc` : 2 fichiers, 167 lignes
- `src/kernel/dev/arch/all/debug` : 1 fichiers, 32 lignes
- `src/kernel/core/core-segger` : 1 fichiers, 12 lignes

<!-- perimetre_complement.py : début -->

## Complément de l'étape 4 (bilan)

Généré par `tools/migration/perimetre_complement.py` (idempotent ; rejouer après `build_closure.py`). Ajoute les sources créées par la migration (étapes 2 à 4 : KAL décomposé, démarrage GCC, pilotes et BSP QEMU, noyau statique hôte, bancs `tests/`, copies `legacy/`), classées par règle de chemin (`REGLES` du script, colonne `justification`) ; retire les lignes des fichiers disparus ; applique les renommages (`inc/legacy` → `inc/Legacy`). Les tableaux ci-dessus restent ceux de l'étape 1.

Lignes retirées : `src/kernel/core/kal.c`, `tests/kal/kal_bench.c`, `tests/kal/kal_fpu_armv7m.S`, `tests/kal/kal_regs_armv7m.S`, `tests/kal/kal_regs_read_armv7m.S`, `src/kernel/dev/bsp/qemu_mps2_an386/qemu_mps2_an386.h`, `src/kernel/dev/bsp/qemu_mps2_an386/qemu_mps2_an386_board.c`, `sys/user/tauon-basic/src/arch/qemu-mps2-an386/kernel_mkconf/include/user_kernel_mkconf.h`, `tests/kal/backend/embos/kal_bench_os.h`, `tests/kal/backend/freertos/kal_bench_os.h`.

| Ensemble | Fichiers ajoutés | Lignes ajoutées | Fichiers (total) | Lignes (total) |
|---|---|---|---|---|
| actif | 443 | 144710 | 1536 | 463888 |
| différé | 0 | 0 | 1360 | 352246 |
| gelé | 0 | 0 | 0 | 0 |
| hors-projet | 97 | 72929 | 856 | 265306 |
| **total** | 540 | 217639 | 3752 | 1081440 |

<!-- perimetre_complement.py : fin -->
