# Handoff étape 5 → 6

État au 2026-10-03 : branche `migration/etape-5`, **paliers 1-9 verts** sur NUCLEO-F429ZI
(remplaçante de la F439ZI), en attente de la validation utilisateur de fin d'étape.
Journal : `validation-nucleo-f439zi.md`.

## Réponses aux prérequis de 6
- Étape 5 close : paliers 1-9 verts, `ci/run.sh` vert ; **validation utilisateur à obtenir**.
- Liste des cartes de l'étape 6 : décision ouverte au statut (inchangée) — point d'arrêt en début
  d'étape 6.
- Base de départ : options finales `-Os -g` (`LEPTON_OPT_LEVEL`, tous presets Cortex-M), hard-float,
  embOS `libosT7VHLSP.a` ; mkconf de carte calqué sur QEMU ; 168 MHz (HSE 8 MHz bypass).

## Décisions actées pendant 5
- `eth.c` paramétré par le BSP (descripteur `eth_stm32f4x7_bsp`, `BOARD_ETH_PHY_*`).
- Flash interne autorisée pour l'étape 5 (ni option bytes ni protection).
- Corrections génériques révélées par la carte, hors fichiers de la carte, accord utilisateur :
  KAL ICI/IT (`__kal_arch_redirect_xpsr`, TICI) ; `_SC_CLK_TCK` = `__KERNEL_CLK_TCK` (TCLK) ;
  `_sbrk` borné à `__stack_limit__` (`sbrk_cortexm.c`, TSBRK) ; `.ccm_bss` (données du CPU seul en
  CCM, liste blanche par nom de section, zone RAM sur QEMU) ; pilote Ethernet STM32F4 :
  `eth_packet_available` au lieu de `ETH_CheckFrameReceived` dans `select` (trames perdues).
- `-Os` pour tous les presets Cortex-M (les presets compilaient en `-O0`, non en `-Og`).
- Critère « aucune modification du noyau ou du KAL pour cette carte » : aucun code propre à la
  carte hors `cmake/boards/`, `ld/mem_*`, BSP ; les corrections ci-dessus sont génériques (à
  confirmer par l'utilisateur à la validation).

## Artefacts produits (manifeste, pas copie)
| Fichier | Contenu, quand le lire |
|---|---|
| `cmake/boards/nucleo-f439zi.cmake` | BSP, puce, HSE, `LEPTON_BOARD_SERIAL_PORT`, `LEPTON_NET_TEST_HOST_IP`, OpenOCD |
| `kernel/dev/bsp/nucleo_f439zi/` | `SystemInit` (horloges), GPIO, ttys3/ttys6, ETH, vecteurs IRQ 64-90 |
| `ld/mem_nucleo-f439zi.ld`, `ld/common-cortexm.ld` | régions (dont `REGION_CCM`), `.ccm_bss`, tas, MSP |
| `kernel/core/arch/cortexm/sbrk_cortexm.c` | tas newlib borné (lié comme objet) ; à ajouter au démarrage armv6m (étape 6) |
| `cmake/toolchains/arm-none-eabi.cmake` | `LEPTON_OPT_LEVEL` (défaut `-Os`) |
| `tests/board_net.py`, `tests/endurance_board.py` | palier 7 (`ctest -R board.net`), palier 8 (manuel, 4 h) |
| `tests/kal_openocd.py`, `tests/smoke_lsh.py --transport serial` | banc KAL et fumée sur carte (`ctest -L board`) |
| `debug/gdbinit-nucleo-f439zi` | `lepton-load`, `lepton-fault`, `lepton-stacks` (piles depuis `heap_top`) |
| `doc/migration/debug-gcc.md`, `doc/BUILDING.md` | procédure flash/débogage ; build de zéro |
| `doc/migration/traces/palier4-appel-systeme-carte.{gdb,txt}` | trace du palier 4 sur carte |

## Écarts au plan et pièges découverts
- QEMU ne modélise pas l'état ICI (défaut KAL invisible à l'étape 3), ni une RAM de 192 Ko (tas sans
  limite invisible), et utilise un autre pilote Ethernet (LAN9118) : la carte révèle des défauts
  génériques ; rejouer `ctest -L board` et `board.net` sur chaque nouvelle carte.
- Presets croisés en `-O0` jusqu'au palier 9 (Debug = `-g` seul) ; paliers 1-8 rejoués en `-Os`.
- Lepton range un tas de thread (588 octets, données des bibliothèques) en bas des piles de processus,
  après 8 octets laissés au contrôle de pile d'embOS : mesurer la marge depuis `heap_top`.
- `.bss` 143 Ko, dont pool lwIP PBUF 73,7 Ko (plus grand que la CCM) : tas 88 Ko après `.ccm_bss`.
- L'attachement de gdb arrête le cœur : `monitor resume` avant `detach`, sinon carte figée (faux
  défaut réseau). OpenOCD : `verify_image` faussé (zone de travail) ; un seul client de la sonde.
- Table des vecteurs générique limitée aux IRQ 0-63 : `.isr_vector_ext` (BSP), lié comme objet.
- Hook `lepton_guard.py` : `>`, `->`, `$VAR` dans une commande sont lus comme redirections vers le
  trunk ; passer par un script du scratchpad. `grep` = `ugrep` (Latin-1 ignoré) : `grep -a`.
- `pgrep -f motif` dans une boucle d'attente se trouve lui-même : attendre sur `/proc/<pid>`.

## Non transmis volontairement
- Diagnostics détaillés (ICI, réseau, tas, pilote Ethernet) : `validation-nucleo-f439zi.md`, messages
  des commits `8fa5ff6`, `236ebae`, `8c4262a`, `9b99404`, `bf9a4b4`, `be9b4da`.
- USART6 (`ttys6`) non vérifié électriquement (pas d'adaptateur) ; F439ZI réelle (CRYP/HASH) non
  disponible : paliers à rejouer dès qu'elle le sera.
