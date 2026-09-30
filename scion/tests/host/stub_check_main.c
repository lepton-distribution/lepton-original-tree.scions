/*
 * Concordance de tools/mklepton/src/kernel_stub.h avec le noyau — côté mklepton (en-têtes de la
 * glibc + kernel_stub.h). Même ordre que stub_check_kernel.c. Code de retour non nul si écart.
 */
#include <stdio.h>
#include <stddef.h>
#include <stdint.h>

#include "kernel_stub.h"

extern const long stub_check_kernel[];
extern const int stub_check_count;

#define K(v) (long)(v)

static const char* const names[] = {
   "sizeof vfs_formatopt_t", "offsetof vfs_formatopt_t.dev_sz", "sizeof statvfs",
   "offsetof statvfs.f_blocks", "offsetof statvfs.f_files", "offsetof statvfs.f_namemax",
   "sizeof exec_file_t", "sizeof timeval", "sizeof tm", "sizeof mode_t", "sizeof desc_t",
   "fs_rootfs", "fs_ufs", "HDGETSZ", "HDSETSZ", "EXEC_SIGNT",
};

static const long stub[] = {
   K(sizeof(struct _vfs_formatopt_t)),
   K(offsetof(struct _vfs_formatopt_t, dev_sz)),
   K(sizeof(struct _vfs_statvfs_st)),
   K(offsetof(struct _vfs_statvfs_st, f_blocks)),
   K(offsetof(struct _vfs_statvfs_st, f_files)),
   K(offsetof(struct _vfs_statvfs_st, f_namemax)),
   K(sizeof(exec_file_t)),
   K(sizeof(struct __k_timeval)),
   K(sizeof(struct k_tm)),
   K(sizeof(_vfs_mode_t)),
   K(sizeof(_vfs_desc_t)),
   K(fs_rootfs),
   K(fs_ufs),
   K(HDGETSZ),
   K(HDSETSZ),
   K(EXEC_SIGNT),
};

int main(void){
   int i, err = 0;
   int n = (int)(sizeof(stub)/sizeof(stub[0]));
   if(n != stub_check_count) {
      printf("nombre de valeurs différent : stub %d, noyau %d\n", n, stub_check_count);
      return 1;
   }
   for(i = 0; i < n; i++) {
      if(stub[i] != stub_check_kernel[i]) {
         printf("ÉCART %-34s stub %ld, noyau %ld\n", names[i], stub[i], stub_check_kernel[i]);
         err = 1;
      }
   }
   if(!err)
      printf("kernel_stub.h concorde avec le noyau (%d valeurs)\n", n);
   return err;
}
