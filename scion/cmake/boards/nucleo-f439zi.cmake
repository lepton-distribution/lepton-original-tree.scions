# Carte NUCLEO-F439ZI (Cortex-M4F, STM32F439ZI, étape 5). Décrite à l'étape 2, validée à l'étape 5.
# Base : application tauon-basic Olimex STM32-P407 et pilotes dev_stm32f4xx (perimetre.md) ;
# BSP embOS de départ : ST/STM32F429_STM32F429ZI_Nucleo (vecteur CRYP et RAM à adapter).
# Mémoire : ld/mem_nucleo-f439zi.ld.
if(NOT LEPTON_CPU STREQUAL "cortex-m4f")
  message(FATAL_ERROR "nucleo-f439zi : LEPTON_CPU=cortex-m4f attendu")
endif()
set(LEPTON_BOARD_MEMORY_LD ${CMAKE_SOURCE_DIR}/ld/mem_nucleo-f439zi.ld)
set(LEPTON_BSP_NAME nucleo_f439zi)
set(LEPTON_BSP_SOURCES)                     # étape 5 : liste dérivée de perimetre.md (353 sources)
set(LEPTON_BOARD_MKCONF sys/user/tauon-basic/etc/mkconf_tauon_basic_lwip_stm32f4-olimex-p407.xml)
set(LEPTON_BOARD_MKCONF_TARGET cortexm_lepton)
