# Micro-noyau embOS (port GCC Segger, V5.20.0.0, hors git : $LEPTON_EMBOS_ROOT, décision 2026-09-30).
# Écarts IAR/GCC : doc/migration/embos-iar-vs-gcc.md.
#
# Bibliothèque : libos<famille><mode>.a, famille donnée par le cœur (LEPTON_EMBOS_LIB_FAMILY,
# cmake/cpu), mode SP en Debug (contrôle de pile + profilage, comme l'existant IAR), R sinon.
# Pas DP (décision 2026-09-30) : le verrou des appels système (kernel_mutex) est rendu par la tâche
# noyau et non par son propriétaire, ce que DP refuse (OS_ERR_MUTEX_OWNER) ; refonte à l'étape 4.

if(NOT LEPTON_EMBOS_LIB_FAMILY)
  message(FATAL_ERROR "LEPTON_KAL_BACKEND=embos : le cœur (LEPTON_CPU) ne fixe pas de famille embOS")
endif()

set(LEPTON_EMBOS_ROOT "$ENV{LEPTON_EMBOS_ROOT}" CACHE PATH "Paquet embOS Segger (Start/Inc, Start/Lib)")
if(NOT EXISTS ${LEPTON_EMBOS_ROOT}/Start/Inc/RTOS.h)
  message(FATAL_ERROR "embOS introuvable : LEPTON_EMBOS_ROOT=${LEPTON_EMBOS_ROOT} (source scripts/lepton-env.sh)")
endif()

set(LEPTON_EMBOS_LIB
    ${LEPTON_EMBOS_ROOT}/Start/Lib/libos${LEPTON_EMBOS_LIB_FAMILY}$<IF:$<CONFIG:Debug>,SP,R>.a)
set(LEPTON_KAL_LINK_LIBS ${LEPTON_EMBOS_LIB})

# Intégration embOS écrite pour Lepton (décision 2026-09-30 : aucun fichier d'exemple Segger) :
# main (OS_Init, OS_InitHW, _start_kernel, OS_Start), OS_InitHW/SysTick/OS_Idle, OS_Error.
list(APPEND LEPTON_FIRMWARE_SOURCES
  kernel/core/core-segger/arch/armv7m/embos_main.c
  kernel/core/core-segger/arch/armv7m/embos_init_hw.c)

target_include_directories(lepton_options INTERFACE ${LEPTON_EMBOS_ROOT}/Start/Inc)
target_compile_definitions(lepton_options INTERFACE
  __KERNEL_UCORE_EMBOS
  $<IF:$<CONFIG:Debug>,OS_LIBMODE_SP,OS_LIBMODE_R>)

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
  kernel/core/core-segger/kernel_syscall_lock.c
  kernel/core/core-segger/signal.c
  kernel/core/core-segger/syscall.c)
