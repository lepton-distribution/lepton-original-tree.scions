# Conditions d'ISA, de cœur et de puce hors des répertoires d'architecture

Généré par `tools/migration/audit_isa_ifdef.py` — ne pas éditer à la main. Occurrences : `isa-ifdef.csv`.

Critère d'ETAPE-4 : aucune directive d'**ISA** ou de **cœur** dans le code Lepton actif hors de `kal/arch/` et des répertoires d'architecture. L'axe **puce** (`__tauon_cpu_device__`) relève de la carte : autorisé dans `dev/arch` et `dev/bsp`, listé ici pour la décomposition.

| Mesure | Valeur |
|---|---:|
| Code Lepton, ISA/cœur hors arch (directives) | **51** |
| Code Lepton, ISA/cœur hors arch (fichiers) | **19** |
| Code Lepton, puce seule hors arch/BSP (directives) | 12 |
| Code Lepton, emplacements autorisés | 1 |
| Code tiers (non modifié, D1a) | 40 |

## Code Lepton hors arch, par fichier

| Fichier | isa | cœur | puce |
|---|---:|---:|---:|
| `sys/root/src/kernel/core/kernelconf.h` | 1 | 11 | 11 |
| `sys/root/src/kernel/core/malloc.c` | 8 | 0 | 0 |
| `sys/root/src/kernel/core/kernel.h` | 4 | 0 | 0 |
| `sys/root/src/lib/libc/stdio/stdio.h` | 4 | 0 | 0 |
| `sys/root/src/kernel/fs/rootfs/rootfscore.c` | 3 | 0 | 0 |
| `sys/user/tauon-basic/src/bin/free/dlmalloc.c` | 3 | 0 | 0 |
| `sys/root/src/kernel/core/core-segger/kernel.c` | 0 | 2 | 0 |
| `sys/root/src/kernel/core/net/lwip_core/ethif_core.c` | 2 | 0 | 0 |
| `sys/root/src/kernel/fs/vfs/vfstypes.h` | 2 | 0 | 0 |
| `sys/root/src/sbin/xmodem.c` | 2 | 0 | 0 |
| `sys/root/src/kernel/core/kernel_pthread.h` | 1 | 0 | 0 |
| `sys/root/src/kernel/core/timer.h` | 1 | 0 | 0 |
| `sys/root/src/kernel/fs/fat/fat16.c` | 1 | 0 | 0 |
| `sys/root/src/kernel/fs/fat/fatcore.h` | 1 | 0 | 0 |
| `sys/root/src/kernel/fs/rootfs/rootfscore.h` | 0 | 0 | 1 |
| `sys/root/src/kernel/fs/ufs/ufs.c` | 1 | 0 | 0 |
| `sys/root/src/kernel/fs/ufs/ufsx.c` | 1 | 0 | 0 |
| `sys/root/src/lib/libc/ctype/ctype.h` | 1 | 0 | 0 |
| `sys/root/src/sbin/initd.c` | 1 | 0 | 0 |
| `sys/root/src/sbin/lsh.c` | 1 | 0 | 0 |
