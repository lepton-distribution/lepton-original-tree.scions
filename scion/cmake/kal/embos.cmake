# Micro-noyau embOS (port GCC Segger, V5.20.0.0, hors git : $LEPTON_EMBOS_ROOT, décision 2026-09-30).
# Décrit à l'étape 2, validé à l'étape 3 : la branche embOS de kal.h n'est pas encore active sous
# GCC (écart E1, doc/migration/embos-iar-vs-gcc.md).
#
# Bibliothèque : libos<famille><mode>.a, famille donnée par le cœur (LEPTON_EMBOS_LIB_FAMILY,
# cmake/cpu), mode DP en Debug (contrôles OS_Error), R sinon (embos-inventaire.md §3).

if(NOT LEPTON_EMBOS_LIB_FAMILY)
  message(FATAL_ERROR "LEPTON_KAL_BACKEND=embos : le cœur (LEPTON_CPU) ne fixe pas de famille embOS")
endif()

set(LEPTON_EMBOS_ROOT "$ENV{LEPTON_EMBOS_ROOT}" CACHE PATH "Paquet embOS Segger (Start/Inc, Start/Lib)")
if(NOT EXISTS ${LEPTON_EMBOS_ROOT}/Start/Inc/RTOS.h)
  message(FATAL_ERROR "embOS introuvable : LEPTON_EMBOS_ROOT=${LEPTON_EMBOS_ROOT} (source scripts/lepton-env.sh)")
endif()

set(LEPTON_EMBOS_LIB
    ${LEPTON_EMBOS_ROOT}/Start/Lib/libos${LEPTON_EMBOS_LIB_FAMILY}$<IF:$<CONFIG:Debug>,DP,R>.a)

target_include_directories(lepton_options INTERFACE ${LEPTON_EMBOS_ROOT}/Start/Inc)
target_compile_definitions(lepton_options INTERFACE
  __KERNEL_UCORE_EMBOS
  $<IF:$<CONFIG:Debug>,OS_LIBMODE_DP,OS_LIBMODE_R>)

# Backend (existant) : kernel/core/core-segger.
set(LEPTON_KAL_SOURCES
  kernel/core/core-segger/core_rttimer.c
  kernel/core/core-segger/fork.c
  kernel/core/core-segger/kernel.c
  kernel/core/core-segger/kernel_clock.c
  kernel/core/core-segger/kernel_object.c
  kernel/core/core-segger/kernel_pthread.c
  kernel/core/core-segger/kernel_pthread_mutex.c
  kernel/core/core-segger/kernel_sem.c
  kernel/core/core-segger/kernel_sigqueue.c
  kernel/core/core-segger/kernel_timer.c
  kernel/core/core-segger/process.c
  kernel/core/core-segger/signal.c
  kernel/core/core-segger/syscall.c)
