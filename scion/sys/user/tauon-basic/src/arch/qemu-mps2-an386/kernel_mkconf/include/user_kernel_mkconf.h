/*
 * Configuration applicative du socle QEMU mps2-an386 (étape 3). Les axes (micro-noyau, cœur,
 * périphérique __tauon_cpu_device__) sont posés par CMake (cmake/kal, cmake/boards), pas ici.
 */
#ifndef _USER_KERNEL_MKCONF_H_
#define _USER_KERNEL_MKCONF_H_

#define __tauon_kernel_profile__ __tauon_kernel_profile_classic__
#define __KERNEL_PIPE_SIZE 1024
#define __KERNEL_RTFS_BLOCK_SIZE 16

#define __file_system_profile__  __file_system_profile_classic__
#define __KERNEL_VFS_SUPPORT_EFFS   0
#define __KERNEL_VFS_SUPPORT_FATFS  0

//kernel printk on /dev/console
#define __KERNEL_PRINTK

//kernel console for initd and printk dev output on /dev/console stream
#define __KERNEL_DEV_TTY "/dev/ttys0"

//ip stack (étape 3b) : lwIP sur Ethernet (eth0 : LAN9118)
#define USE_LWIP
#define USE_IF_ETHERNET

#endif
