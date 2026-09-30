# Étape 5 — Cible de base NUCLEO-F439ZI

## Contexte

Le système est validé sous QEMU (étapes 3 et 4). Cette étape change **seulement le matériel** :
même compilateur, même KAL, même noyau, sur la NUCLEO-F439ZI (STM32F439ZI, Cortex-M4F,
2 Mo de flash, 256 Ko de RAM, Ethernet, ST-Link intégré). À son issue, la base de départ de Lepton
est posée : arborescence, système de build, chaîne croisée, une carte réelle validée. Aucun projet
IAR n'existe pour cette carte : son BSP s'écrit à partir des sources STM32F4 existantes
(`dev/arch/cortexm/stm32f4xx`, projets `discovery_f4`, `olimex_p407`, `stm32f469i-eval`).

## Prérequis

- Étape 4 close ; socle QEMU vert.
- `scripts/install-debian.sh --with-debug-tools` (OpenOCD, `gdb-multiarch`) ; règles udev de la
  sonde ; carte raccordée en USB.
- BSP ou exemple embOS le plus proche du F439 identifié à l'étape 1.

## Tâches

### 1. Carte

- `cmake/boards/nucleo-f439zi.cmake` (cœur `cortex-m4f`, famille `armv7m` : déjà validés sous QEMU).
- `ld/mem_nucleo-f439zi.ld` : flash `0x08000000` 2 Mo, SRAM `0x20000000`, emplacement du système de
  fichiers mklepton ; les `.icf` STM32F4 existants servent de source d'information (tailles de pile,
  sections particulières), pas de référence de comparaison.
- Vecteurs STM32F4 (table propre au MCU, démarrage générique de l'étape 3 réutilisé).

### 2. BSP STM32F4

- Horloges : HSE/PLL (le démarrage générique appelle `SystemInit`), à partir du code STM32F4
  existant.
- Console : USART3 (port série virtuel du ST-Link) comme périphérique standard de `lsh` ; second
  port UART disponible sur les connecteurs.
- Ethernet : MAC du STM32F4 et PHY de la carte ; pilote dérivé de l'existant STM32F4 s'il existe,
  sinon à écrire — à établir d'après la cartographie de l'étape 1.

### 3. Flash et débogage

- `debug/openocd-nucleo-f439zi.cfg`, cible CMake `flash`, `debug/gdbinit-nucleo-f439zi` ;
  procédure dans `doc/migration/debug-gcc.md`.

### 4. Paliers de validation (arrêt au premier échec)

1. Reset → `main` ; 2. mémoire ; 3. warmup ; 4. appel système ; 5. multitâche et signaux —
   mêmes paliers que l'étape 3, sur la carte ; un écart avec QEMU désigne le BSP.
6. Test de fumée canonique : `tests/smoke_lsh.py --transport serial --port /dev/ttyACM<n>
   --expect-machine <valeur F439>` vert.
7. Réseau : ping, `ftpd`.
8. Endurance : plusieurs heures sans faute ; remplissage des piles mesuré.
9. Optimisation : passage de `-Og` à `-Os` ou `-O2`, puis paliers 6 à 8 rejoués.

## Critères de validation

- [ ] Paliers 1 à 9 verts ; journal `doc/migration/validation-nucleo-f439zi.md`.
- [ ] Aucune modification du noyau ou du KAL pour cette carte : tout le spécifique est dans
      `cmake/boards/`, `ld/mem_*`, le BSP.
- [ ] Socle QEMU toujours vert.
- [ ] `doc/BUILDING.md` : cloner (étape 0), configurer, compiler, flasher la F439 sans aide.

## Pièges connus

- Horloge non configurée : tout fonctionne, trop lentement (débit série faux).
- Faute précoce : lire `SCB->CFSR`/`HFSR` avant de chercher ailleurs.
- Un écart de comportement entre QEMU et la carte vient d'abord du BSP ou du timing réel, pas du
  noyau (le noyau est identique).
- Débordements de pile révélés seulement en endurance : peindre les piles.

## À la fin de l'étape

`MIGRATION-STATUS.md` : base de départ posée (carte, options d'optimisation finales, versions
épinglées) ; `handoff/etape-5.md`.
