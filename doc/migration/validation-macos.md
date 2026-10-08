# Validation des cartes depuis macOS (étape 10)

Journal de l'étape 10 (`ETAPE-10-cartes-macos.md`) : les quatre cartes, revalidées depuis le Mac
Intel avec des firmwares construits par la toolchain ARM du Mac. Oracle : les journaux Debian
`validation-<carte>.md` (non modifiés). Une carte par session.

## Hôte

| Élément | Version |
|---|---|
| macOS | 15.8.1 (24H32), Intel x86_64 |
| Toolchain ARM | Arm GNU Toolchain 14.2.Rel1 (Build arm-14.52), GCC 14.2.1 20241119, newlib « 4.4.0 » (écart accepté le 2026-10-08) |
| OpenOCD | 0.12.0 (MacPorts `+ftdi`, `/opt/local/bin/openocd`) |
| gdb | `arm-none-eabi-gdb` 15.2.90 (toolchain Arm) |
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
| SAMD21 Xplained Pro | | | |
| NUCLEO-WL55JC1 A / B | | | |

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
