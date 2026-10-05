# Handoff étape 6, NUCLEO-WL55JC1 session 1 → session 2 (radio FSK, endurance)

État au 2026-10-05 : branche `migration/etape-6`, carte A **paliers 1-6 et banc KAL verts**
(`ctest -L board` 15/15). Journal : `validation-nucleo-wl55jc1.md` (cartes, sondes, registres).

## Réponses aux prérequis de la session 2
- Deux cartes raccordées à la VM (VMware : sondes à connecter à la VM à chaque branchement).
  Carte A : STLINK-V3 `002700253431510837393937`, `/dev/ttyACM0` ; carte B :
  `004F003E3431510937393937`, `/dev/ttyACM1`. Toujours désigner la console par
  `/dev/serial/by-id/usb-STMicroelectronics_STLINK-V3_<n°>-if02` (l'ordre ttyACM peut changer).
- Sonde choisie par `-DLEPTON_BOARD_STLINK_SERIAL=<n°>` (cfg OpenOCD générée dans le build) ;
  carte B : second répertoire de build (ex. `cmake --preset nucleo-wl55jc1-embos -B
  $LEPTON_BUILD/nucleo-wl55jc1-embos-b -D…`), ne pas modifier le preset.
- Flash interne des deux cartes autorisée pour la durée du module (ni option bytes, ni
  protection, ni SFSA/ESE). Carte A laissée avec `lepton.elf` ; carte B jamais flashée.
- Endurance : **1 h** (décision 2026-10-05), carte A, sans réseau (`tests/endurance_board.py`,
  `--fault-check v7m` par défaut).
- `-Os -g` depuis le début : palier 9 acquis de fait.

## Décisions actées pendant la session
- Lepton sur le CM4 seul ; CubeWL de l'arbre ; flash autorisée ; radio **FSK 868 MHz, pilote
  `/dev/radio` inchangé, pas de LoRa** (courte portée, 100 m au plus) : fumée par `lsh`
  (`cat /dev/radio &` sur une carte, `echo … > /dev/radio` sur l'autre ; `echo` ne lit pas son
  entrée), puis pseudo-binaire `radiotst` (émission numérotée, réception avec comptage des
  pertes, écho) et `tests/board_radio.py` (deux consoles) ; endurance 1 h.
- Cœur : `cortex-m4f` + `LEPTON_FLOAT_ABI=soft` imposé par la carte (pas de fichier cœur).
- Horloge MSI 48 MHz par registres dans `SystemInit` ; pilote `cpu0` du portage IAR non repris.

## Artefacts produits (manifeste, pas copie)
| Fichier | Contenu, quand le lire |
|---|---|
| `cmake/boards/nucleo-wl55jc1.cmake`, `ld/mem_nucleo-wl55jc1.ld`, preset `nucleo-wl55jc1-embos` | carte, mémoire, sources BSP/HAL liées, cfg OpenOCD générée |
| `kernel/dev/bsp/stm32wl55jci_nucleo/stm32wl55jci_nucleo_system.c`, `stm32wl55jci_nucleo.h` | horloge, priorités, liens IRQ ; réglages de la carte |
| `kernel/dev/arch/cortexm/stm32wlxx/dev_stm32wlxx/dev_stm32wlxx_hal_tick.c` | `HAL_GetTick`/`HAL_Delay` sur le tick noyau |
| `sys/user/tauon-basic/etc/mkconf_tauon_basic_nucleo_wl55jc1.xml`, `etc/nucleo-wl55jc1/`, `src/arch/nucleo-wl55jc1/` | configuration minimale (à compléter du pilote radio) |
| `debug/openocd-nucleo-wl55jc1.cfg`, `debug/gdbinit-nucleo-wl55jc1` | flash, débogage |
| `tools/migration/transform_iar.py` (règle `include-backslash`), `perimetre_complement.py` (règles WL55), `mass_compile.py` (profil WL) | outillage |
| `doc/migration/traces/palier4-appel-systeme-wl55.txt` | trace du palier 4 |

## Relevés pour la session 2 (radio, non vérifiés sur carte)
- Pilote : `dev_stm32wl55jci_nucleo_radio.c` (device `radio`, BSP) sur
  `dev_stm32wlxx_bsp_radio_if.c` (MX_SUBGHZ_Init, callbacks `Radio.*`, tampon circulaire RX,
  **MODEM_FSK 868 MHz**) et `stm32_radio_target/radio_board_if.c` ; ST : `stm32wlxx_nucleo_radio.c`
  (commutateur RF PC3/PC4/PC5, TCXO PB0). `SUBGHZ_Radio_IRQn` = 50 (CM4) → `IRQ50_Handler` vers
  `SUBGHZ_Radio_IRQHandler`, priorité à poser comme pour l'USART2.
- SubGHz_Phy 1.3.0 (`radio.c`, `radio_driver.c`, `radio_fw.c`, `lr_fhss_mac.c`, `wl_lr_fhss.c`,
  liste du projet IAR) ; `stm32_mem.c` requis (`UTIL_MEM_*`) ; `TimerGetCurrentTime`/
  `TimerGetElapsedTime` → `UTIL_TIMER_*` (`radio.c`, carrier sense) alors que
  `Utilities/timer/stm32_timer.c` n'est pas compilé chez IAR : talon ou implémentation sur le
  tick noyau ; HAL à ajouter : `subghz`, peut-être `exti`.
- **À faire acter en début de session 2** : classement tiers de `radio_subghz_phy/` et
  `Utilities/` (absents d'`ORIGINE_TIERS` d'`audit_iar.py` : comptés « Lepton » s'ils deviennent
  actifs ; `stm32_radio_target/` = adaptation, Lepton) ; paramètres FSK du pilote à relire (débit,
  déviation, puissance) avant émission.
- À relire dans le pilote : `read()` bloquant ou non, découpage des trames, taille maximale ;
  `HAL_Delay` des pilotes ST appelés en contexte de processus seulement (tick figé avant
  l'ordonnanceur).

## Écarts au plan et pièges découverts
- Aucune modification du code commun ; `ajout-coeur.md` §4 complété (cœur sans FPU, sondes
  identiques par numéro de série, gestionnaires nommés CMSIS d'un pilote repris).
- Pilote `cpu0` incompatible avec un micro-noyau propriétaire du SysTick (journal, écarts).
- Le STLINK-V3 garde en tampon la console tant que le port est fermé (anciennes bannières).

## Non transmis volontairement
- Relevés détaillés de registres et de piles : journal de validation.
- Inventaire complet du code WL différé (fichiers, projets IAR) : `perimetre.csv`, projets `.ewp`.
