# Carte STM32F746G-DISCO (Cortex-M7 r0p1, STM32F746NG), étape 6.
# Mémoire : ld/mem_stm32f746g-disco.ld. Périphériques : USART1 (PA9/PB7, port série virtuel du
# ST-LINK, console ttys1), Ethernet RMII + PHY LAN8742A (eth0, pile lwIP). Horloges, caches, interruptions : BSP kernel/dev/bsp/stm32f746g_disco.
# Puce : FPU simple précision (preset : LEPTON_M7_FPU=sp) ; cœur r0p1, erratum 837070 (bibliothèque
# embOS _837070, USE_ERRATUM_837070=1 pour RTOS.h) ; DEV_ID 0x449 et CPUID 0x410FC271 relus par
# la sonde (2026-10-05).
if(NOT LEPTON_CPU STREQUAL "cortex-m7" OR NOT LEPTON_M7_FPU STREQUAL "sp")
  message(FATAL_ERROR "stm32f746g-disco : LEPTON_CPU=cortex-m7 et LEPTON_M7_FPU=sp attendus")
endif()
set(LEPTON_BOARD_MEMORY_LD ${CMAKE_SOURCE_DIR}/ld/mem_stm32f746g-disco.ld)
set(LEPTON_BSP_NAME stm32f746g_disco)
set(stm32f7 kernel/dev/arch/cortexm/stm32f7xx)
set(LEPTON_BSP_SOURCES
  kernel/dev/bsp/stm32f746g_disco/stm32f746g_disco_board.c
  # couche STM32F7 de Lepton
  ${stm32f7}/dev_stm32f7xx/dev_stm32f7xx_uart_x.c
  ${stm32f7}/dev_stm32f7xx/dev_stm32f7xx_eth_x.c
  ${stm32f7}/dev_stm32f7xx/dev_stm32f7xx_hal_tick.c
  # HAL ST (tiers, STM32CubeF7 v1.17.4)
  ${stm32f7}/hal_driver/Src/stm32f7xx_hal_gpio.c
  ${stm32f7}/hal_driver/Src/stm32f7xx_hal_eth.c
  ${stm32f7}/hal_driver/Src/stm32f7xx_hal_rcc.c
  ${stm32f7}/hal_driver/Src/stm32f7xx_hal_cortex.c)
set(LEPTON_NET_STACK lwip)   # USE_LWIP dans user_kernel_mkconf.h
set(LEPTON_EMBOS_LIB_VARIANT _837070)
set(LEPTON_BOARD_MKCONF sys/user/tauon-basic/etc/mkconf_tauon_basic_stm32f746g_disco.xml)
set(LEPTON_BOARD_MKCONF_TARGET cortexm_lepton)
set(LEPTON_BOARD_UNAME_MACHINE cortexM7-stm32f7)
# carte réelle : console série du ST-LINK et sonde OpenOCD
set(LEPTON_BOARD_OPENOCD_CFG ${CMAKE_SOURCE_DIR}/debug/openocd-stm32f746g-disco.cfg)
set(LEPTON_BOARD_SERIAL_PORT "" CACHE STRING
    "Port série de la console de la carte (ex. /dev/ttyACM0) : active le test de fumée sur carte")
# test réseau sur carte (labels board et net, tests/board_net.py) : adresse de la carte (.init),
# fichier téléchargé par FTP et sa source ; adresse de l'hôte sur le câble de la carte : vide, le
# test n'est pas créé
set(LEPTON_NET_TEST_GUEST_IP 192.168.2.5)
set(LEPTON_NET_TEST_FTP_FILE /usr/etc/.boot)
set(LEPTON_NET_TEST_FTP_REFERENCE sys/user/tauon-basic/etc/stm32f746g-disco/.boot)
set(LEPTON_NET_TEST_HOST_IP "" CACHE STRING
    "Adresse de l'hôte sur le lien Ethernet de la carte : active le test réseau sur carte")

# nom vu par uname : posé ici, sans entrée dans la table de kernelconf.h (ajout-coeur.md §4)
target_compile_definitions(lepton_options INTERFACE
  __KERNEL_CPU_DEVICE_NAME="${LEPTON_BOARD_UNAME_MACHINE}"
  STM32F746xx USE_HAL_DRIVER
  HSE_VALUE=25000000   # quartz X2 de la carte
  USE_ERRATUM_837070=1)
target_include_directories(lepton_options INTERFACE
  ${LEPTON_SRC}/kernel/dev/bsp/stm32f746g_disco
  ${LEPTON_SRC}/${stm32f7}/dev_stm32f7xx
  ${LEPTON_SRC}/${stm32f7}/hal_driver/Inc
  ${LEPTON_SRC}/kernel/core/ucore/cmsis-5/Device/ST/STM32F7xx/Include)
