# Cœur Cortex-M4F (NUCLEO-F439ZI, QEMU mps2-an386). Flags ↔ bibliothèque embOS :
# doc/migration/embos-inventaire.md §3.
#   hard : -mfpu=fpv4-sp-d16 -mfloat-abi=hard → libosT7VHL<mode>.a (cible, CLAUDE.md)
#   soft : -mfloat-abi=soft                   → libosT7L<mode>.a (équivalent de l'IAR actuel)
# Choix du premier palier : décision de l'étape 3.
set(LEPTON_FLOAT_ABI hard CACHE STRING "ABI flottante du Cortex-M4F (hard, soft)")
set_property(CACHE LEPTON_FLOAT_ABI PROPERTY STRINGS hard soft)

if(LEPTON_FLOAT_ABI STREQUAL "hard")
  set(cpu_flags -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=hard)
  set(LEPTON_EMBOS_LIB_FAMILY T7VHL)
elseif(LEPTON_FLOAT_ABI STREQUAL "soft")
  set(cpu_flags -mcpu=cortex-m4 -mfloat-abi=soft)
  set(LEPTON_EMBOS_LIB_FAMILY T7L)
else()
  message(FATAL_ERROR "LEPTON_FLOAT_ABI=${LEPTON_FLOAT_ABI} : hard ou soft")
endif()
target_compile_options(lepton_options INTERFACE ${cpu_flags})
target_link_options(lepton_options INTERFACE ${cpu_flags})
# Nom du cœur (uname, __KERNEL_CPU_NAME de kernelconf.h).
target_compile_definitions(lepton_options INTERFACE __KERNEL_CPU_NAME="cortexm4")
