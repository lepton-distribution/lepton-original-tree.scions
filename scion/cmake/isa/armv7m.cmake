# ISA armv7m : Cortex-M3, M4(F), M7 (Thumb-2). Validé sur QEMU mps2-an386 (étape 3).
# Code d'architecture : kernel/core/arch/cortexm (démarrage, vecteurs), kernel/dev/arch/cortexm.
if(NOT CMAKE_C_COMPILER_ID STREQUAL "GNU" OR NOT CMAKE_SYSTEM_PROCESSOR STREQUAL "arm")
  message(FATAL_ERROR "LEPTON_ISA=armv7m exige cmake/toolchains/arm-none-eabi.cmake")
endif()

target_compile_options(lepton_options INTERFACE
  -mthumb $<$<COMPILE_LANGUAGE:C>:-std=gnu99> -ffunction-sections -fdata-sections)
# newlib-nano pour le noyau et le démarrage (décision 2026-09-30) ; appels système newlib
# minimaux (nosys) : l'API POSIX applicative est celle de Lepton.
target_link_options(lepton_options INTERFACE
  -mthumb -Wl,--gc-sections --specs=nano.specs --specs=nosys.specs)
lepton_freestanding()
# CPU_CORTEXM : macro historique de famille, posée uniquement ici (axe ISA).
target_compile_definitions(lepton_options INTERFACE CPU_CORTEXM)
# CMSIS-Core présent dans l'arbre (V3.30) ; en-tête de périphérique fourni par la carte.
target_include_directories(lepton_options INTERFACE ${LEPTON_SRC}/kernel/core/ucore/cmsis/CMSIS/Include)
set(LEPTON_ISA_ARCH_DIR kernel/core/arch/cortexm)
# sections critiques à nom neutre (#include "lepton_irq.h") : kernel/core/arch/cortexm
target_include_directories(lepton_options INTERFACE ${LEPTON_SRC}/${LEPTON_ISA_ARCH_DIR})
set(LEPTON_ISA_STARTUP_SOURCES kernel/core/arch/cortexm/startup_armv7m.c)
set(LEPTON_FIRMWARE_SOURCES ${LEPTON_ISA_STARTUP_SOURCES})
