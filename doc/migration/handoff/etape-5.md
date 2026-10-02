# Handoff étape 5 → 6 (EN COURS : reprise au palier 7)

État au 2026-10-02 : branche `migration/etape-5`, plan approuvé ; paliers 1-6 verts sur
NUCLEO-F429ZI ; palier 7 bloqué par l'environnement (pontage réseau de la VM) ; 8-9 à faire.
Journal : `validation-nucleo-f439zi.md`. À compléter en fin d'étape.

## Réponses aux prérequis de 6
- Étape 5 close : **non** (paliers 7-9, validation utilisateur de fin d'étape).
- Liste des cartes de l'étape 6 : décision ouverte au statut (inchangée).

## Reprise (prochaine session)
1. Vérifier le réseau : `ping 192.168.2.5` depuis la VM après correction du pontage VMware
   (interface reliée au câble de la Nucleo) ; adapter `sys/user/tauon-basic/etc/nucleo-f439zi/.init`
   (adresse, passerelle) si l'adressage change. Contrôle sans privilège : `ip neigh` doit montrer
   `00:bd:3b:33:05:71` (MAC par défaut d'`eth.h`) après une requête ARP de la carte.
2. Palier 7 : ping et session `ftpd` depuis l'hôte. Pas de test automatique encore : adapter
   `tests/net_qemu.py` (transport série + adresse réelle, sans tap) en test `board.net`.
3. Palier 8 : endurance (plusieurs heures, `lsh` + réseau), remplissage des piles : embOS SP
   remplit les piles (`0xCD`) ; mesurer par `OS_STACK_GetTaskStackUsed` ou lecture gdb des piles
   (`OS_Global.pTask`, `pStackBase`, `StackSize`) ; MSP : `__stack_limit__`.
4. Palier 9 : décision utilisateur `-Os`/`-O2` (§4), puis paliers 6-8 rejoués.

## Décisions actées pendant 5
- `eth.c` paramétré par le BSP (descripteur `eth_stm32f4x7_bsp`, `BOARD_ETH_PHY_*`).
- Flash autorisée pour l'étape 5 (flash interne seulement).
- Corrections hors fichiers de l'étape, accord utilisateur : KAL ICI/IT (`__kal_arch_redirect_xpsr`,
  test TICI) ; `_SC_CLK_TCK` = `__KERNEL_CLK_TCK` (1000, embOS), test TCLK.
- mkconf propre de la carte (calqué sur QEMU) ; 168 MHz ; uname `cortexM4-stm32f4`.

## Artefacts produits (manifeste, pas copie)
| Fichier | Contenu, quand le lire |
|---|---|
| `cmake/boards/nucleo-f439zi.cmake` | sources du BSP, puce, HSE, `LEPTON_BOARD_SERIAL_PORT`, OpenOCD |
| `kernel/dev/bsp/nucleo_f439zi/` | `SystemInit` (horloges), GPIO, ttys3/ttys6, ETH, vecteurs IRQ 64-90 |
| `sys/user/tauon-basic/etc/mkconf_tauon_basic_nucleo_f439zi.xml`, `etc/nucleo-f439zi/` | configuration et rootfs (`.init` : adresse IP) |
| `debug/openocd-nucleo-f439zi.cfg`, `debug/gdbinit-nucleo-f439zi` | flash, gdb (`lepton-load`, `lepton-fault`) ; `debug/` ignoré par `.gitignore` (`git add -f`) |
| `tests/kal_openocd.py`, `tests/smoke_lsh.py --transport serial` | banc KAL et fumée sur carte (`ctest -L board`) |
| `doc/migration/debug-gcc.md`, `doc/BUILDING.md` | procédure flash/débogage ; build de zéro (critère de l'étape) |
| `doc/migration/traces/palier4-appel-systeme-carte.{gdb,txt}` | trace du palier 4 sur carte |

## Écarts au plan et pièges découverts
- QEMU ne modélise pas l'état ICI (LDM/STM interruptibles) : défaut KAL invisible à l'étape 3.
- Table des vecteurs générique limitée aux IRQ 0-63 : prolongée par section `.isr_vector_ext`
  (BSP) ; fichier de vecteurs lié comme objet (dans une bibliothèque, il n'est pas tiré).
- OpenOCD : `verify_image` utilise une zone de travail à `0x20000000` (fausses différences) :
  comparer par `dump_image`. Un seul client de la sonde à la fois.
- Hook `lepton_guard.py` : `>`, `->`, `<n>`, `$VAR` dans une commande lancée du trunk sont lus
  comme redirections ; passer par un script du scratchpad.
- `grep` = `ugrep` (fichiers Latin-1 ignorés) : `grep -a`.

## Non transmis volontairement
- Diagnostic détaillé du défaut ICI et du réseau : `validation-nucleo-f439zi.md`, messages des
  commits `8fa5ff6`, `236ebae`.
