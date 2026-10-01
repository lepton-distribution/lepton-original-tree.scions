# Compilation de masse du périmètre actif — tableau de bord

Généré par `tools/migration/mass_compile.sh` — ne pas éditer à la main.  
Commande : `mass_compile.sh -q --report doc/migration/mass-compile.md --csv doc/migration/mass-compile.csv --audit-csv doc/migration/audit-iar.csv`

Résultat : **340/348 fichiers C OK (97.7 %)** ; `arm-none-eabi-gcc -c` (outils hôte : `cc -m32`).

Commande de chaque fichier : entrée du preset qui le compile, sinon gabarit du preset QEMU le plus proche corrigé par profil (voir l'en-tête du script). Détail par fichier : `mass-compile.csv`.

## Par module

| Module | OK / total | IAR-ismes (Lepton) | IAR-ismes (tiers) |
|---|---|---:|---:|
| kernel/core | 50 / 50 | 0 | 38 |
| kernel/dev | 154 / 154 | 0 | 16 |
| kernel/fs | 26 / 26 | 0 | 2 |
| kernel/net | 38 / 38 | 0 | 0 |
| lib | 27 / 27 | 0 | 0 |
| sbin | 34 / 35 | 0 | 0 |
| bin | 3 / 8 | 0 | 0 |
| tauon-basic | 7 / 9 | 32 | 0 |
| tools/mklepton | 1 / 1 | 4 | 0 |

## Histogramme des erreurs (première erreur de chaque fichier)

| Fichiers | Erreur normalisée | Exemple |
|---:|---|---|
| 5 | `implicit declaration of function 'X' [-Wimplicit-function-declaration]` | `sys/root/src/bin/net/mongoose/mongoose.c` |
| 2 | `passing argument N of 'X' from incompatible pointer type [-Wincompatible-pointer-types]` | `sys/root/src/bin/net/telnetd.c` |
| 1 | `implicit declaration of function 'X'; did you mean 'X'? [-Wimplicit-function-declaration]` | `sys/root/src/bin/net/httpc/httpc.c` |

## Fichiers en échec

| Fichier | Première erreur |
|---|---|
| `sys/root/src/bin/net/httpc/httpc.c` | `sys/root/src/bin/net/httpc/httpc.c:223: implicit declaration of function 'perror'; did you mean 'error'? [-Wimplicit-function-declaration]` |
| `sys/root/src/bin/net/mongoose/mongoose.c` | `sys/root/src/bin/net/mongoose/mongoose.c:521: implicit declaration of function 'tolower' [-Wimplicit-function-declaration]` |
| `sys/root/src/bin/net/mongoose/mongoosed.c` | `sys/root/src/bin/net/mongoose/mongoosed.c:288: implicit declaration of function 'strerror' [-Wimplicit-function-declaration]` |
| `sys/root/src/bin/net/telnetd.c` | `sys/root/src/bin/net/telnetd.c:105: passing argument 3 of 'libc_accept' from incompatible pointer type [-Wincompatible-pointer-types]` |
| `sys/root/src/bin/test2.c` | `sys/root/src/bin/test2.c:237: passing argument 3 of 'libc_accept' from incompatible pointer type [-Wincompatible-pointer-types]` |
| `sys/root/src/sbin/stty.c` | `sys/root/src/sbin/stty.c:1158: implicit declaration of function 'toupper' [-Wimplicit-function-declaration]` |
| `sys/user/tauon-basic/src/bin/dhrystone/dhry21b.c` | `sys/user/tauon-basic/src/bin/dhrystone/dhry21b.c:157: implicit declaration of function 'strcmp' [-Wimplicit-function-declaration]` |
| `sys/user/tauon-basic/src/bin/dhrystone/dhrystone_main.c` | `sys/user/tauon-basic/src/bin/dhrystone/dhrystone_main.c:98: implicit declaration of function 'runDhrystone' [-Wimplicit-function-declaration]` |
