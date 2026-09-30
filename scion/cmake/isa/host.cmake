# ISA host : noyau Lepton statique pour Linux x86_64 (bibliothèque de mklepton), gcc de l'hôte.
#
# Le noyau est compilé en freestanding : ses types (pid_t, off_t, time_t…) entrent en conflit avec
# ceux de la glibc. Les seuls en-têtes système visibles sont ceux de gcc (stdint, stdarg, stddef,
# limits) et les déclarations minimales de kernel/core/arch/host/include/libc (ABI glibc à
# l'édition de liens). Format UFS : structures identiques i386 / arm-none-eabi (assertions
# statiques dans tests/host).

# ILP32 (-m32, décision 2026-09-30) : va_list scalaire et dispositions de structures identiques à
# arm-none-eabi ; le noyau transmet des va_list par argument variadique (vfs.c, I_LINK), ce que
# l'ABI x86_64 (va_list tableau) ne permet pas. Paquets : gcc-multilib, libexpat1-dev:i386.
add_compile_options(-m32)
add_link_options(-m32)

execute_process(COMMAND ${CMAKE_C_COMPILER} -m32 -print-file-name=include
                OUTPUT_VARIABLE LEPTON_HOST_GCC_INCLUDE OUTPUT_STRIP_TRAILING_WHITESPACE)

set(LEPTON_HOST_ARCH_DIR ${LEPTON_SRC}/kernel/core/arch/host)

target_compile_options(lepton_options INTERFACE
  -std=gnu99 -ffreestanding -nostdinc -fno-common
  "SHELL:-isystem ${LEPTON_HOST_ARCH_DIR}/include/libc"
  "SHELL:-isystem ${LEPTON_HOST_GCC_INCLUDE}")
# CPU_GNU32 : macro historique de la cible « synthétique » ; seule sa branche USE_KERNEL_STATIC
# (sans ordonnanceur) est utilisée. Elle n'est posée qu'ici (axe ISA).
target_compile_definitions(lepton_options INTERFACE
  CPU_GNU32
  __tauon_cpu_device__=__tauon_cpu_device_gnu_synthetic__)

# libc Lepton : le noyau statique n'en exige que __sprintf et __lepton_libc_isascii, fournis par
# la glibc (lib/libc/stdio entraînerait FILE et appels système).
set(LEPTON_LIBC_SOURCES kernel/core/arch/host/libc_host.c)

# « Carte » hôte : disque sur fichier (.fsflash.o, décision 2026-09-30) et horloge. La partie
# POSIX des pilotes est compilée avec les en-têtes de la glibc, hors lepton_options.
set(LEPTON_BSP_NAME host)
set(LEPTON_BSP_SOURCES
  kernel/dev/arch/host/dev_host_fileflash/dev_host_fileflash.c
  kernel/dev/arch/host/dev_host_rtc/dev_host_rtc.c)
set(LEPTON_BSP_HOST_SOURCES ${LEPTON_SRC}/kernel/dev/arch/host/common/host_posix.c)
