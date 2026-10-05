# Cœur Cortex-M7 (étape 6 ; QEMU mps2-an500, Discovery F7). Bibliothèque embOS libosT7VHL<mode>.a
# (pas de famille M7 dédiée) : bibliothèque VFPv4-SP liée à du code fpv5-d16, validée sous QEMU
# mps2-an500 le 2026-10-05 (banc KAL T1F, T4F, T6F, T7F : en fpv5-d16, D0-D15 sont S0-S31, que la
# bibliothèque et le cadre étendu matériel couvrent). Révision r0p0/r0p1 : variante _837070 +
# USE_ERRATUM_837070=1 (carte). STM32F746 : FPU simple précision (fpv5-sp-d16), à choisir pour
# cette carte (étape 6, session STM32F746G-DISCO).
# ABI flottante fixe (pas de variante soft : le chemin sans FPU est couvert par le M4F soft et le
# M0+) ; lue par le banc KAL (tests/kal : variantes FPU T1F, T4F, T6F, T7F).
set(LEPTON_FLOAT_ABI hard)
set(cpu_flags -mcpu=cortex-m7 -mfpu=fpv5-d16 -mfloat-abi=hard)
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
