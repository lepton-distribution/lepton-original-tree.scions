# Compilation de masse du périmètre actif — tableau de bord

Généré par `tools/migration/mass_compile.sh` — ne pas éditer à la main.  
Commande : `mass_compile.sh -q --report doc/migration/mass-compile.md --csv doc/migration/mass-compile.csv --audit-csv doc/migration/audit-iar.csv`

Résultat : **346/348 fichiers C OK (99.4 %)** ; `arm-none-eabi-gcc -c` (outils hôte : `cc -m32`).

Commande de chaque fichier : entrée du preset qui le compile, sinon gabarit du preset QEMU le plus proche corrigé par profil (voir l'en-tête du script). Détail par fichier : `mass-compile.csv`.

## Par module

| Module | OK / total | IAR-ismes (Lepton) | IAR-ismes (tiers) |
|---|---|---:|---:|
| kernel/core | 50 / 50 | 0 | 38 |
| kernel/dev | 154 / 154 | 0 | 16 |
| kernel/fs | 26 / 26 | 0 | 2 |
| kernel/net | 38 / 38 | 0 | 0 |
| lib | 27 / 27 | 0 | 0 |
| sbin | 35 / 35 | 0 | 0 |
| bin | 8 / 8 | 0 | 0 |
| tauon-basic | 7 / 9 | 32 | 0 |
| tools/mklepton | 1 / 1 | 4 | 0 |

## Histogramme des erreurs (première erreur de chaque fichier)

| Fichiers | Erreur normalisée | Exemple |
|---:|---|---|
| 2 | `implicit declaration of function 'X' [-Wimplicit-function-declaration]` | `sys/user/tauon-basic/src/bin/dhrystone/dhry21b.c` |

## Fichiers en échec

| Fichier | Première erreur |
|---|---|
| `sys/user/tauon-basic/src/bin/dhrystone/dhry21b.c` | `sys/user/tauon-basic/src/bin/dhrystone/dhry21b.c:157: implicit declaration of function 'strcmp' [-Wimplicit-function-declaration]` |
| `sys/user/tauon-basic/src/bin/dhrystone/dhrystone_main.c` | `sys/user/tauon-basic/src/bin/dhrystone/dhrystone_main.c:98: implicit declaration of function 'runDhrystone' [-Wimplicit-function-declaration]` |
