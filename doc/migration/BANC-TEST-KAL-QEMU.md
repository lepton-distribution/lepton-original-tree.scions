# Banc transversal — Tests unitaires du KAL sous QEMU (architecture × micro-noyau)

## Contexte

Les macros du KAL (sauvegarde/restitution de contexte, manipulation des TCB) sont le socle des fonctionnalités POSIX de Lepton : génétique de processus (`vfork`), recouvrement (`execv`, `execl`), déroutement du flux d'exécution (`kill`, `raise`, sighandlers). Une erreur dans ces macros produit des corruptions différées difficiles à diagnostiquer au niveau applicatif. Ce banc les teste **unitairement**, sous QEMU, pour chaque combinaison architecture × backend micro-noyau.

Ce fichier n'est pas une étape séquentielle : le banc est construit à l'étape 3 (socle QEMU `mps2-an386`, premier binaire GCC), rejoué après chaque lot de l'étape 4, puis exigé aux étapes 6 (nouveaux cœurs, CI) et 7 (FreeRTOS), et plus tard pour tout nouveau cœur (annexe RISC-V). Depuis l'abandon de la simulation Linux, il est avec le test de fumée le banc permanent du projet.

## Prérequis

- Étapes 2 et 3 (T0-T8 dès l'étape 3 ; le KAL décomposé de l'étape 4 ensuite).
- QEMU : `qemu-system-arm` (`qemu-system-riscv32` le jour du portage RISC-V).
- Correspondance cible → machine QEMU :

| Architecture | Machine QEMU | Remarques |
|---|---|---|
| Cortex-M0/M0+ | `microbit` (nRF51) | pas de FPU ; ARMv6-M |
| Cortex-M3 | `mps2-an385` | |
| Cortex-M4 | `mps2-an386` | FPU FPv4-SP ; UART CMSDK ×5, LAN9118 (`0x40200000`) — cible QEMU de référence |
| Cortex-M7 | `mps2-an500` | FPU FPv5 |
| RISC-V RV32 (reporté) | `virt` (`-bios none`) | CLINT + PLIC — voir l'annexe |

## Tâches

### 1. Harnais de test bare-metal

- Mini-framework dans `tests/kal/` : `kal_test.h` (macros `TEST_ASSERT`, compteurs, rapport), sortie par UART émulée ou semihosting, **terminaison avec code de retour** exploitable par CTest : semihosting `SYS_EXIT` (`qemu-system-arm -semihosting`) côté ARM, `sifive_test`/écriture au poweroff ou `ecall` semihosting côté RISC-V.
- Un exécutable de test par (architecture × backend), construit par la structure CMake existante : `tests/kal/CMakeLists.txt` réutilise les `cmake/cpu/*.cmake`, les startups et `.ld` des cibles, avec un `mem_qemu_<machine>.ld` par machine QEMU (cartes mémoire MPS2/microbit/virt ≠ MCU réels).
- Intégration CTest : `add_test` lançant QEMU avec timeout (`-nographic`, `-machine <m>`), labels `kal`, `arch:<a>`, `backend:<b>` → `ctest -L kal` exécute toute la matrice.

### 2. Test de fumée canonique (T0)

- **T0 — boot → lsh → `uname -a`** : sur chaque combinaison architecture × backend, le premier test de la matrice est le test de fumée canonique du projet (`tests/smoke_lsh.py`, défini à l'étape 3) branché sur la série émulée QEMU (`-serial stdio` ou `tcp`), avec `--expect-machine` correspondant à l'architecture. Il valide la chaîne complète (warmup, rootfs, pilote tty, création de processus, appels système) avant les tests unitaires ciblés : un T0 rouge rend les tests T1 à T11 ininterprétables, ne pas les exécuter.

### 3. Tests de sauvegarde/restitution de contexte

- **T1 — intégrité aller-retour** : remplir tous les registres visés par le contexte avec des motifs connus, `__bckup_context`, corrompre, restaurer, vérifier motif par motif. Sur M4/M7 : variante avec contexte FPU actif (registres S0-S31, FPSCR) et vérification du comportement lazy stacking (FPCCR) — QEMU l'émule.
- **T2 — contexte de démarrage** : `__bckup_thread_start_context` à la création d'un pthread ; vérifier que le contexte sauvegardé permet un redémarrage propre (PC = point d'entrée, SP = sommet de pile, état processeur initial correct : EPSR.T sur Cortex-M, `mstatus` sur RISC-V).
- **T3 — identité** : `__is_thread_self` vrai pour la tâche courante, faux pour une autre ; cohérence des macros d'accès TCB avec la version embOS/FreeRTOS effectivement liée (détecte les écarts de layout de `embos-iar-vs-gcc.md`).

### 4. Tests de déroutement du flux d'exécution

- **T4 — déroutement simple (mécanisme sighandler)** : reproduire la séquence documentée du noyau (`__stop_sched()` → `__bckup_context()` → `__swap_signal_handler()` → `__restart_sched()`), exécuter le handler, puis restaurer par `__exit_signal_handler()`/`__rstr_context()` ; vérifier l'intégrité complète des registres et de la pile utilisateur après retour au point d'interruption.
- **T5 — déroutement en présence d'appel système** : dérouter une tâche bloquée dans un appel système préemptible (cf. doc « appel système préemptible ») ; vérifier la reprise correcte de l'appel ou son interruption selon la sémantique attendue (EINTR).
- **T6 — imbrication** : déroutement pendant l'exécution d'un handler (signal pendant signal, statut `PTHREAD_STATUS_SIGHANDLER`) ; vérifier l'empilement/dépilement des contextes.

### 5. Tests des mécanismes vfork/exec

- **T7 — vfork (génétique)** : au niveau KAL, reproduire la séquence documentée (`__bckup_stack()` + `__bckup_context()` du parent, statut `PTHREAD_STATUS_FORK`), vérifier la suspension du parent jusqu'au `exec`/`exit` de l'enfant, puis la restitution exacte par `__rstr_stack()`/`__rstr_context()`.
- **T8 — execv (recouvrement)** : réinitialiser le contexte d'un pthread sur un nouveau point d'entrée avec pile réinitialisée ; vérifier qu'aucun état de l'ancien flux ne survit (registres, pile, état FPU).
- Ces deux tests reproduisent au niveau macro ce que `vfork()`/`execv()` font au niveau POSIX ; des tests POSIX de bout en bout (vrai `vfork`+`execv`, vrai `sigaction`+`kill`) complètent en s'exécutant au-dessus du noyau démarré — les inclure comme second niveau du banc (T9-T11). Proposition, <À CONFIRMER> : les écrire comme pseudo-binaires de `bin` (`sys/root/src/bin`, hors de `kernel/`), lancés depuis `lsh` et pilotés par `smoke_lsh.py` ; ils valident alors aussi la chaîne mklepton → table des binaires.

### 6. Matrice et exécution

- Matrice cible : {m0, m3, m4, m7} × {embos, freertos} × {T0..T11} (rv32 le jour du portage). Remplissage progressif : étape 3 : m4 × embos ; étape 6 : + m0, m3, m7 ; étape 7 : + freertos.
- `ctest -L kal --output-junit` en CI (étape 6) ; la matrice complète devient bloquante pour tout commit touchant `src/kernel/core/kal/`.

## Critères de validation

- [ ] Harnais opérationnel : un test trivial passe et échoue correctement (code de retour QEMU) sur chaque machine de la table.
- [ ] T0 (fumée canonique) puis T1-T8 verts pour chaque combinaison architecture × backend intégrée à date.
- [ ] Variante FPU exécutée sur M4/M7 ; variante sans FPU sur M0/M3.
- [ ] Tests niveau POSIX (T9-T11) verts sur les mêmes combinaisons.
- [ ] Matrice intégrée à la CI avec labels par architecture et backend.

## Pièges connus

- QEMU n'émule pas fidèlement le timing ni certains détails (lazy stacking exact, faults d'alignement, MPU selon machine) : un vert QEMU ne dispense pas des paliers matériels de l'étape 5 ; un rouge QEMU est en revanche toujours significatif.
- `microbit` (M0) : pas de semihosting exit standard fiable selon versions QEMU — prévoir un repli « chaîne magique sur UART + grep » dans le wrapper CTest.
- Les cartes mémoire QEMU (MPS2 : flash à 0x0, SRAM à 0x20000000 mais tailles ≠ MCU réels) imposent des `.ld` dédiés : ne jamais réutiliser tel quel le `.ld` d'un MCU réel.
- Sur RISC-V `virt`, l'exécution démarre en mode M à 0x80000000 avec `-bios none` : le startup de test doit le prévoir.
- T4-T6 dépendent de la convention exacte d'empilement du backend (cadre matériel Cortex-M + partie logicielle propre au micro-noyau) : écrire les vérifications à partir de la doc du port embOS/FreeRTOS livré, pas de suppositions.

## À la fin de chaque extension du banc

Mettre à jour `MIGRATION-STATUS.md` : matrice architecture × backend × tests avec statut, et écarts QEMU/matériel constatés.
