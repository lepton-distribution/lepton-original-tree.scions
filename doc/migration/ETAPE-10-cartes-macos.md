# Étape 10 — Validation des quatre cartes sous macOS (embOS)

## Contexte

Les quatre cartes sont validées depuis Debian (étapes 5 à 7, journaux `validation-<carte>.md`).
Cette étape les revalide depuis le Mac Intel de l'étape 9. Deux choses changent : l'hôte qui
pilote la sonde et la console, et le firmware lui-même, construit par la toolchain ARM du Mac.
Le code de Lepton, les BSP et les fichiers OpenOCD ne changent pas.

Archétype : migration. L'oracle est le journal Debian de chaque carte : mêmes tests, mêmes
verdicts. Un écart désigne d'abord l'hôte ou la toolchain, pas le noyau.

## Prérequis

- Étape 9 close ; `handoff/etape-9.md` (OpenOCD, `arm-none-eabi-gdb`, comparaison des `.bin`).
- Cartes raccordées **directement** au Mac en USB (ni machine virtuelle, ni concentrateur non
  alimenté) : NUCLEO-F429ZI (remplaçante de la F439ZI, décision 2026-10-01), STM32F746G-DISCO,
  SAMD21 Xplained Pro, deux NUCLEO-WL55JC1.
- Réseau (F429ZI, F746) : câble Ethernet carte ↔ Mac ; adresse fixe du Mac sur cette interface
  dans le réseau de la carte (192.168.2.5 d'après son `.init`) : `<À CONFIRMER>`.
- Décisions actées le 2026-10-07 : écriture de la flash interne autorisée sur toutes les cartes
  de l'étape ; endurances de 4 h non rejouées.
- Limites de l'écriture en flash reprises des décisions par carte des étapes 5 à 7, `<À
  CONFIRMER>` : flash interne seulement ; ni option bytes, ni fusibles, ni user row, ni
  protection, ni QSPI, ni zone sécurisée SFSA/ESE.
- Une carte par session (ORCHESTRATION §3) ; branche `migration/etape-10`.

## Tâches

### 1. Sondes et consoles

- Pour chaque carte, relever la sonde (numéro de série : `system_profiler SPUSBDataType`) et la
  console : `/dev/cu.usbmodem…`, jamais `/dev/tty.usbmodem…`.
- Consigner le tableau carte / sonde / console dans `doc/migration/validation-macos.md`.
- Une seule carte branchée à la fois, sauf la paire WL55 (`LEPTON_BOARD_STLINK_SERIAL`,
  `LEPTON_RADIO_PEER_STLINK_SERIAL`, `LEPTON_RADIO_PEER_SERIAL_PORT`, `doc/BUILDING.md` §7).
- `debug/openocd-*.cfg` sont partagés avec Debian : **point d'arrêt** avant d'en modifier un.

### 2. Par carte, preset `<carte>-embos`

Ordre : NUCLEO-F429ZI, STM32F746G-DISCO, SAMD21 Xplained Pro, NUCLEO-WL55JC1.

```
cmake --preset <carte>-embos -DLEPTON_BOARD_SERIAL_PORT=/dev/cu.usbmodem… [options de la carte]
cmake --build --preset <carte>-embos
cmake --build --preset <carte>-embos --target flash
caffeinate -i ctest --preset <carte>-embos -L board      # deux fois de suite
cmake --build --preset <carte>-embos --target flash      # le banc laisse kal_bench en flash
```

| Carte | Preset | `ctest -L board` attendu (journal Debian) | En plus |
|---|---|---|---|
| NUCLEO-F429ZI | `nucleo-f439zi-embos` | 19/19, dont TSBRK | `board.net` 5/5 avec `-DLEPTON_NET_TEST_HOST_IP=<Mac>` |
| STM32F746G-DISCO | `stm32f746g-disco-embos` | 19/19 | `board.net` 5/5 |
| SAMD21 Xplained Pro | `samd21-xplained-pro-embos` | 15/15 | relevé des piles (tâche 3) |
| NUCLEO-WL55JC1 | `nucleo-wl55jc1-embos` | 15/15 sur la carte A | `board.radio` entre A et B, dans les deux sens |

- Le journal de la carte fait foi, pas ce tableau : comparer la liste `ctest -N -L board` du Mac
  à celle du journal avant de comparer les verdicts. Une liste différente est un écart.
- Champ machine de `uname -a` identique au journal (`cortexM4-stm32f4`, `cortexM0p-samd21`,
  `cortexM4-stm32wlxx`, valeur du journal pour la F746).
- Paliers 1 à 5 au débogueur : non rejoués (le banc KAL les couvre) ; les rejouer seulement sur
  une carte dont la fumée ou le banc échoue, selon `doc/migration/debug-gcc.md`.

### 3. Occupation mémoire et piles

- Pour chaque carte, `arm-none-eabi-size lepton.elf` comparé aux valeurs text / data / bss du
  journal Debian ; consigner l'écart. Seuil de `ci/run.sh` (90 % par région) inchangé.
- SAMD21 (32 Ko de RAM, marge connue : `lsh` 73 %, MSP 63 % sur Debian) : après la fumée,
  relever les piles par `lepton-stacks` avec `arm-none-eabi-gdb` et
  `debug/gdbinit-samd21-xplained-pro`. **Point d'arrêt** si une pile dépasse 85 %.

### 4. Endurance et FreeRTOS

- Endurance : non rejouée (décision 2026-10-07). `tests/endurance_board.py` appelle
  `gdb-multiarch` en dur : ne pas le modifier dans cette étape, le noter en dette.
- Endurances de 1 h de la SAMD21 et de la WL55 : `<À CONFIRMER>` (non rejouées par défaut).
- FreeRTOS : les quatre presets `<carte>-freertos` sont construits par `ci/run.sh` (étape 9).
  Leur exécution sur carte depuis macOS : `<À CONFIRMER>` (non faite par défaut ; la SAMD21 sous
  FreeRTOS garde son écart accepté, T0 en échec).

### 5. Journal

`doc/migration/validation-macos.md` : par carte, un tableau palier / verdict / date / commande /
écart avec le journal Debian ; versions de l'hôte ; sondes et consoles ; tailles ; piles SAMD21.
Les journaux `validation-<carte>.md` existants ne sont pas modifiés.

## Critères de validation

- [ ] Quatre cartes : `ctest --preset <carte>-embos -L board` vert deux fois de suite, liste de
      tests identique à celle du journal Debian.
- [ ] F429ZI et F746 : `board.net` 5/5 d'affilée.
- [ ] WL55 : `board.radio` vert entre les deux cartes.
- [ ] SAMD21 : piles relevées, aucune au-dessus de 85 %.
- [ ] Aucune région mémoire au-dessus de 90 % ; écarts de taille avec Debian consignés.
- [ ] `git diff master --stat -- scion` : vide, ou limité aux fichiers acceptés à un point
      d'arrêt.
- [ ] Chaque carte laissée avec `lepton.elf` en flash.
- [ ] `doc/BUILDING.md` §4 à §7 : noms de console et commandes macOS à côté de ceux de Debian.

## Pièges connus

- **`tty.` au lieu de `cu.`** : l'ouverture de `/dev/tty.usbmodem…` attend la porteuse ; la
  fumée expire sans rien lire.
- **Nom de console instable** : il change au rebranchement et selon le port USB ; le relever
  avant chaque configuration, ne pas le figer dans un preset.
- **Mise en veille** du Mac pendant un test : la sonde décroche. `caffeinate -i`.
- **Console occupée** : un terminal série resté ouvert (`screen`, `picocom`) fait échouer
  `board.smoke_lsh` sans message clair.
- **`kal_bench` resté en flash** après `ctest -L board` : la carte ne démarre plus Lepton ;
  reflasher par la cible `flash`.
- **Interface Ethernet du Mac** : sans adresse fixe, macOS lui attribue une adresse
  169.254.x.x et le ping de la carte échoue ; avec le Wi-Fi sur le même réseau, la route peut
  passer par le Wi-Fi.
- **Écart de taille ou de pile** : il vient de newlib ou de libgcc si la toolchain du Mac n'est
  pas construite comme celle de Debian (étape 9, tâche 6), pas du code de Lepton.
- **Deux ST-LINK branchés** sans numéro de série : OpenOCD prend le premier trouvé.

## À la fin de l'étape

`MIGRATION-STATUS.md` : ligne de l'étape 10 ; tableau « Cœurs × cartes × statut » : mention
« validé depuis macOS » par carte ; décisions des `<À CONFIRMER>` ; dette (`gdb-multiarch` en dur,
endurances non rejouées sur macOS). `doc/BUILDING.md`. `handoff/etape-10.md`.
