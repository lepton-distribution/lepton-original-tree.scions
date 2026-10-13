# Lepton — build CMake (migration IAR/Windows → GCC/Linux, étape 2).
#
# Quatre axes de variation, choisis par variables de cache (jamais par macros du compilateur) :
#   LEPTON_ISA          host | armv7m | armv6m            → cmake/isa/<isa>.cmake
#   LEPTON_CPU          cortex-m0plus | cortex-m3 | cortex-m4f | cortex-m7 → cmake/cpu/<cœur>.cmake
#   LEPTON_BOARD        nom d'un greffon BSP (manifeste lepton-greffon.json, ENV-6), sinon carte
#                       transitoire cmake/boards/<carte>.cmake (nucleo-f439zi, nucleo-wl55jc1,
#                       stm32f746g-disco, samd21-xplained-pro)
#   LEPTON_KAL_BACKEND  static | embos | freertos         → cmake/kal/<backend>.cmake
# Ajouter une valeur = ajouter un fichier (doc/migration/ajout-coeur.md) ou un greffon BSP.
# LEPTON_MKCONF : XML mklepton (configuration logicielle, relatif au trunk), posé par le preset
# (PA-7 Q4) ; les cartes transitoires le donnent dans leur module (LEPTON_BOARD_MKCONF).
# Les presets placent le build dans $LEPTON_BUILD/<preset>, hors du trunk et du clone.
cmake_minimum_required(VERSION 3.24)

project(lepton VERSION 4.9.0.2 LANGUAGES C)

set(LEPTON_ISA "" CACHE STRING "Famille d'ISA (host, armv7m, armv6m)")
set(LEPTON_CPU "" CACHE STRING "Cœur (cortex-m0plus, cortex-m3, cortex-m4f, cortex-m7)")
set(LEPTON_BOARD "" CACHE STRING "Carte : greffon BSP ou module cmake/boards")
set(LEPTON_MKCONF "" CACHE STRING "XML mklepton de la configuration logicielle, relatif au trunk")
set(LEPTON_KAL_BACKEND "" CACHE STRING "Micro-noyau (static, embos, freertos)")

if(NOT LEPTON_ISA OR NOT LEPTON_KAL_BACKEND)
  message(FATAL_ERROR "LEPTON_ISA et LEPTON_KAL_BACKEND sont obligatoires (utiliser un preset).")
endif()

set(LEPTON_SRC ${CMAKE_CURRENT_SOURCE_DIR}/sys/root/src)
set(LEPTON_TOOLS ${CMAKE_CURRENT_SOURCE_DIR}/tools)
set(LEPTON_GENERATED_DIR ${CMAKE_BINARY_DIR}/generated)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
include(lepton_target)

include(isa/${LEPTON_ISA})
if(LEPTON_CPU)
  include(cpu/${LEPTON_CPU})
endif()
include(lepton_greffons)   # lecture des manifestes : avant la carte (BSP par manifeste)
if(LEPTON_BOARD)
  if(DEFINED LEPTON_GREFFON_${LEPTON_BOARD}_TYPE)
    lepton_greffons_carte(${LEPTON_BOARD})
    if(LEPTON_BOARD_CMAKE)
      include(${LEPTON_BOARD_CMAKE})   # paramètres hors manifeste du BSP (PA-7 Q3)
    endif()
  else()
    include(boards/${LEPTON_BOARD})
  endif()
endif()
if(LEPTON_MKCONF)
  set(LEPTON_BOARD_MKCONF ${LEPTON_MKCONF})
endif()
include(kal/${LEPTON_KAL_BACKEND})

enable_testing()
include(components/kernel)
include(mklepton)
if(NOT LEPTON_ISA STREQUAL "host")
  include(components/firmware)
endif()

if(LEPTON_ISA STREQUAL "host")
  include(components/mklepton)
  add_subdirectory(tests/host)
endif()
