# Lepton — build CMake (migration IAR/Windows → GCC/Linux, étape 2).
#
# Quatre axes de variation, choisis par variables de cache (jamais par macros du compilateur) :
#   LEPTON_ISA          host | armv7m | armv6m            → cmake/isa/<isa>.cmake
#   LEPTON_CPU          cortex-m0plus | cortex-m3 | cortex-m4f | cortex-m7 → cmake/cpu/<cœur>.cmake
#   LEPTON_BOARD        qemu-mps2-an386 | qemu-mps2-an500 | nucleo-f439zi | stm32f746g-disco   → cmake/boards/<carte>.cmake
#   LEPTON_KAL_BACKEND  static | embos | freertos         → cmake/kal/<backend>.cmake
# Ajouter une valeur = ajouter un fichier (doc/migration/ajout-coeur.md).
# Les presets placent le build dans $LEPTON_BUILD/<preset>, hors du trunk et du clone.
cmake_minimum_required(VERSION 3.24)

project(lepton VERSION 4.9.0.2 LANGUAGES C)

set(LEPTON_ISA "" CACHE STRING "Famille d'ISA (host, armv7m, armv6m)")
set(LEPTON_CPU "" CACHE STRING "Cœur (cortex-m0plus, cortex-m3, cortex-m4f, cortex-m7)")
set(LEPTON_BOARD "" CACHE STRING "Carte (qemu-mps2-an386, qemu-mps2-an500, nucleo-f439zi, stm32f746g-disco)")
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
if(LEPTON_BOARD)
  include(boards/${LEPTON_BOARD})
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
