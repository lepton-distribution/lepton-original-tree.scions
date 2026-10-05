# Carte QEMU mps2-an500 (Cortex-M7, étape 6).
# Mémoire : ld/mem_qemu-mps2-an500.ld. Périphériques : UART CMSDK (console ttys0, ttys1),
# LAN9118 (eth0, pile lwIP) à 0xA0000000. Adresses et numéros d'interruption : BSP commun
# kernel/dev/bsp/qemu_mps2, en-tête de machine kernel/dev/bsp/qemu_mps2/an500/qemu_mps2_machine.h.
# Configuration applicative (mkconf, .boot, .init) commune aux machines MPS2.
if(NOT LEPTON_CPU STREQUAL "cortex-m7")
  message(FATAL_ERROR "qemu-mps2-an500 : LEPTON_CPU=cortex-m7 attendu")
endif()
set(LEPTON_BOARD_MEMORY_LD ${CMAKE_SOURCE_DIR}/ld/mem_qemu-mps2-an500.ld)
set(LEPTON_BSP_NAME qemu_mps2_an500)   # bibliothèque lepton_bsp_<nom>, une par carte
set(LEPTON_BSP_SOURCES
  kernel/dev/arch/all/uart/dev_cmsdk_uart/dev_cmsdk_uart_x.c
  kernel/dev/arch/all/eth/dev_eth_lan9118/dev_eth_lan9118_x.c
  kernel/dev/bsp/qemu_mps2/qemu_mps2_board.c)
set(LEPTON_NET_STACK lwip)   # USE_LWIP dans user_kernel_mkconf.h
set(LEPTON_BOARD_MKCONF sys/user/tauon-basic/etc/mkconf_tauon_basic_qemu_mps2.xml)
set(LEPTON_BOARD_MKCONF_TARGET cortexm_lepton)
set(LEPTON_QEMU_MACHINE mps2-an500)
set(LEPTON_BOARD_UNAME_MACHINE cortexM7-qemu-mps2-an500)
# test réseau (label net, tests/net_qemu.py) : adresses du .init commun, fichier téléchargé par
# FTP et sa source dans l'arbre
set(LEPTON_NET_TEST_HOST_IP 192.168.100.1)
set(LEPTON_NET_TEST_GUEST_IP 192.168.100.2)
set(LEPTON_NET_TEST_FTP_FILE /usr/etc/.boot)
set(LEPTON_NET_TEST_FTP_REFERENCE sys/user/tauon-basic/etc/qemu-mps2/.boot)

# nom vu par uname : posé ici, sans entrée dans la table de kernelconf.h (ajout-coeur.md §4)
target_compile_definitions(lepton_options INTERFACE
  __KERNEL_CPU_DEVICE_NAME="${LEPTON_BOARD_UNAME_MACHINE}")
target_include_directories(lepton_options INTERFACE ${LEPTON_SRC}/kernel/dev/bsp/qemu_mps2/an500)
