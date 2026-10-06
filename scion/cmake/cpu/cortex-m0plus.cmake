# Cœur Cortex-M0+ (étape 6 ; carte M0+ à choisir, QEMU microbit = M0). Bibliothèque embOS
# libosT6L<mode>.a (pas de variante VFP en ARMv6-M).
# sans FPU : banc KAL sans variantes FPU (tests/kal)
set(LEPTON_FLOAT_ABI soft)
set(cpu_flags -mcpu=cortex-m0plus -mfloat-abi=soft)
set(LEPTON_EMBOS_LIB_FAMILY T6L)
set(LEPTON_FREERTOS_PORT ARM_CM0)   # FreeRTOS : port GCC (cmake/kal/freertos.cmake)
target_compile_options(lepton_options INTERFACE ${cpu_flags})
target_link_options(lepton_options INTERFACE ${cpu_flags})
# Nom du cœur (uname, __KERNEL_CPU_NAME de kernelconf.h).
target_compile_definitions(lepton_options INTERFACE __KERNEL_CPU_NAME="cortexm0")
# Pile du thread noyau (core-segger/kernel.c, valeur IAR du cœur).
target_compile_definitions(lepton_options INTERFACE __KERNEL_STACK_SIZE=2048)
# Cœur pour le code qui en dépend (__tauon_cpu_core__ de kernelconf.h, backend FreeRTOS).
target_compile_definitions(lepton_options INTERFACE __tauon_cpu_core__=__tauon_cpu_core_arm_cortexM0__)
