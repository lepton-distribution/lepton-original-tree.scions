# Code gelé — décidé le 2026-09-30

> **Supprimé à l'étape 6 (2026-10-05, décision utilisateur)** par `tools/migration/retrait_gele.py`
> (inventaire ci-dessous et 56 copies `legacy/` de l'étape 4 : 820 fichiers ; le fichier gelé de
> `prj/vc-2010` est parti avec le retrait IAR), plus 356 fichiers hors inventaire : annexes non-code
> des cibles gelées (heuristique d'`audit_iar.py`) et restes des paquets embOS IAR `ucore/embOS*`
> (1176 au total). État précédent : tag local `legacy`
> (`git checkout legacy -- <chemin>` pour retrouver un fichier). Ce document reste l'inventaire de
> ce qui a été retiré.

Généré par `tools/migration/build_closure.py` le 2026-09-30. Rien n'est supprimé ni déplacé (suppression : décision de l'étape 6). Le code gelé n'est ni modifié, ni compilé, ni audité au-delà de l'inventaire. Volumétrie : cloc 2.04 (--by-file --skip-uniqueness, colonne « code ») ; 3 fichier(s) non reconnu(s) par cloc comptés par le repli interne.

Total gelé : **765 fichiers, 158061 lignes de code**.

## Par motif

| Motif | Fichiers | Lignes | Répertoires principaux |
|---|---|---|---|
| règle : embOS IAR ARM7/ARM9/Win32 | 57 | 40241 | `src/kernel/core/ucore/embOSARM7_360` (23), `src/kernel/core/ucore/embOSW32_100` (21), `src/kernel/core/ucore/embOSARM7-9_388` (13) |
| règle : embOS IAR Cortex-M (Segger) remplacé par le port GCC (third_party/embos) ; lecture seule (tâche 4) | 130 | 27852 | `src/kernel/core/ucore/embOSCXM4_518` (38), `src/kernel/core/ucore/embOSCXM4_440` (28), `src/kernel/core/ucore/embOSCXM7_430` (27), `src/kernel/core/ucore/embOSCXM3_384` (19), `src/kernel/core/ucore/embOSCXM4_386` (18) |
| règle de répertoire : Atmel AT91 ARM7/ARM9 | 66 | 18189 | `src/kernel/dev/arch/at91/at91lib` (60), `src/kernel/dev/arch/at91/dev_at91_usbdp` (4), `src/kernel/dev/arch/at91/dev_at91_mci` (2) |
| règle : ARM7/ARM9 (cible abandonnée) | 73 | 16941 | `src/kernel/dev/arch/arm9/at91sam9261` (29), `src/kernel/dev/arch/arm7/at91sam7se` (22), `src/kernel/dev/arch/arm9/at91sam9260` (12), `src/kernel/dev/arch/arm7/at91m55800a` (8), `src/kernel/dev/arch/arm7/dev_at91` (2) |
| règle : carte AT91SAM9261-EK (ARM9) | 113 | 16846 | `sys/user/tauon_sampleapp/hal/board_atmel_at91sam9261-ek` (113) |
| règle : Freescale K60 (TWR-K60N512), carte abandonnée, sans projet IAR (proposition) | 148 | 13353 | `sys/user/tauon_sampleapp/hal/board_freescale_twrk60n512` (124), `src/kernel/dev/arch/cortexm/k60n512` (24) |
| règle : simulation Linux/Windows | 75 | 8075 | `src/kernel/dev/arch/win32/dev_win32_filerom` (5), `src/kernel/dev/arch/gnu32/dev_linux_com0` (4), `src/kernel/dev/arch/gnu32/dev_linux_screen` (4), `src/kernel/dev/arch/win32/dev-bsp-nu.tube` (4), `src/kernel/dev/arch/win32/dev_win32_fileflash` (4), `src/kernel/dev/arch/win32/dev_win32_flash` (4) … |
| règle : en-têtes Windows de la simulation | 3 | 4745 | `src/kernel/core` (3) |
| règle : outillage hôte Windows (WinPcap) | 39 | 4427 | `tools/host/win32` (39) |
| règle : simulation (virtual_cpu) | 29 | 2117 | `tools/virtual_cpu/hardware` (24), `tools/virtual_cpu` (4), `tools/virtual_cpu/ui` (1) |
| projets : noyau EWARM 4.x, AT91SAM9261 (ARM9) | 6 | 1795 | `src/kernel/dev/arch/all/sdcard` (5), `src/kernel/dev/arch/all/eth` (1) |
| règle : variante Windows de mklepton | 1 | 1539 | `tools/mklepton/src` (1) |
| règle : simulation Windows | 5 | 751 | `src/kernel/core/arch/win32` (5) |
| projets : projet ARM7/ARM9 (cible abandonnée) | 10 | 653 | `src/kernel/dev/arch/at91/at91lib` (4), `src/kernel/dev/arch/all/eth` (2), `sys/user/tauon-basic` (2), `src/bin` (1), `src/kernel/core` (1) |
| règle : M16C (cible abandonnée) | 7 | 289 | `src/kernel/net/lwip` (7) |
| règle : port eCos (backend core-ecos absent de l'arbre) | 1 | 233 | `tools/host/debian` (1) |
| projets : noyau, configuration ARM926EJ-S | 1 | 8 | `src/kernel/dev/arch/at91/at91lib` (1) |
| règle : projet Visual Studio de la simulation | 1 | 7 | `prj` (1) |

## Par répertoire

| Répertoire | Fichiers | Lignes |
|---|---|---|
| `prj` | 1 | 7 |
| `src/bin` | 1 | 396 |
| `src/kernel/core` | 4 | 4769 |
| `src/kernel/core/arch/win32` | 5 | 751 |
| `src/kernel/core/ucore/embOSARM7-9_388` | 13 | 3110 |
| `src/kernel/core/ucore/embOSARM7_360` | 23 | 7065 |
| `src/kernel/core/ucore/embOSCXM3_384` | 19 | 2930 |
| `src/kernel/core/ucore/embOSCXM4_386` | 18 | 3625 |
| `src/kernel/core/ucore/embOSCXM4_440` | 28 | 6440 |
| `src/kernel/core/ucore/embOSCXM4_518` | 38 | 9126 |
| `src/kernel/core/ucore/embOSCXM7_430` | 27 | 5731 |
| `src/kernel/core/ucore/embOSW32_100` | 21 | 30066 |
| `src/kernel/dev/arch/all/eth` | 3 | 1026 |
| `src/kernel/dev/arch/all/sdcard` | 5 | 804 |
| `src/kernel/dev/arch/arm7/at91m55800a` | 8 | 2386 |
| `src/kernel/dev/arch/arm7/at91sam7se` | 22 | 7758 |
| `src/kernel/dev/arch/arm7/dev_at91` | 2 | 195 |
| `src/kernel/dev/arch/arm9/at91sam9260` | 12 | 2063 |
| `src/kernel/dev/arch/arm9/at91sam9261` | 29 | 4539 |
| `src/kernel/dev/arch/at91/at91lib` | 65 | 16362 |
| `src/kernel/dev/arch/at91/dev_at91_mci` | 2 | 762 |
| `src/kernel/dev/arch/at91/dev_at91_usbdp` | 4 | 1164 |
| `src/kernel/dev/arch/cortexm/k60n512` | 24 | 3691 |
| `src/kernel/dev/arch/gnu32/common` | 3 | 146 |
| `src/kernel/dev/arch/gnu32/dev_linux_com0` | 4 | 446 |
| `src/kernel/dev/arch/gnu32/dev_linux_eth` | 2 | 193 |
| `src/kernel/dev/arch/gnu32/dev_linux_fileflash` | 2 | 206 |
| `src/kernel/dev/arch/gnu32/dev_linux_filerom` | 2 | 145 |
| `src/kernel/dev/arch/gnu32/dev_linux_flash` | 1 | 267 |
| `src/kernel/dev/arch/gnu32/dev_linux_kb` | 2 | 152 |
| `src/kernel/dev/arch/gnu32/dev_linux_leds` | 2 | 89 |
| `src/kernel/dev/arch/gnu32/dev_linux_rtc` | 2 | 135 |
| `src/kernel/dev/arch/gnu32/dev_linux_screen` | 4 | 444 |
| `src/kernel/dev/arch/gnu32/dev_linux_sdcard` | 1 | 130 |
| `src/kernel/dev/arch/gnu32/dev_linux_serial_0` | 2 | 323 |
| `src/kernel/dev/arch/gnu32/dev_linux_serial_1` | 2 | 323 |
| `src/kernel/dev/arch/gnu32/dev_linux_serial_pt` | 2 | 316 |
| `src/kernel/dev/arch/gnu32/dummy_linux_syscall.S` | 1 | 93 |
| `src/kernel/dev/arch/win32/dev-bsp-nu.tube` | 4 | 180 |
| `src/kernel/dev/arch/win32/dev_win32_board` | 1 | 287 |
| `src/kernel/dev/arch/win32/dev_win32_com0` | 3 | 294 |
| `src/kernel/dev/arch/win32/dev_win32_com1` | 3 | 640 |
| `src/kernel/dev/arch/win32/dev_win32_com2` | 3 | 543 |
| `src/kernel/dev/arch/win32/dev_win32_eth` | 3 | 341 |
| `src/kernel/dev/arch/win32/dev_win32_fileflash` | 4 | 207 |
| `src/kernel/dev/arch/win32/dev_win32_filerom` | 5 | 316 |
| `src/kernel/dev/arch/win32/dev_win32_flash` | 4 | 350 |
| `src/kernel/dev/arch/win32/dev_win32_kb` | 1 | 254 |
| `src/kernel/dev/arch/win32/dev_win32_lcd` | 1 | 137 |
| `src/kernel/dev/arch/win32/dev_win32_lcd_matrix` | 1 | 219 |
| `src/kernel/dev/arch/win32/dev_win32_lcd_vga` | 1 | 242 |
| `src/kernel/dev/arch/win32/dev_win32_rotary_switch` | 2 | 296 |
| `src/kernel/dev/arch/win32/dev_win32_rtc` | 3 | 153 |
| `src/kernel/dev/arch/win32/dev_win32_sdcard` | 4 | 208 |
| `src/kernel/net/lwip` | 7 | 289 |
| `sys/user/tauon-basic` | 2 | 107 |
| `sys/user/tauon_sampleapp/hal/board_atmel_at91sam9261-ek` | 113 | 16846 |
| `sys/user/tauon_sampleapp/hal/board_freescale_twrk60n512` | 124 | 9662 |
| `tools/host/debian` | 1 | 233 |
| `tools/host/win32` | 39 | 4427 |
| `tools/mklepton/src` | 1 | 1539 |
| `tools/virtual_cpu` | 4 | 511 |
| `tools/virtual_cpu/hardware` | 24 | 1591 |
| `tools/virtual_cpu/ui` | 1 | 15 |

Décision utilisateur du 2026-09-30 : proposition acceptée, sauf Cortex-M7 Atmel SAMV71/SAME70 (point 3), maintenu en différé comme référence M7. Les points ci-dessous sont ceux de la proposition.

## Points de la proposition

1. **Cibles abandonnées actées** (décision 2026-09-29 : ARM7, ARM9, M16C) et **simulations** (`dev/arch/gnu32`, `dev/arch/win32`, `core/arch/win32`, `tools/virtual_cpu`, `prj/vc-2010`) : gel proposé sans réserve. Réserve : `dev/arch/gnu32` sert au noyau statique hôte de mklepton (`prj/scons/arch/synthetic/x86_static`) — gel à confirmer après la tâche 5.
2. **embOS IAR Cortex-M** (`core/ucore/embOSCXM*`) : proposé gelé car remplacé par le port GCC Segger (`third_party/embos`) ; conservé en lecture pour la comparaison d'API (tâche 4).
3. **Cortex-M7 Atmel** (SAMV71/SAME70 : `dev/arch/cortexm/at91samv7x`, `dev/bsp/same70xplained`, `dev/bsp/samv71xplained_ultra`, `dev/arch/at91/softpack-lib`) : cartes non retenues ; seul code M7 de l'arbre (référence possible pour la Discovery F7, qui n'a aucun projet).
4. **Freescale K60** (`dev/arch/cortexm/k60n512`, `user/tauon_sampleapp/hal/board_freescale_twrk60n512`) : carte abandonnée, sans projet IAR (build scons seulement).
5. **eCos** (`tools/host/debian/ecos`) et **WinPcap** (`tools/host/win32`) : outillage hôte sans usage dans la cible Linux/GCC.
6. **Non gelés, laissés en différé faute de décision** : STM32WL55 Nucleo (`stm32wlxx`, projets EWARM 9.50 les plus récents de l'arbre), variante `discovery_f4-baseboard-modem`, NFC PN7150, USB device STM32F4. `prj/scons` n'est pas gelé (build du noyau statique mklepton).
7. **Candidats étape 6 (différé)** : M3 = STM32F1 (`dev/arch/cortexm/stm32f1xx`, recommandé : même famille ST que la base) ou Stellaris LM3S (`stellaris`, produit TI ancien) ; M0+ = SAMD20 (`samd20xplained_pro`, seul BSP M0+ de l'arbre). HYPOTHÈSE À VALIDER : proposition, cartes non choisies.


## Ajout de l'étape 4 — copies d'origine `legacy/` (décisions D2a, D3a du 2026-10-01)

Ajouté à la main (hors `build_closure.py`). Quand `tools/migration/transform_iar.py` (règles
`garde-cible-gelee`, `garde-compilateur`) retire d'un fichier actif des branches de cibles gelées
(IAR M16C, Win32, ARM7/ARM9, eCos) ou de compilateurs non GCC (Keil, Visual C), il copie d'abord
le fichier d'origine **à l'identique** sous `legacy/<chemin dans le trunk>` (clone :
`scion/legacy/…`). Ces copies sont gelées : ni compilées, ni modifiées ; `audit_iar.py` les classe
« gelé » (règle `^legacy/`). Elles sont supprimées à l'étape 6 avec le reste du code gelé.
Module KAL (2026-10-01) : `tools/migration/kal_split.py` (étape `gel`) y copie de même `kal.h`
et `kal.c` (Win32 seul) avant d'en retirer les branches gelées.
Module `tools/mklepton` (2026-10-01) : `mklepton.c` copié à l'identique avant le retrait, à la
main (commit sémantique), de l'option `cpufs` `-split` (modèles de code IAR M16C62 générés).
Inventaire : `find "$LEPTON_TRUNK/legacy" -type f`.
