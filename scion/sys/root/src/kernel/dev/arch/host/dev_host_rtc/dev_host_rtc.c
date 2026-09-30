/*
 * Lepton — pilotes de l'hôte Linux (noyau statique de mklepton). Licence : voir LICENSE (MPL 1.1).
 *
 * Horloge « rtc0 » de l'hôte : SOURCE_DATE_EPOCH si défini (sorties de mklepton reproductibles,
 * décision 2026-09-30), sinon l'heure système (host_posix.c). Lecture seule. Successeur de
 * dev/arch/gnu32/dev_linux_rtc (code gelé). Format de gettime : 6 octets sec, min, hour, mday,
 * mon, year (comme _kernel_warmup_rtc et _mk_rtc les lisent).
 */
#include <stdint.h>
#include <stdarg.h>

#include "kernel/core/types.h"
#include "kernel/core/interrupt.h"
#include "kernel/core/system.h"
#include "kernel/core/stat.h"
#include "kernel/core/fcntl.h"

#include "kernel/fs/vfs/vfsdev.h"
#include "kernel/fs/vfs/vfstypes.h"

#include "kernel/dev/arch/host/common/host_posix.h"

const char dev_host_rtc_name[]="rtc0\0";

int dev_host_rtc_load(void);
int dev_host_rtc_open(desc_t desc, int o_flag);
int dev_host_rtc_close(desc_t desc);
int dev_host_rtc_settime(desc_t desc,char* buf,int size);
int dev_host_rtc_gettime(desc_t desc,char* buf,int size);

dev_rtc_t dev_host_rtc_ext={
   dev_host_rtc_settime,
   dev_host_rtc_gettime
};

dev_map_t dev_host_rtc_map={
   dev_host_rtc_name,
   S_IFBLK,
   dev_host_rtc_load,
   dev_host_rtc_open,
   dev_host_rtc_close,
   __fdev_not_implemented,
   __fdev_not_implemented,
   __fdev_not_implemented,
   __fdev_not_implemented,
   __fdev_not_implemented,
   __fdev_not_implemented,
   (pfdev_ext_t)&dev_host_rtc_ext
};

int dev_host_rtc_load(void){
   return 0;
}

int dev_host_rtc_open(desc_t desc, int o_flag){
   ofile_lst[desc].offset=0;
   return 0;
}

int dev_host_rtc_close(desc_t desc){
   return 0;
}

int dev_host_rtc_settime(desc_t desc,char* buf,int size){
   return -1;
}

int dev_host_rtc_gettime(desc_t desc,char* buf,int size){
   int f[6];
   int i;
   if(size<6 || host_clock_now(f)<0)
      return -1;
   for(i=0; i<6; i++)
      buf[i]=(char)f[i];
   return 6;
}
