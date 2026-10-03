# Carte NUCLEO-F439ZI (Cortex-M4F, STM32F439ZI), décrite à l'étape 2, BSP et validation à l'étape 5.
# Validée sur NUCLEO-F429ZI (décision 2026-10-01) : la F439 n'ajoute que CRYP/HASH, inutilisés ;
# la puce est déclarée STM32F429xx ici (en-tête stm32f439xx.h absent de la SPL de l'arbre).
# Mémoire : ld/mem_nucleo-f439zi.ld. Périphériques : USART3 (PD8/PD9, port série virtuel du
# ST-LINK, console ttys3), USART6 (connecteur, ttys6), Ethernet RMII + PHY LAN8742A (eth0, lwIP).
# Horloge, broches, interruptions : BSP kernel/dev/bsp/nucleo_f439zi.
if(NOT LEPTON_CPU STREQUAL "cortex-m4f")
  message(FATAL_ERROR "nucleo-f439zi : LEPTON_CPU=cortex-m4f attendu")
endif()
set(LEPTON_BOARD_MEMORY_LD ${CMAKE_SOURCE_DIR}/ld/mem_nucleo-f439zi.ld)
set(LEPTON_BSP_NAME nucleo_f439zi)
set(stm32f4 kernel/dev/arch/cortexm/stm32f4xx)
set(LEPTON_BSP_SOURCES
  kernel/dev/bsp/nucleo_f439zi/nucleo_f439zi_board.c
  # couche STM32F4 de Lepton (pilotes génériques, paramétrés par le BSP)
  ${stm32f4}/gpio.c
  ${stm32f4}/uart.c
  ${stm32f4}/eth.c
  ${stm32f4}/dev_stm32f4xx/dev_stm32f4xx_uart_x.c
  ${stm32f4}/dev_stm32f4xx/dev_stm32f4xx_eth.c
  # SPL ST (tiers)
  ${stm32f4}/driverlib/misc.c
  ${stm32f4}/driverlib/stm32f4xx_rcc.c
  ${stm32f4}/driverlib/stm32f4xx_gpio.c
  ${stm32f4}/driverlib/stm32f4xx_usart.c
  ${stm32f4}/driverlib/stm32f4xx_dma.c
  ${stm32f4}/driverlib/stm32f4xx_syscfg.c
  ${stm32f4}/driverlib/stm32f4x7_eth.c)
# vecteurs : objet de l'exécutable, comme le démarrage (dans une bibliothèque, rien ne le tirerait :
# les gestionnaires faibles du démarrage suffisent à l'éditeur de liens)
list(APPEND LEPTON_FIRMWARE_SOURCES kernel/dev/bsp/nucleo_f439zi/nucleo_f439zi_vectors.c)
set(LEPTON_NET_STACK lwip)   # USE_LWIP dans user_kernel_mkconf.h
set(LEPTON_BOARD_MKCONF sys/user/tauon-basic/etc/mkconf_tauon_basic_nucleo_f439zi.xml)
set(LEPTON_BOARD_MKCONF_TARGET cortexm_lepton)
set(LEPTON_BOARD_UNAME_MACHINE cortexM4-stm32f4)   # __KERNEL_CPU_DEVICE_NAME (kernelconf.h)
# carte réelle (étape 5) : console série du ST-LINK et sonde OpenOCD
set(LEPTON_BOARD_OPENOCD_CFG ${CMAKE_SOURCE_DIR}/debug/openocd-nucleo-f439zi.cfg)
set(LEPTON_BOARD_SERIAL_PORT "" CACHE STRING
    "Port série de la console de la carte (ex. /dev/ttyACM0) : active le test de fumée sur carte")
# test réseau sur carte (labels board et net, tests/board_net.py) : adresse de la carte (.init),
# fichier téléchargé par FTP et sa source ; l'adresse de l'hôte sur le câble de la carte dépend
# du banc (ex. 192.168.2.20) : vide, le test n'est pas créé
set(LEPTON_NET_TEST_GUEST_IP 192.168.2.5)
set(LEPTON_NET_TEST_FTP_FILE /usr/etc/.boot)
set(LEPTON_NET_TEST_FTP_REFERENCE sys/user/tauon-basic/etc/nucleo-f439zi/.boot)
set(LEPTON_NET_TEST_HOST_IP "" CACHE STRING
    "Adresse de l'hôte sur le lien Ethernet de la carte : active le test réseau sur carte")

target_compile_definitions(lepton_options INTERFACE
  __tauon_cpu_device__=__tauon_cpu_device_cortexM4_stm32f4__
  STM32F429xx USE_STDPERIPH_DRIVER
  HSE_VALUE=8000000)   # MCO du ST-LINK (8 MHz, HSE en bypass), UM1974
target_include_directories(lepton_options INTERFACE
  ${LEPTON_SRC}/kernel/dev/bsp/nucleo_f439zi
  ${LEPTON_SRC}/${stm32f4}/driverlib
  ${LEPTON_SRC}/${stm32f4}/dev_stm32f4xx)
