# Carte SAMD21 Xplained Pro (Cortex-M0+ r0p1, ATSAMD21J18A), étape 6. Sans réseau (décision
# 2026-10-05). Mémoire : ld/mem_samd21-xplained-pro.ld (256 Ko / 32 Ko). Périphériques : SERCOM3
# en USART (TX PA22 PAD0, RX PA23 PAD1, mux C : port série virtuel de l'EDBG, console ttys3 ;
# en-tête de carte ASF samd21_xplained_pro.h). Horloges : BSP kernel/dev/bsp/samd21_xplained_pro.
# Paquet constructeur : Microchip SAMD21_DFP 3.8.270 (ucore/cmsis-5/Device/Microchip/SAMD21,
# LEPTON-PROVENANCE.md). DID 0x10010100 (SAMD21J18A rév. B), CPUID 0x410CC601 relus par la sonde.
if(NOT LEPTON_CPU STREQUAL "cortex-m0plus")
  message(FATAL_ERROR "samd21-xplained-pro : LEPTON_CPU=cortex-m0plus attendu")
endif()
set(LEPTON_BOARD_MEMORY_LD ${CMAKE_SOURCE_DIR}/ld/mem_samd21-xplained-pro.ld)
set(LEPTON_BSP_NAME samd21_xplained_pro)
set(samd21 kernel/dev/arch/cortexm/samd21)
set(LEPTON_BSP_SOURCES
  kernel/dev/bsp/samd21_xplained_pro/samd21_xplained_pro_board.c
  # couche SAMD21 de Lepton
  ${samd21}/dev_samd21/dev_samd21_uart_x.c)
set(LEPTON_BOARD_MKCONF sys/user/tauon-basic/etc/mkconf_tauon_basic_samd21_xplained_pro.xml)
# FreeRTOS (étape 7, module 7.3 ; décision 2026-10-06) : piles idle 128 mots et temporisateurs
# 256 mots (défauts 256 et 512) : avec 1 Ko et 2 Ko, le tas ne laisse que 484 o après le
# démarrage et lsh ne peut plus lancer de commande (pics relevés : idle 88 o, temporisateurs 236 o)
set(LEPTON_FREERTOS_CONFIG configMINIMAL_STACK_SIZE=128 configTIMER_TASK_STACK_DEPTH=256)
set(LEPTON_BOARD_MKCONF_TARGET cortexm_lepton)
set(LEPTON_BOARD_UNAME_MACHINE cortexM0p-samd21)
# carte réelle : console série de l'EDBG et sonde OpenOCD (CMSIS-DAP)
set(LEPTON_BOARD_OPENOCD_CFG ${CMAKE_SOURCE_DIR}/debug/openocd-samd21-xplained-pro.cfg)
set(LEPTON_BOARD_SERIAL_PORT "" CACHE STRING
    "Port série de la console de la carte (ex. /dev/ttyACM0) : active le test de fumée sur carte")

# nom vu par uname : posé ici, sans entrée dans la table de kernelconf.h (ajout-coeur.md §4)
target_compile_definitions(lepton_options INTERFACE
  __KERNEL_CPU_DEVICE_NAME="${LEPTON_BOARD_UNAME_MACHINE}"
  __SAMD21J18A__)
# CMSIS-Core 5 (core_cm0plus.h V5.0.9, exigé par le DFP) avant le CMSIS 3.30 de l'ISA
target_include_directories(lepton_options BEFORE INTERFACE
  ${LEPTON_SRC}/kernel/core/ucore/cmsis-5/CMSIS/Core/Include)
target_include_directories(lepton_options INTERFACE
  ${LEPTON_SRC}/kernel/dev/bsp/samd21_xplained_pro
  ${LEPTON_SRC}/${samd21}/dev_samd21
  ${LEPTON_SRC}/kernel/core/ucore/cmsis-5/Device/Microchip/SAMD21/include)
