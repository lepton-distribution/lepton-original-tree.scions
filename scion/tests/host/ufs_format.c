/*
 * Format de l'image UFS : structures écrites sur le support (doc/migration/noyau-statique.md §4).
 * Compilé pour l'hôte (noyau statique, -m32) ET par arm-none-eabi-gcc (test host.ufs_format_arm) :
 * les mêmes assertions garantissent qu'une image construite par mklepton est lue telle quelle
 * par la cible ARM. Le superbloc est écrit champ par champ (pas de sizeof(superblk_t)).
 */
#include <stdint.h>
#include <stddef.h>

#include "kernel/core/kernel.h"
#include "kernel/core/bin.h"
#include "kernel/fs/vfs/vfstypes.h"
#include "kernel/fs/ufs/ufscore.h"

#define UFS_ASSERT_SIZE(t, n)       _Static_assert(sizeof(t) == (n), "sizeof(" #t ") != " #n)
#define UFS_ASSERT_OFFSET(t, m, n)  _Static_assert(offsetof(t, m) == (n), "offsetof(" #t ", " #m ") != " #n)

/* nœud (pilote 1.5 = ufs_block_node_1_4_t) */
UFS_ASSERT_SIZE(ufs_block_node_1_4_t, 24);
UFS_ASSERT_OFFSET(ufs_block_node_1_4_t, size, 4);
UFS_ASSERT_OFFSET(ufs_block_node_1_4_t, ino_mod, 8);
UFS_ASSERT_OFFSET(ufs_block_node_1_4_t, cmtime, 12);
UFS_ASSERT_OFFSET(ufs_block_node_1_4_t, blk, 16);
UFS_ASSERT_OFFSET(ufs_block_node_1_4_t, blk_smpl, 18);
UFS_ASSERT_OFFSET(ufs_block_node_1_4_t, blk_dbl, 20);
/* entrée de répertoire, types de base */
UFS_ASSERT_SIZE(ufs_block_dir_t, 16);
UFS_ASSERT_SIZE(ino_mod_t, 2);
UFS_ASSERT_SIZE(time_t, 4);
UFS_ASSERT_SIZE(inodenb_t, 4);
UFS_ASSERT_SIZE(blocknb_t, 2);
/* en-tête des pseudo-binaires écrit par mklepton */
UFS_ASSERT_SIZE(exec_file_t, 8);
UFS_ASSERT_OFFSET(exec_file_t, priority, 1);
UFS_ASSERT_OFFSET(exec_file_t, stacksize, 2);
UFS_ASSERT_OFFSET(exec_file_t, timeslice, 4);
UFS_ASSERT_OFFSET(exec_file_t, index, 6);

/* unité non vide */
int ufs_format_checked = 1;
