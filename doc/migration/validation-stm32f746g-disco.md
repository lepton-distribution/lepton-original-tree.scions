# Journal de validation — STM32F746G-DISCO (étape 6)

Carte : **STM32F746G-DISCO**, STM32F746NG (DBGMCU_IDCODE `0x10016449` : DEV_ID 0x449, rév. Z ;
CPUID `0x410FC271` : Cortex-M7 **r0p1** ; flash 1024 Ko, `0x1FF0F442`), ST-LINK/V2-1 intégrée
(`0483:374b`), console `/dev/ttyACM0`. embOS-Classic V5.20.0.0 `libosT7VHLSP_837070.a`
(`USE_ERRATUM_837070=1`), `-mcpu=cortex-m7 -mfpu=fpv5-sp-d16 -mfloat-abi=hard` (newlib
`v7e-m+fp`), `arm-none-eabi-gcc` 14.2.1, `-Os -g`, OpenOCD 0.12.0 (`board/stm32f746g-disco.cfg`),
`gdb-multiarch` 16.3. STM32CubeF7 v1.17.4 (HAL V1.3.3 : GPIO seulement ; CMSIS Device V1.2.10).
Preset `stm32f746g-disco-embos` : `lepton.elf` text 215 596 / data 1 112 / bss 30 592 octets
(flash 21 %, RAM 10 % de 320 Ko ; sans réseau). Horloge : HSE 25 MHz (quartz), PLL 216 MHz,
over-drive ; caches I et D actifs.

## Paliers (ordre ETAPE-5, arrêt au premier échec) — session 1 du module (2026-10-05)

| Palier | Statut | Date | Preuve |
|---|---|---|---|
| 1. Reset → `main` | VERT | 2026-10-05 | console : bannière et invite `lsh` au premier flash ; sonde (cœur en marche) : `RCC_PLLCFGR` `0x09406C19` (M 25, N 432, P 2, HSE, Q 9), `RCC_CFGR` `0x940A` (SYSCLK = PLL, APB1 /4, APB2 /2), `FLASH_ACR` `0x307` (7 états d'attente, prefetch, ART), `PWR_CSR1` `0x34000` (over-drive prêt et commuté), `SCB_CCR` `0x70200` (caches I et D), `stm32f746g_disco_clock_error` = 0, `USART1_BRR` `0x3AA` (108 MHz / 115 200) |
| 2. Mémoire | VERT | 2026-10-05 | 320 Ko `0x20000000-0x2004FFFF` (DTCM + SRAM1 + SRAM2) : motif aléatoire écrit par OpenOCD juste après reset, relecture brute identique (`load_image`/`dump_image`, comparé sur l'hôte) |
| 3. Warmup du noyau | VERT | 2026-10-05 | gdb : arrêt sur `_kernel_warmup_boot` (trace du palier 4) ; console : bannière, `.init` exécuté (`echo stm32f746g-disco`) |
| 4. Premier appel système tracé | VERT | 2026-10-05 | `traces/palier4-appel-systeme-carte.gdb` (script de l'étape 5, inchangé) → `traces/palier4-appel-systeme-f746.txt` : `initd` → `_system_setpgid` → `kernel_syscall_lock` ; `kernel_thread` → `_kernel_syscall` (pid 1, syscall 49) → `_syscall_setpgid` → `kernel_syscall_unlock` — identique à QEMU et à la F439 |
| 5. Multitâche et signaux | VERT | 2026-10-05 | banc KAL sur carte (semihosting OpenOCD) : T1-T8, T1F/T4F/T6F/T7F (FPU simple précision, cadre étendu), TICI, TCLK, TSBRK, IRQ verts ; `HARNESS_FAIL` rend 1 ; caches actifs |
| 6. Fumée canonique | VERT | 2026-10-05 | `ctest --preset stm32f746g-disco-embos -L board` **19/19** : `board.smoke_lsh` (flash, reset, `uname -a` = `lepton-cortexm7-32 4.10.0.2 … cortexM7-stm32f7`, `ls`, `ps`) puis banc KAL |
| 7. Réseau | À FAIRE | | session 2 du module : pilote Ethernet STM32F7 (HAL ETH V1.3.x), descripteurs DMA en zone non cachée (MPU) ; broches RMII relevées (exemple ST LwIP) : PA1, PA2, PA7, PC1, PC4, PC5, PG2, PG11, PG13, PG14 |
| 8. Endurance | À FAIRE | | session 2 |
| 9. Optimisation | À FAIRE | | `-Os` déjà appliqué (décision 2026-10-02) ; paliers 6-8 en `-Os` à constater en session 2 |

Aucun défaut rencontré pendant la session 1 (paliers 1-6 verts du premier coup).
