/*
 * Lepton — pilotes de l'hôte Linux (noyau statique de mklepton). Licence : voir LICENSE (MPL 1.1).
 *
 * Disque bloc « hdc » adossé au fichier .fsflash.o du répertoire courant : support de l'image UFS
 * construite par mklepton (décision 2026-09-30, étape 2). Successeur portable de
 * dev/arch/gnu32/dev_linux_fileflash (code gelé, appels système i386) : même nom, mêmes ioctl
 * (HDGETSZ, HDSETSZ), E/S par host_posix.c.
 */
#include <stdint.h>
#include <stdarg.h>

#include "kernel/core/types.h"
#include "kernel/core/interrupt.h"
#include "kernel/core/system.h"
#include "kernel/core/stat.h"
#include "kernel/core/fcntl.h"
#include "kernel/core/ioctl_hd.h"

#include "kernel/fs/vfs/vfsdev.h"
#include "kernel/fs/vfs/vfstypes.h"

#include "kernel/dev/arch/host/common/host_posix.h"

#define DEV_HOST_FILEFLASH_PATH ".fsflash.o"
#define DEV_HOST_FILEFLASH_DFLT_SIZE (32L*1024L)

const char dev_host_fileflash_name[]="hdc\0";

int dev_host_fileflash_load(void);
int dev_host_fileflash_open(desc_t desc, int o_flag);
int dev_host_fileflash_close(desc_t desc);
int dev_host_fileflash_read(desc_t desc, char* buf,int size);
int dev_host_fileflash_write(desc_t desc, const char* buf,int size);
int dev_host_fileflash_seek(desc_t desc,int offset,int origin);
int dev_host_fileflash_ioctl(desc_t desc,int request,va_list ap);

dev_map_t dev_host_fileflash_map={
   dev_host_fileflash_name,
   S_IFBLK,
   dev_host_fileflash_load,
   dev_host_fileflash_open,
   dev_host_fileflash_close,
   __fdev_not_implemented,
   __fdev_not_implemented,
   dev_host_fileflash_read,
   dev_host_fileflash_write,
   dev_host_fileflash_seek,
   dev_host_fileflash_ioctl
};

static int fh=-1;
static int instance_counter=0;
static long memory_size=DEV_HOST_FILEFLASH_DFLT_SIZE;

int dev_host_fileflash_load(void){
   return 0;
}

int dev_host_fileflash_open(desc_t desc, int o_flag){
   if(fh==-1) {
      long sz;
      if((fh=host_file_open(DEV_HOST_FILEFLASH_PATH))<0)
         return -1;
      sz=host_file_size(fh);
      if(sz>0)
         memory_size=sz;
   }
   instance_counter++;
   return 0;
}

int dev_host_fileflash_close(desc_t desc){
   if(fh==-1)
      return -1;
   if(--instance_counter<=0) {
      instance_counter=0;
      host_file_close(fh);
      fh=-1;
   }
   return 0;
}

int dev_host_fileflash_read(desc_t desc, char* buf,int size){
   int r;
   if(ofile_lst[desc].offset>memory_size)
      return -1;
   if(ofile_lst[desc].offset+size>memory_size)
      size=(int)(memory_size-ofile_lst[desc].offset);
   if((r=host_file_pread(fh,buf,size,(long)ofile_lst[desc].offset))<0)
      return -1;
   ofile_lst[desc].offset+=r;
   return r;
}

int dev_host_fileflash_write(desc_t desc, const char* buf,int size){
   int w;
   if(ofile_lst[desc].offset>memory_size)
      return -1;
   if(ofile_lst[desc].offset+size>memory_size)
      size=(int)(memory_size-ofile_lst[desc].offset);
   if((w=host_file_pwrite(fh,buf,size,(long)ofile_lst[desc].offset))<0)
      return -1;
   ofile_lst[desc].offset+=w;
   return w;
}

int dev_host_fileflash_seek(desc_t desc,int offset,int origin){
   long pos;
   switch(origin) {
   case SEEK_SET: pos=offset; break;
   case SEEK_CUR: pos=(long)ofile_lst[desc].offset+offset; break;
   case SEEK_END: pos=memory_size+offset; break;
   default: return -1;
   }
   if(pos<0 || pos>memory_size)
      return -1;
   ofile_lst[desc].offset=pos;
   return (int)pos;
}

int dev_host_fileflash_ioctl(desc_t desc,int request,va_list ap){
   switch(request) {
   case HDGETSZ: {
      long* hdsz_p=va_arg(ap,long*);
      if(!hdsz_p)
         return -1;
      *hdsz_p=memory_size;
   }
   break;
   case HDSETSZ: {
      long hdsz=va_arg(ap,long);
      if(hdsz<=0 || fh<0)
         return -1;
      if(host_file_resize(fh,hdsz)<0)
         return -1;
      memory_size=hdsz;
   }
   break;
   default:
      return -1;
   }
   return 0;
}
