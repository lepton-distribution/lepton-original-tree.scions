# Carte QEMU mps2-an386 (Cortex-M4, socle de l'étape 3). Décrite à l'étape 2, validée à l'étape 3.
# Mémoire : ld/mem_qemu-mps2-an386.ld. Périphériques (relevés QEMU 8.2, MIGRATION-STATUS) :
# UART CMSDK 0x40004000-0x40007000 et 0x40009000, LAN9118 0x40200000 — pilotes à écrire (étape 3).
if(NOT LEPTON_CPU STREQUAL "cortex-m4f")
  message(FATAL_ERROR "qemu-mps2-an386 : LEPTON_CPU=cortex-m4f attendu")
endif()
set(LEPTON_BOARD_MEMORY_LD ${CMAKE_SOURCE_DIR}/ld/mem_qemu-mps2-an386.ld)
set(LEPTON_BSP_NAME qemu_mps2_an386)
set(LEPTON_BSP_SOURCES)                     # étape 3 : UART CMSDK, puis LAN9118
set(LEPTON_BOARD_MKCONF "")                 # étape 3 : mkconf du socle (sans BSP carte)
set(LEPTON_QEMU_MACHINE mps2-an386)
