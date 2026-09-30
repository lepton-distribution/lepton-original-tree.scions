/*
 * Concordance de tools/mklepton/src/kernel_stub.h avec le noyau — côté noyau (en-têtes Lepton,
 * freestanding). Le côté mklepton (stub_check_main.c) compare ces valeurs aux siennes.
 */
#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>

#include "kernel/core/kernel.h"
#include "kernel/core/bin.h"
#include "kernel/core/time.h"
#include "kernel/core/ioctl_hd.h"
#include "kernel/fs/vfs/vfstypes.h"

#define K(v) (long)(v)

const long stub_check_kernel[] = {
   K(sizeof(struct vfs_formatopt_t)),
   K(offsetof(struct vfs_formatopt_t, dev_sz)),
   K(sizeof(struct statvfs)),
   K(offsetof(struct statvfs, f_blocks)),
   K(offsetof(struct statvfs, f_files)),
   K(offsetof(struct statvfs, f_namemax)),
   K(sizeof(exec_file_t)),
   K(sizeof(struct __timeval)),
   K(sizeof(struct tm)),
   K(sizeof(mode_t)),
   K(sizeof(desc_t)),
   K(fs_rootfs),
   K(fs_ufs),
   K(HDGETSZ),
   K(HDSETSZ),
   K(EXEC_SIGNT),
};
const int stub_check_count = sizeof(stub_check_kernel)/sizeof(stub_check_kernel[0]);
