# Handoff étape 6, module STM32F746G-DISCO (fin) → module suivant (SAMD21 Xplained Pro)

État au 2026-10-05 : branche `migration/etape-6`, **STM32F746G-DISCO validée sur carte, paliers
1-9 verts** (journal `validation-stm32f746g-disco.md`). Complète `handoff/etape-6-f746-1.md`
(session 1 : axes, BSP, USART), qui reste valable.

## Réponses aux prérequis du module suivant
- Cœurs validés : M4F (QEMU an386, NUCLEO-F429ZI), M7 (QEMU an500, STM32F746G-DISCO) ;
  `ci/run.sh` vert (host, an386 hard/soft, an500) ; carte F746 laissée avec `lepton.elf` réseau.
- Restent à l'étape 6 : SAMD21 Xplained Pro (M0+, carte physique, 32 Ko de RAM), CI
  (`ci/Dockerfile`, tous presets, mémoire archivée), retrait IAR (tag `legacy-iar`), code gelé
  (tag `legacy`), table cœurs × cartes × statut.

## Relevés pour la session SAMD21 (non vérifiés sur carte)
- Carte décidée le 2026-10-05 (pas de QEMU M0) ; non raccordée au 2026-10-05 (seule la ST-LINK
  de la F746 est branchée : débrancher ou choisir la sonde par numéro de série).
- OpenOCD : `board/atmel_samd21_xplained_pro.cfg` présent (sonde EDBG) ; embOS : bibliothèques
  ARMv6-M `libosT6L*` ; BSP embOS le plus proche : `Microchip/SAMD20J18_SAMD20_XPlainedPro`.
- Arbre : `kernel/dev/arch/cortexm/at91samd20/at91samd20_uart`, BSP `samd20xplained_pro` (différé),
  `cmsis-5/Device/Atmel` ne contient que `samv71` (pas de SAMD21 : paquet Microchip à décider) ;
  `kal/arch/armv6m/` : `kal_arch_conf.h` seul (HYPOTHÈSE À VALIDER), `kal_arch.h` à écrire ;
  démarrage armv6m et `_sbrk` borné (`sbrk_cortexm.c`) à ajouter ; banc KAL : sources armv7m seules.
- RAM 32 Ko contre `.bss` 128-155 Ko avec réseau. **Décision utilisateur (2026-10-05) : pas de
  réseau** (ni lwIP, ni `ifconfig`/`ftpd`) ; **réduire les piles des pseudo-binaires en diminuant
  les tampons stdio** : `__KERNEL_STDIO_PRINTF_BUFSIZ` dans le `user_kernel_mkconf.h` de la carte
  (`BUFSIZ`, 3 tampons stdin/stdout/stderr par processus, `lib/libc/stdio/stdio.h` ; défaut
  armv6m 128), puis les `stack=` du mkconf de la carte, mesurés (`lepton-stacks`) ;
  `rootfscore.h` réduit le rootfs pour la SAMD20 par `__tauon_cpu_device__` (à reporter dans la
  configuration de la carte, mécanisme `__KERNEL_CPU_DEVICE_NAME` sans table).

## Décisions actées pendant la session 2
- Pilote Ethernet STM32F7 sur la HAL ETH V1.3.3 ; mémoire DMA (descripteurs, 8 tampons RX, 1 TX)
  dans un bloc de 16 Ko aligné, région MPU 0 non cachable (TEX 1, C 0, B 0) ; aucune
  maintenance de cache.
- Base de temps de la HAL (`HAL_GetTick`, `HAL_Delay`) sur `__kernel_get_timer_ticks()` : HAL
  utilisable sous tout micro-noyau, en contexte de processus seulement (tick arrêté avant
  l'ordonnanceur) ; d'où le démarrage du MAC à l'ouverture de `eth0`, pas au chargement.
- Sommes de contrôle : lwIP seule (options par défaut) ; insertion matérielle explicitement
  contournée.
- MAC par défaut administrée localement (`02:…`), dérivée de l'UID 96 bits de la puce.

## Artefacts produits (manifeste, pas copie)
| Fichier | Contenu, quand le lire |
|---|---|
| `kernel/dev/arch/cortexm/stm32f7xx/dev_stm32f7xx/dev_stm32f7xx_eth_x.c` | pilote Ethernet F7 (modèle pour une autre carte F7) |
| `kernel/dev/arch/cortexm/stm32f7xx/dev_stm32f7xx/dev_stm32f7xx_hal_tick.c` | base de temps de la HAL |
| `kernel/dev/bsp/stm32f746g_disco/` | `HAL_ETH_MspInit` (RMII), PHY, `eth0` ; `stm32f7xx_hal_conf.h` régénéré (+ETH) |
| `doc/migration/validation-stm32f746g-disco.md` | paliers 1-9, défaut des sommes de contrôle |
| `$LEPTON_BUILD/stm32f746g-disco-embos/endurance_board.log` (hors git) | journal de l'endurance |

## Écarts au plan et pièges découverts
- **HAL ETH V1.3.x** : chaque descripteur d'émission est initialisé en insertion matérielle
  complète des sommes ; `ChecksumCtrl` n'est appliqué qu'avec `ETH_TX_PACKETS_FEATURES_CSUM`.
  Symptôme : ARP correct, ping sans réponse ; diagnostic sans capture (pas de `tcpdump`, pas de
  root) : compteurs MMC de la carte (`0x40028168` émises, `0x400281C4` unicast reçues), tampon
  d'émission relu par la sonde, `nstat IcmpInCsumErrors` de l'hôte.
- Diagnostics réseau sans root : `nstat`, `ip -s link`, `ip neigh` côté hôte ; registres MMC et
  mémoire de la carte par OpenOCD (cœur en marche).
- `mass_compile.py` lit le preset de la carte (le configurer avant) ; 382/382.
- L'endurance tient la sonde (OpenOCD, un seul client) jusqu'au relevé final : ne rien lancer sur
  la sonde pendant 4 h.

## Non transmis volontairement
- Relevés détaillés (registres, piles) : journal de validation.
- Exemples ST de référence (hors git) : `handoff/etape-6-f746-1.md`, section « Non transmis ».
