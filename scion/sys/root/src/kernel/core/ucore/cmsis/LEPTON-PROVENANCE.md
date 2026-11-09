# Provenance (chantier environnement de développement Lepton, ENV-44)

CMSIS d'Arm, licence `CMSIS/CMSIS END USER LICENCE AGREEMENT.pdf`. Le paquet est présent depuis
le premier commit de l'arbre (037cd59, « initial commit. new original tree depot. »), sans autre
trace d'origine. Il est **mixte** : sa base vient de CMSIS 2.10, ses en-têtes de cœur de CMSIS 3.30.

| Ensemble | Version relevée dans les fichiers | Date |
| --- | --- | --- |
| `CMSIS/README.txt`, `CMSIS/index.htm`, `CMSIS/Documentation/` | « Revision: V2.10 » | 25 juillet 2011 |
| `Device/ARM/ARMCM{0,3,4}/` (`ARMCM*.h`, `system_ARMCM*.h`, modèles de démarrage) | `@version V1.05` | 19 et 26 juillet 2011 |
| `CMSIS/Include/core_*.h` (`core_cm0.h`, `core_cm0plus.h`, `core_cm3.h`, `core_cm4.h`, `core_cm4_simd.h`, `core_cmFunc.h`, `core_cmInstr.h`, `core_sc000.h`, `core_sc300.h`) | `@version V3.30` | 17 et 24 février 2014 |
| `CMSIS/Include/arm_math.h`, `arm_common_tables.h`, `arm_const_structs.h` | sans numéro | — |

Remarques :

- `source.txt` (« Version 2.10 », `http://www.onarm.com/cmsis/download/`) est le fichier d'origine.
  Il décrit la base du paquet, pas les en-têtes `core_*.h`. Il est conservé tel quel.
- Dans `core_cm4.h` V3.30, la macro `__CM4_CMSIS_VERSION_SUB` vaut `0x20` (3.20), alors que
  l'en-tête annonce V3.30. Aucun clone de CMSIS 3.x n'est disponible pour comparer avec l'amont :
  la valeur est relevée, pas expliquée (O-2 d'ENV-42).
- Aucun fichier de ce répertoire n'est modifié par Lepton, et il ne faut pas en modifier : ce sont
  des en-têtes amont.

Utilisateurs depuis ENV-42 et ENV-44 :

- Le CMSIS-Core V3.30 (`CMSIS/Include`) n'est vu par aucun preset M4 ni M7 : leur CMSIS-Core est
  `../cmsis-5/CMSIS/Core/Include` (`cmake/cpu/cortex-m4f.cmake`, `cmake/cpu/cortex-m7.cmake`).
- `cmake/cpu/cortex-m3.cmake` pose encore V3.30 pour le M3, qui n'a aucun preset.
- `cmake/isa/armv6m.cmake` le pose aussi pour l'armv6m, mais pour le SAMD21 le `BEFORE` du BSP le
  masque.
- `Device/ARM/ARMCM4` n'est plus inclus par le BSP `qemu-mps2-an386`. Depuis ENV-44 (D5), celui-ci
  prend `../cmsis-5/Device/ARM/ARMCM4/Include/ARMCM4_FP.h`.
- Code non configuré par les presets qui inclut encore ce paquet : familles anciennes, dont
  `kernel/dev/arch/cortexm/stm32f1xx/`. Relevé dans `rapports/env-42-core-cm4.md` §1.4 (dépôt
  lepton-tools).

Le paquet reste dans l'arbre tant que ces utilisateurs existent.
