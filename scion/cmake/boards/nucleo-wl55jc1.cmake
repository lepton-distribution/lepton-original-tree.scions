# Carte NUCLEO-WL55JC1 (STM32WL55JC, Cortex-M4 r0p1 sans FPU + Cortex-M0+ arrêté), étape 6.
# Lepton sur le CPU1 (Cortex-M4) seul, CPU2 jamais démarré (décision 2026-10-05).
# Mémoire : ld/mem_nucleo-wl55jc1.ld. Périphériques : USART2 (PA2/PA3, port série virtuel du
# STLINK-V3, console ttys2). Horloge (MSI 48 MHz), priorités, interruptions : BSP
# kernel/dev/bsp/stm32wl55jci_nucleo. DEV_ID 0x497, CPUID 0x410FC241, flash 256 Ko, RDP 0, ESE 0,
# relus par la sonde (2026-10-05).
# Cœur sans FPU : axe cortex-m4f en ABI soft (libosT7L), comme le preset QEMU an386 soft.
if(NOT LEPTON_CPU STREQUAL "cortex-m4f" OR NOT LEPTON_FLOAT_ABI STREQUAL "soft")
  message(FATAL_ERROR "nucleo-wl55jc1 : LEPTON_CPU=cortex-m4f et LEPTON_FLOAT_ABI=soft attendus")
endif()
set(LEPTON_BOARD_MEMORY_LD ${CMAKE_SOURCE_DIR}/ld/mem_nucleo-wl55jc1.ld)
set(LEPTON_BSP_NAME stm32wl55jci_nucleo)
set(stm32wl kernel/dev/arch/cortexm/stm32wlxx)
set(wlbsp kernel/dev/bsp/stm32wl55jci_nucleo)
set(LEPTON_BSP_SOURCES
  ${wlbsp}/stm32wl55jci_nucleo_system.c
  ${wlbsp}/dev_stm32wl55jci_nucleo_peripherals/dev_stm32wl55jci_nucleo_usart_2.c
  # couche STM32WL de Lepton
  ${stm32wl}/dev_stm32wlxx/dev_stm32wlxx_uart_x.c
  ${stm32wl}/dev_stm32wlxx/dev_stm32wlxx_hal_tick.c
  # HAL ST (tiers, STM32CubeWL, HAL V1.3.0)
  ${stm32wl}/cubemx_hal_driver/src/stm32wlxx_hal.c
  ${stm32wl}/cubemx_hal_driver/src/stm32wlxx_hal_cortex.c
  ${stm32wl}/cubemx_hal_driver/src/stm32wlxx_hal_gpio.c
  ${stm32wl}/cubemx_hal_driver/src/stm32wlxx_hal_rcc.c
  ${stm32wl}/cubemx_hal_driver/src/stm32wlxx_hal_rcc_ex.c
  ${stm32wl}/cubemx_hal_driver/src/stm32wlxx_hal_pwr.c
  ${stm32wl}/cubemx_hal_driver/src/stm32wlxx_hal_pwr_ex.c
  ${stm32wl}/cubemx_hal_driver/src/stm32wlxx_hal_dma.c
  ${stm32wl}/cubemx_hal_driver/src/stm32wlxx_hal_dma_ex.c
  ${stm32wl}/cubemx_hal_driver/src/stm32wlxx_hal_uart.c
  ${stm32wl}/cubemx_hal_driver/src/stm32wlxx_hal_uart_ex.c)
set(LEPTON_BOARD_MKCONF sys/user/tauon-basic/etc/mkconf_tauon_basic_nucleo_wl55jc1.xml)
set(LEPTON_BOARD_MKCONF_TARGET cortexm_lepton)
set(LEPTON_BOARD_UNAME_MACHINE cortexM4-stm32wlxx)
# carte réelle : console série du STLINK-V3 et sonde OpenOCD. Deux cartes identiques sur le banc
# (radio) : la sonde est choisie par son numéro de série (cfg générée dans le répertoire de build).
set(LEPTON_BOARD_STLINK_SERIAL "" CACHE STRING
    "Numéro de série du STLINK-V3 de la carte (vide : première sonde trouvée)")
set(LEPTON_BOARD_OPENOCD_CFG ${CMAKE_BINARY_DIR}/openocd-nucleo-wl55jc1.cfg)
if(LEPTON_BOARD_STLINK_SERIAL)
  set(lepton_wl_serial "adapter serial ${LEPTON_BOARD_STLINK_SERIAL}\n")
else()
  set(lepton_wl_serial "")
endif()
file(WRITE ${LEPTON_BOARD_OPENOCD_CFG}
  "# généré par cmake/boards/nucleo-wl55jc1.cmake\n"
  "source [find interface/stlink.cfg]\n${lepton_wl_serial}"
  "source ${CMAKE_SOURCE_DIR}/debug/openocd-nucleo-wl55jc1.cfg\n")
set(LEPTON_BOARD_SERIAL_PORT "" CACHE STRING
    "Port série de la console de la carte (ex. /dev/ttyACM0) : active le test de fumée sur carte")

# nom vu par uname : posé ici, sans entrée dans la table de kernelconf.h (ajout-coeur.md §4)
target_compile_definitions(lepton_options INTERFACE
  __KERNEL_CPU_DEVICE_NAME="${LEPTON_BOARD_UNAME_MACHINE}"
  STM32WL55xx CORE_CM4 USE_HAL_DRIVER)
target_include_directories(lepton_options INTERFACE
  ${LEPTON_SRC}/${wlbsp}
  ${LEPTON_SRC}/${stm32wl}/dev_stm32wlxx
  ${LEPTON_SRC}/${stm32wl}/cubemx_hal_driver/inc
  ${LEPTON_SRC}/kernel/core/ucore/cmsis/Device/st/stm32wlxx)
