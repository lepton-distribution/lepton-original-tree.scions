# Journal de validation — NUCLEO-WL55JC1 (étape 6)

Cartes : **deux NUCLEO-WL55JC1**, STM32WL55JC (DBGMCU_IDCODE `0x10036497` : DEV_ID 0x497,
REV_ID 0x1003 ; CPUID `0x410FC241` : Cortex-M4 **r0p1**, sans FPU ; flash 256 Ko, SRAM 64 Ko ;
`FLASH_OPTR` `0x3FFFF0AA` : RDP 0, ESE 0), sondes STLINK-V3 intégrées (`0483:374e`) :

| Carte | STLINK-V3 (n° de série) | Console | UID64 | Rôle |
|---|---|---|---|---|
| A | `002700253431510837393937` | `/dev/ttyACM0` (`…0837393937-if02`) | `0x0530F7BF 0x0080E115` | paliers, banc KAL, endurance |
| B | `004F003E3431510937393937` | `/dev/ttyACM1` (`…0937393937-if02`) | `0x0530FA5F 0x0080E115` | radio (session 2) |

Lepton sur le **Cortex-M4 (CPU1) seul**, CPU2 (Cortex-M0+) jamais démarré (`PWR_CR4.C2BOOT` = 0,
relu). embOS-Classic V5.20.0.0 `libosT7LSP.a`, `-mcpu=cortex-m4 -mfloat-abi=soft` (newlib
`v7e-m/nofp`), `arm-none-eabi-gcc` 14.2.1, `-Os -g`, OpenOCD 0.12.0 (`target/stm32wlx.cfg`, cfg
générée avec `adapter serial`), `gdb-multiarch` 16.3. HAL STM32CubeWL V1.3.0 de l'arbre. Sans
réseau. Preset `nucleo-wl55jc1-embos` : `lepton.elf` text 100 220 / data 832 / bss 23 512
octets en session 1 ; 112 952 / 840 / 25 528 avec la radio (flash 43 %, RAM statique 40 % de
64 Ko ; MSP 4 Ko). Horloge : MSI range 11, 48 MHz,
sans PLL ni HSE, 2 états d'attente flash.

## Paliers (ordre ETAPE-5, arrêt au premier échec) — sessions 1 (paliers 1-6) et 2 (radio, 8), 2026-10-05

| Palier | Statut | Date | Preuve |
|---|---|---|---|
| 1. Reset → `main` | VERT | 2026-10-05 | sonde (cœur en marche) : `RCC_CR` `0xBB` (MSION, MSIRDY, MSIRGSEL, MSIRANGE 11), `RCC_CFGR` `0x00070000` (SWS = MSI), `FLASH_ACR` `0x702` (latence 2, PRFTEN, ICEN, DCEN), `PWR_CR1` `0x200` (échelle 1), `PWR_CR4` 0 (C2BOOT 0), USART2 `CR1` `0x1D`, `BRR` 417 (115 108 Bd à 48 MHz), `SHPR3` `0xC0F00000` (SysTick 0xC0, PendSV 0xF0), priorités NVIC 11, 12, 37 = `0xC0`, `SystemCoreClock` 48 000 000, `stm32wl55jci_nucleo_clock_error` = 0 |
| 2. Mémoire | VERT | 2026-10-05 | 64 Ko `0x20000000-0x2000FFFF` (SRAM1 + SRAM2 contiguës) : motif aléatoire écrit par OpenOCD après `reset halt`, relecture brute identique (`load_image`/`dump_image`, comparé sur l'hôte) |
| 3. Warmup du noyau | VERT | 2026-10-05 | gdb : arrêt sur `_kernel_warmup_boot` (`_start_kernel` ← `main`) ; console : bannière, `.init` exécuté (`echo nucleo-wl55jc1`) |
| 4. Premier appel système tracé | VERT | 2026-10-05 | `traces/palier4-appel-systeme-carte.gdb` (script de l'étape 5, inchangé) → `traces/palier4-appel-systeme-wl55.txt` : `initd` → `_system_setpgid` → `kernel_syscall_lock` ; `kernel_thread` → `_kernel_syscall` (pid 1, syscall 49) → `_syscall_setpgid` → `kernel_syscall_unlock` (état 2) ; reprise de `initd`, errno 0 — identique à QEMU, à la F439, à la F746 et à la SAMD21 |
| 5. Multitâche et signaux | VERT | 2026-10-05 | banc KAL sur carte (semihosting OpenOCD) : T1-T8, TICI, TCLK, TSBRK, IRQ verts (pas de variante FPU : cœur sans FPU) ; `HARNESS_FAIL` rend 1 |
| 6. Fumée canonique | VERT | 2026-10-05 | `ctest --preset nucleo-wl55jc1-embos -L board` **15/15** : `board.smoke_lsh` (flash, reset, `uname -a` = `lepton-cortexm4-32 4.10.0.2 … cortexM4-stm32wlxx`) puis banc KAL ; à la main : `ls`, `ps`, `cat /usr/etc/.boot`, `pwd`, `ls /dev` |
| 7. Réseau / radio | VERT (radio) | 2026-10-05 | pas de réseau IP ; radio Sub-GHz **FSK 868 MHz** (pilote `/dev/radio` du portage IAR inchangé : 14 dBm, 50 kbit/s, déviation 25 kHz, trames fixes de 64 octets), cartes A et B à courte distance : `ctest -L radio` (`board.radio`, `tests/board_radio.py`, même image flashée sur les deux cartes) : `radiotst tx/rx` **20/20** A→B et B→A (perdus 0, désordre 0, doublons 0), `radiotst ping/pong` **20/20** (A ping, B pong), fumée `cat /dev/radio &` / `echo <jeton> > /dev/radio` dans les deux sens ; période 1 s (rapport cyclique 1 % de la sous-bande 868,0-868,6 MHz) ; RSSI non exposé par le pilote (inchangé) |
| 8. Endurance | VERT | 2026-10-05 | `tests/endurance_board.py` sans réseau (`--fault-check v7m`), **1 h en `-Os`** (18:17-19:17, durée fixée par l'utilisateur), cycles `lsh` (uname -a, ps, ls /usr/sbin, cat /usr/etc/.boot, pwd) toutes les 30 s : **120 cycles**, aucun redémarrage, nombre de processus constant, CFSR = HFSR = 0. Piles : `lsh` 52,9 % (964 octets libres sur 2 048), `initd` 46,7 %, `kernel_thread` 20,0 %, MSP 23,2 % ; `stack=` du mkconf conservés. Premier essai en échec (18:06, cycle 15) : défaut du pilote USART corrigé (ci-dessous), essai rejoué en entier |
| 9. Optimisation | VERT (de fait) | 2026-10-05 | `-Os -g` dès la session 1 |

Session 1, piles après fumée et `ls`, `ps`, `cat`, `pwd`, `ls /dev` (`lepton-stacks`, marge depuis
`heap_top`) : `lsh` 52,9 % (964 octets libres sur 2 048), `initd` 46,7 %, `kernel_thread`
19,8 % (sur 4 096), MSP 23,2 % (3 144 octets libres sur 4 096) ; tas de thread 396 octets par
processus (3 tampons stdio de 64 octets) ; `lepton_embos_last_error` = `OS_OK`.

## Écarts au portage IAR (code différé repris)

| Élément IAR | Constat | Traitement |
|---|---|---|
| `#include "kernel\dev\…"` (3 fichiers) | séparateurs Windows, refusés par GCC | règle `include-backslash` de `transform_iar.py` (commits mécaniques) |
| `utilities_conf.h` (SubGHz_Phy) | branches `__CC_ARM`, `__ICCARM__`, `WIN32` | règles de gardes (copie `legacy/`) ; fichier non compilé en session 1 |
| pilote `cpu0` (`dev_stm32wlxx_cpu_x.c`) | `HAL_Init` (réarme le SysTick d'embOS), horloge changée au chargement des pilotes (après le calcul du tick d'embOS), fautes → `NVIC_SystemReset`, `HAL_GetTick` en ticks bruts | non compilé ; horloge dans `SystemInit` du BSP, base de temps HAL `dev_stm32wlxx_hal_tick.c` (millisecondes), fautes : gestionnaires de Lepton |
| `startup_stm32wl55xx_cm4.s` | assembleur IAR seul | démarrage générique `startup_armv7m.c` ; `IRQ11/12/37_Handler` du BSP vers les gestionnaires nommés du pilote USART2 |
| pilote USART2 (HAL + DMA) | priorités NVIC non réglées (0 : hors de la plage d'embOS, alors que les rappels appellent le noyau) | priorités `0xC0` posées par `SystemInit` ; pilote inchangé |
| mkconf IAR | `cpu freq="80000000"` pour une horloge MSI à 48 MHz | 48 MHz dans le mkconf de la carte |

## Défauts trouvés et corrigés (session 2)

| Symptôme | Cause | Correction |
|---|---|---|
| Console muette après ajout de la radio ; cœur dans `HAL_Delay` ← `RadioInit` ← `_kernel_warmup_dev` | (1) `HAL_GetTick`/`HAL_Delay` de `dev_stm32wlxx_hal_tick.c` non liés : dans la bibliothèque du BSP, les définitions faibles de `stm32wlxx_hal.c` suffisaient (`uwTick` figé à 0, déjà le cas en session 1 sans effet visible) ; (2) la pile radio attend (`RADIO_DELAY_MS`) au chargement des pilotes, avant `OS_Start` : interruptions masquées, SysTick en attente (`ICSR` VECTPENDING 15), tick figé | fichier dans `LEPTON_FIRMWARE_SOURCES` (objet de l'exécutable) ; `HAL_Delay` sur le compteur de cycles DWT ; fumée et banc 15/15 rejoués |
| Endurance (essai 1) : caractères perdus sur la console au bout de quelques minutes (`cat ` puis `p` de `pwd`), `.boot` exécuté comme script par des `lsh` imbriqués | pilote USART du portage IAR (`dev_stm32wlxx_uart_x.c`) : `read` à cheval sur la fin du tampon circulaire copiant la suite depuis `p_rx_buffer+(sz-r)` ; index d'écriture laissé à la taille du tampon à la fin d'un transfert DMA (`Size` = 255) | copie depuis le début du tampon ; index ramené à 0 ; suite carte 16/16, endurance rejouée verte |
| `board.radio` : jeton de fumée tronqué | jeton `…A->B…` : `>` est une redirection pour `lsh` (défaut du test) | jeton `…A-vers-B…` |

## Pièges

- Le STLINK-V3 garde en tampon la sortie de la console tant que le port n'est pas ouvert : les
  bannières des resets précédents (OpenOCD, gdb) s'affichent à l'ouverture (l'heure de `ps` le
  confirme), ce ne sont pas des redémarrages.
- `ctest -L board` laisse `kal_bench` en flash : reflasher `lepton.elf` (cible `flash`).
- Deux sondes identiques : sans `LEPTON_BOARD_STLINK_SERIAL`, OpenOCD prend la première trouvée.
- Scripts Python des tests importés depuis le trunk : `PYTHONDONTWRITEBYTECODE=1` (sinon un
  `__pycache__` est écrit dans le trunk ; `board.radio` le pose dans son environnement).
- Le pilote radio émet une trame de 64 octets par `write` : `echo` en émet deux (texte, puis fin
  de ligne) ; la réception affiche les octets nuls de remplissage.
