# Handoff étape 6, module NUCLEO-WL55JC1 (fin) → suite de l'étape 6 (CI, retrait IAR, code gelé)

État au 2026-10-05 : branche `migration/etape-6`, **NUCLEO-WL55JC1 validée sur carte** (Cortex-M4
du CPU1 sans FPU, CPU2 arrêté, sans réseau IP) : paliers 1-6, banc KAL 15/15, radio Sub-GHz FSK
868 MHz entre deux cartes (`board.radio`), endurance 1 h verte (journal
`validation-nucleo-wl55jc1.md`). Complète `handoff/etape-6-wl55-1.md` (session 1), qui reste
valable pour la carte, le BSP et la configuration.

## Réponses aux prérequis de la suite
- Cœurs et cartes validés : M4F (QEMU an386, NUCLEO-F429ZI), M7 (QEMU an500, STM32F746G-DISCO),
  M0+ (SAMD21 Xplained Pro), M4 sans FPU (NUCLEO-WL55JC1, carte seulement).
- `ci/run.sh` vert (host, an386 hard/soft, an500) ; presets carte (`nucleo-f439zi-embos`,
  `stm32f746g-disco-embos`, `samd21-xplained-pro-embos`, `nucleo-wl55jc1-embos`) hors
  `ci/run.sh` : build seul dans la tâche CI (pas d'exécution sans sonde).
- Restent à l'étape 6 : CI (`ci/Dockerfile`, tous presets, `--print-memory-usage` archivé avec
  seuil), retrait IAR (tag `legacy-iar`), code gelé (tag `legacy`), table cœurs × cartes × statut,
  `doc/BUILDING.md`.

## Décisions actées pendant le module
- CM4 seul ; STM32CubeWL de l'arbre (HAL V1.3.0, SubGHz_Phy 1.3.0) ; flash interne des deux cartes.
- Radio : FSK 868 MHz, pilote `/dev/radio` inchangé, pas de LoRa ; test `radiotst` + fumée lsh.
- `radio_subghz_phy/` (sauf `stm32_radio_target/`) et `stm32wlxx/Utilities/` : code tiers.
- Endurance 1 h.

## Artefacts produits en session 2 (manifeste, pas copie)
| Fichier | Contenu, quand le lire |
|---|---|
| `sys/root/src/bin/radiotst.c` | tx/rx/ping/pong de chaînes sur `/dev/radio`, bilan des pertes |
| `tests/board_radio.py`, `cmake/boards/nucleo-wl55jc1.cmake` (`board.radio`) | test entre deux cartes : `-DLEPTON_RADIO_PEER_STLINK_SERIAL=…`, `-DLEPTON_RADIO_PEER_SERIAL_PORT=…` |
| `dev_stm32wlxx/dev_stm32wlxx_util_timer.c` | `UTIL_TIMER_GetCurrentTime/GetElapsedTime` sur le tick noyau |
| `dev_stm32wlxx/dev_stm32wlxx_hal_tick.c` | `HAL_GetTick` (tick), `HAL_Delay` (DWT), objet de l'exécutable |
| `doc/migration/validation-nucleo-wl55jc1.md` | paliers 1-9, défauts, piles |
| `$LEPTON_BUILD/nucleo-wl55jc1-embos/{endurance_board,board_radio}.log` (hors git) | consoles |

## Écarts au plan et pièges découverts
- Aucune modification du code commun (`board.radio` enregistré par le fichier de carte) ;
  `ajout-coeur.md` §4 complété (HAL faible, attente avant `OS_Start`, test entre deux cartes).
- Défauts du code repris de l'IAR, révélés sur carte : base de temps HAL faible liée, attente
  radio avant `OS_Start`, tampon circulaire du pilote USART (endurance, premier essai en échec).
  Le code différé d'autres cartes peut cacher les mêmes défauts : se méfier des pilotes HAL+DMA
  « ReceiveToIdle » et des fonctions faibles de la HAL.
- Python importé depuis le trunk : `PYTHONDONTWRITEBYTECODE=1` (sinon `__pycache__` dans le trunk).
- Le hook du trunk prend `>` (y compris dans un message de commit ou un commentaire Python passé
  en ligne de commande) pour une redirection : `git commit -F <fichier>`, outil d'édition.
- RSSI non exposé par le pilote radio (inchangé) ; pas de `/dev/cpu0` (pilote non repris).

## Non transmis volontairement
- Paramètres radio détaillés et trames : `dev_stm32wlxx_bsp_radio_if.c`, journal de validation.
- Relevés de registres de la session 1 : journal ; handoff `etape-6-wl55-1.md`.
