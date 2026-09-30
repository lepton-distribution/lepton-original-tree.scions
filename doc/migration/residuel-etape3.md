# Transformation des IAR-ismes — rapport

Généré par `tools/migration/transform_iar.py` — ne pas éditer à la main.  
Commande : `transform_iar.py --files-from doc/migration/chaine-minimale.txt --report doc/migration/residuel-etape3.md`

Résumé : 0 fichier(s) modifié(s), 0 occurrence(s) automatique(s), 10 résiduelle(s).

## Résiduels (10)

| Fichier | Ligne | Règle | Motif | Détail |
|---|---:|---|---|---|
| `sys/root/src/kernel/core/core-segger/kernel.c` | 93 | garde-iar-arm | IAR M16C (code gelé, étape 6) | `( (__tauon_compiler__==__compiler_iar_m16c__))` |
| `sys/root/src/kernel/core/heap.c` | 27 | garde-iar-arm | IAR M16C (code gelé, étape 6) | `(__tauon_compiler__==__compiler_iar_m16c__)` |
| `sys/root/src/kernel/core/interrupt.h` | 125 | garde-iar-arm | IAR M16C (code gelé, étape 6) | `(__tauon_compiler__==__compiler_iar_m16c__)` |
| `sys/root/src/kernel/core/kal.h` | 912 | garde-iar-arm | IAR M16C (code gelé, étape 6) | `( defined(__IAR_SYSTEMS_ICC) && defined (__KERNEL_UCORE_EMBOS) && defined(CPU_M16C62))` |
| `sys/root/src/kernel/core/kernelconf.h` | 75 | garde-iar-arm | IAR M16C (code gelé, étape 6) | `defined(__IAR_SYSTEMS_ICC)` |
| `sys/root/src/kernel/core/kernelconf.h` | 198 | garde-iar-arm | IAR M16C (code gelé, étape 6) | `(__tauon_compiler__==__compiler_iar_m16c__)` |
| `sys/root/src/kernel/core/kernelconf.h` | 224 | garde-iar-arm | IAR M16C (code gelé, étape 6) | `(__tauon_compiler__==__compiler_iar_m16c__)` |
| `sys/root/src/kernel/core/kernelconf.h` | 276 | garde-iar-arm | IAR M16C (code gelé, étape 6) | `(__tauon_compiler__==__compiler_iar_m16c__)` |
| `sys/root/src/kernel/core/kernelconf.h` | 285 | garde-iar-arm | branche IAR avec _Pragma/#pragma (placement, étape 5) | `(__tauon_compiler__==__compiler_iar_arm__)` |
| `sys/root/src/kernel/core/system.h` | 43 | garde-iar-arm | IAR M16C (code gelé, étape 6) | `( defined(__IAR_SYSTEMS_ICC) && defined (__KERNEL_UCORE_EMBOS) && defined(CPU_M16C62))` |

## Automatiques (0)

| Fichier | Ligne | Règle | Motif | Détail |
|---|---:|---|---|---|

