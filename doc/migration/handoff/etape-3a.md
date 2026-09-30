# Handoff étape 3a (UART) → 3b (Ethernet)

État au 2026-09-30 : 3a **TERMINÉE**, validée par l'utilisateur le 2026-09-30. Paliers 1-6 verts (`validation-qemu-mps2-an386.md`), hard-float
(preset principal) et soft-float ; banc KAL T0-T8 + T1F/T4F/T6F/T7F verts ; `ci/run.sh` vert.

## Réponses aux prérequis de 3b (ETAPE-3 : « relecture du handoff 3a », tâche 5, palier 7)
- Socle : `source scripts/lepton-env.sh && cd "$LEPTON_TRUNK"` ; `ci/run.sh` vert (host 5/5 ;
  hard : fumée + 14 tests KAL ; soft : fumée + 10) ; à garder vert avant chaque commit.
- Presets : `qemu-mps2-an386-embos` = hard-float (`libosT7VHLSP.a`), principal ;
  `qemu-mps2-an386-embos-soft` = soft-float (`libosT7LSP.a`), non-régression. Le pilote LAN9118 se
  développe sur le preset principal.
- Carte : `cmake/boards/qemu-mps2-an386.cmake` (liste des sources BSP, mkconf, machine QEMU) ;
  adresses et IRQ dans le BSP `kernel/dev/bsp/qemu_mps2_an386/` (LAN9118 à `0x40200000`, IRQ à
  relever dans la doc AN386/QEMU : non encore définie dans le BSP).
- Modèle de pilote : `kernel/dev/arch/all/eth/dev_eth_dm9000a/` (contrôleur externe mappé mémoire) ;
  nouveau pilote attendu sous `kernel/dev/arch/all/eth/dev_eth_lan9118/`.
- Pile réseau : **aucune n'est encore compilée** (`kernel/net/{lwip,uip,uip2.5}` hors des listes
  CMake) ; choix de la pile du socle à faire en plan de session 3b (celle du mkconf de la carte de
  référence Olimex/`tauon-kernel-cortex-m4-debug`, `perimetre.md`).
- Contrat pilote → noyau : voir « Pièges » (fin d'opération signalée par `__fire_io_int`).
- Application : `sys/user/tauon-basic/etc/mkconf_tauon_basic_qemu_mps2_an386.xml` (+ `etc/qemu-mps2-an386/`,
  `.boot`) ; `ftpd` présent dans `src/bin/net/`, à ajouter au mkconf.
- Test : `tests/smoke_lsh.py` (transport série QEMU) ; QEMU réseau prévu
  `-nic user,model=lan9118,hostfwd=tcp::<port>-:21` (ETAPE-3 tâche 5).

## Décisions actées pendant 3a (MIGRATION-STATUS, 2026-09-30)
- soft-float d'abord, puis hard-float comme preset principal, soft conservé en CI.
- Frontière libc : newlib-nano noyau et démarrage ; API POSIX applicative = `lib/libc` Lepton.
- Démarrage et RTOSInit écrits pour Lepton ; du paquet Segger, seuls `RTOS.h` et `libos*.a`.
- `bin` = T9-T11 (pseudo-binaires POSIX, à écrire) ; embOS en mode SP ; verrou des appels système
  en sémaphore ; T2/T8 alignés sur Lepton ; T8 FPU = aucun contexte FPU hérité.
- Sécurité : registres FPU résiduels non effacés, à traiter avec l'isolation MPU (dette).

## Artefacts produits (manifeste)
| Fichier | Contenu, quand le lire |
|---|---|
| `validation-qemu-mps2-an386.md` | journal des paliers, défauts corrigés, hypothèses ; à compléter du palier 7 |
| `traces/palier4-appel-systeme.gdb`, `-hard.txt` | trace gdb du premier appel système (modèle pour tracer le pilote) |
| `scion/cmake/components/firmware.cmake` | mklepton de la carte, sbin/bin, `lepton.elf`, test `smoke.lsh` |
| `scion/cmake/cpu/cortex-m4f.cmake`, `cmake/kal/embos.cmake` | ABI flottante ↔ bibliothèque embOS |
| `scion/ld/common-cortexm.ld`, `mem_*.ld` | liens Cortex-M |
| `scion/sys/root/src/kernel/core/arch/cortexm/startup_armv7m.c` | vecteurs `IRQ<n>_Handler` faibles, CPACR |
| `scion/sys/root/src/kernel/core/core-segger/arch/armv7m/` | `main`, `OS_InitHW`, `OS_Error` |
| `scion/sys/root/src/kernel/core/core-segger/kernel_syscall_lock.c` | verrou des appels système |
| `scion/sys/root/src/kernel/core/kal.h` (branche embOS) | contexte, cadre FPU (E3), déroutement |
| `scion/sys/root/src/kernel/dev/arch/all/uart/dev_cmsdk_uart/`, `dev/bsp/qemu_mps2_an386/` | UART, BSP : modèle d'intégration d'un pilote |
| `scion/tests/kal/` | banc KAL (T1-T8, variantes FPU), harnais semihosting |
| `scion/tests/smoke_lsh.py`, `ci/run.sh` | fumée canonique, non-régression |

## Écarts au plan et pièges découverts
- Pas de SVC avec embOS : appel système = événement embOS vers la tâche noyau (écart ETAPE-3 t.2).
- E3 : avec FPU, cadre embOS étendu (208 octets) si la tâche a un contexte FPU actif ; `kal.h`
  corrigé (bit 4 d'EXC_RETURN). Tout code qui lit un cadre sauvegardé doit en tenir compte.
- En-têtes de la libc système interdits au code Lepton : `kernel/core/include/libc` seulement.
- Contrat pilote : fin d'émission signalée (`__fire_io_int` sur `owner_pthread_ptr_write`) ; même
  schéma attendu pour le pilote Ethernet (réception et émission).
- Console : `__KERNEL_DEV_TTY` est pris par `/dev/console` ; les processus utilisent `/dev/console`.
- `LINK_GROUP:RESCAN` à définir pour `CMAKE_SYSTEM_NAME Generic` (toolchain).
- Hook `lepton_guard.py` : pas de heredoc ni de `>` dans une commande quand le répertoire courant
  est le trunk (lu comme une redirection vers le trunk) ; passer par un script du scratchpad.
- `grep -r` ne suit pas les liens du trunk : `grep -R`, ou chercher dans le clone.
- `pkill -f qemu…` tue aussi le shell appelant : `pkill -x qemu-system-arm`.

## Incertain / non vérifié
- Palier 2 : `.noinit` sans test dédié (vert indirect).
- E4 `OS_MakeTaskReady(OS_TASK*)` : signature non documentée (fonctionnelle sur T4-T8).
- Retour au mode embOS DP possible depuis le verrou en sémaphore : non essayé.
- QEMU n'émule pas fidèlement le lazy stacking ni le timing : E3 à revérifier sur F439 (étape 5).

## Non transmis volontairement
- Détail de la mise au point UART et du banc KAL : historique git (`git log migration/etape-3`).
- Écarts embOS IAR/GCC complets : `embos-iar-vs-gcc.md` (E1-E10).
