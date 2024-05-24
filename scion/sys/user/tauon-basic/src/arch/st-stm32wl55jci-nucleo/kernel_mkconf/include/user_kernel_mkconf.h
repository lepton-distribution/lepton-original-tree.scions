/*===========================================
Compiler Directive
=============================================*/
#ifndef _USER_KERNEL_MKCONF_H_
#define _USER_KERNEL_MKCONF_H_


//force definition of realtime kernel
//#define __KERNEL_UCORE_FREERTOS
#define __KERNEL_UCORE_EMBOS

//see kernel/core/kernelconf.h
#ifdef __KERNEL_UCORE_FREERTOS
   #include "kernel/core/ucore/freeRTOS_8-0-0/source/arch/cortex-m4/stm32f4/FreeRTOSConfig.h"
#endif 

// xxx_hal_conf.h
#if 0
//force definition ST cpu target type
#ifndef STM32WL55xx
   #define STM32WL55xx
#endif

#endif

//force definition of cpu device
#define __tauon_cpu_device__ __tauon_cpu_device_cortexM4_stm32wlxx__

//use pipe
#define __tauon_kernel_profile__ __tauon_kernel_profile_minimal__
//#define __tauon_kernel_profile__ __tauon_kernel_profile_classic__
//#define __KERNEL_PIPE_SIZE 256

//rootfs 
#if 1
#define __KERNEL_RTFS_NODETBL_SIZE 60
#define __KERNEL_RTFS_NODE_BLOCK_NB_MAX 64
#define __KERNEL_RTFS_BLOCK_SIZE 16
#define __KERNEL_RTFS_MAX_FILENAME 8
#endif



//force EFFS for stm32f407 on olimex-stm32-p407 board
#define __file_system_profile__  __file_system_profile_classic__
#define __KERNEL_VFS_SUPPORT_EFFS   0
#define __KERNEL_VFS_SUPPORT_FATFS  0


#define __KERNEL_STDIO_PRINTF_BUFSIZ 64

#if 0
//kernel printk on /dev/console
#define __KERNEL_PRINTK
//kernel trace_printk on /dev/trace
#define __KERNEL_TRACE_PRINTK
//kernel console for initd and printk dev output on /dev/console stream
#define __KERNEL_DEV_TTY "/dev/ttys1"
#endif

//ip stack
//#define USE_UIP 
//#define USE_LWIP
//#define USE_IF_ETHERNET
//specific target include for pinout definition
//#include "kernel/dev/bsp/stm32f469i-eval/dev_stm32f469i_eval_board/dev_stm32f469i_eval_board.h"

//#define __USER_MONGOOSE_PTHREAD_STACK_SIZE   (8*1024)
//#define __USER_MONGOOSE_CGI_ENVIRONMENT_SIZE (2*1024)
//#define __USER_MONGOOSE_MAX_REQUEST_SIZE     (2*1024)

//#define USE_LWIP
//#define USE_IF_ETHERNET




/*===========================================
Includes
=============================================*/


/*===========================================
Declaration
=============================================*/

#endif
