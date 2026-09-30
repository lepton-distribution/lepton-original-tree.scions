# ISA armv7m : Cortex-M3, M4(F), M7 (Thumb-2). Décrit à l'étape 2, validé à l'étape 3 (QEMU).
# Code d'architecture : kernel/core/arch/cortexm (existant), kernel/dev/arch/cortexm.
if(NOT CMAKE_C_COMPILER_ID STREQUAL "GNU" OR NOT CMAKE_SYSTEM_PROCESSOR STREQUAL "arm")
  message(FATAL_ERROR "LEPTON_ISA=armv7m exige cmake/toolchains/arm-none-eabi.cmake")
endif()

target_compile_options(lepton_options INTERFACE
  -mthumb -std=gnu99 -ffunction-sections -fdata-sections)
target_link_options(lepton_options INTERFACE -mthumb -Wl,--gc-sections)
# CPU_CORTEXM : macro historique de famille, posée uniquement ici (axe ISA).
target_compile_definitions(lepton_options INTERFACE CPU_CORTEXM)
set(LEPTON_ISA_ARCH_DIR kernel/core/arch/cortexm)
