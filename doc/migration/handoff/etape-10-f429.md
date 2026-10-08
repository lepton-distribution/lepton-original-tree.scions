# Handoff étape 10, session 1 (NUCLEO-F429ZI) → session 2 (STM32F746G-DISCO)

État au 2026-10-08 : session 1 faite sur le Mac Intel, branche `migration/etape-10` (locale) ;
plan approuvé ; NUCLEO-F429ZI validée depuis macOS sous embOS **et** FreeRTOS. Journal :
`doc/migration/validation-macos.md`.

## Décisions actées en début d'étape (détail : MIGRATION-STATUS)
- Flash interne seulement (ni option bytes, ni fusibles, ni user row, ni protection, ni QSPI,
  ni SFSA/ESE).
- Adresse du Mac sur le câble des cartes : 192.168.2.10/16 sur `en0` (déjà configurée).
- FreeRTOS **exécuté sur carte** depuis macOS (pas seulement construit) : `-L board` ×2 et, pour
  les cartes réseau, `board.net` ×5, comme embOS.
- Endurances de 1 h de la SAMD21 et de la WL55 **rejouées** depuis macOS ; endurances de 4 h non.

## Résultats F429ZI
- embOS : `-L board` 20/20 deux fois, `board.net` 5/5 ; FreeRTOS : 20/20 deux fois, `board.net` 5/5.
- Liste de tests identique au journal Debian (19 + `board.net`, créé par l'adresse de l'hôte).
- `uname -a` : `cortexM4-stm32f4` ; tailles = `etape-9-bin-macos.md` ; régions < 90 %.
- Carte laissée avec `lepton.elf` embOS en flash.

## Correction (point d'arrêt, accord de l'utilisateur)
- `scion/tests/net_qemu.py`, `ping()` : sortie du `ping` de macOS (« packets received ») et `-W`
  en millisecondes sous Darwin. Seul fichier de `scion/` modifié par l'étape à ce jour.
  `ci/run.sh` vert sur le Mac ; **à rejouer sur Debian avant fusion** (label `net` QEMU, qui
  utilise ce `ping()`).

## Pour la session 2 (STM32F746G-DISCO)
- Débrancher la F429ZI (une seule carte à la fois) ; relever sonde et console
  (`system_profiler SPUSBDataType`, `ls /dev/cu.usbmodem*`) : le nom de console change.
- Adresse de la carte : `.init` de `sys/user/tauon-basic/etc/stm32f746g-disco/` (à lire) ; même
  câble, même `-DLEPTON_NET_TEST_HOST_IP=192.168.2.10` si le réseau est le même.
- Oracle : `validation-stm32f746g-disco.md` (Debian : 20/20, `board.net` 5/5, embOS et FreeRTOS).
- `uname -a` attendu : valeur du journal F746.
- Puis les deux presets (`-embos`, `-freertos`), et finir sur `lepton.elf` embOS en flash.

## Pièges constatés
- `ctest -R '^board\.net$'` ajoute `board.smoke_lsh` (fixture `board_t0`) : 2 tests, normal.
- Shell de l'agent zsh : `echo =====` échoue (`=` développé) ; passer par `bash <<'EOF'`.
- Le build incrémental garde la date de `SOURCE_DATE_EPOCH=0` (« Jan 1 1970 ») : sans effet.

## Dette relevée
- `tests/endurance_board.py` appelle `gdb-multiarch` en dur (absent du Mac) : à traiter avant
  les endurances de 1 h (sessions SAMD21 et WL55) — point d'arrêt, le fichier est partagé.
- Tailles exactes des binaires Debian finals absentes des journaux : à produire sur Debian
  (`map_origine.py`) si l'écart doit être chiffré.
