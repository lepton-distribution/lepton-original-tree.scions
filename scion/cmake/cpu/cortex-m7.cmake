# Cœur Cortex-M7 (étape 6 ; QEMU mps2-an500, Discovery F7). Bibliothèque embOS libosT7VHL<mode>.a
# (pas de famille M7 dédiée). HYPOTHÈSE À VALIDER (étape 6, embos-inventaire.md) : bibliothèque
# VFPv4-SP liée à du code fpv5-d16. Révision r0p0/r0p1 : variante _837070 + USE_ERRATUM_837070=1.
set(cpu_flags -mcpu=cortex-m7 -mfpu=fpv5-d16 -mfloat-abi=hard)
set(LEPTON_EMBOS_LIB_FAMILY T7VHL)
target_compile_options(lepton_options INTERFACE ${cpu_flags})
target_link_options(lepton_options INTERFACE ${cpu_flags})
# Nom du cœur (uname, __KERNEL_CPU_NAME de kernelconf.h).
target_compile_definitions(lepton_options INTERFACE __KERNEL_CPU_NAME="cortexm7")
# Pile du thread noyau (core-segger/kernel.c, valeur IAR du cœur).
target_compile_definitions(lepton_options INTERFACE __KERNEL_STACK_SIZE=4096)
