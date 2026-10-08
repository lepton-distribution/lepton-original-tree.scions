# Validation des cartes depuis macOS (étape 10)

Journal de l'étape 10 (`ETAPE-10-cartes-macos.md`) : les quatre cartes, revalidées depuis le Mac
Intel avec des firmwares construits par la toolchain ARM du Mac. Oracle : les journaux Debian
`validation-<carte>.md` (non modifiés). Une carte par session.

## Hôte

| Élément | Version |
|---|---|
| macOS | 15.8.1 (24H32), Intel x86_64 |
| Toolchain ARM | Arm GNU Toolchain 14.2.Rel1 (Build arm-14.52), GCC 14.2.1 20241119, newlib « 4.4.0 » (écart accepté le 2026-10-08) |
| OpenOCD | 0.12.0 (MacPorts `+ftdi`, `/opt/local/bin/openocd`) ; **`+ftdi +cmsis`** (hidapi 0.15.0) depuis la session 3 (SAMD21) |
| gdb | sessions 1-2 : `arm-none-eabi-gdb` 15.2.90 (toolchain Arm, **sans Python**) ; depuis la session 3 : `arm-none-eabi-gdb` 17.2 de MacPorts (`+python313`, Python 3.13.16), lien `~/.local/bin/gdb-multiarch` |
| CMake / Python | 3.31.12 / Python.org 3.11.5 |
| Réseau des cartes | `en0` (Ethernet), 192.168.2.10/16 fixe ; Wi-Fi `en1` 192.168.1.7/24 ; `route -n get 192.168.2.5` → `en0` |

Décisions (2026-10-08) : flash interne seulement (ni option bytes, ni fusibles, ni user row, ni
protection, ni QSPI, ni SFSA/ESE) ; adresse du Mac 192.168.2.10 ; FreeRTOS exécuté sur carte ;
endurances de 1 h (SAMD21, WL55) rejouées ; endurances de 4 h non rejouées (2026-10-07).

## Sondes et consoles

| Carte | Sonde (USB) | Numéro de série | Console |
|---|---|---|---|
| NUCLEO-F429ZI | ST-LINK V2-1 (`0483:374b`), Location ID `0x14543000` | `0672FF495252717267243842` | `/dev/cu.usbmodem1454303` |
| STM32F746G-DISCO | ST-LINK V2-1 (`0483:374b`), Location ID `0x14543000` (même port USB) | `0671FF495351885087181231` | `/dev/cu.usbmodem1454303` |
| SAMD21 Xplained Pro | EDBG CMSIS-DAP (`03eb:2111`, FW 01.1A.00FB), Location ID `0x14543000` (même port USB) | `ATML2130021800003505` (même sonde que Debian) | `/dev/cu.usbmodem1454302` |
| NUCLEO-WL55JC1 A | STLINK-V3, Location ID `0x14543000` | `002700253431510837393937` (même sonde que Debian) | `/dev/cu.usbmodem1454303` |
| NUCLEO-WL55JC1 B | STLINK-V3, Location ID `0x14544000` | `004F003E3431510937393937` (même sonde que Debian) | `/dev/cu.usbmodem1454403` |

## NUCLEO-F429ZI (presets `nucleo-f439zi-*`) — 2026-10-08

Configuration : `-DLEPTON_BOARD_SERIAL_PORT=/dev/cu.usbmodem1454303
-DLEPTON_NET_TEST_HOST_IP=192.168.2.10`. Journaux : `$LEPTON_BUILD/etape10-f429-*.log`.

| Palier | Verdict | Commande | Écart avec le journal Debian |
|---|---|---|---|
| Liste des tests | identique | `ctest -N -L board` : 20 tests (`board.smoke_lsh`, `board.net`, `kal.board_flash`, T1-T8, T1F/T4F/T6F/T7F, TICI, TCLK, TSBRK, IRQ, `harness_fail`) | aucun : 19 du journal embOS (palier 9, sans adresse d'hôte) + `board.net`, créé par `LEPTON_NET_TEST_HOST_IP` ; 20 du journal FreeRTOS (module 7.3) |
| Flash (embOS) | VERT | `cmake --build --preset nucleo-f439zi-embos --target flash` | aucun (« Verified OK », flash interne) |
| `-L board` embOS | **VERT ×2** (20/20, 20/20) | `caffeinate -i ctest --preset nucleo-f439zi-embos -L board` | aucun, après correction de `net_qemu.py` (ci-dessous) ; avant : 19/20 ×2, `board.net` seul en échec |
| `board.net` embOS | **VERT 5/5** d'affilée | `ctest -R '^board\.net$'` | aucun : ping 3/3, FTP `LIST` 13 entrées, `RETR /usr/etc/.boot` 57 o identique, errno `ECONNRESET`=15 |
| `uname -a` | identique | fumée | `lepton-cortexm4-32 4.10.0.2 … cortexM4-stm32f4` |
| `-L board` FreeRTOS | **VERT ×2** (20/20, 20/20) | `caffeinate -i ctest --preset nucleo-f439zi-freertos -L board` | aucun (Debian : 20/20) |
| `board.net` FreeRTOS | **VERT 5/5** d'affilée | `ctest -R '^board\.net$'` | aucun (Debian : 5/5) |
| Paliers 1 à 5 au débogueur | non rejoués | — | couverts par le banc KAL (décision du plan) |
| Endurance 4 h | non rejouée | — | décision 2026-10-07 |
| Fin de session | `lepton.elf` embOS en flash | `--target flash` | — |

**Défaut d'hôte corrigé** (accord de l'utilisateur, point d'arrêt) : `ping()` de
`tests/net_qemu.py` (partagé par `board_net.py`) ne reconnaissait pas la sortie du `ping` de
macOS (« 3 packets received » contre « 3 received » sous Linux) : 0/3 compté pour un ping 3/3 ;
et `-W` est en millisecondes sous macOS (2 ms de délai au lieu de 2 s). Expression
`(\d+) (?:packets )?received`, `-W` converti sous Darwin. Inoffensif sous Linux (même expression,
même valeur) ; non vu à l'étape 9, le label `net` n'y étant pas exécuté.

**Occupation mémoire** (`arm-none-eabi-size lepton.elf`, `-Os`) :

| Preset | text / data / bss (Mac) | Debian | Régions (`memoire.py`, Mac) |
|---|---|---|---|
| `nucleo-f439zi-embos` | 281 864 / 1 064 / 143 824 | text ≈ 287 Ko au palier 9 de l'étape 5 (valeur exacte du binaire final non disponible sur le Mac) | FLASH 13,5 %, RAM 50,7 %, CCM 69,1 % |
| `nucleo-f439zi-freertos` | 288 560 / 1 188 / 155 372 | non relevée dans le journal (Debian, module 7.3 : RAM 51 %, CCM 85 %) | FLASH 13,8 %, RAM 50,9 %, CCM 86,1 % |

Les tailles du Mac sont celles de `etape-9-bin-macos.md` (même binaire) ; l'écart avec Debian
vient de newlib (répartition par origine dans ce fichier ; `map_origine.py` à rejouer sur Debian
pour le chiffrer). Aucune région au-dessus du seuil de 90 %.

Note : le binaire testé porte la date de compilation « Jan 1 1970 » (`kernel.c` compilé en
dernier par `SOURCE_DATE_EPOCH=0 ci/run.sh` à l'étape 9, non recompilé en incrémental) ; sans
effet sur les tests.

## STM32F746G-DISCO (presets `stm32f746g-disco-*`) — 2026-10-08

Même câble et mêmes options que la F429ZI (`-DLEPTON_BOARD_SERIAL_PORT=/dev/cu.usbmodem1454303
-DLEPTON_NET_TEST_HOST_IP=192.168.2.10`) ; carte en 192.168.2.5 (`.init`). Journaux :
`$LEPTON_BUILD/etape10-f746-*.log`. Aucune modification de code ni de test.

| Palier | Verdict | Commande | Écart avec le journal Debian |
|---|---|---|---|
| Liste des tests | identique | `ctest -N -L board` : 20 tests (mêmes noms que la F429ZI) | aucun (Debian : 20/20 dont `board.net`, sessions 6.2 et 7.3) |
| Flash (embOS) | VERT | `--target flash` | aucun (« Verified OK », flash interne) |
| `-L board` embOS | **VERT ×2** (20/20, 20/20) | `caffeinate -i ctest --preset stm32f746g-disco-embos -L board` | aucun |
| `board.net` embOS | **VERT 5/5** d'affilée | `ctest -R '^board\.net$'` | aucun : ping 3/3, `LIST` 13 entrées, `RETR /usr/etc/.boot` 57 o identique, errno `ECONNRESET`=15 |
| `uname -a` | identique | fumée | `lepton-cortexm7-32 4.10.0.2 … cortexM7-stm32f7` |
| `-L board` FreeRTOS | **VERT ×2** (20/20, 20/20) | `caffeinate -i ctest --preset stm32f746g-disco-freertos -L board` | aucun (Debian : 20/20) |
| `board.net` FreeRTOS | **VERT 5/5** d'affilée | `ctest -R '^board\.net$'` | aucun (Debian : 5/5) |
| Paliers 1 à 5 au débogueur ; endurance 4 h | non rejoués | — | banc KAL ; décision 2026-10-07 |
| Fin de session | `lepton.elf` embOS en flash | `--target flash` | — |

**Occupation mémoire** (`-Os`) :

| Preset | text / data / bss (Mac) | Debian (journal) | Écart | Régions (`memoire.py`, Mac) |
|---|---|---|---|---|
| `stm32f746g-disco-embos` | 280 936 / 1 180 / 155 640 | 285 944 / 1 180 / 155 640 | text −5 008 | FLASH 26,9 %, RAM 48,7 % |
| `stm32f746g-disco-freertos` | 286 100 / 1 184 / 166 784 | 291 108 / 1 184 / 166 784 | text −5 008 | FLASH 27,4 %, RAM 53,7 % |

Même écart de text sous les deux backends, data et bss identiques : l'écart vient de la
bibliothèque C (newlib « 4.4.0 » du Mac, écart accepté), pas du noyau. Aucune région au-dessus
de 90 %.

## SAMD21 Xplained Pro (presets `samd21-xplained-pro-*`) — 2026-10-08

Configuration : `-DLEPTON_BOARD_SERIAL_PORT=/dev/cu.usbmodem1454302` ; sans réseau. Aucune
modification de code ni de test ; deux corrections de l'environnement du Mac (ci-dessous).

| Palier | Verdict | Commande | Écart avec le journal Debian |
|---|---|---|---|
| Liste des tests | identique | `ctest -N -L board` : 15 tests (`board.smoke_lsh`, `kal.board_flash`, T1-T8, TICI, TCLK, TSBRK, IRQ, `harness_fail`) | aucun (Debian : 15/15) ; même liste sous FreeRTOS |
| Flash (embOS) | VERT | `--target flash` | aucun (« Verified OK », flash interne), après ajout de `+cmsis` à OpenOCD |
| `-L board` embOS | **VERT ×2** (15/15, 15/15) | `caffeinate -i ctest --preset samd21-xplained-pro-embos -L board` | aucun |
| `uname -a` | identique | fumée | machine `cortexM0p-samd21` |
| Piles après fumée | VERT (max 72,7 %) | reset sans écriture puis `ls`, `ps`, `cat /usr/etc/.boot`, `pwd`, `ls /dev` (`smoke_lsh.py`), puis `lepton-stacks` (OpenOCD + `gdb-multiarch`) | `lsh` 72,7 % (Debian 72,7), `initd` 65,4 (65,4), `kernel_thread` 40,8 (41,6), MSP 62,0 (62,8) ; ICSR 0, `lepton_embos_last_error` `OS_OK` |
| Endurance 1 h embOS | **VERT** | `tests/endurance_board.py --duration 3600 --fault-check v6m` (sans `--ping-ip`), 18:46-19:46 | aucun : 120 cycles de 5 commandes, aucun redémarrage, ICSR 0, `EMBOS=0` ; piles à la fin : `lsh` 72,7 %, `initd` 65,4, `kernel_thread` 42,8, MSP 62,0 |
| `-L board` FreeRTOS | **ÉCHEC ×2 — écart accepté** (fumée en échec, banc non lancé par la fixture) | `caffeinate -i ctest --preset samd21-xplained-pro-freertos -L board` | aucun : même symptôme que Debian (`lsh` démarre, `uname -a`, `ls`, `ps` reviennent à l'invite sans sortie) |
| Banc KAL FreeRTOS | **VERT ×2** (14/14, 14/14) | `ctest -L kal -FS board_t0 -E board.smoke_lsh` | aucun (Debian : 14/14) |
| Paliers 1 à 5 au débogueur ; endurance 4 h | non rejoués | — | banc KAL ; décision 2026-10-07 |
| Fin de session | `lepton.elf` embOS en flash, démarrage contrôlé (`uname -a`) | `--target flash` | — |

**Occupation mémoire** (`-Os`) :

| Preset | text / data / bss (Mac) | Debian (journal) | Écart | Régions (`memoire.py`, Mac) |
|---|---|---|---|---|
| `samd21-xplained-pro-embos` | 94 092 / 924 / 13 008 | 97 056 / 924 / 13 008 | text −2 964 | FLASH 36,3 %, RAM 42,5 % |
| `samd21-xplained-pro-freertos` | 98 724 / 928 / 17 472 | 101 684 / 928 / 17 472 | text −2 960 | FLASH 38,0 %, RAM 56,1 % |

Comme sur les cartes précédentes, seul text diffère (newlib « 4.4.0 », `v6-m/nofp`). Aucune
région au-dessus de 90 %.

**Corrections de l'environnement du Mac** (décisions de l'utilisateur, dépôt Lepton inchangé
sous `scion/`) :
- OpenOCD de MacPorts installé en `+ftdi` seul : pas de hidapi, la sonde EDBG (CMSIS-DAP v1, HID)
  est introuvable (« unable to find a matching CMSIS-DAP device ») ; diagnostic confirmé en
  lecture seule par l'OpenOCD de Homebrew (lié à hidapi) ; réinstallé par l'utilisateur :
  `sudo port upgrade --enforce-variants openocd +ftdi +cmsis`.
- gdb de l'Arm GNU Toolchain construit sans Python : `lepton-stacks` et `lepton-fault`
  (Python, `debug/gdbinit-*`) inutilisables. `arm-none-eabi-gdb +python313` de MacPorts (après
  `texinfo`, que le port ne déclare pas : `makeinfo` manquant, erreur 127) ; lien
  `~/.local/bin/gdb-multiarch` pour `tests/endurance_board.py` (inchangé). Le port tire aussi
  `arm-none-eabi-gcc` 16.1.0 dans `/opt/local/bin` : le build reste sur la toolchain d'Arm 14.2.1,
  placée avant dans le `PATH` (cache CMake vérifié).
- Ces deux points : `scripts/install-macos.sh --with-debug-tools`, `BUILDING.md` §3 ter et §6.

## NUCLEO-WL55JC1 (presets `nucleo-wl55jc1-*`) — 2026-10-08

Paire A/B branchée en même temps, sans réseau IP. Configuration (une seule, pour les deux
cartes) : `-DLEPTON_BOARD_SERIAL_PORT=/dev/cu.usbmodem1454303
-DLEPTON_BOARD_STLINK_SERIAL=002700253431510837393937
-DLEPTON_RADIO_PEER_STLINK_SERIAL=004F003E3431510937393937
-DLEPTON_RADIO_PEER_SERIAL_PORT=/dev/cu.usbmodem1454403` ; `board.radio` flashe la même image
sur B par `openocd-nucleo-wl55jc1-paire.cfg` (généré). Appariement console ↔ sonde déduit du
Location ID, confirmé par un démarrage de chaque carte par sa propre sonde (fin de session).
Aucune modification de code ni de test.

| Palier | Verdict | Commande | Écart avec le journal Debian |
|---|---|---|---|
| Liste des tests | identique | `ctest -N -L board` : 16 tests (`board.radio`, `board.smoke_lsh`, `kal.board_flash`, T1-T8, TICI, TCLK, TSBRK, IRQ, `harness_fail`) | aucun : 15 de la carte A + `board.radio` (labels `board` et `radio`, créé par les options du pair) ; Debian 7.3 : 16/16 ; même liste sous FreeRTOS |
| Flash (embOS) | VERT | `--target flash` | aucun (« Verified OK », flash interne) ; premier flash ST-LINK avec OpenOCD `+cmsis` : sans effet ; avertissement « Unable to match requested speed 500 kHz, using 200 kHz » sans conséquence |
| `-L board` embOS | **VERT ×2** (16/16, 16/16), 20:02 et 20:03 | `caffeinate -i ctest --preset nucleo-wl55jc1-embos -L board` | aucun ; deux passes antérieures (19:58) terminées, la seconde vérifiée à 16/16, synthèse de la première non conservée |
| `board.radio` embOS | **VERT** à chaque passe | inclus dans `-L board` | aucun : `radiotst tx/rx` 20/20 A→B et B→A (perdus 0, désordre 0, doublons 0), `ping/pong` 20/20, fumée `/dev/radio` reçue dans les deux sens |
| `uname -a` | identique | fumée | machine `cortexM4-stm32wlxx` |
| Endurance 1 h embOS | **VERT** | `tests/endurance_board.py --duration 3600 --fault-check v7m` (sans `--ping-ip`), 20:06-21:06, carte A | aucun : 120 cycles de 5 commandes, aucun redémarrage, CFSR = HFSR = 0 ; piles à la fin : `lsh` 53,7 % (Debian 52,9), `initd` 46,7 (46,7), `kernel_thread` 21,0 (20,0), MSP 23,4 (23,2) |
| `-L board` FreeRTOS | **VERT ×2** (16/16, 16/16), 21:06 et 21:08 | `caffeinate -i ctest --preset nucleo-wl55jc1-freertos -L board` | aucun (Debian 7.3 : 16/16) ; `board.radio` : mêmes comptes qu'embOS (20/20 partout) |
| Paliers 1 à 5 au débogueur ; endurance FreeRTOS | non rejoués | — | banc KAL ; endurance de 1 h rejouée sous embOS seulement (décision 2026-10-08) |
| Fin de session | `lepton.elf` embOS en flash sur A **et** B, démarrage contrôlé (`uname -a`) sur chacune | `openocd … program lepton.elf verify reset exit` (cfg A et `-paire`), puis `smoke_lsh.py` avec reset sans écriture | — |

**Occupation mémoire** (`-Os`) :

| Preset | text / data / bss (Mac) | Debian (journal) | Écart | Régions (édition de liens, Mac) |
|---|---|---|---|---|
| `nucleo-wl55jc1-embos` | 109 056 / 840 / 25 528 | 112 952 / 840 / 25 528 (module 7.3) | text −3 896 | FLASH 41,9 %, RAM 40,2 % |
| `nucleo-wl55jc1-freertos` | 114 424 / 844 / 32 172 | 118 308 / 844 / 32 164 (module 7.3) | text −3 884, **bss +8** | FLASH 44,0 %, RAM 50,4 % |

text : newlib « 4.4.0 » (`v7e-m/nofp`), comme sur les autres cartes. bss +8 octets sous
FreeRTOS seulement : origine non établie (le binaire Debian final n'est pas disponible sur le
Mac ; la valeur Debian date du module 7.3, avant d'éventuelles retouches ultérieures) ; à
chiffrer par `map_origine.py` sur Debian. Aucune région au-dessus de 90 %.

