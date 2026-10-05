# ISA armv6m : Cortex-M0/M0+ (Thumb-1). Décrit à l'étape 2, validé à l'étape 6 (SAMD21 Xplained Pro).
# Code d'architecture : kernel/core/arch/cortexm (démarrage, vecteurs), kernel/dev/arch/cortexm.
if(NOT CMAKE_C_COMPILER_ID STREQUAL "GNU" OR NOT CMAKE_SYSTEM_PROCESSOR STREQUAL "arm")
  message(FATAL_ERROR "LEPTON_ISA=armv6m exige cmake/toolchains/arm-none-eabi.cmake")
endif()

target_compile_options(lepton_options INTERFACE
  -mthumb $<$<COMPILE_LANGUAGE:C>:-std=gnu99> -ffunction-sections -fdata-sections
  ${LEPTON_OPT_LEVEL})
# newlib-nano pour le noyau et le démarrage (décision 2026-09-30), comme armv7m.
target_link_options(lepton_options INTERFACE
  -mthumb -Wl,--gc-sections --specs=nano.specs --specs=nosys.specs)
lepton_freestanding()
# CPU_CORTEXM : macro historique de famille, posée uniquement par les fichiers d'ISA Cortex-M.
target_compile_definitions(lepton_options INTERFACE CPU_CORTEXM)
# CMSIS-Core présent dans l'arbre (V3.30, core_cm0plus.h) ; en-tête de périphérique fourni par
# la carte (qui peut lui substituer le CMSIS-Core 5 de ucore/cmsis-5).
target_include_directories(lepton_options INTERFACE ${LEPTON_SRC}/kernel/core/ucore/cmsis/CMSIS/Include)
set(LEPTON_ISA_ARCH_DIR kernel/core/arch/cortexm)
# sections critiques à nom neutre (#include "lepton_irq.h", PRIMASK) : kernel/core/arch/cortexm
target_include_directories(lepton_options INTERFACE ${LEPTON_SRC}/${LEPTON_ISA_ARCH_DIR})
# KAL, axe ISA : kal_arch.h (inclus par kernel/core/kal.h, dispatcher).
set(LEPTON_KAL_ARCH_DIR kernel/core/kal/arch/armv6m)
target_include_directories(lepton_options INTERFACE ${LEPTON_SRC}/${LEPTON_KAL_ARCH_DIR})
# démarrage de la famille et _sbrk borné à __stack_limit__ (commun Cortex-M)
set(LEPTON_ISA_STARTUP_SOURCES kernel/core/arch/cortexm/startup_armv6m.c
  kernel/core/arch/cortexm/sbrk_cortexm.c)
set(LEPTON_FIRMWARE_SOURCES ${LEPTON_ISA_STARTUP_SOURCES})
