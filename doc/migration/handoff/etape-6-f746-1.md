# Handoff étape 6, STM32F746G-DISCO session 1 → session 2 (réseau, endurance, -Os)

État au 2026-10-05 : branche `migration/etape-6`, carte **STM32F746G-DISCO paliers 1-6 et banc
KAL verts** (`ctest -L board` 19/19). Journal : `validation-stm32f746g-disco.md`.

## Réponses aux prérequis de la session 2
- Carte raccordée (ST-LINK/V2-1, `/dev/ttyACM0`), flash interne autorisée pour la durée du module
  (ni option bytes, ni protection, ni QSPI) ; carte laissée avec `lepton.elf` (sans réseau).
- STM32CubeF7 v1.17.4 dans l'arbre : HAL complète (`Inc/`, `Src/`), seule `stm32f7xx_hal_gpio.c`
  liée ; `stm32f7xx_hal_conf.h` du BSP **généré** par `tools/migration/hal_conf_stm32f7.py`
  (rejouer avec `ETH` et les modules requis, ne pas l'éditer).
- Broches RMII (exemple ST `LwIP_HTTP_Server_Netconn_RTOS/ethernetif.c`, v1.17.4) : PA1 REF_CLK,
  PA2 MDIO, PA7 CRS_DV, PC1 MDC, PC4 RXD0, PC5 RXD1, PG2 RXER, PG11 TX_EN, PG13 TXD0, PG14 TXD1 ;
  PHY LAN8742A, adresse 0 (`stm32f7xx_hal_conf.h` du modèle ST).

## Décisions actées pendant la session
- `LEPTON_M7_FPU` (cache, `dp`/`sp`) dans `cortex-m7.cmake`, réglée par le preset ; la carte
  contrôle `sp`. `LEPTON_EMBOS_LIB_VARIANT` (`cmake/kal/embos.cmake`, une ligne) posé par la
  carte : `_837070` + `USE_ERRATUM_837070=1` (Lepton n'écrit jamais BASEPRI).
- Horloge par registres dans `SystemInit` (valeurs de l'exemple ST), HAL pour les GPIO seulement :
  la HAL RCC exige le tick HAL (`HAL_GetTick`, `uwTickPrio`), absent sous embOS.
- Caches I et D actifs dès `SystemInit`.
- Pilote USART STM32F7 neuf (registres F7 ≠ F4), modèle du pilote CMSDK ; console `ttys1`.
- CMSIS Device F7 réduit à la F746 (19 Mo d'en-têtes sinon) ; HAL non modifiée.

## Artefacts produits (manifeste, pas copie)
| Fichier | Contenu, quand le lire |
|---|---|
| `cmake/boards/stm32f746g-disco.cmake`, `ld/mem_stm32f746g-disco.ld` | carte, mémoire (1 Mo / 320 Ko) |
| `kernel/dev/bsp/stm32f746g_disco/` | `SystemInit`, caches, USART1, `stm32f7xx_hal_conf.h` généré |
| `kernel/dev/arch/cortexm/stm32f7xx/dev_stm32f7xx/` | pilote USART F7 (y ajouter l'Ethernet) |
| `kernel/dev/arch/cortexm/stm32f7xx/hal_driver/`, `ucore/cmsis-5/Device/ST/STM32F7xx/` | paquets ST, `LEPTON-PROVENANCE.md` |
| `sys/user/tauon-basic/etc/mkconf_tauon_basic_stm32f746g_disco.xml` | sans réseau (`network off`, pas de lwIP, ni `ifconfig`/`ftpd`) |
| `debug/openocd-stm32f746g-disco.cfg`, `debug/gdbinit-stm32f746g-disco` | flash, débogage |
| `tools/migration/hal_conf_stm32f7.py` | génération de la configuration HAL |
| `doc/migration/traces/palier4-appel-systeme-f746.txt` | trace du palier 4 |

## Écarts au plan et pièges découverts
- Paliers 1-6 verts du premier coup ; aucune modification du noyau ni du KAL.
- La HAL F7 n'était pas reconnue comme tiers par `audit_iar.py` (motif `hal_driver` ajouté).
- `mass_compile.py` : les fichiers F7 sont compilés avec les commandes exactes du preset de la
  carte (profil sans gabarit) ; il faut donc avoir configuré `stm32f746g-disco-embos`.
- Session 2 : D-cache actif → descripteurs et tampons DMA Ethernet en région MPU non cachée (ou
  maintenance de cache) ; la HAL ETH V1.3.x a changé d'API (`HAL_ETH_ReadData`, callbacks
  `HAL_ETH_RxAllocateCallback`…) ; l'ancien pilote `eth.c` F4 (SPL) ne s'applique pas tel quel.
- Lien Ethernet du banc : comme à l'étape 5, adresse de l'hôte à fournir (`LEPTON_NET_TEST_HOST_IP`).

## Non transmis volontairement
- Sources d'exemple ST (`main.c`, `ethernetif.c`, BSP Discovery) : hors git, à retélécharger à
  `v1.17.4` (`raw.githubusercontent.com/STMicroelectronics/STM32CubeF7/v1.17.4/…`).
- Relevés de registres détaillés : journal de validation.
