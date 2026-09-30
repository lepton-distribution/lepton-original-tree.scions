/*
 * Lepton — backend « static » du noyau (aucun ordonnanceur) : noyau statique hôte de mklepton.
 * Licence : voir LICENSE (MPL 1.1).
 *
 * Successeur de core-ecos/ + arch/synthetic/x86_static/ (absents de l'arbre ; ETAPE-2, tâche 3 ;
 * doc/migration/noyau-statique.md §3). Le noyau statique est mono-fil, sans processus ni appel
 * système : mklepton appelle directement l'API interne du VFS (_vfs_*).
 *
 *  - état global du noyau (définitions de core-segger/kernel.c et process.c qui dépendent d'embOS) ;
 *  - primitives de synchronisation mono-fil : verrous et sémaphores toujours disponibles (aucun
 *    autre fil ne peut les détenir) ;
 *  - aucun processus : les fonctions de descripteurs de processus échouent (-1), elles ne sont pas
 *    atteintes par les chemins _vfs_* utilisés par mklepton ;
 *  - amorçage _kernel_warmup_{rootfs,dev,rtc,mount}, adapté de core-segger/kernel.c.
 */
#include <stdint.h>
#include <stdarg.h>
#include <string.h>

#include "kernel/core/kernelconf.h"
#include "kernel/core/types.h"
#include "kernel/core/interrupt.h"
#include "kernel/core/kernel.h"
#include "kernel/core/process.h"
#include "kernel/core/kernel_pthread.h"
#include "kernel/core/kernel_pthread_mutex.h"
#include "kernel/core/kernel_sem.h"
#include "kernel/core/fcntl.h"
#include "kernel/core/stat.h"
#include "kernel/core/time.h"
#include "kernel/core/systime.h"
#include "kernel/core/errno.h"
#include "kernel/core/dirent.h"
#include "kernel/core/pipe.h"
#include "kernel/fs/vfs/vfsdev.h"
#include "kernel/fs/vfs/vfs.h"
#include "kernel/fs/vfs/vfskernel.h"
#include "kernel/core/kernel_device.h"

/* --- état global du noyau ----------------------------------------------------------------- */
const char * _kernel_date = __DATE__;
const char * _kernel_time = __TIME__;

volatile pid_t _syscall_owner_pid=0;
kernel_pthread_t* _syscall_owner_pthread_ptr=(kernel_pthread_t*)0;
volatile int _kernel_in_static_mode=KERNEL_IN_STATIC_MODE;
int __g_kernel_static_errno=0;
kernel_pthread_mutex_t kernel_mutex;

desc_t __g_kernel_desc_tty=-1;
fdev_map_t* __g_kernel_cpu=(fdev_map_t*)0;
desc_t __g_kernel_desc_cpu=-1;

/* --- processus : aucun ---------------------------------------------------------------------- */
process_t* __process_lst[PROCESS_MAX];
process_t** process_lst=&__process_lst[0];
kernel_pthread_t* g_pthread_lst=(kernel_pthread_t*)0;

const char __fds_size = sizeof(fds_bits_t)*8;
const unsigned char __shl_fds_bits = (sizeof(fds_bits_t)+2);

int _get_fd(pid_t pid,int limit){
   return -1;
}

int _put_fd(pid_t pid,int fd){
   return -1;
}

int _unset_cloexec(pid_t pid,int fd){
   return -1;
}

int _syscall_kill(kernel_pthread_t* pthread_ptr, pid_t pid, void* data){
   return -1;
}

/* --- synchronisation mono-fil ------------------------------------------------------------- */
kernel_pthread_t* kernel_pthread_self(void){
   return (kernel_pthread_t*)0;
}

int kernel_pthread_mutex_lock(kernel_pthread_mutex_t *mutex){
   return 0;
}

int kernel_pthread_mutex_unlock(kernel_pthread_mutex_t *mutex){
   return 0;
}

int kernel_sem_init(kernel_sem_t* sem, int pshared, unsigned int value){
   return 0;
}

int kernel_sem_post(kernel_sem_t* sem){
   return 0;
}

int kernel_sem_wait(kernel_sem_t* sem){
   return 0;
}

/* Enveloppe fonctionnelle de la macro __kernel_dev_gettime (kernel.h) : appelée par mklepton, qui
 * ne voit pas les en-têtes du noyau (kernel_stub.h). */
void __wrpr_kernel_dev_gettime(desc_t __desc, char * __buf, int __size){
   __kernel_dev_gettime(__desc,__buf,__size);
}

/* --- amorçage (adapté de core-segger/kernel.c) --------------------------------------------- */
void _kernel_warmup_rootfs(void){
   _vfs_rootmnt();
   _vfs_mkdir("/dev",0);
   _vfs_mkdir("/dev/hd",0);
   _vfs_mkdir("/kernel",0);
   _vfs_mkdir("/bin",0);
   _vfs_mkdir("/usr",0);
   _vfs_mkdir("/etc",0);
   _vfs_mkdir("/var",0);
   _vfs_mkdir("/mnt",0);
}

/* Les disques « hd… » sont créés sous /dev/hd/ avec leur propre nom (le disque hôte hdc est
 * celui qu'ouvre mklepton) ; core-segger les renumérote à partir de hdb. */
void _kernel_warmup_dev(void){
   char ref[PATH_MAX+1];
   dev_t dev;

   for(dev=0; dev<max_dev; dev++) {
      char* p_ref=ref;
      if(!pdev_lst[dev])
         continue;
      if(pdev_lst[dev]->fdev_load && pdev_lst[dev]->fdev_load()<0)
         continue;
      if(pdev_lst[dev]->dev_name[0]=='h' && pdev_lst[dev]->dev_name[1]=='d')
         strcpy(ref,"/dev/hd/");
      else
         strcpy(ref,"/dev/");
      strcat(ref,pdev_lst[dev]->dev_name);
      while((*(++p_ref))!='\0') {
         if(*p_ref=='/' && p_ref>ref+4) {
            *p_ref='\0';
            _vfs_mkdir(ref,0);
            *p_ref='/';
         }
      }
      _vfs_mknod(ref,(int16_t)pdev_lst[dev]->dev_attr,dev);
   }
}

int _kernel_warmup_rtc(void){
   desc_t desc;
   char buf[8]={0};
   struct tm _tm;
   time_t t;

   if((desc=_vfs_open("/dev/rtc0",O_RDONLY,0))<0)
      return -1;
   memset(&_tm,0,sizeof(struct tm));
   __kernel_dev_gettime(desc,buf,6);
   _tm.tm_sec  = buf[0];
   _tm.tm_min  = buf[1];
   _tm.tm_hour = buf[2];
   _tm.tm_mday = buf[3];
   _tm.tm_mon  = buf[4];
   _tm.tm_year = buf[5];
   t = mktime(&_tm);
   xtime.tv_usec=0;
   xtime.tv_sec=t;
   _vfs_close(desc);
   return 0;
}

/* Aucun montage au démarrage : mklepton formate et monte lui-même hdc sur /usr. */
int _kernel_warmup_mount(void){
   return 0;
}
