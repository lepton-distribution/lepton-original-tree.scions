/*
 * Relecture d'une image UFS produite par mklepton (.fsflash.o du répertoire courant, copie de
 * travail) : montage sur /usr par le noyau statique, puis, pour chaque chemin donné en argument,
 * ouverture et lecture de l'en-tête exec_file_t (signature EXEC_SIGNT des pseudo-binaires).
 * Usage : test_ufs_check <chemin>...  (chemins sous /usr). Code de retour non nul si échec.
 */
#include <stdint.h>
#include <stdarg.h>
#include <string.h>

#include "kernel/core/kernelconf.h"
#include "kernel/core/types.h"
#include "kernel/core/fcntl.h"
#include "kernel/core/bin.h"
#include "kernel/fs/vfs/vfstypes.h"
#include "kernel/fs/vfs/vfs.h"

int printf(const char* fmt, ...); /* glibc (unité freestanding) */

int  _vfs(void);
void _kernel_warmup_rootfs(void);
void _kernel_warmup_dev(void);

int main(int argc, char* argv[]){
   int i, err = 0;

   if(_vfs()!=0) {
      printf("ÉCHEC : _vfs\n");
      return 1;
   }
   _kernel_warmup_rootfs();
   _kernel_warmup_dev();
   if(_vfs_mount(fs_ufs, "/dev/hd/hdc", "/usr")!=0) {
      printf("ÉCHEC : montage de l'image sur /usr\n");
      return 1;
   }
   for(i = 1; i < argc; i++) {
      exec_file_t hdr;
      desc_t desc = _vfs_open(argv[i], O_RDONLY, 0);
      if(desc < 0) {
         printf("ABSENT  %s\n", argv[i]);
         err = 1;
         continue;
      }
      memset(&hdr, 0, sizeof(hdr));
      if(_vfs_read(desc, (char*)&hdr, sizeof(hdr)) != (int)sizeof(hdr) || hdr.signature != EXEC_SIGNT) {
         printf("EN-TÊTE %s : signature 0x%02x\n", argv[i], (unsigned)(unsigned char)hdr.signature);
         err = 1;
      }
      _vfs_close(desc);
   }
   _vfs_umount("/usr");
   if(!err)
      printf("ufs_check : %d pseudo-binaire(s) relu(s) dans l'image\n", argc-1);
   return err;
}
