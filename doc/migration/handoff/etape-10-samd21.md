# Handoff étape 10, session 3 (SAMD21 Xplained Pro) → session 4 (NUCLEO-WL55JC1)

État au 2026-10-08 : sessions 1 (F429ZI), 2 (F746) et 3 (SAMD21) faites sur le Mac Intel,
branche `migration/etape-10` (locale) ; journal `doc/migration/validation-macos.md` ; décisions
de l'étape : `handoff/etape-10-f429.md` et `MIGRATION-STATUS.md`.

## Résultats SAMD21 (aucune modification sous `scion/`)
- embOS : `-L board` 15/15 deux fois ; `uname -a` : `cortexM0p-samd21` ; liste = Debian.
- Piles après fumée : `lsh` 72,7 %, `initd` 65,4, `kernel_thread` 40,8, MSP 62,0 (< 85 %).
- Endurance de 1 h embOS verte (18:46-19:46, 120 cycles, ICSR 0, `EMBOS=0`).
- FreeRTOS : banc KAL 14/14 deux fois (`-L kal -FS board_t0 -E board.smoke_lsh`) ; `-L board`
  en échec par la fumée = écart accepté de Debian, même symptôme.
- Tailles : text −2 964 (embOS) / −2 960 (FreeRTOS), data/bss identiques ; régions < 90 %.
- Carte laissée avec `lepton.elf` embOS en flash (démarrage contrôlé).

## Environnement du Mac corrigé (décisions de l'utilisateur)
- OpenOCD MacPorts réinstallé en `+ftdi +cmsis` (hidapi) : la sonde EDBG est en HID seulement.
  Sans effet attendu sur les ST-LINK (libusb), mais à vérifier au premier flash de la WL55.
- gdb : `arm-none-eabi-gdb` 17.2 de MacPorts (`+python313`) ; le gdb de la toolchain d'Arm
  est sans Python (`lepton-stacks` absent). Lien `~/.local/bin/gdb-multiarch` : l'endurance
  tourne sans modification de `tests/endurance_board.py`.
- Le port gdb a tiré `arm-none-eabi-gcc` 16.1.0 dans `/opt/local/bin` : la toolchain d'Arm
  14.2.1 doit rester avant dans le `PATH` (vérifier `which arm-none-eabi-gcc` en début de session).
- Reporté dans `scripts/install-macos.sh --with-debug-tools`, `BUILDING.md` §3 ter et §6.

## Pour la session 4 (NUCLEO-WL55JC1, paire A/B, sans réseau IP)
- Débrancher la SAMD21 ; brancher les deux WL55 ; relever les deux ST-LINK et les deux
  consoles (`system_profiler SPUSBDataType`, `ls /dev/cu.usbmodem*`). Debian : A
  `002700253431510837393937`, B `004F003E3431510937393937`.
- Configuration : `-DLEPTON_BOARD_STLINK_SERIAL=<A> -DLEPTON_BOARD_SERIAL_PORT=<console A>
  -DLEPTON_RADIO_PEER_STLINK_SERIAL=<B> -DLEPTON_RADIO_PEER_SERIAL_PORT=<console B>`
  (`BUILDING.md` §7 ; la carte B se construit dans un autre répertoire de build).
- Oracle : `validation-nucleo-wl55jc1.md` (15/15 sur A, `board.radio` A↔B, endurance 1 h
  `--fault-check v7m` sans `--ping-ip`) ; `uname -a` attendu : `cortexM4-stm32wlxx`.
- FreeRTOS exécuté sur carte (décision 2026-10-08) : `-L board` ×2 et `board.radio`.
- Fin : `lepton.elf` embOS en flash sur les deux cartes ; puis clôture de l'étape 10
  (`handoff/etape-10.md`, critères, `ci/run.sh` à rejouer sur Debian avant fusion).

## Pièges constatés
- Python.org 3.11 sans pyserial : piloter la console par `tests/smoke_lsh.py` (termios), avec
  un reset sans écriture : `--reset-command "openocd -f <cfg> -c init -c 'reset run' -c exit"`.
- Hook `lepton_guard.py` : une variable shell dans une redirection est lue comme un chemin du
  trunk ; écrire les scripts du scratchpad par l'outil Write.
- Le mode auto peut refuser la cible `flash` : demander l'accord explicite de l'utilisateur.
