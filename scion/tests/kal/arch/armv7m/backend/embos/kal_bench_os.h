/*
 * Banc KAL — services du micro-noyau embOS utilisés par kal_bench.c (étape 7 : le banc est
 * commun aux backends, seuls ces services et l'oracle de trame diffèrent). Licence : voir
 * LICENSE (MPL 1.1).
 *
 * Code repris tel quel de kal_bench.c (étapes 3 à 6) : la couche est neutre pour embOS (code
 * objet de kal_bench.c identique avant et après l'extraction, contrôlé à l'étape 7).
 * Oracle de trame : calculé d'après RTOS.h, indépendamment de kal.h.
 */
#ifndef _KAL_BENCH_OS_H
#define _KAL_BENCH_OS_H

#include <stdint.h>
#include "RTOS.h"

#define BENCH_HAS_VFP        (OS_CPU_HAS_VFP == 1)
#define BENCH_STACKPTR       OS_STACKPTR
#define BENCH_FPU_WORDS      (sizeof(OS_REGS_BASE_FPU) / 4)
/* pile sauvegardée d'un contexte KAL (context_t du backend embOS) */
#define BENCH_CTX_SP(__ctx__)    ((__ctx__).os_task.pStack)
#define BENCH_CTX_REGS(__ctx__)  (&(__ctx__).os_regs)
#define BENCH_START_PC_DESC  "T2 : PC de départ = trampoline embOS OS_StartTask"

typedef OS_TASK bench_task_t;

static inline void bench_task_create(bench_task_t* task, const char* name, unsigned prio,
                                     void (*routine)(void), uint32_t* stack, unsigned size){
   OS_TASK_Create(task, name, prio, routine, stack, size, 2);
}
static inline void bench_task_terminate(bench_task_t* task){ OS_TASK_Terminate(task); }
static inline uint32_t bench_task_sp(bench_task_t* task){ return (uint32_t)task->pStack; }
static inline int bench_is_current(bench_task_t* task){ return OS_pCurrentTask == task; }
static inline void bench_region_enter(void){ OS_TASK_EnterRegion(); }
static inline void bench_region_leave(void){ OS_TASK_LeaveRegion(); }
static inline void bench_event_set(bench_task_t* task, unsigned ev){ OS_TASKEVENT_Set(task, ev); }
static inline unsigned bench_event_get_blocked(unsigned ev){
   return (unsigned)OS_TASKEVENT_GetBlocked(ev);
}
static inline unsigned bench_event_get_timed(unsigned ev, unsigned timeout_ms){
   return (unsigned)OS_TASKEVENT_GetTimed((OS_TASKEVENT)ev, (OS_TIME)timeout_ms);
}
static inline void bench_delay(unsigned ms){ OS_TASK_Delay(ms); }
static inline uint32_t bench_ticks(void){ return (uint32_t)OS_TIME_GetTicks32(); }

/* démarrage : interruptions masquées, noyau, matériel du micro-noyau, contrôleur, multitâche */
static inline void bench_start(bench_task_t* ctrl, const char* name, unsigned prio,
                               void (*routine)(void), uint32_t* stack, unsigned size){
   OS_IncDI();
   OS_Init();
   OS_InitHW();
   OS_TASK_Create(ctrl, name, prio, routine, stack, size, 2);
   OS_Start();
}

/* PC de départ attendu (T2) : sous embOS 5.20, trampoline OS_StartTask (symbole exporté par la
   bibliothèque), la routine étant rangée sur la pile au-dessus du cadre OS_REGS_BASE */
extern void OS_StartTask(void);
static inline uint32_t bench_start_pc(void (*routine)(void)){
   (void)routine;
   return (uint32_t)OS_StartTask;
}

/* --- oracle de trame (RTOS.h) : étendu (FPU) si le bit 4 d'EXC_RETURN est à 0 -------------- */
static inline int bench_frame_is_fpu(const void* frame){
   return (((const OS_REGS_BASE*)frame)->OS_REG_EXC_RETURN & 0x10u) == 0;
}
static inline uint32_t bench_frame_size(const void* frame){
   return bench_frame_is_fpu(frame) ? sizeof(OS_REGS_BASE_FPU) : sizeof(OS_REGS_BASE);
}
static inline uint32_t bench_frame_pc(const void* frame){
   return ((const OS_REGS_BASE*)frame)->OS_REG_PC;
}
static inline uint32_t* bench_frame_xpsr(void* frame){
   return bench_frame_is_fpu(frame) ? &((OS_REGS_BASE_FPU*)frame)->OS_REG_XPSR
                                    : &((OS_REGS_BASE*)frame)->OS_REG_XPSR;
}
static inline void bench_frame_corrupt_r4_r11(void* frame){
   OS_REGS_BASE* f = (OS_REGS_BASE*)frame;
   f->OS_REG_R4 = f->OS_REG_R5 = f->OS_REG_R6 = f->OS_REG_R7 = 0xDEAD0000u;
   f->OS_REG_R8 = f->OS_REG_R9 = f->OS_REG_R10 = f->OS_REG_R11 = 0xDEAD0001u;
}
#if (OS_CPU_HAS_VFP == 1)
static inline void bench_frame_corrupt_fpu(void* frame){
   OS_REGS_BASE_FPU* f = (OS_REGS_BASE_FPU*)frame;
   f->OS_REG_R4 = f->OS_REG_R5 = f->OS_REG_R6 = f->OS_REG_R7 = 0xDEAD0000u;
   f->OS_REG_R8 = f->OS_REG_R9 = f->OS_REG_R10 = f->OS_REG_R11 = 0xDEAD0001u;
   memset(&f->S16_S31, 0xEE, sizeof(f->S16_S31));
   memset(&f->S0_S15, 0xEE, sizeof(f->S0_S15));
}
#endif

#endif /* _KAL_BENCH_OS_H */
