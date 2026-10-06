/*
 * Banc KAL — services du micro-noyau FreeRTOS (V11.3.0) utilisés par kal_bench.c (étape 7).
 * Licence : voir LICENSE (MPL 1.1).
 *
 * Mêmes services que tests/kal/arch/armv7m/backend/embos/kal_bench_os.h :
 *   - tâche de banc = StaticTask_t (handle) + groupe d'événements statique, équivalent des
 *     événements de tâche d'embOS (OS_TASKEVENT_Set / GetBlocked / GetTimed) ;
 *   - région = vTaskSuspendAll / xTaskResumeAll (OS_TASK_EnterRegion / LeaveRegion) ;
 *   - priorités Lepton du banc (100, 200) projetées par __kal_priority (KAL), comme les tâches
 *     du noyau.
 * Oracle de trame : calculé d'après le port GCC (port.c, xPortPendSVHandler et
 * pxPortInitialiseStack), indépendamment de kal.h.
 *   ARM_CM4F (hard-float) : [R4-R11, EXC_RETURN] [S16-S31 si EXC_RETURN bit 4 = 0]
 *                           [R0-R3, R12, LR, PC, xPSR] [S0-S15, FPSCR, réservé]
 *   ARM_CM3 (soft-float)  : [R4-R11] [R0-R3, R12, LR, PC, xPSR]
 */
#ifndef _KAL_BENCH_OS_H
#define _KAL_BENCH_OS_H

#include <stdint.h>
#include <string.h>
#include "FreeRTOS.h"
#include "task.h"
#include "event_groups.h"

#if defined(__VFP_FP__) && !defined(__SOFTFP__)
   #define BENCH_HAS_VFP     1
   #define BENCH_SW_WORDS    9u     /* R4-R11, EXC_RETURN */
#else
   #define BENCH_HAS_VFP     0
   #define BENCH_SW_WORDS    8u     /* R4-R11 */
#endif
#define BENCH_FPU_HI_WORDS   16u    /* S16-S31 */
#define BENCH_HW_WORDS       8u     /* R0-R3, R12, LR, PC, xPSR */
#define BENCH_FPU_LO_WORDS   18u    /* S0-S15, FPSCR, réservé */
#define BENCH_FPU_WORDS      (BENCH_SW_WORDS + BENCH_FPU_HI_WORDS + BENCH_HW_WORDS + BENCH_FPU_LO_WORDS)
#define BENCH_STACKPTR
/* pile sauvegardée d'un contexte KAL (context_t du backend FreeRTOS) */
#define BENCH_CTX_SP(__ctx__)    ((__ctx__).sp)
#define BENCH_CTX_REGS(__ctx__)  ((__ctx__).os_regs)
#define BENCH_START_PC_DESC  "T2 : PC de départ = point d'entrée de la tâche (pxPortInitialiseStack)"

typedef struct {
   StaticTask_t tcb;                 /* en tête : handle de tâche = adresse de la structure */
   StaticEventGroup_t events_static;
   EventGroupHandle_t events;
} bench_task_t;

static inline void bench_task_create(bench_task_t* task, const char* name, unsigned prio,
                                     void (*routine)(void), uint32_t* stack, unsigned size){
   task->events = xEventGroupCreateStatic(&task->events_static);
   (void)xTaskCreateStatic((TaskFunction_t)routine, name, size / sizeof(StackType_t), (void*)0,
                           __kal_priority(prio), (StackType_t*)stack, &task->tcb);
}
static inline void bench_task_terminate(bench_task_t* task){
   vTaskDelete((TaskHandle_t)&task->tcb);
   vEventGroupDelete(task->events);
}
static inline uint32_t bench_task_sp(bench_task_t* task){ return *(uint32_t*)&task->tcb; }
static inline int bench_is_current(bench_task_t* task){
   return xTaskGetCurrentTaskHandle() == (TaskHandle_t)&task->tcb;
}
static inline void bench_region_enter(void){ vTaskSuspendAll(); }
static inline void bench_region_leave(void){ (void)xTaskResumeAll(); }
static inline void bench_event_set(bench_task_t* task, unsigned ev){
   (void)xEventGroupSetBits(task->events, (EventBits_t)ev);
}
static inline unsigned bench_event_get_timed(unsigned ev, unsigned timeout_ms){
   bench_task_t* self = (bench_task_t*)xTaskGetCurrentTaskHandle();
   return (unsigned)xEventGroupWaitBits(self->events, (EventBits_t)ev, pdTRUE, pdFALSE,
                                        pdMS_TO_TICKS(timeout_ms)) & ev;
}
static inline unsigned bench_event_get_blocked(unsigned ev){
   bench_task_t* self = (bench_task_t*)xTaskGetCurrentTaskHandle();
   return (unsigned)xEventGroupWaitBits(self->events, (EventBits_t)ev, pdTRUE, pdFALSE,
                                        portMAX_DELAY) & ev;
}
static inline void bench_delay(unsigned ms){ vTaskDelay(pdMS_TO_TICKS(ms)); }
static inline uint32_t bench_ticks(void){ return (uint32_t)xTaskGetTickCount(); }

/* démarrage : contrôleur, multitâche (SysTick, PendSV, SVC programmés par le port) */
static inline void bench_start(bench_task_t* ctrl, const char* name, unsigned prio,
                               void (*routine)(void), uint32_t* stack, unsigned size){
   portDISABLE_INTERRUPTS();
   bench_task_create(ctrl, name, prio, routine, stack, size);
   vTaskStartScheduler();
}

/* PC de départ attendu (T2) : le point d'entrée lui-même (bit 0 effacé par le port) */
static inline uint32_t bench_start_pc(void (*routine)(void)){
   return (uint32_t)routine;
}

/* --- oracle de trame (port.c) --------------------------------------------------------------- */
static inline int bench_frame_is_fpu(const void* frame){
#if BENCH_HAS_VFP
   return (((const uint32_t*)frame)[8] & 0x10u) == 0;   /* EXC_RETURN, bit 4 (FType) */
#else
   (void)frame;
   return 0;
#endif
}
static inline uint32_t* bench_frame_hw(void* frame){
   return (uint32_t*)frame + BENCH_SW_WORDS + (bench_frame_is_fpu(frame) ? BENCH_FPU_HI_WORDS : 0u);
}
static inline uint32_t bench_frame_size(const void* frame){
   return 4u * (bench_frame_is_fpu(frame) ? BENCH_FPU_WORDS : BENCH_SW_WORDS + BENCH_HW_WORDS);
}
static inline uint32_t bench_frame_pc(const void* frame){
   return bench_frame_hw((void*)frame)[6];
}
static inline uint32_t* bench_frame_xpsr(void* frame){
   return &bench_frame_hw(frame)[7];
}
static inline void bench_frame_corrupt_r4_r11(void* frame){
   uint32_t* f = (uint32_t*)frame;
   f[0] = f[1] = f[2] = f[3] = 0xDEAD0000u;   /* R4-R7 */
   f[4] = f[5] = f[6] = f[7] = 0xDEAD0001u;   /* R8-R11 */
}
#if BENCH_HAS_VFP
static inline void bench_frame_corrupt_fpu(void* frame){
   uint32_t* f = (uint32_t*)frame;
   bench_frame_corrupt_r4_r11(frame);
   memset(f + BENCH_SW_WORDS, 0xEE, 4u * BENCH_FPU_HI_WORDS);                          /* S16-S31 */
   memset(f + BENCH_SW_WORDS + BENCH_FPU_HI_WORDS + BENCH_HW_WORDS, 0xEE, 4u * 16u);  /* S0-S15 */
}
#endif

#endif /* _KAL_BENCH_OS_H */
