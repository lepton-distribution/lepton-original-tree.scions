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
| 7. Réseau | **EN COURS** (ping vert, `ftpd` : LIST bloqué) | 2026-10-03 | ping vert depuis la VM (réception Ethernet validée ; endurance 4 h : 14 393/14 395). **Défaut 1, corrigé** : HardFault (BusFault imprécise) à la connexion FTP, `malloc` d'une session de 18 Ko au-delà du tas (`_sbrk` de libnosys sans limite, débordement sur la MSP puis hors SRAM) → `_sbrk` borné (`sbrk_cortexm.c`, test TSBRK) et `.ccm_bss` (45 Ko en CCM, tas 43 → 88 Ko), commits `8c4262a`, `9b99404`. **Défaut 2, ouvert** : `board.net` en `-O0` : connexion, login, `CWD`, `PASV` corrects, `LIST` → « 150 Accepted data connection » puis aucune donnée (délai). La session `ftpd` boucle dans `sfgets` → `select` sur la socket de contrôle ; aucune faute (CFSR/HFSR nuls). Sous QEMU, `LIST` passe. **Défaut 3, ouvert** : après un arrêt du cœur par le débogueur, le réseau ne répond plus (ARP), et `lsh` a disparu de la liste des tâches (hypothèse : descripteurs de réception ETH épuisés pendant l'arrêt, sans reprise par le pilote ; non vérifiée) |
| 8. Endurance | À FAIRE | | |
| 9. Optimisation | À FAIRE | | décision 2026-10-02 : `-Os` (tous presets Cortex-M) ; paliers 6-8 à rejouer |

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
- À VALIDER : USART6 sur PG14/PG9 (connecteur) ; `ftpd` sur carte (palier 7).
- Puce déclarée `STM32F429xx` (pas d'en-tête `stm32f439xx.h` dans la SPL) : sans effet pour la
  F439 tant que CRYP/HASH ne sont pas utilisés ; à rejouer sur une F439ZI.
