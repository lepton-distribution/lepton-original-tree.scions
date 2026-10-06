/*
 * Lepton — configuration de FreeRTOS V11.3.0 (backend KAL FreeRTOS, étape 7). Licence : voir
 * LICENSE (MPL 1.1).
 *
 * Commune aux cartes : les valeurs propres à la carte viennent de CMake (tick : __KERNEL_CLK_TCK,
 * cmake/kal/freertos.cmake) ou de l'exécution (SystemCoreClock du BSP). Priorités d'interruption
 * exprimées sur 8 bits (indépendantes du nombre de bits NVIC de la puce) : même partition
 * qu'embOS (embos_init_hw.c) — les IRQ de priorité numérique >= 0x80 peuvent appeler l'API,
 * PendSV et SysTick à la plus basse. Une carte peut surcharger une valeur par définition de
 * compilation (#ifndef).
 *
 * Exigences de l'étape 7 : configASSERT et configCHECK_FOR_STACK_OVERFLOW=2 actifs pendant
 * toute la validation ; allocation statique uniquement (TCB, piles, objets fournis par Lepton).
 */
#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

#include <stdint.h>

extern uint32_t SystemCoreClock;
void lepton_freertos_assert(const char* file, int line);

/* --- ordonnanceur -------------------------------------------------------------------------- */
#define configUSE_PREEMPTION                       1
#define configUSE_TIME_SLICING                     1   /* tourniquet d'un tick (embOS : timeslice par tâche) */
#define configUSE_PORT_OPTIMISED_TASK_SELECTION    0
#define configUSE_TICKLESS_IDLE                    0
#define configCPU_CLOCK_HZ                         (SystemCoreClock)
#ifndef configTICK_RATE_HZ
   #define configTICK_RATE_HZ                      ((TickType_t)__KERNEL_CLK_TCK)
#endif
#define configMAX_PRIORITIES                       32  /* projection des priorités Lepton : __kal_priority */
#ifndef configMINIMAL_STACK_SIZE
   #define configMINIMAL_STACK_SIZE                ((uint16_t)256)   /* mots, tâche idle */
#endif
#define configMAX_TASK_NAME_LEN                    16
#define configTICK_TYPE_WIDTH_IN_BITS              TICK_TYPE_WIDTH_32_BITS
#define configIDLE_SHOULD_YIELD                    1
#define configUSE_TASK_NOTIFICATIONS               1
#define configTASK_NOTIFICATION_ARRAY_ENTRIES      1
#define configENABLE_BACKWARD_COMPATIBILITY        1   /* en-têtes Lepton : xSemaphoreHandle, portBASE_TYPE */
#define configNUM_THREAD_LOCAL_STORAGE_POINTERS    0
#define configUSE_NEWLIB_REENTRANT                 0
#define configSTACK_DEPTH_TYPE                     uint32_t
#define configMESSAGE_BUFFER_LENGTH_TYPE           size_t

/* --- objets ---------------------------------------------------------------------------------- */
#define configUSE_MUTEXES                          1
#define configUSE_RECURSIVE_MUTEXES                1   /* kernel_pthread_mutex (OS_RSEMA d'embOS) */
#define configUSE_COUNTING_SEMAPHORES              1
#define configUSE_QUEUE_SETS                       0
#define configQUEUE_REGISTRY_SIZE                  0
#define configUSE_EVENT_GROUPS                     1
#define configUSE_STREAM_BUFFERS                   0
#define configUSE_CO_ROUTINES                      0

/* --- mémoire : tout statique (fourni par Lepton) ------------------------------------------- */
#define configSUPPORT_STATIC_ALLOCATION            1
#define configSUPPORT_DYNAMIC_ALLOCATION           0
#define configKERNEL_PROVIDED_STATIC_MEMORY        1   /* tâches idle et temporisateurs */

/* --- temporisateurs logiciels : rappels dans la tâche de service, au-dessus de toutes les
 *     tâches Lepton (sous embOS, les rappels s'exécutent dans le contexte du tick) ------------ */
#define configUSE_TIMERS                           1
#define configTIMER_TASK_PRIORITY                  (configMAX_PRIORITIES-1)
#define configTIMER_QUEUE_LENGTH                   16
#ifndef configTIMER_TASK_STACK_DEPTH
   #define configTIMER_TASK_STACK_DEPTH            512   /* mots : _sys_kill depuis les rappels */
#endif

/* --- crochets et contrôles (actifs pendant toute la validation) ---------------------------- */
#define configUSE_IDLE_HOOK                        0
#define configUSE_TICK_HOOK                        0
#define configUSE_MALLOC_FAILED_HOOK               0
#define configUSE_DAEMON_TASK_STARTUP_HOOK         0
#define configCHECK_FOR_STACK_OVERFLOW             2
#define configUSE_TRACE_FACILITY                   0
#define configGENERATE_RUN_TIME_STATS              0
#define configASSERT(x)                            do { if(!(x)) lepton_freertos_assert(__FILE__, __LINE__); } while(0)

/* --- protection mémoire : sans MPU (ports ARMv6-M ARM_CM0 : définition exigée) -------------- */
#define configENABLE_MPU                           0

/* --- interruptions (ARMv7-M : BASEPRI ; ARMv6-M : PRIMASK, priorités sans effet) ---------- */
#define configKERNEL_INTERRUPT_PRIORITY            0xFF
#define configMAX_SYSCALL_INTERRUPT_PRIORITY       0x80
#define configMAX_API_CALL_INTERRUPT_PRIORITY      configMAX_SYSCALL_INTERRUPT_PRIORITY

/* gestionnaires du port sous les noms CMSIS des vecteurs Lepton (startup_<isa>.c, faibles) */
#define vPortSVCHandler                            SVC_Handler
#define xPortPendSVHandler                         PendSV_Handler
#define xPortSysTickHandler                        SysTick_Handler

/* --- API incluse -------------------------------------------------------------------------- */
#define INCLUDE_vTaskPrioritySet                   1
#define INCLUDE_uxTaskPriorityGet                  1
#define INCLUDE_vTaskDelete                        1
#define INCLUDE_vTaskSuspend                       1
#define INCLUDE_xTaskDelayUntil                    1
#define INCLUDE_vTaskDelay                         1
#define INCLUDE_xTaskGetSchedulerState             1
#define INCLUDE_xTaskGetCurrentTaskHandle          1
#define INCLUDE_uxTaskGetStackHighWaterMark        1
#define INCLUDE_eTaskGetState                      1
#define INCLUDE_xTaskAbortDelay                    1   /* réveil sur signal (__kal_frt_make_ready) */
#define INCLUDE_xSemaphoreGetMutexHolder           1
#define INCLUDE_xTimerPendFunctionCall             0

#endif /* FREERTOS_CONFIG_H */
