# Handoff étape 7, module 7.2 (NUCLEO-F439ZI) → module 7.3 (autres cibles, CI, clôture)

État au 2026-10-06 : branche `migration/etape-7` (locale). Module 7.1 validé le 2026-10-06.

## Réponses aux prérequis de 7.3
- Critère d'étape « F439 validée en FreeRTOS (paliers 1 à 8) » : **tenu** sur la NUCLEO-F429ZI
  (journal `validation-nucleo-f439zi.md`, section « Étape 7 ») ; preset `nucleo-f439zi-freertos`
  (build seul dans `ci/run.sh`).
- Banc KAL FreeRTOS : an386 QEMU 18/18, F429ZI 19/19 ; identique à embOS test par test.
- Restent pour 7.3 : an386 soft-float (port `ARM_CM3`), an500 (M7, `ARM_CM4F`), F746 (`ARM_CM7/r0p1`),
  WL55 (M4 sans FPU, `ARM_CM3`), SAMD21 (`ARM_CM0` : trame `kal/backend/freertos/arch/armv6m` à
  écrire, `#error` aujourd'hui) ; `LEPTON_FREERTOS_PORT` à poser dans `cmake/cpu/{cortex-m7,cortex-m3,
  cortex-m0plus}.cmake` ; matrice micro-noyau × machine dans `ci/run.sh` ; devenir d'embOS (décision).

## Décisions actées pendant 7.2
- Flash interne autorisée pour le module ; endurance 4 h (2026-10-06).
- Correctif du pilote commun `dev_cmsdk_uart` (acquittement avant lecture), accord utilisateur.

## Artefacts produits (manifeste, pas copie)
| Fichier | Contenu, quand le lire |
|---|---|
| `doc/migration/validation-nucleo-f439zi.md` (section étape 7) | paliers 1-8 FreeRTOS, défauts et corrections |
| `doc/migration/traces/palier4-appel-systeme-carte-freertos.{gdb,txt}` | trace d'un appel système sur carte (pxCurrentTCB) |
| `debug/gdbinit-nucleo-f439zi` | `lepton-stacks` sous FreeRTOS (motif 0xA5, idle, temporisateurs, cause d'arrêt) |
| `ld/common-cortexm.ld` | mémoire statique de FreeRTOS en `.ccm_bss` (toutes cartes à CCM) |

## Écarts au plan et pièges découverts
- TCB FreeRTOS **jamais** dans `kernel_pthread_t` : le noyau copie et efface ces structures
  (vfork, exec) ; défaut invisible au banc et en fumée simple, révélé sous charge (8 QEMU en parallèle).
- Méthode de chasse aux blocages (QEMU sous charge + `-gdb` + relevé des pthreads, registres de
  faute, état UART) : scripts du scratchpad non versionnés ; à reprendre si un blocage réapparaît.
- Le tas F439 est juste (pic d'une session FTP ≈ 84 Ko sur 88 Ko) : toute RAM statique ajoutée
  sous FreeRTOS le réduit ; CCM désormais à 85 %.
- Fuite de tas par session FTP (préexistante, deux backends) : dette, `MIGRATION-STATUS.md`.
- Outils : ne pas importer `tests/*.py` sans `PYTHONDONTWRITEBYTECODE=1` (cache `.pyc` dans le
  trunk, contrôle CI en échec) ; arrêter tout OpenOCD de diagnostic avant `ctest -L board`.

## Non transmis volontairement
- Journaux : `$LEPTON_BUILD/nucleo-f439zi-freertos/{endurance.log,endurance_rapport.txt,board_net.log}`,
  `$LEPTON_BUILD/ci_run_etape7_*.log`.
