/*
 * Configuration applicative de la carte NUCLEO-WL55JC1 (étape 6). Les axes (micro-noyau,
 * cœur, puce, nom uname) sont posés par CMake (cmake/kal, cmake/cpu, cmake/boards), pas ici.
 * 64 Ko de RAM, sans réseau : profil minimal du noyau, rootfs et tampons stdio du portage IAR
 * (src/arch/st-stm32wl55jci-nucleo, inchangé).
 */
#ifndef _USER_KERNEL_MKCONF_H_
#define _USER_KERNEL_MKCONF_H_

#define __tauon_kernel_profile__ __tauon_kernel_profile_minimal__
#define __file_system_profile__  __file_system_profile_classic__
#define __KERNEL_VFS_SUPPORT_EFFS   0
#define __KERNEL_VFS_SUPPORT_FATFS  0

//rootfs (RAM) : valeurs du portage IAR
#define __KERNEL_RTFS_NODETBL_SIZE 60
#define __KERNEL_RTFS_NODE_BLOCK_NB_MAX 64
#define __KERNEL_RTFS_BLOCK_SIZE 16
#define __KERNEL_RTFS_MAX_FILENAME 8

//BUFSIZ de stdio (lib/libc/stdio/stdio.h) : 3 tampons par processus, pris sur la pile du
//processus ; valeur du portage IAR, prioritaire sur kal/arch/armv7m/kal_arch_conf.h
#define __KERNEL_STDIO_PRINTF_BUFSIZ (64)

//kernel printk on /dev/console
#define __KERNEL_PRINTK
//kernel console for initd and printk dev output on /dev/console stream (USART2, STLINK-V3)
#define __KERNEL_DEV_TTY "/dev/ttys2"

//configuration de la carte (fréquences, priorités)
#include "kernel/dev/bsp/stm32wl55jci_nucleo/stm32wl55jci_nucleo.h"

#endif
