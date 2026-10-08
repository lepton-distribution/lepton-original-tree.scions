# Handoff étape 10 (quatre cartes depuis macOS) → fin du chantier « hôte macOS »

État au 2026-10-08 : quatre sessions faites sur le Mac Intel (F429ZI, F746, SAMD21, WL55),
branche `migration/etape-10` (locale, non poussée) ; journal `doc/migration/validation-macos.md` ;
handoffs de session `etape-10-{f429,f746,samd21}.md`. Pas d'étape suivante dans le plan :
prochaine action = validation de l'utilisateur, puis fusion (ci-dessous).

## Critères de validation d'ETAPE-10
- [x] Quatre cartes, `-L board` embOS vert deux fois, liste = journal Debian : F429ZI 20/20,
      F746 20/20, SAMD21 15/15, WL55 16/16 (15 sur la carte A + `board.radio`).
- [x] F429ZI et F746 : `board.net` 5/5 d'affilée (embOS et FreeRTOS).
- [x] WL55 : `board.radio` vert entre A et B (20/20 dans les deux sens, ping/pong 20/20).
- [x] SAMD21 : piles relevées, max 72,7 % (`lsh`) < 85 %.
- [x] Aucune région au-dessus de 90 % (max : CCM F429 FreeRTOS 86,1 %) ; écarts de taille
      consignés (text −2 960 à −5 008 o, newlib « 4.4.0 ») ; WL55 FreeRTOS bss +8 o non expliqué.
- [x] `git diff master --stat -- scion` : `scion/tests/net_qemu.py` seul (accepté au point
      d'arrêt de la session 1).
- [x] Chaque carte laissée avec `lepton.elf` embOS en flash (les deux WL55 comprises).
- [x] `doc/BUILDING.md` §4 à §7 : consoles et commandes macOS.
- En plus (décisions 2026-10-08) : FreeRTOS exécuté sur les quatre cartes (SAMD21 : banc KAL
  14/14, fumée en échec = écart accepté inchangé) ; endurances de 1 h SAMD21 et WL55 vertes.

## Avant fusion dans `master` (ORCHESTRATION §5)
- `ci/run.sh` vert sur le Mac (session 1, après la correction de `net_qemu.py` ; rien de modifié
  sous `scion/` depuis). **À rejouer sur Debian** : `net_qemu.py` sert le label `net` QEMU,
  non exécuté sur macOS. Transfert de la branche par `git bundle`, `git fetch` par l'utilisateur.
- Aucun push sans accord explicite (branche et commits nommés).

## Décisions actées pendant l'étape 10 (détail : MIGRATION-STATUS)
- Flash interne seulement ; adresse du Mac 192.168.2.10/16 sur `en0`.
- FreeRTOS exécuté sur carte ; endurances de 1 h SAMD21 et WL55 rejouées ; 4 h non.
- `ping()` de `tests/net_qemu.py` portable sur macOS (session 1).
- Environnement du Mac : OpenOCD `+ftdi +cmsis`, gdb de MacPorts `+python313`, lien
  `~/.local/bin/gdb-multiarch` (session 3) ; `install-macos.sh`, `BUILDING.md` §3 ter.

## Artefacts produits (manifeste, pas copie)
| Fichier | Contenu |
|---|---|
| `doc/migration/validation-macos.md` | hôte, sondes et consoles, tableaux par carte, tailles, piles |
| `doc/migration/handoff/etape-10-*.md` | transitions entre sessions (détail par carte) |
| `doc/BUILDING.md` §3 ter, §4-§7 | commandes macOS par carte |
| `$LEPTON_BUILD/<preset>/{endurance_board,board_radio,smoke_*}.log` (hors git) | consoles |

## Écarts au plan et pièges découverts
- WL55 : une seule configuration suffit pour la paire (`board.radio` flashe B par
  `openocd-nucleo-wl55jc1-paire.cfg`) ; `BUILDING.md` §7 évoquait un second répertoire de build.
- Console macOS d'un ST-LINK = préfixe de son Location ID (`0x14543000` → `cu.usbmodem1454303`).
- Hook `lepton_guard.py` : `A->B` dans une commande est lu comme une redirection ; un chemin
  relatif est résolu depuis le répertoire courant de la session (parfois le trunk) : passer par
  un script du scratchpad ou par l'outil Edit.
- WL55 FreeRTOS bss +8 o par rapport au module 7.3 : origine non établie (dette).

## Non transmis volontairement
- Paliers 1 à 5 au débogueur (couverts par le banc KAL, non rejoués) ; endurances de 4 h.
- Dette restante : `MIGRATION-STATUS.md`, « Blocages et dette » (`ping -W` d'`endurance_board.py`
  sous macOS, tailles Debian finales absentes des journaux).
