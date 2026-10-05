/*
 * Configuration applicative de la carte STM32F746G-DISCO (étape 6). Les axes (micro-noyau, cœur,
 * puce STM32F746xx, nom uname) sont posés par CMake (cmake/kal, cmake/cpu, cmake/boards), pas ici.
 * Réseau : session suivante du module (pilote Ethernet STM32F7).
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

//kernel console for initd and printk dev output on /dev/console stream (USART1, ST-LINK)
#define __KERNEL_DEV_TTY "/dev/ttys1"

//configuration de la carte (fréquences, priorités)
#include "kernel/dev/bsp/stm32f746g_disco/stm32f746g_disco.h"

#endif
