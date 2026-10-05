# Cœur Cortex-M7 (étape 6 ; QEMU mps2-an500, Discovery F7). Bibliothèque embOS libosT7VHL<mode>.a
# (pas de famille M7 dédiée) : bibliothèque VFPv4-SP liée à du code fpv5-d16, validée sous QEMU
# mps2-an500 le 2026-10-05 (banc KAL T1F, T4F, T6F, T7F : en fpv5-d16, D0-D15 sont S0-S31, que la
# bibliothèque et le cadre étendu matériel couvrent). Révision r0p0/r0p1 : variante _837070 +
# USE_ERRATUM_837070=1, posés par la carte (LEPTON_EMBOS_LIB_VARIANT, cmake/kal/embos.cmake).
# ABI flottante fixe (pas de variante soft : le chemin sans FPU est couvert par le M4F soft et le
# M0+) ; lue par le banc KAL (tests/kal : variantes FPU T1F, T4F, T6F, T7F).
set(LEPTON_FLOAT_ABI hard)
# Précision de la FPU, propriété de la puce (choisie par le preset de la carte) : dp (fpv5-d16,
# STM32F76x/F77x, QEMU mps2-an500) ou sp (fpv5-sp-d16, STM32F74x/F75x). La bibliothèque embOS
# (VFPv4-SP, S16-S31) convient aux deux.
set(LEPTON_M7_FPU dp CACHE STRING "Précision de la FPU du Cortex-M7 (dp, sp)")
set_property(CACHE LEPTON_M7_FPU PROPERTY STRINGS dp sp)
if(LEPTON_M7_FPU STREQUAL "dp")
  set(cpu_flags -mcpu=cortex-m7 -mfpu=fpv5-d16 -mfloat-abi=hard)
elseif(LEPTON_M7_FPU STREQUAL "sp")
  set(cpu_flags -mcpu=cortex-m7 -mfpu=fpv5-sp-d16 -mfloat-abi=hard)
else()
  message(FATAL_ERROR "LEPTON_M7_FPU=${LEPTON_M7_FPU} : dp ou sp")
endif()
set(LEPTON_EMBOS_LIB_FAMILY T7VHL)
target_compile_options(lepton_options INTERFACE ${cpu_flags})
target_link_options(lepton_options INTERFACE ${cpu_flags})
# Nom du cœur (uname, __KERNEL_CPU_NAME de kernelconf.h).
target_compile_definitions(lepton_options INTERFACE __KERNEL_CPU_NAME="cortexm7")
# Pile du thread noyau (core-segger/kernel.c, valeur IAR du cœur).
target_compile_definitions(lepton_options INTERFACE __KERNEL_STACK_SIZE=4096)
# Cœur pour le code qui en dépend (__tauon_cpu_core__ de kernelconf.h, backend FreeRTOS).
target_compile_definitions(lepton_options INTERFACE __tauon_cpu_core__=__tauon_cpu_core_arm_cortexM7__)
# CMSIS-Core 5 (core_cm7.h, Apache-2.0 ; le CMSIS 3.20 de cmake/isa/armv7m.cmake s'arrête au M4) :
# copie non modifiée du paquet embOS (CoreSupport STM32F769I-Discovery, V5.6), décision 2026-10-05.
target_include_directories(lepton_options INTERFACE ${LEPTON_SRC}/kernel/core/ucore/cmsis-5/CMSIS/Core/Include)
