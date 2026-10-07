# ISA host : noyau Lepton statique pour Linux x86_64 (bibliothèque de mklepton), gcc de l'hôte.
#
# Le noyau est compilé en freestanding (lepton_freestanding, cmake/lepton_target.cmake) : glibc à
# l'édition de liens seulement. Format UFS : structures identiques x86_64 / i386 / arm-none-eabi
# (assertions statiques dans tests/host).

# ABI native de l'hôte (LP64 en x86_64 ; -m32 retiré à l'étape 8, décision 2026-10-07). Le va_list
# reçu par argument variadique (vfs.c, I_LINK) passe par __va_list_from_arg (kal/arch/host). gcc
# et clang (ci/run.sh). Sorties de mklepton gardées par host.mklepton_empreintes.

set(LEPTON_HOST_ARCH_DIR ${LEPTON_SRC}/kernel/core/arch/host)
# KAL, axe ISA : kal_arch.h (inclus par kernel/core/kal.h, dispatcher).
set(LEPTON_KAL_ARCH_DIR kernel/core/kal/arch/host)
target_include_directories(lepton_options INTERFACE ${LEPTON_SRC}/${LEPTON_KAL_ARCH_DIR})

# Contrôle croisé du format UFS (tests/host, host.ufs_format_arm) : ABI AAPCS de la cible de base.
set(LEPTON_HOST_ARM_CHECK_FLAGS -mcpu=cortex-m4 -mthumb)

target_compile_options(lepton_options INTERFACE -std=gnu99 -fno-common)
lepton_freestanding()
# CPU_GNU32 : macro historique de la cible « synthétique » ; seule sa branche USE_KERNEL_STATIC
# (sans ordonnanceur) est utilisée. Elle n'est posée qu'ici (axe ISA).
target_compile_definitions(lepton_options INTERFACE
  CPU_GNU32
  __KERNEL_CPU_NAME="gnu32"
  __tauon_cpu_device__=__tauon_cpu_device_gnu_synthetic__)

# libc Lepton : le noyau statique n'en exige que __sprintf et __lepton_libc_isascii, fournis par
# la glibc (lib/libc/stdio entraînerait FILE et appels système).
set(LEPTON_LIBC_SOURCES kernel/core/arch/host/libc_host.c)
# Bibliothèques système liées après le groupe du noyau (dev_part : pow).
set(LEPTON_SYSTEM_LIBS m)

# « Carte » hôte : disque sur fichier (.fsflash.o, décision 2026-09-30) et horloge. La partie
# POSIX des pilotes est compilée avec les en-têtes de la glibc, hors lepton_options.
set(LEPTON_BSP_NAME host)
set(LEPTON_BSP_SOURCES
  kernel/dev/arch/host/dev_host_fileflash/dev_host_fileflash.c
  kernel/dev/arch/host/dev_host_rtc/dev_host_rtc.c)
set(LEPTON_BSP_HOST_SOURCES ${LEPTON_SRC}/kernel/dev/arch/host/common/host_posix.c)
