# Conditions d'ISA, de cœur et de puce hors des répertoires d'architecture

Généré par `tools/migration/audit_isa_ifdef.py` — ne pas éditer à la main. Occurrences : `isa-ifdef.csv`.

Critère d'ETAPE-4 : aucune directive d'**ISA** ou de **cœur** dans le code Lepton actif hors de `kal/arch/` et des répertoires d'architecture. L'axe **puce** (`__tauon_cpu_device__`) relève de la carte : autorisé dans `dev/arch` et `dev/bsp`, listé ici pour la décomposition.

| Mesure | Valeur |
|---|---:|
| Code Lepton, ISA/cœur hors arch (directives) | **0** |
| Code Lepton, ISA/cœur hors arch (fichiers) | **0** |
| Code Lepton, puce seule hors arch/BSP (directives) | 12 |
| Code Lepton, emplacements autorisés | 12 |
| Code tiers (non modifié, D1a) | 67 |

## Code Lepton hors arch, par fichier

| Fichier | isa | cœur | puce |
|---|---:|---:|---:|
| `sys/root/src/kernel/core/kernelconf.h` | 0 | 0 | 11 |
| `sys/root/src/kernel/fs/rootfs/rootfscore.h` | 0 | 0 | 1 |
