# Handoff étape 10, session 2 (STM32F746G-DISCO) → session 3 (SAMD21 Xplained Pro)

État au 2026-10-08 : sessions 1 (F429ZI) et 2 (F746) faites sur le Mac Intel, branche
`migration/etape-10` (locale) ; journal `doc/migration/validation-macos.md` ; décisions de
l'étape et contexte de la session 1 : `handoff/etape-10-f429.md`.

## Résultats F746 (aucune modification de code ni de test)
- embOS et FreeRTOS : `-L board` 20/20 deux fois, `board.net` 5/5 ; liste = Debian.
- `uname -a` : `cortexM7-stm32f7`. Tailles : text −5 008 o par rapport à Debian sous les deux
  backends, data/bss identiques (newlib). Régions < 90 % (RAM 53,7 % au plus).
- Carte laissée avec `lepton.elf` embOS en flash.
- Sonde `0671FF495351885087181231`, même port USB que la F429ZI : même console
  `/dev/cu.usbmodem1454303` (le nom dépend du port, pas de la carte).

## Pour la session 3 (SAMD21 Xplained Pro, sans réseau)
- Débrancher la F746 ; sonde EDBG (Atmel/Microchip), relever le numéro de série et la console
  (`system_profiler SPUSBDataType`, `ls /dev/cu.usbmodem*`) ; pas d'adresse d'hôte.
- Oracle : `validation-samd21-xplained-pro.md` (Debian : 15/15 sous embOS ; FreeRTOS : banc KAL
  14/14, système complet non supporté, T0/fumée en échec = **écart accepté**, à retrouver tel quel).
- `uname -a` attendu : `cortexM0p-samd21`.
- Piles (tâche 3) : `lepton-stacks` avec `arm-none-eabi-gdb` et `debug/gdbinit-samd21-xplained-pro`
  après la fumée ; Debian : `lsh` 73 %, MSP 63 %. **Point d'arrêt** au-dessus de 85 %.
- Endurance de 1 h (décision 2026-10-08) : `tests/endurance_board.py` appelle `gdb-multiarch`
  en dur, absent du Mac. **Point d'arrêt à présenter** : proposition sans modifier le dépôt, un
  lien `gdb-multiarch` → `arm-none-eabi-gdb` dans un répertoire du `PATH` hors dépôt (environnement
  du Mac, à documenter dans `BUILDING.md` §3 ter) ; alternative : option `--gdb` dans le script
  (fichier partagé avec Debian). Le script fait aussi `ping -W 1` (secondes sous Linux) : sans
  objet sur SAMD21 et WL55 (sans réseau), à noter en dette pour les cartes réseau.
- Commande de l'endurance : docstring de `endurance_board.py` et `validation-samd21-xplained-pro.md`
  (`--fault-check v6m`, sans `--ping-ip`).

## Pièges constatés
- `ctest -R '^board\.net$'` ajoute `board.smoke_lsh` (fixture) : 2 tests, normal.
- Finder ouvert sur le trunk : `.DS_Store` → contrôle final de `ci/run.sh` en échec ;
  l'utilisateur les supprime (l'agent n'écrit pas dans le trunk).
