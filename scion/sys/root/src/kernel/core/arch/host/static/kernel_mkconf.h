/*
 * Lepton — configuration fixe du noyau statique hôte (bibliothèque de mklepton).
 * Écrite à la main : elle ne peut pas être générée par mklepton, qui dépend de ce noyau
 * (ETAPE-2, tâche 3). Valeurs reprises de kernel/core/arch/win32/kernel_mkconf.h (généré).
 * Trouvé par chemin d'inclusion (cmake/kal/static.cmake), comme les kernel_mkconf.h générés.
 */
#ifndef _KERNEL_MKCONF_H
#define _KERNEL_MKCONF_H

#include "dev_dskimg.h"

#define __KERNEL_CPU_FREQ 10000000L
#define __KERNEL_HEAP_SIZE 10000
#define __KERNEL_PTHREAD_MAX 11
#define __KERNEL_PROCESS_MAX 10
#define MAX_OPEN_FILE 64
#define OPEN_MAX 24
#define __KERNEL_ENV_PATH {"/usr","/usr/sbin","/usr/bin"}

/* Systèmes de fichiers de mklepton seulement (guide §1.1 : rootfs, ufs) ; ni kofs ni fat. */
#define __file_system_profile__ __file_system_profile_user_defined__
#define __KERNEL_VFS_SUPPORT_ROOTFS 1
#define __KERNEL_VFS_SUPPORT_UFS    1
#define __KERNEL_VFS_SUPPORT_UFSX   1
#define __KERNEL_VFS_SUPPORT_KOFS   0
#define __KERNEL_VFS_SUPPORT_MSDOS  0
#define __KERNEL_VFS_SUPPORT_VFAT   0
#define __KERNEL_VFS_SUPPORT_YAFFS  0

#endif
