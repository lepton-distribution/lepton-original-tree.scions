# Micro-noyau « static » : aucun ordonnanceur (noyau statique hôte de mklepton, guide §1.1).
#
# USE_KERNEL_STATIC sélectionne les branches sans ordonnanceur de interrupt.h / kernel.h ; le KAL
# (kal/backend/static) est choisi par chemin d'inclusion.
# La configuration (kernel_mkconf.h, dev_mkconf.c, bin_mkconf.c) est fixe et écrite à la main
# dans kernel/core/arch/host/static : elle ne peut pas venir de mklepton, qui dépend de ce noyau.

if(NOT LEPTON_ISA STREQUAL "host")
  message(FATAL_ERROR "LEPTON_KAL_BACKEND=static n'existe que pour LEPTON_ISA=host.")
endif()

set(LEPTON_KAL_STATIC_DIR kernel/core/arch/host/static)

target_compile_definitions(lepton_options INTERFACE USE_KERNEL_STATIC)
target_include_directories(lepton_options INTERFACE ${LEPTON_SRC}/${LEPTON_KAL_STATIC_DIR})
# KAL, axe micro-noyau : kal_backend.h (inclus par kernel/core/kal.h, dispatcher).
set(LEPTON_KAL_BACKEND_DIR kernel/core/kal/backend/static)
target_include_directories(lepton_options INTERFACE ${LEPTON_SRC}/${LEPTON_KAL_BACKEND_DIR})

# Backend : successeur de core-ecos/ + arch/synthetic/x86_static/ (absents de l'arbre).
set(LEPTON_KAL_SOURCES
  kernel/core/core-static/kernel_static.c
  ${LEPTON_KAL_STATIC_DIR}/dev_mkconf.c
  ${LEPTON_KAL_STATIC_DIR}/bin_mkconf.c)
