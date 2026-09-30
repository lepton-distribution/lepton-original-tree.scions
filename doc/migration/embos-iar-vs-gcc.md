# embOS : usages Lepton (IAR) comparés à l'API V5.20 GCC (étape 1, tâche 4)

Produit le 2026-09-30 par `tools/migration/embos_inventory.py` (données brutes :
`$LEPTON_BUILD/etape-1/kal/usages.csv`, `usage-summary.csv`, `tcb-fields.csv`). Référence
« attendue » : `kernel/core/ucore/embOSCXM4_518/inc/RTOS.h` (IAR) ; cible :
`$LEPTON_EMBOS_ROOT/Start/Inc/RTOS.h` (GCC 5.20.0.0) + symboles de `libosT7VHLDP.a`.
Chemins relatifs à `sys/root/src/`. « Actif » ci-dessous = hors zones gelées présumées
(ARM7/ARM9/AT91 legacy/M16C/win32/gnu32 ; le classement définitif est celui de la tâche 2).

## 1. Version embOS attendue par le code

| Indice | Constat |
|---|---|
| `kal.h` | branche `#if (OS_VERSION_GENERIC >= 51800u)` (champ `OS_Global_Counters`) + branches historiques 3.28h → 3.84 → 3.88 en commentaires |
| `ucore/` | 7 copies embOS : ARM7 3.60, ARM7-9 3.88a, CM3 3.84, CM4 3.86n, CM4 4.40, CM7 4.30, **CM4 5.18.3.1** (`embOSCXM4_518`, IAR, `os7m_tl__sp.a`) |
| Projets IAR | `tauon_8.40/9.50.ewp` (configuration `tauon-kernel-cortex-m4-debug`), `dev_stm32f4xx_8.40.ewp`, `stm32f4_usb_core_8.40.ewp`, `*stm32wlxx*`, `bsp_stm32wl55jci_nucleo` → `embOSCXM4_518`, `OS_LIBMODE_SP`, **FPU = none** ; M7 (`samv71`, `same70`) → `embOSCXM7_430` (4.30) |

**Version attendue : embOS Cortex-M IAR V5.18.3.1** (M4), 4.30 pour M7 (étape 6). Écart
5.18.3 → 5.20.0 : faible (§2).

## 2. Écart d'API 5.18.3 (IAR) → 5.20.0 (GCC)

Comparaison automatique des deux `RTOS.h` :

- Fonctions de 5.18.3 absentes de 5.20 : `OS_MPU_AssertPrivilegedState`,
  `OS_MPU_EnterPrivilegedState`, `OS_MPU_GetPrivilegedState` (non utilisées par Lepton).
- Macros absentes : `OS_ASSERT_INIT_CALLED`, `OS_DEBUG_HALT`, `OS_INIT_SYS_LOCKS`,
  `OS_IntEnterRegion`, `OS_MARK_OUTOF_ISR_SWITCH`, `OS_TRACE_ID_TIME_CVT_MSEC_TO_CYCL`
  (non utilisées).
- `OS_TASK` : `pWaitList` → `pMultiWaitList` ; type `OS_PRIO` → `OS_TASK_PRIO` (non utilisés).
- `OS_GLOBAL` : `MPUDebug` retiré. `OS_REGS_BASE`, `OS_REGS_BASE_FPU`, `OS_TIMER`,
  `OS_SEMAPHORE`, `OS_MUTEX` : identiques.
- **Les 40 fonctions effectivement atteintes par Lepton (après résolution des alias) ont la
  même signature (types) dans les deux versions.** Seule différence : `OS_EnterRegion` est une
  macro inline en 5.18 IAR et une fonction (`OS_EnterRegionFunc`) en 5.20 GCC — sans impact source.
- Le `RTOS.h` IAR est spécifique compilateur (`<intrinsics.h>`, `#pragma language=extended`,
  `__CORE__`) : il ne peut pas servir sous GCC ; le header GCC du paquet le remplace entièrement.

## 3. Usages Lepton et écarts

### 3.1 Volumétrie (zone active)

189 occurrences `OS_*`, 70 identifiants, 25 fichiers (total arbre hors `ucore/` : 410 / 92).

| Catégorie | Occ. | Fichiers | Statut en 5.20 GCC |
|---|---|---|---|
| Noms d'API pré-V5 (`OS_CreateTask`, `OS_Use`, `OS_WaitCSemaTimed`, `OS_SignalEvent`, `OS_CreateTimer`…) — 49 identifiants | 91 (+4 `OS_SignalEvent`) | 14 | **tous fournis** par les macros de compatibilité de 5.20 (ex. `OS_Use` → `OS_MUTEX_LockBlocked`, `OS_SignalEvent` → `OS_TASKEVENT_Set` avec inversion d'arguments gérée par la macro) |
| Types (`OS_TASK`, `OS_TIMER`, `OS_REGS`, `OS_REGS_BASE`, `OS_MAILBOX`) | 28 | 9 | présents |
| Macros (`OS_STACKPTR`, `OS_U32`, `OS_Time`, `OS_pCurrentTask`, `OS_VERSION_GENERIC`) | 23 | 3 | présentes (`OS_pCurrentTask` → `OS_Global.pCurrentTask`, `OS_Time` → `OS_Global.Time`) |
| Champs de trame (`OS_REG_PC`, `OS_Global_Counters`) | 4 | 1 | présents, mais voir E3 |
| Fonction interne `OS_MakeTaskReady` | 4 | 1 (`kal.h`) | exportée par la lib, **non déclarée** dans `RTOS.h` (5.18 comme 5.20) — voir E4 |
| Identifiants Lepton préfixés `OS_` (non embOS) : `OS_REGS_GENERIC` (alias local), `OS_FSYS` (`cpu.h`), `OS_DEBUG_*_BUFF_SZ` (`dev/arch/all/debug`), `OS_TID`/`OS_MUT` (`dev/arch/cortexm/stm32f1xx,f4xx` : types CMSIS-RTOS/Keil), `OS_SUPPORT_CLEANUP_ON_TERMINATE` (test `#ifndef`) | 35 | 10 | sans objet ; `OS_TID`/`OS_MUT` non définis nulle part → à traiter avec les pilotes (étape 4/5) |

Champs de TCB accédés (branche embOS de `kal.h`, 1 fichier) : `pStack` ×15, `Timeout` ×3,
`Stat` ×3, `pNext` ×2, `pPrev` ×2 ; trame de pile via `OS_REGS_BASE*` : `OS_REG_PC` ×2,
`OS_Global_Counters` ×1 (≥ 5.18), `Counters` ×2 (< 5.18, branche morte). `core-segger/*.c`
n'accède à aucun champ directement (seulement via les macros `kal.h`).

### 3.2 Écarts bloquants ou sémantiques

| # | Écart | Occurrences / fichiers | Effort |
|---|---|---|---|
| E1 | **La branche embOS de `kal.h` n'est active que pour IAR/Keil** (`__tauon_compiler__ == __compiler_iar_arm__ \|\| __compiler_keil_arm__`). Sous GCC + embOS + Cortex-M, le préprocesseur tombe dans la branche `#else` générique (`typedef CONTEXT context_t`, non défini) → échec de compilation. Il faut une branche embOS neutre compilateur (et à terme `kal/arch/armv7m/`). | 1 condition, `kal.h` l.1083-1089 | faible (sémantique, petite) |
| E2 | **Précédence erronée** dans la même condition : `\|\| (__tauon_cpu_core__ == …cortexM7__)` est hors du groupe `&&` → pour un M7, la branche embOS est prise quel que soit le compilateur **et le micro-noyau** (masque la branche FreeRTOS : impact étape 6/7). | `kal.h` l.1089 | faible |
| E3 | **Trame de pile FPU** : `__swap_signal_handler` écrit `OS_REG_PC` et `OS_Global_Counters` via un cast en `OS_REGS_BASE`. Avec les libs `V`/`VH` (FPU), une tâche ayant utilisé la FPU a une trame `OS_REGS_BASE_FPU` (S16–S31 insérés avant R0 : `OS_REG_PC` décalé de 64 octets) → détournement de signal corrompu. Configuration IAR actuelle sans FPU : problème latent. Correctif : tester `OS_REG_EXC_RETURN` (bit 4) ou utiliser l'union `OS_REGS` ; même problème pour la copie de contexte `__bckup/__rstr_context` (`sizeof(OS_REGS_BASE)`). | 3 macros, `kal.h` | moyen (+ test banc KAL signaux/vfork avec FPU) — **corrigé le 2026-09-30** (étape 3) : union `OS_REGS`, bit 4 d'EXC_RETURN ; banc KAL T1F/T4F/T6F/T7F |
| E4 | `OS_MakeTaskReady` : API interne non déclarée ; GCC ≥ 14 fait de la déclaration implicite une erreur. Prototype à déclarer côté Lepton. HYPOTHÈSE À VALIDER : `void OS_MakeTaskReady(OS_TASK*)` (signature non documentée ; à vérifier par désassemblage interdit par la SFL → demander à Segger ou remplacer par l'API publique `OS_TASK_Resume`/événements). | 3 appels actifs (`kal.h` l.1218, 1257 ; l.1234 branche < 5.18) | faible à moyen (dépend de la réponse) |
| E5 | Manipulation directe du TCB (`memcpy` de `OS_TASK` entier, restauration de `pNext`/`pPrev`, écriture de `Timeout`/`Stat`) : dépend de la disposition interne de `OS_TASK`, qui **varie selon le mode de lib** (`pPrev` absent sans round-robin, donc en `XR` ; `sName`, `StackSize`, `NumActivations`, `ExecTotal`, `Id`… selon `OS_SUPPORT_*`). Les `OS_LIBMODE_*` définis à la compilation Lepton doivent correspondre exactement à la lib liée, sinon `sizeof(OS_TASK)` diffère. Interdire `XR`. Valeurs de `Stat` (0 = prête) non documentées. | 6 macros `kal.h` (vfork, exec, signaux) | moyen ; risque fonctionnel (vfork/kill) → banc KAL |
| E6 | `KERNEL_STACK_SIZE` défini uniquement pour `__compiler_iar_arm__`/`keil`/`win32` → non défini sous GCC (erreur). | `core-segger/kernel.c` l.93-110 (idem `core-freertos/kernel.c`) | faible |
| E7 | Includes `"RTOS.H"` (majuscules) : échec sur système de fichiers sensible à la casse (fichier `RTOS.h`). | `kal.h` l.1091 (+ l.915 M16C gelé) ; `dev/arch/all/debug/dev_os_debug.c` utilise `RTOS.h` | trivial |
| E8 | `OS_LIBMODE_*` : défini aujourd'hui dans les `.ewp` (`OS_LIBMODE_SP`). À porter dans CMake (cohérent avec la lib choisie) ; `OS_Config.h` du paquet force `DP`/`R` selon `DEBUG`. | configuration | trivial |
| E9 | Fichiers Segger adaptés par Lepton dans `ucore/embOSCXM4_518` (`main.c` : `OS_IncDI`, `OS_InitKern`, `OS_InitHW`, `_start_kernel`, `OS_Start` ; `RTOSInit_STM32F4xx.c` ; `SEGGER_RTT_Syscalls_IAR.c`) : à remplacer par les versions GCC du paquet (`RTOSInit_STM32F4xx.c`, `OS_Syscalls.c`, `OS_ThreadSafe.c`) et un `main` Lepton. `OS_InitKern` → `OS_Init` (alias présent). Question de licence (voir `embos-inventaire.md` §5). | 3 fichiers | faible |
| E10 | Bibliothèque : IAR `os7m_tl__sp.a` (ARMv7-M, sans FPU, mode SP) ↔ GCC `libosT7LSP.a` (équivalent strict) ou `libosT7VHLSP.a`/`DP` (FPU hard, nécessite E3). | — | décision étape 2 |

Écarts non bloquants : signatures identiques ; noms pré-V5 toujours disponibles (migration vers
les noms V5 facultative, à faire par script si décidée : 91 occurrences / 14 fichiers actifs) ;
`lwip/ports/arm/sys_arch.c` (19 occ., mailbox/sémaphores) compile tel quel côté embOS.

### 3.3 Chiffrage global

| Lot | Contenu | Estimation |
|---|---|---|
| A | E1, E2, E6, E7, E8 : faire compiler `core-segger` + `kal.h` sous GCC | 0,5 à 1 j |
| B | E4, E9, E10 : édition de liens avec la lib 5.20, `main`/`RTOSInit` | 1 j (hors attente Segger pour E4) |
| C | E3, E5 : robustesse du contexte (FPU, disposition TCB), avec banc KAL (vfork, exec, kill) | 2 à 3 j |

HYPOTHÈSE À VALIDER : estimations sans compter la mise au point sur QEMU/carte.

## 4. Autres usages de l'API embOS dans Lepton

- `interrupt.h` (commun, branche `__KERNEL_UCORE_EMBOS`) : appels système par **événements de
  tâche** (`OS_SignalEvent`/`OS_WaitEvent`/`OS_ClearEvents` sur le TCB du thread noyau), et
  non par SVC ; `__kernel_get_timer_ticks()` = `OS_Time`.
- `kernel.h` : `__mk_syscall` = `__atomic_in` (`OS_EnterRegion`) + `__make_interrupt` +
  `__wait_ret_int` pour embOS et FreeRTOS ; le chemin SVC (`do_swi`) n'existe que pour eCos.
- `core-segger` : `kernel_sem.c` (sémaphores comptés `OS_CSEMA`), `kernel_pthread_mutex.c`
  (`OS_RSEMA` = mutex), `kernel_timer.c` / `core_rttimer.c` (`OS_TIMER`), `kernel_pthread.c`
  (`OS_CreateTask`, `OS_Terminate`, `OS_GetResourceOwner`).
- Pilotes : `dev/arch/all/debug/dev_os_debug.c` (embOSView/`OS_SendString`, `OS_SetRxCallback`)
  — dépend de la communication embOSView, sans intérêt sous Linux.
