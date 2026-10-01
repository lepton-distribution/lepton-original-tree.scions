# ISA armv6m : Cortex-M0/M0+ (Thumb-1). Décrit à l'étape 2, validé à l'étape 6.
if(NOT CMAKE_C_COMPILER_ID STREQUAL "GNU" OR NOT CMAKE_SYSTEM_PROCESSOR STREQUAL "arm")
  message(FATAL_ERROR "LEPTON_ISA=armv6m exige cmake/toolchains/arm-none-eabi.cmake")
endif()

target_compile_options(lepton_options INTERFACE
  -mthumb -std=gnu99 -ffunction-sections -fdata-sections)
target_link_options(lepton_options INTERFACE -mthumb -Wl,--gc-sections)
target_compile_definitions(lepton_options INTERFACE CPU_CORTEXM)
set(LEPTON_ISA_ARCH_DIR kernel/core/arch/cortexm)
# sections critiques à nom neutre (#include "lepton_irq.h") : kernel/core/arch/cortexm
target_include_directories(lepton_options INTERFACE ${LEPTON_SRC}/${LEPTON_ISA_ARCH_DIR})
# KAL, axe ISA : kernel/core/kal/arch/armv6m/ (kal_arch.h) à créer à l'étape 6.
