# Carte QEMU mps2-an386 (Cortex-M4, socle de l'étape 3).
# Mémoire : ld/mem_qemu-mps2-an386.ld. Périphériques : UART CMSDK (console ttys0, ttys1),
# LAN9118 (étape 3b). Adresses et numéros d'interruption : BSP kernel/dev/bsp/qemu_mps2_an386.
if(NOT LEPTON_CPU STREQUAL "cortex-m4f")
  message(FATAL_ERROR "qemu-mps2-an386 : LEPTON_CPU=cortex-m4f attendu")
endif()
set(LEPTON_BOARD_MEMORY_LD ${CMAKE_SOURCE_DIR}/ld/mem_qemu-mps2-an386.ld)
set(LEPTON_BSP_NAME qemu_mps2_an386)
set(LEPTON_BSP_SOURCES
  kernel/dev/arch/all/uart/dev_cmsdk_uart/dev_cmsdk_uart_x.c
  kernel/dev/bsp/qemu_mps2_an386/qemu_mps2_an386_board.c)
set(LEPTON_BOARD_MKCONF sys/user/tauon-basic/etc/mkconf_tauon_basic_qemu_mps2_an386.xml)
set(LEPTON_BOARD_MKCONF_TARGET cortexm_lepton)
set(LEPTON_QEMU_MACHINE mps2-an386)

target_compile_definitions(lepton_options INTERFACE
  __tauon_cpu_device__=__tauon_cpu_device_cortexM4_qemu_mps2_an386__)
target_include_directories(lepton_options INTERFACE ${LEPTON_SRC}/kernel/dev/bsp/qemu_mps2_an386)
