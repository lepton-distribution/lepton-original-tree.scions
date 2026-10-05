# Handoff étape 6, module M7 QEMU → session STM32F746G-DISCO

État au 2026-10-05 : branche `migration/etape-6`, cœur **Cortex-M7 validé sous QEMU `mps2-an500`**
(fumée, réseau, banc KAL complet) ; preset `qemu-mps2-an500-embos` dans `ci/run.sh`.

## Réponses aux prérequis de la session suivante (carte STM32F746G-DISCO)
- Cœur M7 : `cmake/cpu/cortex-m7.cmake` validé sous QEMU (bibliothèque embOS `T7VHL`, `fpv5-d16`).
- Périmètre et décisions : statut, décisions du 2026-10-05 (M7 an500 + F746G-DISCO, M0+ SAMD21
  Xplained Pro, CMSIS-Core 5 copié du paquet embOS, suppression IAR et code gelé après tags).
- Aucune carte n'est raccordée à ce jour (`lsusb`) ; aucun BSP ni HAL STM32F7 dans l'arbre.

## Décisions actées pendant le module
- BSP commun `kernel/dev/bsp/qemu_mps2/` (an386 et an500), en-tête d'adresses par machine
  `<machine>/qemu_mps2_machine.h`, choisi par le chemin d'inclusion de `cmake/boards`.
- Configuration applicative `tauon-basic` commune aux machines MPS2 (`mkconf_tauon_basic_qemu_mps2.xml`,
  `etc/qemu-mps2/`, `src/arch/qemu-mps2/`), renommée depuis `…an386…`.
- CMSIS-Core 5.6 (`core_cm7.h` et dépendances, Apache-2.0) copié sans modification du paquet embOS
  (CoreSupport STM32F769I-Discovery) dans `ucore/cmsis-5/CMSIS/Core/Include`.

## Défauts d'architecture corrigés (ETAPE-6 : « un ajout ne modifie pas le code commun »)
- `kernelconf.h` : une carte nouvelle exigeait une entrée dans la table `__tauon_cpu_device__`.
  Désormais `__KERNEL_CPU_DEVICE_NAME` vient de `cmake/boards` (`LEPTON_BOARD_UNAME_MACHINE`) et
  `__tauon_cpu_core__` de `cmake/cpu` ; la table ne sert qu'aux cartes antérieures.
- `LEPTON_FLOAT_ABI` n'était défini que par `cortex-m4f.cmake` (banc KAL sans variantes FPU sur
  M7, édition de liens en échec) : déclaré par chaque fichier de cœur (M7 hard, M3/M0+ soft).
- `ajout-coeur.md` §2 et §4 mis à jour.

## Artefacts produits (manifeste, pas copie)
| Fichier | Contenu, quand le lire |
|---|---|
| `cmake/boards/qemu-mps2-an500.cmake`, `ld/mem_qemu-mps2-an500.ld` | carte QEMU M7 (modèle pour une carte décrite par CMake) |
| `kernel/dev/bsp/qemu_mps2/` | BSP commun, en-têtes `an386/`, `an500/` |
| `cmake/cpu/cortex-m7.cmake` | flags, bibliothèque embOS, CMSIS-Core 5, notes F746 |
| `tools/migration/perimetre_complement.py` | règles étape 6, table `RECLASSEMENTS` (lignes existantes) |
| `build/ci_run_etape6_m7.log` (hors git) | journal de `ci/run.sh` du module |

## Écarts au plan et pièges découverts
- **STM32F746 : FPU simple précision** (`fpv5-sp-d16`) ; le double précision est réservé aux
  F76x/F77x. `cortex-m7.cmake` fixe `fpv5-d16` : la précision FPU dépend de la puce, à porter par
  la carte (ou une variante de cœur) à la session F746 ; newlib `v7e-m+fp` au lieu de `+dp`.
- F746 = Cortex-M7 **r0p1** : variante embOS `_837070` + `USE_ERRATUM_837070=1` à sélectionner.
- Cache I/D du M7 : absent de QEMU ; à activer et valider sur carte (DMA Ethernet : cohérence).
- Test « registres D » prévu au plan non ajouté : en `fpv5-d16`, D0-D15 = S0-S31, déjà remplis
  et maintenus par T1F/T4F/T6F/T7F (redondant).
- Les renommages sont absorbés par `scion graft` (aucun lien pendant dans le trunk).
- `perimetre_complement.py` ajoutait seulement les fichiers nouveaux : `sbrk_cortexm.c` (étape 5)
  manquait au périmètre, ajouté ; fichiers existants devenus actifs : `RECLASSEMENTS`.
- Hook `lepton_guard.py` : une ligne `…/.cmake,\n"` dans une commande est lue comme redirection
  vers le trunk ; scripts dans le scratchpad.

## Non transmis volontairement
- Relevés QEMU (`info mtree`, `info qtree` : 25 MHz) : dans les commentaires de l'en-tête an500.
- SAMD21 Xplained Pro (M0+) : 32 Ko de RAM pour une image à 128 Ko de `.bss` (réseau compris) ;
  `rootfscore.h` réduit déjà le rootfs pour la SAMD20 (`__tauon_cpu_device__`, à reporter dans la
  configuration de la carte) ; à traiter à sa session.
