# Cartographie du KAL, des backends et du code dépendant du matériel (étape 1, tâche 4)

Produit le 2026-09-30. Chemins relatifs à `sys/root/src/` sauf mention. Données :
`tools/migration/embos_inventory.py` → `$LEPTON_BUILD/etape-1/kal/{usages,tcb-fields,ifdef-hors-arch}.csv`.
Écarts d'API embOS détaillés dans `embos-iar-vs-gcc.md` ; paquet Segger dans `embos-inventaire.md`.
Le classement actif/différé/gelé indiqué ici est provisoire (tâche 2 fait foi).

## 1. Rôle réel du KAL

`kernel/core/kal.h` (2119 l.) n'est **pas** une couche d'abstraction à interface unique : c'est un
**dispatcher par préprocesseur** qui, pour chaque couple compilateur × micro-noyau × CPU,
définit les mêmes macros et types, **directement en termes des structures internes du
micro-noyau** (TCB, trame de pile). Il n'y a ni fichiers `kal_<backend>.c`, ni `kal/arch/`.

Branches de premier niveau (ordre du fichier) :

| Lignes | Condition | Micro-noyau | État |
|---|---|---|---|
| 53-292 | `__KERNEL_UCORE_ECOS && CPU_GNU32` | eCos synthétique | gelé présumé |
| 293-532 | `__GNUC__ && (CPU_ARM7\|CPU_ARM9)` | eCos ARM | gelé présumé |
| 533-666 | `CPU_GNU32 && USE_KERNEL_STATIC` | aucun (noyau statique mklepton) | **à conserver** (tâche 5, mklepton) |
| 667-911 | `CPU_WIN32` | embOS simulation Win32 | gelé |
| 912-1082 | `IAR && EMBOS && CPU_M16C62` | embOS M16C | gelé |
| **1083-1426** | `(IAR\|Keil) && EMBOS && (ARM7\|ARM9\|M3\|M4) \|\| M7` | **embOS ARM/Cortex-M** | **actif — ne s'active pas sous GCC** (E1) ; précédence erronée pour M7 (E2) |
| 1427-1597 | `__GNUC__ && CPU_CORTEXM && ECOS` | eCos Cortex-M (SVC) | gelé présumé |
| 1598-1944 | `(IAR\|Keil\|GCC) && FREERTOS && (ARM7…M7)` | FreeRTOS | base de l'étape 7 |
| 1945-2083 | `#else` | documentation du contrat (Doxygen), macros vides, `context_t = CONTEXT` | sert de spécification |
| 2085-2114 | `USE_DEBUG_KAL` | redirige vers les fonctions `_debug_*` de `kal.c` | Win32 seulement |

`kernel/core/kal.c` (169 l.) : uniquement les fonctions `_debug_*` sous `#ifdef CPU_WIN32`
(`GetThreadContext`…) → vide pour toute cible ARM ; gelé de fait.

### 1.1 Contrat du KAL (défini par la branche `#else`, implémenté par chaque branche)

| Élément | Sémantique | Utilisateurs (hors `kal.h`) |
|---|---|---|
| `tcb_t`, `thr_id_t`, `context_t`, `pthreadstart_routine_t` | TCB du micro-noyau ; contexte sauvegardé = copie du TCB + trame de registres sur la pile | `kernel_pthread.h` (champ `tcb`, `start_context`, `bckup_context`), `core-*/kernel_pthread.c`, `core-*/fork.c` |
| `__begin_pthread(n)` / `__end_pthread()` | prologue/épilogue d'une routine de tâche | `core-*/kernel_pthread.c` |
| `__is_thread_self(tcb)` | tâche courante ? (`OS_pCurrentTask`) | `core-*/kernel_pthread.c` |
| `_macro_stack_addr` | qualificatif des piles (`OS_STACKPTR`) | `core-*/kernel.c`, `net/*_core`, 1 pilote i2c |
| `__bckup_thread_start_context`, `__bckup_context`, `__rstr_context` | copie TCB + trame (`OS_REGS_BASE`) ; restauration en préservant `pNext/pPrev` | `core-*/process.c`, `core-*/fork.c` (vfork, exec) |
| `__bckup_stack`, `__rstr_stack` | sauvegarde/restauration de la portion de pile utilisée (malloc) | `core-*/fork.c`, `core-*/process.c` |
| `__swap_signal_handler(p, h)` | détourne le PC sauvegardé vers le gestionnaire de signal, remet `Timeout/Stat` à 0, rend la tâche prête (`OS_MakeTaskReady`) | `core-*/process.c` (kill) |
| `__exit_signal_handler(p)` | restaure le contexte après signal | aucun appel direct (via `__rstr_context`) |
| `__set_active_pthread(p)` | rend une tâche prête | `core-*/kernel.c`, `kernel_timer.c`, `process.c` |
| `__atomic_in/out` | verrou d'ordonnanceur (`OS_EnterRegion/LeaveRegion`) | 28 fichiers (noyau, VFS, lib/pthread) |
| `__stop_sched/__restart_sched` | coupe/relance l'IT du tick : **écriture directe de SysTick CTRL.TICKINT (0xE000E010)** pour Cortex-M | `core-*/kernel.c`, `process.c`, `syscall.c` |
| `__disable_interrupt_section_in/out` | section non interruptible (`OS_IncDI/OS_DecRI`) | 10 fichiers |
| `__va_list_copy` | copie de `va_list` (dépend ABI/compilateur) | `core-*/syscall.c`, `lib/libc/unistd/io.c`, `fs/vfs/vfs.c` |
| `__kernel_profiler_*`, `__io_profiler_*` | profilage (timers AT91 ; vide pour Cortex-M) | 10 fichiers |

Autres points d'entrée micro-noyau hors `kal.h` : `interrupt.h` (appels système par événements
de tâche, `OS_Time`), `kernel.h` (`__mk_syscall`), `kernel_pthread.h` (champ `OS_TASK* tcb` /
`freertos_tcb_t*`), `kernel_sem.h`, `kernel_pthread_mutex.h`, `kernel_timer.h`, `rttimer.h`,
`core_rttimer.h` (types `OS_CSEMA`, `OS_RSEMA`, `OS_TIMER`), `malloc.c` (FreeRTOS).

## 2. Backends

| Backend | Contenu | État |
|---|---|---|
| `core-segger/` (15 fichiers, 8490 l.) | `kernel.c` (thread noyau, boucle d'appels système), `syscall.c`, `process.c`, `fork.c`, `signal.c`, `kernel_pthread*.c`, `kernel_sem.c`, `kernel_timer.c`, `core_rttimer.c`, `kernel_object.c`, `kernel_elfloader.c`, `kernel_clock.c`, `heap.c` | **actif**. Ce n'est pas un adaptateur mince : c'est la moitié « processus » du noyau POSIX dupliquée par micro-noyau. Appels embOS directs : 59 occurrences dans 5 fichiers (`kernel_sem.c`, `kernel_pthread_mutex.c`, `kernel_timer.c`, `core_rttimer.c`, `kernel_pthread.c`), le reste passe par `kal.h`. Défaut GCC : `KERNEL_STACK_SIZE` défini seulement pour IAR/Keil/Win32 (E6). |
| `core-freertos/` (15 fichiers, 8095 l.) | mêmes fichiers, même découpage | **copie divergente** de `core-segger` (écarts : 291 lignes dans `syscall.c`, 180 `kernel_pthread.c`, 150 `kernel.c`…) ; 7 TODO dans `signal.c`. Dépend de `ucore/freeRTOS_8-0-0/.../kal_freertos.h` qui **recopie la structure privée `tskTCB`** de FreeRTOS (`freertos_tcb_t`, `pStack` en tête) → liée à une version exacte (8.0.0 rc2 ; 9.0.0 présent aussi). Ports FreeRTOS fournis : IAR (CM0/CM3/CM4F/CM7 r0p1, ARM9) et GCC **CM0 seulement** (8.0.0) → port GCC CM3/CM4F/CM7 à reprendre (FreeRTOS amont). Base réutilisable de l'étape 7, complétude à mesurer au banc KAL. |
| `core-generic/` (1 fichier) | `kernel_pthread_tsd.c` (données spécifiques de thread, POSIX) | indépendant du micro-noyau ; commun aux deux backends. |
| `kernel/core/arch/` | `win32/` (mkconf, disque image) ; `cortexm/` ne contient qu'un `.md` vide ; `kernel_mkconf.h` Cortex-M est **généré par mklepton** (inclus par `kernelconf.h`) | aucun code d'architecture Cortex-M dans `core/arch` |
| `ucore/` | copies vendored : 7 embOS, FreeRTOS 8.0.0 / 9.0.0, CMSIS 4 et 5 (dont startups IAR/Keil/GCC génériques ARMCMx) | lecture seule ; `embOSCXM4_518` remplacé par `$LEPTON_EMBOS_ROOT` |

## 3. Classement par axe de variation

Légende « Fourni » : **S** = fourni par Segger (paquet embOS, lib ou exemple BSP) ;
**L** = à porter/écrire par Lepton ; **S→L** = exemple Segger à adapter (question de licence,
`embos-inventaire.md` §5) ; **ST/ARM** = fichier fabricant (CMSIS).

### 3.1 ISA (Thumb-2 / ARMv7-M ; ARMv6-M pour M0)

| Élément | Où aujourd'hui | Fourni | Commentaire |
|---|---|---|---|
| Commutation de contexte (PendSV, sauvegarde R4-R11, EXC_RETURN) | lib embOS (`PendSV_Handler`, `OS_ARMv7M_ISR.o`) | S | rien à écrire pour embOS ; FreeRTOS : `port.c`/`portasm` à reprendre (étape 7) |
| SVC | non utilisé avec embOS (appels système par événements) ; `do_swi` eCos seulement | — | aucun portage SVC requis pour le socle |
| Trame de pile vue par Lepton (`OS_REGS_BASE`, `cpu_regs_t` FreeRTOS) | `kal.h` (commun) | L | dépend ISA + FPU ; à déplacer dans `kal/arch/armv7m` (et `armv6m`) ; offsets à générer (annexe RISC-V) |
| Accès SysTick pour `__stop_sched` (adresse 0xE000E010 codée en dur) | `kal.h` (commun) | L | hypothèse Cortex-M dans le code commun (interdit cible) |
| Sections critiques | `kal.h` → `OS_IncDI/OS_DecRI` (BASEPRI dans la lib) | S | la macro neutre `__lepton_disable_irq` reste à définir (`compiler.h`/arch) |
| Startup générique (`Reset_Handler`, copie .data/.bss, vecteurs système) | IAR : `ucore/cmsis/.../startup/iar/*.s` ; CMSIS 5 GCC : `ucore/cmsis-5/Device/ARM/ARMCMx/Source/GCC` | ST/ARM ou S→L | GCC : startup du BSP embOS ou CMSIS 5 ; séparer partie ISA (générique) et vecteurs carte |
| HardFault | `HardFaultHandler.S` + `SEGGER_HardFaultHandler.c` (ucore 518, IAR) | S→L | version GNU dans le paquet |
| `__va_list_copy` | `kal.h` | L | dépend de l'ABI (AAPCS : `va_copy` standard sous GCC) |
| Chargeur ELF (`kernel_elfloader.h`) | commun | L | 1 référence ARM ; à vérifier pour RISC-V |
| ARMv6-M (M0) : pas de BASEPRI, Thumb-1 | libs `libosT6*` ; `cpu_regs_t` M0 dans la branche FreeRTOS uniquement | S (embOS) / L (KAL) | la branche embOS de `kal.h` ne connaît pas M0 |

### 3.2 Cœur (M3 / M4F / M7 / M0)

| Élément | Où | Fourni | Commentaire |
|---|---|---|---|
| Sauvegarde FPU (lazy stacking, trame étendue) | lib `V`/`VH` | S | exige FPU activée au démarrage + `FPCCR.ASPEN/LSPEN` laissés à 1 (UM01039 §8.2) |
| Activation FPU (CPACR) | `system_stm32f4xx.c` (`__FPU_USED`) | ST | |
| Trame FPU dans le KAL (signaux, vfork) | `kal.h` | L | écart E3 |
| Choix de lib / flags `-mcpu -mfpu -mfloat-abi` | `.ewp` (IAR) | L | `cmake/cpu/*.cmake`, table dans `embos-inventaire.md` §3 |
| Cache M7 (I/D-cache, cohérence DMA) | pilotes SAMV7 (`dev/arch/cortexm/at91samv7x`, `at91/softpack-lib/samv71`) | L | étape 6 |
| Erratum M7 r0p1 837070 | lib `_837070` + `USE_ERRATUM_837070` ; FreeRTOS `portable/IAR/ARM_CM7/r0p1` | S | étape 6 |
| `KERNEL_STACK_SIZE` par cœur × compilateur | `core-segger/kernel.c`, `core-freertos/kernel.c` | L | `#if` de cœur hors arch ; à paramétrer par `cmake/cpu` |
| Identité de cœur `__tauon_cpu_core__` / `__tauon_cpu_device__` | `kernelconf.h` (42 `#if`), dérivée de `__CORE__` IAR ou `kernel_mkconf.h` généré | L | à remplacer par des définitions CMake (axes) |

### 3.3 Carte (horloges, UART, Ethernet, mémoire)

| Élément | Où | Fourni | Commentaire |
|---|---|---|---|
| Horloges / PLL | `ucore/cmsis/Device/st/stm32f4xx/system_stm32f4xx.c`, `dev/arch/cortexm/stm32f4xx/driverlib/system_stm32f4xx.*` | ST (BSP embOS F429 : 168 MHz, HSE 8 MHz) | HSE bypass NUCLEO à vérifier |
| Tick système (`OS_InitHW`, `SysTick_Handler`, `OS_Idle`) | `ucore/embOSCXM4_518/arch/cmsis/cpu/RTOSInit_STM32F4xx.c` (IAR) | S→L | version GCC du paquet (`RTOSInit_STM32F4xx.c` F429 Nucleo) ; QEMU : `RTOSInit_CMSIS.c` |
| `main` (`OS_IncDI`, `OS_InitKern`, `OS_InitHW`, `_start_kernel`, `OS_Start`) | `ucore/embOSCXM4_518/arch/cmsis/main.c` | L | à sortir de `ucore/` (fichier Lepton) |
| Vecteurs périphériques | startups IAR F4xx (dont `startup_stm32f439xx.s`) | ST | GCC : F429 du paquet (vecteur CRYP manquant) ou CMSIS ST F439 |
| Mémoire / édition de liens | `.icf` dans `sys/user/tauon-basic/prj/iar/arch/arm/*` | S→L | `ld/mem_*` + `ld/common-*.ld` (étape 2) ; symboles `__stack_start__/__stack_end__` requis par la lib |
| UART, SPI, I2C, SDIO, DAC, Flash, CPU, Ethernet (SPL + CubeMX HAL) | `dev/arch/cortexm/stm32f4xx/` (291 fichiers) | L | actif ; `uart.h` utilise un type `OS_TID` non défini (CMSIS-RTOS/Keil) |
| Cartes (brochage, périphériques de carte) | `dev/bsp/{discovery_f4, discovery_f4-baseboard-modem, olimex_p407, stm32f469i-eval}` | L | base de la NUCLEO-F439ZI (aucun BSP NUCLEO existant) |
| Autres cartes / familles | `dev/bsp/{samd20xplained_pro, same70xplained, samv71xplained_ultra, stm32wl55jci_nucleo}`, `dev/arch/cortexm/{stm32f1xx, stm32wlxx, at91samv7x, at91samd20, stellaris, k60n512}`, `dev/arch/at91/{asf,softpack-lib,…}`, `dev/arch/cmsis/` | L | différé (étape 6) ou abandonné — tâche 2 |
| ARM7/ARM9/M16C, `gnu32`, `win32` | `dev/arch/{arm7,arm9,gnu32,win32}`, `dev/arch/at91/at91lib/boards/at91sam9*` | — | gelé |
| QEMU `mps2-an386` (UART CMSDK, LAN9118) | inexistant | L | nouveaux pilotes (étape 3) |

### 3.4 Micro-noyau

| Élément | Où | Fourni |
|---|---|---|
| Noyau embOS (ordonnanceur, objets, tick) | lib `libosT*.a` + `RTOS.h` | S |
| Adaptation Lepton ↔ embOS | branche embOS de `kal.h`, `core-segger/`, `interrupt.h`, `kernel.h` (`__mk_syscall`), types dans `kernel_*.h` | L |
| Idem FreeRTOS | branche FreeRTOS de `kal.h`, `core-freertos/`, `ucore/freeRTOS_*` (dont `kal_freertos.h`) | L (FreeRTOS amont pour le noyau) |
| Port lwIP (`sys_arch`) | `net/lwip/ports/arm/` (embOS) ; `ports/{gnu,m16c,win32}` gelés | L |
| embOSView / debug | `dev/arch/all/debug/dev_os_debug.*` | L (abandonnable) |

## 4. `#if` d'ISA/cœur hors des répertoires d'architecture (interdits cible)

Détection : `#if/#ifdef/#elif` testant `__tauon_cpu_core__`, `__tauon_cpu_device__`, `CPU_*`,
`__CORE__`, `__ARM*__`, `__FPU_*`, `__ARM_ARCH*`, `__thumb*`, hors `kernel/dev/arch/`,
`kernel/core/arch/`, `kernel/core/ucore/`. **161 lignes dans 34 fichiers**
(`ifdef-hors-arch.csv`) ; beaucoup ne concernent que des cibles gelées (`CPU_WIN32`, `CPU_GNU32`,
`CPU_M16C62`, ARM7/9) et disparaîtront avec elles.

| Fichier | Lignes | Jetons | À traiter |
|---|---|---|---|
| `kernel/core/kernelconf.h` | 42 | `__CORE__`, `__ARM6M__/__ARM7M__/__ARM7EM__`, `__tauon_cpu_*`, `CPU_*` | oui : sélection cœur/carte → CMake |
| `kernel/core/kal.h` | 36 | `__tauon_cpu_core__`, `__tauon_cpu_device__`, `CPU_*` | oui : éclater en `kal/arch/<famille>/` + dispatcher |
| `kernel/core/core-freertos/kernel.c` | 10 | `__tauon_cpu_core__` | oui (taille de pile) |
| `kernel/core/core-segger/kernel.c` | 8 | `__tauon_cpu_core__` | oui (taille de pile) |
| `kernel/core/kal.c` | 7 | `CPU_WIN32` | gelé |
| `kernel/net/lwip/ports/gnu/lwipopts.h` | 6 | `CPU_CORTEXM` | port gelé présumé |
| `kernel/core/kernel.h` | 5 | `CPU_ARM7/9`, `CPU_CORTEXM`, `CPU_GNU32` | oui (branches eCos/SVC) |
| `kernel/core/kernel_pthread.h` | 4 | `CPU_ARM7/9`, `CPU_CORTEXM`, `CPU_WIN32` | oui (`KERNEL_STACK`) |
| `lib/libc/stdio/stdio.h` | 4 | `CPU_*` | oui |
| `kernel/core/interrupt.h` | 3 | `CPU_ARM7/9`, `CPU_CORTEXM`, `CPU_GNU32` | oui |
| `kernel/fs/vfs/vfstypes.h` | 2 | `CPU_CORTEXM` | oui |
| `kernel/core/timer.h` | 1 | `CPU_CORTEXM`, `CPU_GNU32` | oui |
| `kernel/fs/rootfs/rootfscore.h` | 1 | `__tauon_cpu_device__` | oui |
| `kernel/fs/ufs/ufs.c`, `ufsx.c`, `fat/fat16.c`, `fat/fatcore.h`, `rootfs/rootfscore.c`, `net/lwip_core/ethif_core.c`, `net/uip_core/uip_core.c`, `sbin/{xmodem,lsh,initd}.c`, `core/{process,system,types,kernel_sem}.h`, `core-*/{process,kernel_object,kernel_pthread,kernel_pthread_mutex}.c`, `lib/libc/ctype/ctype.h` | 1-3 chacun | `CPU_WIN32/GNU32/M16C62/ARM7/ARM9` | nettoyage au gel (étape 6) ; vérifier ceux qui touchent au format UFS (tâche 5) |

S'y ajoutent des **adresses matérielles dans le code commun** : SysTick `0xE000E010` et PIT AT91
dans `kal.h` ; timers AT91 des profileurs dans `kal.h`.

## 5. Vérification des exigences de l'annexe RISC-V (état actuel)

| Exigence (annexe) | État actuel | Écart |
|---|---|---|
| Famille ajoutée par `kal/arch/<famille>/` + une ligne du dispatcher `kal.h` | `kal.h` monolithique, conditions compilateur × cœur × device ; aucun `kal/arch/` | **non satisfaite** : restructuration à l'étape 2/4 |
| `cmake/toolchains`, `cmake/cpu`, `cmake/boards`, `ld/mem_*` | inexistants ; cœur/carte déduits de `__CORE__` IAR et de `kernel_mkconf.h` généré | à créer (étape 2) |
| `ld/common-*.ld` séparables de `.ARM.exidx` | pas de `.ld` Lepton (seulement `.icf` IAR) | à créer |
| Aucune hypothèse Cortex-M dans le commun (NVIC, SVC, PendSV, SysTick, CMSIS) | SysTick codé en dur dans `kal.h` ; `OS_REGS_BASE` (trame ARM) dans `kal.h` ; `kernelconf.h` teste `__ARM7EM__` | **non satisfaite** (3 zones) |
| Sections critiques par macros neutres | `__disable_interrupt_section_in/out` et `__atomic_in/out` sont déjà neutres dans leur nom, mais implémentés dans `kal.h` par micro-noyau ; pas de `__lepton_disable_irq` | partiel |
| Offsets de contexte générés (`asm-offsets`) | trame via structures C du micro-noyau (`OS_REGS_BASE`, `cpu_regs_t` recopiée à la main pour FreeRTOS) | **non satisfaite** pour FreeRTOS ; embOS : offsets fournis par `RTOS.h` (acceptable) |
| Banc KAL paramétré par machine QEMU | banc inexistant | étape 3 |
| Paquet embOS RISC-V | **absent** de `third_party/embos/` | à fournir |

Point favorable : avec embOS, Lepton n'a **aucun assembleur propre** pour la commutation de
contexte ni pour les appels système (événements de tâche) ; un nouveau cœur se réduit, côté
Lepton, à la trame de pile vue par le KAL, au démarrage, au tick et aux pilotes.
