# Compilation de masse du périmètre actif — tableau de bord

Généré par `tools/migration/mass_compile.sh` — ne pas éditer à la main.  
Commande : `mass_compile.sh -q --report doc/migration/mass-compile.md --csv doc/migration/mass-compile.csv --audit-csv doc/migration/audit-iar.csv`

Résultat : **382/382 fichiers C OK (100.0 %)** ; `arm-none-eabi-gcc -c` (outils hôte : `cc -m32`).

Commande de chaque fichier : entrée du preset qui le compile, sinon gabarit du preset QEMU le plus proche corrigé par profil (voir l'en-tête du script). Détail par fichier : `mass-compile.csv`.

## Par module

| Module | OK / total | IAR-ismes (Lepton) | IAR-ismes (tiers) |
|---|---|---:|---:|
| kernel/core | 59 / 59 | 0 | 52 |
| kernel/dev | 170 / 170 | 0 | 24 |
| kernel/fs | 26 / 26 | 0 | 2 |
| kernel/net | 38 / 38 | 0 | 0 |
| lib | 28 / 28 | 0 | 0 |
| sbin | 36 / 36 | 0 | 0 |
| bin | 8 / 8 | 0 | 0 |
| tauon-basic | 9 / 9 | 0 | 0 |
| tools/mklepton | 1 / 1 | 0 | 0 |
| tests | 7 / 7 | 0 | 0 |

## Histogramme des erreurs (première erreur de chaque fichier)

| Fichiers | Erreur normalisée | Exemple |
|---:|---|---|

## Fichiers en échec

| Fichier | Première erreur |
|---|---|
