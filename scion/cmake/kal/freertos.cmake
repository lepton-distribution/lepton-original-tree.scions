# Micro-noyau FreeRTOS (étape 7) : noyau FreeRTOS-Kernel V11.3.0 (202604 LTS) vendored sous
# kernel/core/ucore/freeRTOS_11-3-0 (décision 2026-10-06, LEPTON-PROVENANCE.md), compilé avec
# Lepton. Écarts avec embOS et accès internes : doc/migration/kal-freertos-ecarts.md.
#
# Port GCC donné par le cœur (LEPTON_FREERTOS_PORT, cmake/cpu, comme LEPTON_EMBOS_LIB_FAMILY).
# Configuration commune : kal/backend/freertos/FreeRTOSConfig.h (allocation statique seule,
# configASSERT et contrôle de pile actifs). Noyau Lepton du backend : core-freertos (reprise,
# décision 2026-10-06 ; remise à niveau par tools/migration/freertos_rebase.py).

if(NOT LEPTON_FREERTOS_PORT)
  message(FATAL_ERROR "LEPTON_KAL_BACKEND=freertos : le cœur (LEPTON_CPU) ne fixe pas de port FreeRTOS")
endif()

set(LEPTON_FREERTOS_DIR kernel/core/ucore/freeRTOS_11-3-0)
set(LEPTON_FREERTOS_PORT_DIR ${LEPTON_FREERTOS_DIR}/portable/GCC/${LEPTON_FREERTOS_PORT})
if(NOT EXISTS ${LEPTON_SRC}/${LEPTON_FREERTOS_PORT_DIR}/port.c)
  message(FATAL_ERROR "FreeRTOS : port ${LEPTON_FREERTOS_PORT} absent de ${LEPTON_FREERTOS_DIR}")
endif()

# Intégration écrite pour Lepton : main (_start_kernel, vTaskStartScheduler) ; crochets
# (configASSERT, débordement de pile) communs aux ISA, aussi liés au banc KAL.
set(LEPTON_KAL_MAIN_SOURCES kernel/core/core-freertos/arch/armv7m/freertos_main.c)
set(LEPTON_KAL_HW_SOURCES kernel/core/core-freertos/arch/armv7m/freertos_hooks.c)
list(APPEND LEPTON_FIRMWARE_SOURCES ${LEPTON_KAL_MAIN_SOURCES} ${LEPTON_KAL_HW_SOURCES})

# KAL, axe micro-noyau : kal_backend.h, FreeRTOSConfig.h, trame du port par ISA.
set(LEPTON_KAL_BACKEND_DIR kernel/core/kal/backend/freertos)
target_include_directories(lepton_options INTERFACE
  ${LEPTON_SRC}/${LEPTON_KAL_BACKEND_DIR}
  ${LEPTON_SRC}/${LEPTON_KAL_BACKEND_DIR}/arch/${LEPTON_ISA}
  ${LEPTON_SRC}/${LEPTON_FREERTOS_DIR}/include
  ${LEPTON_SRC}/${LEPTON_FREERTOS_PORT_DIR})
# lwIP : sys_arch FreeRTOS (cmake/components/kernel.cmake)
set(LEPTON_LWIP_SYS_ARCH_DIR kernel/net/lwip/ports/freertos)
# Tick 1 kHz, comme embOS (_SC_CLK_TCK du noyau, timer.h).
target_compile_definitions(lepton_options INTERFACE
  __KERNEL_UCORE_FREERTOS
  __KERNEL_CLK_TCK=1000)
# surcharges de FreeRTOSConfig.h propres à la carte (LEPTON_FREERTOS_CONFIG, cmake/boards : ex.
# piles idle et temporisateurs réduites sur une carte à 32 Ko de RAM)
if(LEPTON_FREERTOS_CONFIG)
  target_compile_definitions(lepton_options INTERFACE ${LEPTON_FREERTOS_CONFIG})
endif()

set(LEPTON_KAL_SOURCES
  kernel/core/core-freertos/core_rttimer.c
  kernel/core/core-freertos/fork.c
  kernel/core/core-freertos/kernel.c
  kernel/core/core-freertos/kernel_clock.c
  kernel/core/core-freertos/kernel_object.c
  kernel/core/core-freertos/kernel_pthread.c
  kernel/core/core-freertos/kernel_pthread_mutex.c
  kernel/core/core-freertos/kernel_sem.c
  kernel/core/core-freertos/kernel_sigqueue.c
  kernel/core/core-freertos/kernel_timer.c
  kernel/core/core-freertos/process.c
  kernel/core/core-freertos/kernel_syscall_lock.c
  kernel/core/core-freertos/signal.c
  kernel/core/core-freertos/syscall.c
  # KAL FreeRTOS : région atomique et attentes bloquantes (kal_backend.h)
  ${LEPTON_KAL_BACKEND_DIR}/kal_freertos.c
  # noyau FreeRTOS (tiers) : sans heap_*.c (configSUPPORT_DYNAMIC_ALLOCATION=0)
  ${LEPTON_FREERTOS_DIR}/tasks.c
  ${LEPTON_FREERTOS_DIR}/queue.c
  ${LEPTON_FREERTOS_DIR}/list.c
  ${LEPTON_FREERTOS_DIR}/timers.c
  ${LEPTON_FREERTOS_DIR}/event_groups.c
  ${LEPTON_FREERTOS_PORT_DIR}/port.c)
# ports à assembleur séparé (ARM_CM0 : gestionnaires SVC/PendSV, masque d'interruptions)
if(EXISTS ${LEPTON_SRC}/${LEPTON_FREERTOS_PORT_DIR}/portasm.c)
  list(APPEND LEPTON_KAL_SOURCES ${LEPTON_FREERTOS_PORT_DIR}/portasm.c)
endif()
