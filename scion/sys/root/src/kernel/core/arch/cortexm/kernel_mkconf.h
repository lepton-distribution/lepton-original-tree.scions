//generated from c:/tauon/sys/user/bret-interface/etc/mkconf_tauon_stm32f469i_bret-interface.xml by mklepton
/*===========================================
Compiler Directive
=============================================*/
#ifndef _KERNEL_MKCONF_H
#define _KERNEL_MKCONF_H

/*===========================================
Includes
=============================================*/

#include "c:/tauon/sys/user/bret-interface/src/arch/stm32f469i/kernel_mkconf/include/user_kernel_mkconf.h" 
#include "c:/tauon/sys/root/src/kernel/core/arch/cortexm/dev_dskimg.h" 

/*===========================================
Declaration
=============================================*/

#define CPU_ARM7 

#define __KERNEL_CPU_FREQ 80000000L

#define __KERNEL_HEAP_SIZE 10000

#define __KERNEL_PTHREAD_MAX 16

#define __KERNEL_PROCESS_MAX 15

#define MAX_OPEN_FILE 32

#define OPEN_MAX 32

#define __KERNEL_ENV_PATH {"/usr","/usr/sbin","/usr/bin","usr/bin/net"}


#endif