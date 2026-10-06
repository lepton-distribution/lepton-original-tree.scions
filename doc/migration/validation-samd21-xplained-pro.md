# Journal de validation — SAMD21 Xplained Pro (étape 6)

Carte : **SAMD21 Xplained Pro**, ATSAMD21J18A (DSU DID `0x10010100` : SAMD21J18A rév. B ;
CPUID `0x410CC601` : Cortex-M0+ **r0p1** ; flash 256 Ko, SRAM 32 Ko), sonde EDBG intégrée
(CMSIS-DAP `03eb:2111`, n° `ATML2130021800003505`), console `/dev/ttyACM0` (CDC de l'EDBG).
embOS-Classic V5.20.0.0 `libosT6LSP.a`, `-mcpu=cortex-m0plus -mfloat-abi=soft` (newlib
`v6-m/nofp`), `arm-none-eabi-gcc` 14.2.1, `-Os -g`, OpenOCD 0.12.0
(`board/atmel_samd21_xplained_pro.cfg`), `gdb-multiarch` 16.3. Microchip SAMD21_DFP 3.8.270
(`LEPTON-PROVENANCE.md`). Sans réseau (décision 2026-10-05).
Preset `samd21-xplained-pro-embos` : `lepton.elf` text 97 056 / data 924 / bss 13 008 octets
(flash 38 %, RAM statique 43 % de 32 Ko ; tas des processus `0x20003668`-`0x20007A00`, 17 304
octets ; MSP 1 536 octets). Horloge : DFLL48M en boucle fermée sur XOSC32K (quartz 32,768 kHz),
48 MHz, 1 état d'attente flash.

## Paliers (ordre ETAPE-5, arrêt au premier échec) — sessions 1 (paliers 1-6) et 2 (8), 2026-10-05

| Palier | Statut | Date | Preuve |
|---|---|---|---|
| 1. Reset → `main` | VERT | 2026-10-05 | sonde (cœur en marche) : `SYSCTRL_PCLKSR` `0x5AD2` (XOSC32KRDY, DFLLRDY, DFLLLCKF, DFLLLCKC ; DFLLOOB = 0), `XOSC32K` `0x060E` (STARTUP 6, XTALEN, EN32K, ENABLE), `DFLLCTRL` `0x0A06` (boucle fermée, WAITLOCK, QLDIS), `DFLLMUL` `0x7DFF05B9` (MUL 1465, FSTEP 511, CSTEP 31), GCLK0 `0x00030700` (DFLL48M, IDC), GCLK1 `0x00010501` (XOSC32K), `CLKCTRL` SERCOM3_CORE `0x4017` (GCLK0), `NVMCTRL_CTRLB` `0x2` (RWS 1), SERCOM3 `CTRLA` `0x40100006`, `BAUD` `0xF62C` (115 200 Bd à 48 MHz), `SHPR3` `0x80C00000` (SysTick 0x80, PendSV 0xC0), `SystemCoreClock` 48 000 000, `samd21_xplained_pro_clock_error` = 0 |
| 2. Mémoire | VERT | 2026-10-05 | 32 Ko `0x20000000-0x20007FFF` : motif aléatoire écrit par OpenOCD juste après reset, relecture brute identique (`load_image`/`dump_image`, comparé sur l'hôte) |
| 3. Warmup du noyau | VERT | 2026-10-05 | gdb : arrêt sur `_kernel_warmup_boot` (trace du palier 4) ; console : bannière, `.init` exécuté (`echo samd21-xplained-pro`) |
| 4. Premier appel système tracé | VERT | 2026-10-05 | `traces/palier4-appel-systeme-carte.gdb` (script de l'étape 5, inchangé, 4 points d'arrêt matériels suffisent) → `traces/palier4-appel-systeme-samd21.txt` : `initd` → `_system_setpgid` → `kernel_syscall_lock` ; `kernel_thread` → `_kernel_syscall` (pid 1, syscall 49) → `_syscall_setpgid` → `kernel_syscall_unlock` — identique à QEMU, à la F439 et à la F746 |
| 5. Multitâche et signaux | VERT | 2026-10-05 | banc KAL sur carte (semihosting OpenOCD) : T1-T8, TICI, TCLK, TSBRK, IRQ verts (pas de variante FPU en ARMv6-M) ; `HARNESS_FAIL` rend 1 |
| 6. Fumée canonique | VERT | 2026-10-05 | `ctest --preset samd21-xplained-pro-embos -L board` **15/15** : `board.smoke_lsh` (flash, reset, `uname -a` = `lepton-cortexm0-32 4.10.0.2 … cortexM0p-samd21`, `ls`, `ps`) puis banc KAL |
| 7. Réseau | sans objet | — | pas de réseau sur cette carte (décision utilisateur 2026-10-05) |
| 8. Endurance | VERT | 2026-10-05 | session 2 : `tests/endurance_board.py` (sans réseau, `--fault-check v6m`), **1 h en `-Os`** (15:51-16:51, durée fixée par l'utilisateur), cycles `lsh` (uname -a, ps, ls /usr/sbin, cat /usr/etc/.boot, pwd) toutes les 30 s : **120 cycles**, aucun redémarrage, nombre de processus constant, `ICSR` 0 (aucune exception active), `lepton_embos_last_error` 0. Piles (`lepton-stacks`, marge depuis `heap_top`) : `lsh` 73,2 % (412 octets libres sur 1 536), `initd` 65,4 %, `kernel_thread` 43,6 %, MSP 62,8 % (572 octets libres) ; `stack=` du mkconf conservés |
| 9. Optimisation | VERT (de fait) | 2026-10-05 | `-Os -g` dès la session 1 (décision 2026-10-02) : paliers 1-6 en `-Os` |

Session 1, piles après fumée et `ls`, `ps`, `cat`, `pwd`, `ls /dev` (`lepton-stacks`, marge depuis
`heap_top`) : `lsh` 72,7 % (420 octets libres sur 1 536), `initd` 65,4 %, `kernel_thread`
41,6 %, MSP 62,8 % (572 octets libres) ; tas de thread 396 octets par processus (3 tampons stdio
de 64 octets). Les commandes terminées (`ls`, `ps`…) ne sont pas mesurables après coup (pile rendue au tas) : leurs 1 280-1 536 octets n'ont pas fait défaut en 600 exécutions (endurance).

## Défauts trouvés et corrigés (session 1)

| Symptôme | Cause | Correction |
|---|---|---|
| Console muette ; cœur dans `OS_TerminateError` (retour de la fonction d'une tâche) | `initd` (pid 1) sort en erreur : son second `open("/dev/console", O_WRONLY)` échoue, la table des fichiers ouverts (8 entrées) n'en ayant plus qu'une libre (relevé `ofile_lst` : 7 occupées, dont la console du noyau, `/dev/ttys3` et `/dev/head` liés, l'entrée de `initd`, deux entrées d'inode 14 non identifiées) et l'ouverture d'un flux attaché en consommant manifestement plus d'une (non analysé plus avant) ; l'`exit` du pid 1 arrête la tâche noyau (`_kernel_syscall` < 0 → `_kernel_powerdown`, puis retour) | `openfiles max="12"` dans le mkconf de la carte (8 dans l'ancienne configuration SAMD20, 32 sur la F746) ; diagnostic par `_g_kernel_syscall_trace` et la table `ofile_lst` au point `_syscall_exit` |
| `kal.TSBRK` en échec (« allocation possible après libération ») | bloc fixe de 16 Ko plus grand que tout le tas du banc (16 000 octets) : aucun bloc alloué | taille de bloc = huitième de l'étendue du tas, assertions inchangées (décision utilisateur 2026-10-05) ; rejoué vert sur QEMU an386 (hard, soft), an500 (`ci/run.sh`) et sur la carte ; F429 et F746 non rejouées (cartes débranchées) |

## Écarts d'architecture corrigés (ETAPE-6 : défaut de l'étape 2)

- `cmake/kal/embos.cmake` : `embos_init_hw.c` choisi par `LEPTON_ISA` (était codé en dur
  `arch/armv7m`) ; `core-segger/arch/armv6m/embos_init_hw.c` : SysTick à 0x80 (2 bits de
  priorité : 0xC0 l'aurait mis au niveau de PendSV ; exemple embOS SAMD20 : `(1 << bits) - 2`).
- `tests/kal/CMakeLists.txt` : `kal_bench.c` désigné sans dépendre de l'ISA, branche `armv6m`
  (registres en Thumb-1 sous `tests/kal/arch/armv6m/`).
- `ajout-coeur.md` §3 et §4 complétés.

## Étape 7 — backend FreeRTOS (module 7.3, 2026-10-06) : écart accepté

Même carte, preset `samd21-xplained-pro-freertos` : FreeRTOS V11.3.0, port **`ARM_CM0`** (sans
MPU), `-Os -g`. Flash interne autorisée pour le module (décision 2026-10-06). Empreinte : text
101 684 / data 928 / bss 17 472 (embOS 97 056 / 924 / 13 008) : code +4,8 %, RAM statique +4,5 Ko ;
tas (`__heap_start__` → `__stack_limit__`) **12 840 o** contre 17 304 sous embOS.

| Palier | Statut | Preuve (FreeRTOS) |
|---|---|---|
| 5. Multitâche et signaux | VERT | banc KAL sur carte (`ctest -L kal -FS board_t0 -E board.smoke_lsh`) **14/14** : T1-T8, TICI, TCLK, TSBRK, IRQ, `HARNESS_FAIL` ; trame ARMv6-M du KAL validée (déroutement de signal, sauvegarde et restauration de contexte) |
| 6. Fumée canonique | **ÉCHEC — écart accepté** | démarrage jusqu'à `lsh`, mais aucune commande ne s'exécute (`uname -a`, `ls`, `ps` : retour à l'invite sans sortie) |
| 1-4, 8 | non faits | sans objet tant que `lsh` ne lance pas de commande |

Diagnostic (gdb, point d'arrêt sur l'échec de `_sbrk`) : `_sys_vfork` (`core-freertos/fork.c:146`)
alloue une copie de `kernel_pthread_t` (2 380 o ; 2 472 sous embOS) et ne trouve que 484 o
libres (2 020 o après réduction des piles idle et temporisateurs, ci-dessous). Une commande
demande ≈ 4 Ko au pic (copie du parent, pile du fils, TCB). Sous embOS, ≈ 6,5 Ko restent libres
après le démarrage. Écart de RAM statique FreeRTOS − embOS, par poste :

| Poste | embOS | FreeRTOS | Cause |
|---|---|---|---|
| `ofile_lst` (12 fichiers, 2 `kernel_sem_t` chacun) | 1 344 o | 3 072 o | sémaphore FreeRTOS = file générique (`StaticSemaphore_t`, 80 o) ; embOS `OS_CSEMA` 8 o |
| pile de la tâche des temporisateurs | — | 1 024 o (2 048 par défaut) | embOS : rappels dans l'interruption du tick, sur la MSP |
| listes de prêts `pxReadyTasksLists` | 104 o (`OS_Global`) | 640 o | une liste par priorité (32 × 20 o) ; embOS : une liste triée |
| pile de la tâche idle | — | 512 o (1 024 par défaut) | `OS_Idle` d'embOS tourne sur la MSP |
| divers (TCB idle et temporisateurs, file, globales) | — | ≈ 600 o | |

Mesures prises : piles idle 128 mots et temporisateurs 256 mots, propres à la carte
(`LEPTON_FREERTOS_CONFIG`, `cmake/boards/samd21-xplained-pro.cmake` ; pics relevés 88 o et
236 o) : insuffisant. **Décision utilisateur (2026-10-06) : écart accepté** — sur la SAMD21,
FreeRTOS est validé au niveau du KAL (banc), pas du système complet ; le preset reste construit
par `ci/run.sh`. Pistes non retenues : sémaphore `core-freertos` plus léger (≈ 1,7 Ko),
`configMAX_PRIORITIES` réduit, tampons stdio (64 o, 3 par processus) : gains insuffisants seuls.

Défaut corrigé : l'oracle de trame du banc (`tests/kal/arch/armv7m/backend/freertos/kal_bench_os.h`)
supposait la trame du port `ARM_CM3` (R4-R11 en tête) sans FPU ; le port `ARM_CM0` V11 range
EXC_RETURN en tête : T2 et TICI en échec, KAL non en cause. Variante ARMv6-M ajoutée.
