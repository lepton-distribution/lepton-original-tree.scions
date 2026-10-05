# Handoff étape 6, SAMD21 Xplained Pro session 1 → session 2 (endurance, piles)

État au 2026-10-05 : branche `migration/etape-6`, carte **SAMD21 Xplained Pro paliers 1-6 et banc
KAL verts** (`ctest -L board` 15/15). Journal : `validation-samd21-xplained-pro.md`.

## Réponses aux prérequis de la session 2
- Carte raccordée (EDBG CMSIS-DAP `03eb:2111`, `/dev/ttyACM0`), flash interne autorisée pour la
  durée du module (ni fusibles, ni user row, ni protection) ; carte laissée avec `lepton.elf`.
- Pas de réseau (décision 2026-10-05) : palier 7 sans objet ; endurance sans ping.
- `-Os -g` depuis le début : palier 9 acquis de fait.
- Restent à l'étape 6 après la SAMD21 : CI (`ci/Dockerfile`, tous presets, mémoire archivée),
  retrait IAR (tag `legacy-iar`), code gelé (tag `legacy`), table cœurs × cartes × statut.

## Décisions actées pendant la session
- En-têtes Microchip SAMD21_DFP 3.8.270 (Apache-2.0), sous-ensemble SAMD21J18A, et
  `core_cm0plus.h` CMSIS 5 (paquet embOS) ; ajoutés en tête des chemins par la carte (`BEFORE`).
- Horloge par registres (séquence ASF errata 9905) : DFLL48M boucle fermée sur XOSC32K, 48 MHz.
- Pilote USART SERCOM neuf (registres), pas l'ASF SAMD20 ; console `ttys3` (SERCOM3, PA22/PA23).
- Configuration : profil noyau `minimal`, 4 processus, **12** fichiers ouverts, 8 descripteurs,
  `cpufs` 3 Ko, rootfs réduit (`__KERNEL_RTFS_NODETBL_SIZE` 32, `NODE_BLOCK_NB_MAX` 10), BUFSIZ
  64, piles 1 536 (`initd`, `lsh`, `ls`, `ps`) et 1 280 octets, MSP 1 536 octets.
- SysTick à la priorité 0x80 en ARMv6-M (`core-segger/arch/armv6m/embos_init_hw.c`).
- TSBRK : bloc = tas / 8 (décision utilisateur).

## Artefacts produits (manifeste, pas copie)
| Fichier | Contenu, quand le lire |
|---|---|
| `cmake/isa/armv6m.cmake`, `kal/arch/armv6m/kal_arch.h`, `arch/cortexm/startup_armv6m.c` | axe ISA ARMv6-M |
| `core-segger/arch/armv6m/embos_init_hw.c` | tick embOS ARMv6-M |
| `cmake/boards/samd21-xplained-pro.cmake`, `ld/mem_samd21-xplained-pro.ld` | carte, mémoire, MSP |
| `kernel/dev/bsp/samd21_xplained_pro/`, `kernel/dev/arch/cortexm/samd21/dev_samd21/` | BSP, pilote USART |
| `ucore/cmsis-5/Device/Microchip/SAMD21/` | DFP, `LEPTON-PROVENANCE.md` |
| `sys/user/tauon-basic/etc/mkconf_tauon_basic_samd21_xplained_pro.xml`, `src/arch/samd21-xplained-pro/` | configuration minimale |
| `tests/kal/arch/armv6m/` | banc KAL Thumb-1 |
| `debug/openocd-samd21-xplained-pro.cfg`, `debug/gdbinit-samd21-xplained-pro` | flash, débogage (`lepton-fault-v6m`) |
| `doc/migration/traces/palier4-appel-systeme-samd21.txt` | trace du palier 4 |

## Écarts au plan et pièges découverts
- Build et banc sans aucune modification du noyau ni du KAL hors axe ISA ; écarts d'architecture
  corrigés : `embos.cmake` (intégration matérielle par ISA), `tests/kal/CMakeLists.txt` ;
  `ajout-coeur.md` complété.
- **8 fichiers ouverts ne suffisent pas** : `initd` sort, l'`exit` du pid 1 arrête la tâche noyau,
  qui retourne dans `OS_TerminateError` (console muette). Diagnostic : `_g_kernel_syscall_trace`.
- M0+ : **4 points d'arrêt matériels** seulement ; `finish` échoue s'ils sont tous pris.
- Le hook du trunk prend `->` ou `$VAR` relatifs pour des redirections : scripts gdb dans le
  scratchpad, chemins absolus.
- `mass_compile.py` : profil SAMD21 (lire le preset configuré de la carte) ; 386/386.
- `ctest -L board` laisse `kal_bench` en flash : reflasher `lepton.elf` (cible `flash`).
- XOSC32K : démarrage d'environ 2 s (STARTUP 6, valeur ASF) avant la bannière.

## Session 2 (proposée)
- Endurance 4 h (`tests/endurance_board.py` sans réseau : vérifier qu'il accepte l'absence d'hôte
  IP), cycles `lsh` ; `lepton-stacks` en fin ; ajuster les `stack=` mesurés.
- Rejouer TSBRK sur F429/F746 si elles sont rebranchées.

## Non transmis volontairement
- Relevés de registres détaillés : journal de validation.
- Pack DFP complet (gcc/iar/keil, SVD) : `LEPTON-PROVENANCE.md` donne l'URL et le SHA-256.
