# Compilation de masse du périmètre actif — tableau de bord

Généré par `tools/migration/mass_compile.sh` — ne pas éditer à la main.  
Commande : `mass_compile.sh -q --report doc/migration/mass-compile.md --csv doc/migration/mass-compile.csv`

Résultat : **373/373 fichiers C OK (100.0 %)** ; `arm-none-eabi-gcc -c` (outils hôte : `cc -m32`).

Commande de chaque fichier : entrée du preset qui le compile, sinon gabarit du preset QEMU le plus proche corrigé par profil (voir l'en-tête du script). Détail par fichier : `mass-compile.csv`.

## Par module

| Module | OK / total |
|---|---|
| kernel/core | 58 / 58 |
| kernel/dev | 162 / 162 |
| kernel/fs | 26 / 26 |
| kernel/net | 38 / 38 |
| lib | 28 / 28 |
| sbin | 36 / 36 |
| bin | 8 / 8 |
| tauon-basic | 9 / 9 |
| tools/mklepton | 1 / 1 |
| tests | 7 / 7 |

## Histogramme des erreurs (première erreur de chaque fichier)

| Fichiers | Erreur normalisée | Exemple |
|---:|---|---|

## Fichiers en échec

| Fichier | Première erreur |
|---|---|
