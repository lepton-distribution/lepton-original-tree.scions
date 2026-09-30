# Handoff intermédiaire 3a (UART) — reprise de session

État au 2026-09-30 : **3a EN COURS**. Paliers 1-3, 5 (KAL) et 6 verts, 4 partiel
(`validation-qemu-mps2-an386.md`). Banc KAL T0-T8 vert (M4 soft-float), `ci/run.sh` complet
vert. Reste pour clore 3a : palier hard-float (E3), puis handoff final 3a.

## Reprendre ici
1. `source scripts/lepton-env.sh && cd "$LEPTON_TRUNK"` ; `ci/run.sh` (banc KAL compris) doit être vert.
2. FAIT — banc KAL (`tests/kal/`, `ctest -L kal`). Pour mémoire, conception : harnais
   `tests/kal/kal_test.h` (semihosting `SYS_EXIT`, `qemu -semihosting`), firmware de test par
   preset (même `lepton_kernel`, `main` de test à la place de `embos_main.c`), macros de
   `kal.h` branche embOS (l. ~1100-1270 : `__inline_bckup_context`, `__inline_rstr_context`,
   `__inline_bckup_stack`/`__inline_rstr_stack`, `__inline_swap_signal_handler`,
   `__inline_exit_signal_handler`, `__set_active_pthread`).
3. Palier hard-float : `LEPTON_FLOAT_ABI=hard` (preset), `libosT7VHLSP.a`, puis E3 (trame
   `OS_REGS_BASE_FPU` : `OS_REG_PC` décalé de 64 octets si la tâche a utilisé la FPU).

## Décisions actées pendant 3a (MIGRATION-STATUS)
- soft-float d'abord ; frontière libc (newlib noyau / Lepton appli) ; démarrage et RTOSInit
  Lepton ; `bin` = T9-T11 ; embOS en mode SP (Debug) ; verrou des appels système en sémaphore.

## Artefacts (manifeste)
| Fichier | Contenu |
|---|---|
| `scion/cmake/components/firmware.cmake` | mklepton de la carte, sbin/bin du mkconf, `lepton.elf`, test `smoke.lsh` |
| `scion/ld/common-cortexm.ld`, `mem_*.ld` | liens Cortex-M (alias de régions, symboles de pile embOS) |
| `scion/sys/root/src/kernel/core/arch/cortexm/startup_armv7m.c` | vecteurs `IRQ<n>_Handler` faibles |
| `scion/sys/root/src/kernel/core/core-segger/arch/armv7m/` | `main`, `OS_InitHW`, `OS_Error`, `OS_JLINKMEM_BufferSize` |
| `scion/sys/root/src/kernel/core/core-segger/kernel_syscall_lock.c` | verrou des appels système |
| `scion/sys/root/src/kernel/dev/arch/all/uart/dev_cmsdk_uart/`, `dev/bsp/qemu_mps2_an386/` | UART, BSP |
| `scion/sys/user/tauon-basic/etc/mkconf_tauon_basic_qemu_mps2_an386.xml` + `etc/qemu-mps2-an386/` | application du socle |
| `scion/tests/smoke_lsh.py`, `ci/run.sh` | fumée canonique, non-régression |

## Pièges découverts
- Pas de SVC avec embOS : appel système = événement embOS vers la tâche noyau (écart ETAPE-3 t.2).
- En-têtes de la libc système interdits au code Lepton (types en conflit avec newlib) :
  `kernel/core/include/libc` seulement (`lepton_freestanding`).
- Contrat pilote série : fin d'émission signalée (`__fire_io_int` sur `owner_pthread_ptr_write`).
- Console : `__KERNEL_DEV_TTY` est pris par `/dev/console` ; les processus utilisent `/dev/console`.
- `LINK_GROUP:RESCAN` à définir pour `CMAKE_SYSTEM_NAME Generic` (toolchain).
- Hook : pas de heredoc ni de `>` dans une commande si le cwd de session est le trunk.
