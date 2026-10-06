# Journal de validation — NUCLEO-F439ZI (étape 5)

Carte : **NUCLEO-F429ZI** (remplaçante de la F439ZI, décision 2026-10-01 ; paliers à rejouer sur
une F439ZI). STM32F429ZI rév. 3 (DBGMCU_IDCODE `0x20016419`, flash 2048 Ko), ST-LINK/V2-1
(`V2J27M15`), `/dev/ttyACM0`. embOS-Classic V5.20.0.0 (`libosT7VHLSP.a`, hard-float),
`arm-none-eabi-gcc` 14.2.1, `-O0 -g` (Debug : aucune option `-O`, constat du 2026-10-02 ; paliers 1-8 en `-O0`, palier 9 en `-Os`), OpenOCD 0.12.0, `gdb-multiarch` 16.3.
Preset `nucleo-f439zi-embos` : `lepton.elf` text 411 620 / data 1 124 / bss 143 840 octets
(flash 20 %, RAM 74 % de 192 Ko). Horloge : HSE 8 MHz (MCO du ST-LINK, bypass), PLL 168 MHz.

## Paliers (ordre ETAPE-5, arrêt au premier échec)

| Palier | Statut | Date | Preuve |
|---|---|---|---|
| 1. Reset → `main` | VERT | 2026-10-02 | gdb (`lepton-load`, `break main`) : PC initial `Reset_Handler`, MSP `0x20030000` ; à `main` : `nucleo_f439zi_clock_hse` = 1 (HSE retenu), `SystemCoreClock` = 168 000 000, `RCC_CFGR` = `0x940A` (SYSCLK = PLL, APB1 /4, APB2 /2), `FLASH_ACR` = `0x705` (5 états d'attente, prélecture, caches I/D) |
| 2. Mémoire | VERT | 2026-10-02 | à `main` (gdb) : `.data` identique à l'image de l'ELF sauf `SystemCoreClock` (écrit par `SystemInit`), `.bss` nul sauf `nucleo_f439zi_clock_hse` ; SRAM 192 Ko contiguë `0x20000000-0x2002FFFF` (SRAM1+2+3 : hypothèse de `mem_nucleo-f439zi.ld` validée) et CCM 64 Ko : écriture d'un motif aléatoire par OpenOCD juste après reset, relecture brute identique (`dump_image`, comparé sur l'hôte) |
| 3. Warmup du noyau | VERT | 2026-10-02 | gdb : `main` → `_start_kernel` → `_kernel_warmup_boot` ; console : bannière, rootfs, `ls /` : `dev kernel bin usr etc var mnt` ; `/dev` : `board ttys3 ttys6 eth0 console`… |
| 4. Premier appel système tracé | VERT | 2026-10-02 | `traces/palier4-appel-systeme-carte.gdb` → `traces/palier4-appel-systeme-carte.txt` : `initd` → `_system_setpgid` → `kernel_syscall_lock` ; tâche `kernel_thread` → `_kernel_syscall` (pid 1, syscall 49) → `_syscall_setpgid` (0) → `kernel_syscall_unlock` (état `END`) ; `initd` reprend, errno 0 — identique à QEMU |
| 5. Multitâche et signaux | VERT | 2026-10-02 | banc KAL sur carte (semihosting OpenOCD, `tests/kal_openocd.py`) : T1-T8, T1F/T4F/T6F/T7F, TICI, TCLK, IRQ verts ; `HARNESS_FAIL` rend 1 ; `ps` : initd, lsh, ftpd |
| 6. Fumée canonique | VERT | 2026-10-02 | `ctest --preset nucleo-f439zi-embos -L board` (18 tests, deux fois de suite) : `board.smoke_lsh` (flash, reset, `uname -a` = `lepton-cortexm4-32 4.10.0.2 … cortexM4-stm32f4`, `ls`, `ps`) puis banc KAL ; 30 démarrages consécutifs sans faute après correction du défaut ICI (ci-dessous) |
| 7. Réseau | **VERT** | 2026-10-03 | `ctest -R board.net` (`tests/board_net.py` : reset, ping 3/3, session FTP `tauon` : `CWD`/`LIST` de `/usr/sbin` avec `lsh`, `RETR /usr/etc/.boot` identique à la source, errno de `tsterrno` en numérotation Lepton, `ftpd` présent) : **8/8 en `-O0`, 5/5 en `-Os`**. Trois défauts traités : (1) HardFault à la connexion FTP, tas sans limite : `_sbrk` borné + TSBRK, `.ccm_bss` (tas 43 → 88 Ko), `8c4262a`, `9b99404` ; (2) transferts de données bloqués (environ une fois sur deux), trames reçues perdues : `isset_read` appelait `ETH_CheckFrameReceived`, qui incrémente `Seg_Count` à chaque appel, et `eth_packet_read` rendait au DMA des descripteurs de trames non lues (MMC rx unicast 38 contre 35 trames reçues par lwIP, descripteur bloqué `OWN=0`) : `eth_packet_available` sans effet de bord, `bf9a4b4` (durée du test 40 → 23 s) ; (3) « réseau mort après le débogueur » : **faux défaut**, cœur laissé arrêté par la procédure gdb (`DHCSR` `0x00030003`), relance explicite, `be9b4da` |
| 8. Endurance | VERT | 2026-10-03 | `tests/endurance_board.py`, 4 h, cycles `lsh` (uname, ps, ls, cat, pwd) toutes les 30 s et ping continu : **`-O0`** (2026-10-02, avant les corrections du palier 7) 480 cycles, ping 14 393/14 395 ; **`-Os`** (2026-10-03) 480 cycles, ping 14 396/14 396, aucune coupure ; aucun redémarrage, CFSR/HFSR nuls. Piles (`lepton-stacks`, marge depuis `heap_top`), `-Os` : `lsh` 66 % (692 octets libres sur 2 048), `initd` 59 %, `lwip_core` 45 %, `kernel_thread` 21 %, `ftpd` 14 %, `tcpip_thread` 3 %, MSP 11,6 % ; `-O0` : `lsh` 76 %, MSP 12,6 % |
| 9. Optimisation | VERT | 2026-10-03 | `-Os` (décision 2026-10-02, tous presets Cortex-M ; text 413 → 287 Ko) ; paliers 6 (`ctest -L board` 19/19 dont TSBRK), 7 (`board.net` 5/5) et 8 (endurance 4 h) rejoués verts |

Second port (USART6, `ttys6`, CN10 D1/D0 = PG14/PG9) : chargé, non vérifié électriquement (pas
d'adaptateur série) — HYPOTHÈSE À VALIDER (UM1974).

## Défauts trouvés et corrigés pendant la mise au point

| Symptôme | Cause | Correction |
|---|---|---|
| Environ un démarrage sur cinq : pas d'invite `lsh` ; cœur dans `Default_Handler` | HardFault forcé (HFSR `0x40000000`) par UsageFault INVSTATE (CFSR `0x00020000`) : `lsh` préemptée au milieu d'un LDM/STM reçoit SIGCHLD (fin d'`ifconfig`) ; `__inline_swap_signal_handler` redirige le PC sauvegardé vers `sighandler` en laissant l'état ICI du xPSR (`0x610FF000`, ICI = `0x3C`). QEMU ne modélise pas ICI | KAL : `__kal_arch_redirect_xpsr` (kal_arch.h armv7m) appliqué au cadre dérouté ; test TICI (décision utilisateur 2026-10-02) |
| `ps` : STIME dix fois trop rapide (aussi sous QEMU ; déjà sous IAR) | `_SC_CLK_TCK` (HZ) = 100 sur Cortex-M, tick embOS 1 kHz | `__KERNEL_CLK_TCK` (cmake/kal/embos.cmake) pour le SysTick et `_SC_CLK_TCK` ; test TCLK (décision utilisateur 2026-10-02) |
| Vecteurs du BSP absents de l'ELF | fichier dans une bibliothèque statique : les `IRQ<n>_Handler` faibles du démarrage satisfont l'éditeur de liens | `nucleo_f439zi_vectors.c` lié comme objet de l'exécutable (`LEPTON_FIRMWARE_SOURCES`) |
| `eth.c` : broches et PHY de l'Olimex en dur, PHY à l'adresse 0 introuvable | — | descripteur `eth_stm32f4x7_bsp` fourni par le BSP (décision utilisateur) ; `dev_stm32f4xx_eth_load` : retour non initialisé corrigé |
| `board.smoke_lsh` : un échec sur la première exécution après raccordement USB de la sonde (`uname -a` reçoit la bannière de démarrage) | des invites d'une session précédente, tamponnées par le ST-LINK, arrivent après l'ouverture (et le `tcflush`) du port : prises pour l'invite de démarrage, « uname -a » est consommé par l'attente d'initd | `smoke_lsh.py` : console vidée (`drain`) avant le reset ; non reproduit à la demande après correction (3 passages verts avec données en attente), vérifié en non-régression seulement |

## Hypothèses validées / restant à valider

- VALIDÉE : SRAM 192 Ko contiguë à `0x20000000` (F42x/43x), CCM 64 Ko à `0x10000000`.
- VALIDÉE : HSE 8 MHz en bypass (MCO du ST-LINK) ; repli HSI non exercé.
- VALIDÉE : PHY LAN8742A à l'adresse 0, registre d'état spécial 31 (vitesse/duplex bits 4:2).
- VALIDÉE : USART3 PD8/PD9 (console), DMA1 Stream1 canal 4 en réception.
- VALIDÉE : réception Ethernet (ARP, ICMP depuis la VM, 2026-10-02).
- VALIDÉE : `ftpd` sur carte (palier 7, 2026-10-03).
- À VALIDER : USART6 sur PG14/PG9 (connecteur).
- Puce déclarée `STM32F429xx` (pas d'en-tête `stm32f439xx.h` dans la SPL) : sans effet pour la
  F439 tant que CRYP/HASH ne sont pas utilisés ; à rejouer sur une F439ZI.

## Étape 7 — backend FreeRTOS (module 7.2, 2026-10-06)

Même carte (NUCLEO-F429ZI, ST-LINK `V2J27M15`, `/dev/ttyACM0`, hôte 192.168.2.20 sur `ens37`),
preset `nucleo-f439zi-freertos` (FreeRTOS V11.3.0, port `ARM_CM4F`, hard-float, `-Os -g`). Flash
interne autorisée pour le module (décision 2026-10-06). Comparaison avec la colonne embOS ci-dessus.

| Palier | Statut | Preuve (FreeRTOS) |
|---|---|---|
| 1. Reset → `main` | VERT | gdb : vecteur 0 = MSP `0x20030000`, `Reset_Handler` ; à `main` : HSE retenu, `SystemCoreClock` 168 000 000, `RCC_CFGR` `0x940A`, `FLASH_ACR` `0x705` — identique |
| 2. Mémoire | VERT | `.data` en RAM identique à l'ELF sauf `SystemCoreClock` ; `.bss` nul sauf `nucleo_f439zi_clock_hse` ; `.ccm_bss` (55 760 o) nul — identique |
| 3. Warmup | VERT | `main` → `_start_kernel` → `_kernel_warmup_boot` ; console : bannière, invite `lsh` |
| 4. Appel système tracé | VERT | `traces/palier4-appel-systeme-carte-freertos.gdb` → `.txt` : `initd` → `kernel_syscall_lock` ; `kernel_thread` (réveil par groupe d'événements) → `_kernel_syscall` (pid 1, syscall 49) → `_syscall_setpgid` → `kernel_syscall_unlock` (END) ; `initd` reprend, errno 0 — identique |
| 5. Multitâche et signaux | VERT | banc KAL sur carte (`ctest -L board`) : T1-T8, T1F/T4F/T6F/T7F, TICI, TCLK, TSBRK, IRQ verts, `HARNESS_FAIL` rend 1 |
| 6. Fumée canonique | VERT | `ctest --preset nucleo-f439zi-freertos -L board -E board.net` : 19/19, deux fois de suite (build final) |
| 7. Réseau | VERT | `board.net` 5/5 : ping 3/3, FTP `LIST` 13 entrées, `RETR /usr/etc/.boot` identique, errno `ECONNRESET`=15 (numérotation Lepton), `ftpd` présent |
| 8. Endurance | VERT | `tests/endurance_board.py`, 4 h (10:46-14:46), cycles `lsh` toutes les 30 s et ping continu : **480 cycles, ping 14 395/14 395** (perte 0 %, plus longue coupure 0 s), aucun redémarrage, CFSR/HFSR nuls, aucune assertion ni débordement FreeRTOS. Piles (`lepton-stacks`) : `lsh` 77,7 % (456 o libres sur 2 048 ; embOS 66 %), `initd` 66,8 %, `lwip_core` 48,4 %, `kernel_thread` 21,5 %, `ftpd` 16 %, `tcpip_thread` 3 %, idle 9,4 %, temporisateurs 10,2 %, MSP 11,6 % |

Défauts trouvés et corrigés pendant le module (QEMU et carte) :

| Symptôme | Cause | Correction |
|---|---|---|
| Carte : FTP `LIST` vide, « Error during reading of . » | tas newlib 4,7 Ko plus petit que sous embOS (mémoire statique de FreeRTOS en SRAM) : `malloc` de `sreaddir` en échec dès la première session | piles, TCB, listes et file internes de FreeRTOS en `.ccm_bss` (`ld/common-cortexm.ld`) : tas au niveau embOS (−528 o), CCM 85 % |
| QEMU sous charge : HardFault (INVSTATE, `pxCurrentTCB` NULL) et réveils perdus | TCB FreeRTOS intégré à `kernel_pthread_t`, que le noyau copie et efface entier (vfork, exec) | TCB alloué à part (`thread->tcb`), comme `OS_TASK` sous embOS |
| QEMU sous charge : console figée (aussi sous embOS) | `dev_cmsdk_uart` acquittait l'IRQ de réception après la lecture : octet sans interruption | acquittement avant la lecture (accord utilisateur) ; 0 blocage en 8 × 150 itérations sur les deux backends |

Constat préexistant (embOS et FreeRTOS, non corrigé : dette) : chaque session FTP laisse ~420 à
650 o de tas consommés (`_sbrk`) ; la 8ᵉ session consécutive échoue (« 421 Out of memory »)
sous les deux backends, au même rang. `board.net` (une session par reset) n'est pas concerné.

Rejeu du module 7.3 (2026-10-06), binaire final (`kernelconf.h` aligné sur embOS : verrous de
fichiers ajoutés sous FreeRTOS ; RAM 51 %, CCM 85 %) : `ctest --preset nucleo-f439zi-freertos -L
board` **20/20**, `board.net` **5/5** d'affilée ; endurance non rejouée.
