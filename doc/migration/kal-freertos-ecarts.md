# Écarts KAL embOS → FreeRTOS (étape 7, tâche 1)

État au 2026-10-06 : analyse (tâche 1), puis implémentation du module 7.1 (QEMU `mps2-an386`,
§5). Référence : FreeRTOS
202604 LTS, noyau **V11.3.0** (décision 2026-10-06), archive `FreeRTOS-Kernel` tag `V11.3.0`,
SHA-256 `76530a6bab55233e34e07c8df59f0d4c2e06db473763f8d33277d4fa10950084` (calculé au
téléchargement ; GitHub ne publie pas d'empreinte pour les archives de tag). Licence MIT.

## 1. Constat sur `core-freertos/`

| Mesure (lignes de `diff -w`) | Valeur |
|---|---|
| `core-freertos` vs `core-segger` d'origine (`037cd59`) | ~1 200 lignes (dont `syscall.c` 291, `kernel_pthread.c` 180, `kernel.c` 150, `process.c` 119, `kernel_pthread_mutex.c` 122, `kernel_timer.c` 90) |
| `core-segger` d'origine → actuel (étapes 3-6) | ~80 lignes (`kernel.c` 32, `kernel_pthread.c` 31, `kernel_elfloader.c` 8) + `kernel_syscall_lock.c` nouveau |

L'écart d'origine n'est **pas** propre au micro-noyau : `core-freertos` est une **lignée plus
ancienne** du noyau (en-têtes d'auteur différents ; il manque notamment `pthread_kill` en
diffusion `PTHREAD_ID_BROADCAST[_EXCEPT]`, le réveil des `pthread_join`, `_syscall_realloc`, la
correction `fcntl_dt->ret`, l'`ioctl` par descripteur lié). En le reprenant, on régresserait le
noyau par rapport à `core-segger`, validé de l'étape 3 à l'étape 6. `core-freertos` n'a été
touché par aucune règle de l'étape 4 (seul commit : `037cd59`).

Fichiers de `core-segger` qui dépendent réellement d'embOS (`#ifdef __KERNEL_UCORE_EMBOS`) :

| Fichier | Dépendance | Équivalent `core-freertos` |
|---|---|---|
| `kernel_pthread.c` (2 blocs) | `OS_CreateTask`, `OS_Terminate` | `xTaskCreate…` (à refaire, voir §3) |
| `kernel_sem.c` | `OS_*CSema*` | oui (sémaphore à compteur) |
| `kernel_pthread_mutex.c` | `OS_*RSema`, `OS_GetResourceOwner` | oui (mutex récursif) |
| `kernel_timer.c`, `core_rttimer.c` | `OS_*Timer`, `OS_GetpCurrentTimer` | oui (`xTimer*`) |
| `arch/<isa>/embos_main.c`, `embos_init_hw.c` | `OS_Init`, `OS_InitHW`, `SysTick_Handler`, `OS_Idle`, `OS_Error` | aucun (à écrire) |

Tous les autres fichiers (`kernel.c`, `process.c`, `syscall.c`, `fork.c`, `signal.c`,
`kernel_object.c`, `kernel_clock.c`, `kernel_sigqueue.c`, `kernel_elfloader.c`, `heap.c`,
`kernel_syscall_lock.c`) ne passent que par le KAL : ils sont indépendants du micro-noyau.

En-têtes communs : `kernel_sem.h`, `kernel_pthread_mutex.h`, `kernel_timer.h`, `rttimer.h`,
`core_rttimer.h`, `kernel_pthread.h`, `interrupt.h` et `malloc.c` contiennent **déjà** une branche
`__KERNEL_UCORE_FREERTOS`, à vérifier contre la V11 (types `xTaskHandle`, `portTICK_RATE_MS`
obsolètes : `configENABLE_BACKWARD_COMPATIBILITY` ou mise à jour).

Hors du noyau : `kernel/net/lwip/ports/arm/sys_arch.c` (portage lwIP actif) est **propre à embOS**
(boîtes aux lettres `OS_*MB`, `OS_*CSema`, `OS_GetTime`). Le palier réseau exige un `sys_arch`
FreeRTOS. Inactifs (aucun preset) : `dev_os_debug.c`, pilotes LCD, `uip_core`.

## 2. Macros du KAL

| Macro | embOS (actuel) | FreeRTOS V11.3.0 | Accès |
|---|---|---|---|
| `tcb_t` | `OS_TASK` alloué par Lepton | `StaticTask_t` alloué par Lepton, `xTaskCreateStatic` (`configSUPPORT_STATIC_ALLOCATION=1`) | public |
| `__is_thread_self` | `== OS_pCurrentTask` | `== xTaskGetCurrentTaskHandle()` | public |
| lien pthread ↔ tâche | `thread->tcb` | idem (handle = adresse du `StaticTask_t`) ; inverse si besoin : `vTaskSetThreadLocalStoragePointer` | public |
| `__bckup_context` / `__rstr_context` | copie de tout l'`OS_TASK` (liens préservés) + trame `OS_REGS` | **ne pas copier le TCB** (listes, priorité, notifications, mutex détenus) : sauvegarder `pxTopOfStack` + trame | **interne I1** : `pxTopOfStack` = premier membre du TCB (garanti par l'ABI des ports, lu par `PendSV`) |
| trame de contexte | `OS_REGS_BASE[_FPU]`, E3 | CM3 et CM0 : R4-R11 puis trame matérielle ; CM4F et CM7 : R4-R11, EXC_RETURN, [S16-S31 si bit 4 d'EXC_RETURN à 0], trame matérielle [S0-S15, FPSCR] | **interne I2** : défini par `port.c` (`xPortPendSVHandler`), pas par un en-tête ; `cpu_regs_t` à décrire dans `kal/backend/freertos` par ISA, contrôlé par `_Static_assert` sur `pxPortInitialiseStack` |
| `__bckup_stack` / `__rstr_stack` | `start_context.os_task.pStack` | `pxTopOfStack` sauvegardé (même algorithme) | I1 |
| `__swap_signal_handler` | PC et xPSR (ICI/IT) de la trame, `OS_MakeTaskReady` | même modification de trame (I2) ; réveil par `xTaskAbortDelay` (tâche bloquée, attente infinie comprise depuis V10) ou `vTaskResume` (suspendue) | public ; **HYPOTHÈSE À VALIDER** (banc T2/T3) : `xTaskAbortDelay` sur attente infinie de sémaphore |
| `__exit_signal_handler` | `__rstr_context(bckup)` | idem (I1, I2) | |
| `__set_active_pthread` | `OS_MakeTaskReady` | `xTaskAbortDelay` / `vTaskResume` selon `eTaskGetState` | public |
| `__atomic_in` / `__atomic_out` | `OS_EnterRegion` / `OS_LeaveRegion` | `vTaskSuspendAll` / `xTaskResumeAll` | public |
| `__disable_interrupt_section_in/out` | `OS_IncDI` / `OS_DecRI` | `taskENTER_CRITICAL` / `taskEXIT_CRITICAL` (BASEPRI sur ARMv7-M, PRIMASK sur ARMv6-M) | public |
| `__stop_sched` / `__restart_sched` | non définies pour embOS (vides) | branche existante : arrêt de SysTick (`TICKINT`) — **adresse SysTick hors `kal/arch`** : à déplacer dans `kal/arch/<isa>` ou remplacer par `vTaskSuspendAll` | à décider au plan |
| `__hw_enter/leave_interrupt` | `OS_EnterInterrupt` | branche existante (`portYIELD_FROM_ISR`) | public |

## 3. Points à trancher

- **Priorités** : la « convention inversée » citée par le fichier d'étape est **inexacte**. Dans
  embOS comme dans FreeRTOS, une valeur plus grande donne une priorité plus haute. L'écart réel
  est l'**étendue** : Lepton utilise 0-255 (`KERNEL_PRIORITY` 150), alors que FreeRTOS a
  `configMAX_PRIORITIES` (≤ 32 avec la sélection optimisée). Il faut une projection monotone
  dans le KAL, à documenter. Autre écart : embOS a un timeslice par tâche
  (`attr.timeslice`), FreeRTOS un découpage global d'un tick (`configUSE_TIME_SLICING`).
- **SVC / PendSV / SysTick** : Lepton n'utilise **aucun** SVC (appels système par sémaphore et
  tâche noyau). Le port FreeRTOS prend donc les trois exceptions sans conflit :
  `vPortSVCHandler` (`svc 0`, démarrage), `xPortPendSVHandler`, `xPortSysTickHandler`. Les
  ports V11 vérifient le routage direct par `configASSERT` sur la table des vecteurs : les
  noms `SVC_Handler`/`PendSV_Handler`/`SysTick_Handler` des démarrages Lepton sont à associer
  par `#define` dans `FreeRTOSConfig.h`.
- **Tick** : 1 kHz (`__KERNEL_CLK_TCK=1000`, identique à embOS) ; `_SC_CLK_TCK` inchangé.
- **Pilotes en interruption** : `configMAX_SYSCALL_INTERRUPT_PRIORITY` doit couvrir les priorités
  des IRQ qui appellent l'API (UART, Ethernet) ; `configASSERT` actif le vérifie
  (`vPortValidateInterruptPriority`).
- **ARMv6-M** (SAMD21) : port `ARM_CM0` (pas de BASEPRI), trame R8-R11/R4-R7 à vérifier (I2).
  **M4 sans FPU** (WL55) : port `ARM_CM3`. **M7 r0p1** (F746) : `ARM_CM7/r0p1`. M4F, M7 (an500) :
  `ARM_CM4F`.
- **FPU** : FreeRTOS fait le lazy stacking par EXC_RETURN comme embOS (E3) ; le constat de sécurité
  (registres FPU non effacés) s'applique de la même manière.
- **`kal_freertos.h`** (copie de `tskTCB` 8.0.0) : à abandonner ; les seuls accès internes
  retenus sont I1 et I2.
- **Mémoire** : `heap_*.c` FreeRTOS inutile si tout est alloué statiquement
  (`configSUPPORT_DYNAMIC_ALLOCATION=0`) ; les temporisateurs logiciels demandent une tâche de
  service (`configUSE_TIMERS`), en plus des tâches embOS : empreinte à comparer.

## 4. Accès internes retenus

| Id | Accès | Justification | Garde |
|---|---|---|---|
| I1 | `*(StackType_t**)handle` = `pxTopOfStack` | vfork, exec et signaux reposent sur la pile sauvegardée ; premier membre du TCB, exigé par tous les ports | `_Static_assert(offsetof…)` impossible (TCB privé) → test de banc T1 |
| I2 | trame sauvegardée par `xPortPendSVHandler` | redirection de PC, copie de contexte | `_Static_assert` sur la taille ; test d'unité hôte comparant `pxPortInitialiseStack` à `cpu_regs_t` |

## 5. Constats de l'implémentation (module 7.1, QEMU `mps2-an386`)

| Point | Constat | Traitement |
|---|---|---|
| Région atomique | `kernel_io_write` appelle le pilote dans `__atomic_in` ; le pilote socket (lwIP) y attend ses mutex et sémaphores. embOS laisse une tâche bloquer dans `OS_EnterRegion` ; FreeRTOS interdit de bloquer ordonnanceur suspendu (`configASSERT`, `queue.c:1682`, révélé par `ftpd`) | `kal_freertos.c` : toute attente bloquante du backend (`__kal_frt_block`) relâche les suspensions de la tâche courante, puis les rétablit au réveil (sémantique embOS) |
| `__wait_ret_int` | réveillée par `xTaskAbortDelay` (signal), la tâche ressortait de `xEventGroupWaitBits` sans le bit | boucle jusqu'au bit (comme `OS_WaitEvent`) ; `KERNEL_RET` effacé dans `__make_interrupt` (comme `OS_ClearEvents`) |
| Verrou des appels système | `kernel_mutex` rendu par la tâche noyau : interdit sous FreeRTOS aussi (`xTaskPriorityDisinherit`) | sémaphore, comme embOS (`kernel_syscall_lock.c`, `kernel.h`) |
| Mutex | ancienne version : sémaphore binaire (ni récursif, ni propriétaire) | mutex récursif statique, comme `OS_RSEMA` ; `owner_destroy` non portable (inutilisé) |
| Temporisateurs | `tmr_t` différent entre `rttimer.h` (handle) et `core_rttimer.h` (structure) : écriture hors de `kernel_tmr` ; `xTimerChangePeriod` démarre le temporisateur, période nulle = `configASSERT` ; temps restant non implémenté | `tmr_t` unique, trampoline `void(void)` ; période nulle = arrêt ; `xTimerGetExpiryTime` |
| Rappels de temporisateur | embOS : contexte du tick ; FreeRTOS : tâche de service | tâche de service à `configMAX_PRIORITIES-1`, au-dessus de toutes les tâches Lepton |
| Priorités | aucune inversion (§3) | `__kal_priority` : 0-255 → [1, 30] ; « freeRTOS temporary patch » (priorité 4) retiré de `process.c` |
| Pile | Lepton utilise le bas de pile comme tas (`kernel_pthread_alloca`) : il recouvrait la zone de contrôle de FreeRTOS | 20 octets réservés (`KERNEL_PTHREAD_STACK_GUARD`) |
| Interruptions | `xSemaphoreGiveFromISR` ne remet jamais l'indicateur à `pdFALSE` | initialisé à chaque appel, cumulé dans `kernel_in_interrupt_higher_priority_task_woken` |
| TCB (module 7.2) | intégré d'abord à `kernel_pthread_t` ; le noyau copie et efface des `kernel_pthread_t` entiers (`fork.c`, `process.c`) : listes de FreeRTOS corrompues (HardFault, `pxCurrentTCB` NULL ; réveils perdus), sous charge seulement | TCB et groupe d'événements alloués à part (`thread->tcb`), comme `OS_TASK` |
| Mémoire (module 7.2) | mémoire statique propre à FreeRTOS (piles idle 1 Ko et temporisateurs 2 Ko, listes de prêts 640 o, TCB, file) en SRAM : tas newlib −4,7 Ko, session FTP en échec sur la F439 | placée en `.ccm_bss` (`ld/common-cortexm.ld`) ; tas au niveau embOS |
| Hypothèse `xTaskAbortDelay` | — | **validée** : T4, T6, T7 et leurs variantes FPU réveillent une cible en attente infinie (groupe d'événements) |

Timeslice par tâche (embOS) : non transposable, tourniquet global d'un tick (`configUSE_TIME_SLICING`).
