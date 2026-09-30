# Bibliothèques du noyau (chemins relatifs à sys/root/src), une par composant.
# Découpage et cycles : doc/migration/dependances.md ; édition de liens en groupe (lepton_kernel).

# --- kernel/core, partie commune ---------------------------------------------------------------
set(LEPTON_CORE_SOURCES
  kernel/core/bin.c
  kernel/core/cpu.c
  kernel/core/env.c
  kernel/core/flock.c
  kernel/core/heap.c
  kernel/core/kernel_io.c
  kernel/core/kernel_printk.c
  kernel/core/kernel_ring_buffer.c
  kernel/core/malloc.c
  kernel/core/pipe.c
  kernel/core/sysctl.c
  kernel/core/systime.c
  kernel/core/time.c)
# API système POSIX (entrée par appel système __mk_syscall) : absente du noyau statique, qui n'a
# ni ordonnanceur ni appels système (doc/migration/noyau-statique.md §3).
set(LEPTON_CORE_SYSCALL_SOURCES
  kernel/core/dirent.c
  kernel/core/fcntl.c
  kernel/core/kernel_mqueue.c
  kernel/core/lib.c
  kernel/core/net.c
  kernel/core/posix_mqueue.c
  kernel/core/select.c
  kernel/core/stat.c
  kernel/core/statvfs.c
  kernel/core/system.c
  kernel/core/timer.c
  kernel/core/truncate.c
  kernel/core/wait.c)
if(NOT LEPTON_KAL_BACKEND STREQUAL "static")
  list(APPEND LEPTON_CORE_SOURCES ${LEPTON_CORE_SYSCALL_SOURCES})
endif()
lepton_add_library(lepton_core SOURCES ${LEPTON_CORE_SOURCES})

# --- micro-noyau (axe kal) ----------------------------------------------------------------------
lepton_add_library(lepton_kal_${LEPTON_KAL_BACKEND} SOURCES ${LEPTON_KAL_SOURCES})

# --- pilotes logiciels (jamais de pilote matériel ici) ------------------------------------------
lepton_add_library(lepton_dev SOURCES
  kernel/dev/dev_cpufs/dev_cpufs.c
  kernel/dev/dev_head/dev_head.c
  kernel/dev/dev_null/dev_null.c
  kernel/dev/dev_part/dev_part.c
  kernel/dev/dev_proc/dev_proc.c
  kernel/dev/dev_tty/dev_tty.c
  kernel/dev/dev_tty/tty_font-8x8.c
  kernel/dev/dev_tty/tty_font-8x16.c)

# --- VFS et systèmes de fichiers ---------------------------------------------------------------
lepton_add_library(lepton_vfs SOURCES
  kernel/fs/vfs/vfs.c
  kernel/fs/vfs/vfscore.c
  kernel/fs/vfs/vfsdev.c
  kernel/fs/vfs/vfskernel.c)
lepton_add_library(lepton_fs_rootfs SOURCES
  kernel/fs/rootfs/rootfs.c
  kernel/fs/rootfs/rootfscore.c)
lepton_add_library(lepton_fs_ufs SOURCES
  kernel/fs/ufs/ufs.c
  kernel/fs/ufs/ufscore.c
  kernel/fs/ufs/ufsdriver.c
  kernel/fs/ufs/ufsdriver_1_3.c
  kernel/fs/ufs/ufsdriver_1_4.c
  kernel/fs/ufs/ufsdriver_1_5.c
  kernel/fs/ufs/ufsx.c)

# --- libc Lepton (lib/, hors kernel/) -------------------------------------------------------------
# L'ISA peut fournir sa propre liste (hôte : adaptateur vers la glibc, cmake/isa/host.cmake).
lepton_add_library(lepton_libc SOURCES ${LEPTON_LIBC_SOURCES})

# --- pilotes matériels de la « carte » (axe carte ; hôte : disque sur fichier) -------------------
lepton_add_library(lepton_bsp_${LEPTON_BSP_NAME} SOURCES ${LEPTON_BSP_SOURCES})
if(LEPTON_BSP_HOST_SOURCES)
  # Partie hors noyau (en-têtes de la glibc) d'un pilote hôte : sans lepton_options.
  add_library(lepton_bsp_${LEPTON_BSP_NAME}_posix STATIC ${LEPTON_BSP_HOST_SOURCES})
  target_link_libraries(lepton_bsp_${LEPTON_BSP_NAME} PUBLIC lepton_bsp_${LEPTON_BSP_NAME}_posix)
endif()

# --- noyau assemblé : cycle core ↔ kal ↔ vfs ↔ fs ↔ dev ↔ libc (dependances.md, CFC 1) ------------
add_library(lepton_kernel INTERFACE)
target_link_libraries(lepton_kernel INTERFACE
  "$<LINK_GROUP:RESCAN,lepton_core,lepton_kal_${LEPTON_KAL_BACKEND},lepton_vfs,lepton_fs_rootfs,lepton_fs_ufs,lepton_dev,lepton_bsp_${LEPTON_BSP_NAME},lepton_libc>")
