/*
 * Premier banc du VFS et de l'UFS (ETAPE-2, tâche 3) sur le noyau statique hôte : amorçage rootfs et
 * pilotes, formatage UFS du disque hôte /dev/hd/hdc (.fsflash.o du répertoire courant), écriture
 * d'un répertoire et d'un fichier, démontage, remontage, relecture. Code de retour non nul si échec.
 */
#include <stdint.h>
#include <stdarg.h>
#include <string.h>

#include "kernel/core/kernelconf.h"
#include "kernel/core/types.h"
#include "kernel/core/fcntl.h"
#include "kernel/core/ioctl_hd.h"
#include "kernel/fs/vfs/vfstypes.h"
#include "kernel/fs/vfs/vfs.h"

int printf(const char* fmt, ...); /* glibc (unité freestanding) */

int  _vfs(void);
void _kernel_warmup_rootfs(void);
void _kernel_warmup_dev(void);
int  _kernel_warmup_rtc(void);

#define CHECK(cond, what) do { if(!(cond)) { printf("ÉCHEC : %s\n", what); return 1; } } while(0)

static const char payload[] = "Lepton : premier fichier UFS écrit par le noyau statique hôte.\n";

int main(void){
   struct vfs_formatopt_t opt;
   long dev_sz = 32L*1024L;
   char buf[sizeof(payload)+16];
   desc_t desc;
   int n;

   CHECK(_vfs()==0, "_vfs");
   _kernel_warmup_rootfs();
   _kernel_warmup_dev();
   CHECK(_kernel_warmup_rtc()==0, "horloge /dev/rtc0");

   desc = _vfs_open("/dev/hd/hdc", O_RDWR, 0);
   CHECK(desc>=0, "ouverture /dev/hd/hdc");
   CHECK(_vfs_ioctl(desc, HDSETSZ, dev_sz)==0, "HDSETSZ");
   _vfs_close(desc);

   opt.max_node = 64;
   opt.max_blk  = 256;
   opt.blk_sz   = 64;
   opt.dev_sz   = dev_sz;
   CHECK(_vfs_makefs(fs_ufs, "/dev/hd/hdc", &opt)==0, "makefs ufs");

   CHECK(_vfs_mount(fs_ufs, "/dev/hd/hdc", "/usr")==0, "montage /usr");
   CHECK(_vfs_mkdir("/usr/etc", 0)==0, "mkdir /usr/etc");
   desc = _vfs_open("/usr/etc/hello.txt", O_CREAT|O_WRONLY, 0);
   CHECK(desc>=0, "création /usr/etc/hello.txt");
   n = _vfs_write(desc, (char*)payload, sizeof(payload)-1);
   CHECK(n==(int)(sizeof(payload)-1), "écriture");
   _vfs_close(desc);
   CHECK(_vfs_umount("/usr")==0, "démontage /usr");

   CHECK(_vfs_mount(fs_ufs, "/dev/hd/hdc", "/usr")==0, "remontage /usr");
   desc = _vfs_open("/usr/etc/hello.txt", O_RDONLY, 0);
   CHECK(desc>=0, "réouverture /usr/etc/hello.txt");
   memset(buf, 0, sizeof(buf));
   n = _vfs_read(desc, buf, sizeof(buf));
   _vfs_close(desc);
   CHECK(n==(int)(sizeof(payload)-1), "taille relue");
   CHECK(memcmp(buf, payload, sizeof(payload)-1)==0, "contenu relu");
   CHECK(_vfs_umount("/usr")==0, "démontage final");

   printf("vfs_ufs : rootfs, makefs, écriture, remontage et relecture OK\n");
   return 0;
}
