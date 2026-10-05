# Handoff étape 6, module SAMD21 Xplained Pro (fin) → suite de l'étape 6 (CI, retrait IAR, code gelé)

État au 2026-10-05 : branche `migration/etape-6`, **SAMD21 Xplained Pro validée sur carte** (sans
réseau) : paliers 1-6, banc KAL 15/15, endurance 1 h verte (journal
`validation-samd21-xplained-pro.md`). Complète `handoff/etape-6-samd21-1.md` (session 1 : ISA
armv6m, DFP, BSP, configuration), qui reste valable.

## Réponses aux prérequis de la suite
- Cœurs validés : M4F (QEMU an386, NUCLEO-F429ZI), M7 (QEMU an500, STM32F746G-DISCO), M0+
  (SAMD21 Xplained Pro, carte seulement : pas de QEMU M0+ utilisable, décision 2026-10-05).
- `ci/run.sh` vert (host, an386 hard/soft, an500) ; presets carte (`nucleo-f439zi-embos`,
  `stm32f746g-disco-embos`, `samd21-xplained-pro-embos`) hors `ci/run.sh` : à intégrer en build
  seul par la tâche CI (pas d'exécution sans sonde).
- Restent à l'étape 6 : CI (`ci/Dockerfile`, tous presets, `--print-memory-usage` archivé avec
  seuil), retrait IAR (tag `legacy-iar`), code gelé (tag `legacy`), table cœurs × cartes × statut,
  `doc/BUILDING.md`.

## Relevés pour la session NUCLEO-WL55JC1 (non vérifiés sur carte)
- Décision 2026-10-05 : module ajouté **avant la CI**, nouvelle session ; deux cartes, la seconde
  pour valider la radio. Cartes non raccordées au 2026-10-05 (seule l'EDBG SAMD21 est branchée).
- Puce STM32WL55JC : double cœur Cortex-M4 (CPU1) + Cortex-M0+ (CPU2), radio Sub-GHz intégrée
  (LoRa, (G)FSK) ; tailles mémoire à relire par la sonde (règle anti-invention).
- Portage IAR existant, **classé différé** (`code-gele.md` §6 ; `perimetre.csv` : 241 différé,
  41 gelé, 1 hors-projet) : projets EWARM 8.40 et 9.50 (`OGChipSelectEditMenu` =
  `STM32WL55JC_M4`, configurations Release « Cortex-M3 » et `freertos-debug` à examiner) ;
  `kernel/dev/arch/cortexm/stm32wlxx/` (`cubemx_hal_driver`, `dev_stm32wlxx`,
  `radio_subghz_phy`, `Utilities`) ; BSP `kernel/dev/bsp/stm32wl55jci_nucleo/` (USART2, carte,
  radio) ; `sys/user/tauon-basic/etc/mkconf_tauon_basic_stm32wl55jci_nucleo.xml`,
  `src/arch/st-stm32wl55jci-nucleo/` (configuration à recalquer sur la chaîne minimale, comme la
  F439 : mkconf de carte propre).
- Axes : cœur `cortex-m4f` existant (FPU du CM4 du WL55 à vérifier), ISA `armv7m` ; carte =
  fichiers nouveaux (`cmake/boards`, `ld/mem_*`, preset), `__KERNEL_CPU_DEVICE_NAME` par CMake ;
  passer le code différé de la carte par `transform_iar.py` et `mass_compile` (ETAPE-6 tâche 1).
- Décisions à faire acter en début de module : `MIGRATION-STATUS.md`, « Décisions ouvertes ».

## Décisions actées pendant la session 2
- Endurance de la SAMD21 limitée à 1 h (utilisateur).
- `tests/endurance_board.py` : `--ping-ip` facultatif (carte sans réseau) ; `--fault-check v6m`
  (ICSR.VECTACTIVE ≠ 3 et `lepton_embos_last_error` nul) ; défaut `v7m` inchangé.
- `stack=` du mkconf conservés (marges mesurées suffisantes).

## Artefacts produits (manifeste, pas copie)
| Fichier | Contenu, quand le lire |
|---|---|
| `tests/endurance_board.py` | endurance sur carte, avec ou sans réseau, ARMv7-M ou ARMv6-M |
| `doc/migration/validation-samd21-xplained-pro.md` | paliers, défauts, piles |
| `$LEPTON_BUILD/samd21-xplained-pro-embos/endurance_board.log` (hors git) | console de l'endurance |

## Écarts au plan et pièges découverts
- Piles des commandes terminées non mesurables après coup (pile rendue au tas) ; seules les
  tâches vivantes (`lsh`, `initd`, `kernel_thread`, MSP) sont relevées.
- Modifications de `endurance_board.py` et de TSBRK non rejouées sur la F429 et la F746 (cartes
  débranchées) : à rejouer si elles sont rebranchées.

## Non transmis volontairement
- Journal console de l'endurance (hors git, ci-dessus).
