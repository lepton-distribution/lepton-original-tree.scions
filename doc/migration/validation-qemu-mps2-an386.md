# Journal de validation — QEMU mps2-an386 (étape 3)

Machine : `qemu-system-arm -M mps2-an386` (QEMU 10.0.13), Cortex-M4, embOS-Classic V5.20.0.0
`libosT7LSP.a` (soft-float, mode SP), `arm-none-eabi-gcc` 14.2.1, `-Og -g` (Debug), newlib-nano.
Preset `qemu-mps2-an386-embos`, binaire `$LEPTON_BUILD/qemu-mps2-an386-embos/lepton.elf`
(code 285 Ko, RAM 32 Ko au 2026-09-30).

## Paliers (ordre ETAPE-3, arrêt au premier échec)

| Palier | Statut | Date | Preuve |
|---|---|---|---|
| 1. Reset → `main` | VERT | 2026-09-30 | gdb (`qemu -s -S`) : PC initial `Reset_Handler`, SP `0x20400000` (fin de SSRAM2), arrêt sur `main` |
| 2. `.data`, `.bss`, `.noinit` | VERT (indirect) | 2026-09-30 | amorçage complet et variables initialisées correctes (`SystemCoreClock`, tables) ; pas de test dédié de `.noinit` — à couvrir par le banc KAL (T0) |
| 3. Warmup du noyau | VERT | 2026-09-30 | gdb : `_start_kernel` → `_kernel_warmup_boot` atteint ; rootfs, cpufs `/usr` (image mklepton), pilotes, console `/dev/console` ; `ls /` : `dev kernel bin usr etc var mnt` |
| 4. Premier appel système tracé | VERT (partiel) | 2026-09-30 | gdb : `_kernel_syscall` → `_syscall_dup`, `_syscall_exit` (trace `_g_kernel_syscall_trace`) ; mécanisme embOS par événements (pas de SVC) ; trace pas à pas complète non archivée |
| 5. Multitâche | VERT (niveau KAL) | 2026-09-30 | `ps` : initd, lsh, ps (création de processus, attente, `exit`) ; banc KAL `ctest -L kal` : préemption par priorité, déroutement de signaux T4-T6, vfork T7, exec T8 ; signaux de bout en bout (T9-T11) : pseudo-binaires `bin` à écrire |
| 6. Fumée canonique | VERT | 2026-09-30 | `ctest -L smoke` : `uname -a` = `lepton-cortexm4-32 4.10.0.2 … cortexM4-qemu-mps2-an386`, `ls`, `ps`, UART1 (`echo … > /dev/ttys1`) |
| 7. Réseau | À FAIRE (3b) | | |

## Défauts trouvés et corrigés pendant la mise au point

| Symptôme | Cause | Correction |
|---|---|---|
| `OS_Error(OS_ERR_MUTEX_OWNER)` dès le premier appel système | verrou des appels système (mutex embOS) pris par l'appelant, rendu par la tâche noyau ; embOS 5.20 le refuse (DP) ou le laisse incohérent (SP) | sémaphore (`core-segger/kernel_syscall_lock.c`), décision 2026-09-30 |
| Noyau arrêté (`OS_TerminateError`) : processus 1 terminé (`ENOENT`) | `.boot` lançait `initd` sur `/dev/ttys0`, déjà pris par la console | `.boot` QEMU : `initd -i /dev/console -o /dev/console` (comme Olimex) |
| Blocage après la bannière d'initd | contrat `kernel_io` : l'écrivain attend l'interruption de fin d'émission ; pilote UART synchrone sans signalement | `dev_cmsdk_uart_x_write` : `__fire_io_int` de fin d'émission |

## Hypothèses validées / restant à valider

- VALIDÉE : UART CMSDK : interruptions RX sur les lignes 0-5/12 routées (réception de `lsh` fonctionnelle).
- VALIDÉE (fonctionnellement) : `OS_MakeTaskReady(OS_TASK*)` (E4) — exercée par T4-T8 ; signature toujours non documentée par Segger.
- CONSTAT : cadre de départ embOS 5.20 = `OS_REGS_BASE` + routine et retour `OS_StartTask` au-dessus (T2).
- À VALIDER : trame FPU (E3) — palier hard-float.
