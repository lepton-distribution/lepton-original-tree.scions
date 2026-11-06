# Provenance (chantier environnement de développement Lepton, ENV-34)

CMSIS-Core 5 (Arm, Apache-2.0, en-tête SPDX de chaque fichier), `Include/`.

| Fichier | Origine | Fins de ligne | Écart avec CMSIS_5 `5.9.0` |
| --- | --- | --- | --- |
| `cachel1_armv7.h`, `cmsis_compiler.h`, `cmsis_gcc.h`, `cmsis_version.h`, `core_cm0plus.h`, `core_cm7.h`, `mpu_armv7.h` | paquet embOS de Segger (CoreSupport STM32F769I-Discovery, V5.6), copie non modifiée, décision du 2026-10-05 (migration Lepton ; commentaire de `cmake/cpu/cortex-m7.cmake`) | CRLF | aucun une fois les CR retirés |
| `core_cm4.h` (V5.1.2) | https://github.com/ARM-software/CMSIS_5, tag `5.9.0` (commit `2b7495b8535bdcb306dac29b9ded4cfb679d7e5c`), `CMSIS/Core/Include/core_cm4.h`, extrait par `git archive` et non modifié (ENV-34, PA-55) | LF (amont) | aucun (identique octet pour octet) |

Code tiers non modifié (D1a). Contrôle : `scripts/tiers/verifier-import.sh --liste-seule
<clone CMSIS_5> 5.9.0 kernel/core/ucore/cmsis-5 <liste>` (dépôt lepton-tools), liste des
8 fichiers `CMSIS/Core/Include/…` : `OK` pour `core_cm4.h`, `CR` pour les 7 autres.

Inclusion : `cmake/cpu/cortex-m7.cmake` (M7), `cmake/cpu/cortex-m4f.cmake` (M4, depuis ENV-42,
PA-71) et le BSP `samd21-xplained-pro` (M0+, `BEFORE`). Le CMSIS-Core V3.30 de l'arbre
(`kernel/core/ucore/cmsis/CMSIS/Include`) n'est plus posé que par `cmake/cpu/cortex-m3.cmake` et
`cmake/isa/armv6m.cmake` : un preset M4 ou M7 ne voit qu'un `core_cm4.h` ou `core_cm7.h`, celui-ci.
Historique : de ENV-34 à ENV-41, `cmake/isa/armv7m.cmake` plaçait V3.30 en tête pour tous les
presets armv7m et ce `core_cm4.h` n'était vu par aucun preset (PA-57 d'ENV-35 : V3.30 conservé).
Ajouter un autre en-tête de cœur depuis le même tag si un preset l'exige.
