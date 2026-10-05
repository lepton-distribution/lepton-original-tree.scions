/*
 * Configuration applicative de la carte SAMD21 Xplained Pro (étape 6). Les axes (micro-noyau,
 * cœur, puce, nom uname) sont posés par CMake (cmake/kal, cmake/cpu, cmake/boards), pas ici.
 * 32 Ko de RAM, sans réseau (décision 2026-10-05) : profil minimal du noyau (un tube de 32
 * octets), rootfs réduit comme l'était celui de la SAMD20 (rootfscore.h), tampons stdio réduits.
 */
#ifndef _USER_KERNEL_MKCONF_H_
#define _USER_KERNEL_MKCONF_H_

#define __tauon_kernel_profile__ __tauon_kernel_profile_minimal__
#define __file_system_profile__  __file_system_profile_classic__
#define __KERNEL_VFS_SUPPORT_EFFS   0
#define __KERNEL_VFS_SUPPORT_FATFS  0

//rootfs (RAM) réduit : valeurs de la SAMD20 (kernel/fs/rootfs/rootfscore.h, par
//__tauon_cpu_device__), reportées ici sans entrée dans la table des puces (ajout-coeur.md §4)
#define __KERNEL_RTFS_NODETBL_SIZE 32
#define __KERNEL_RTFS_NODE_BLOCK_NB_MAX 10

//BUFSIZ de stdio (lib/libc/stdio/stdio.h) : 3 tampons par processus (stdin, stdout, stderr),
//pris sur la pile du processus ; prioritaire sur kal/arch/armv6m/kal_arch_conf.h (128)
#define __KERNEL_STDIO_PRINTF_BUFSIZ (64)

//kernel printk on /dev/console
#define __KERNEL_PRINTK
//kernel console for initd and printk dev output on /dev/console stream (SERCOM3, EDBG)
#define __KERNEL_DEV_TTY "/dev/ttys3"

//configuration de la carte (fréquences, priorités)
#include "kernel/dev/bsp/samd21_xplained_pro/samd21_xplained_pro.h"

#endif
