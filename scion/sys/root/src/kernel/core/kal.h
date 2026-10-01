/*
The contents of this file are subject to the Mozilla Public License Version 1.1
(the "License"); you may not use this file except in compliance with the License.
You may obtain a copy of the License at http://www.mozilla.org/MPL/

Software distributed under the License is distributed on an "AS IS" basis,
WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License for the
specific language governing rights and limitations under the License.

The Original Code is Lepton.

The Initial Developer of the Original Code is Chauvin-Arnoux.
Portions created by Chauvin-Arnoux are Copyright (C) 2011. All Rights Reserved.

Alternatively, the contents of this file may be used under the terms of the eCos GPL license
(the  [eCos GPL] License), in which case the provisions of [eCos GPL] License are applicable
instead of those above. If you wish to allow use of your version of this file only under the
terms of the [eCos GPL] License and not to allow others to use your version of this file under
the MPL, indicate your decision by deleting  the provisions above and replace
them with the notice and other provisions required by the [eCos GPL] License.
If you do not delete the provisions above, a recipient may use your version of this file under
either the MPL or the [eCos GPL] License."
*/


/*============================================
| Compiler Directive
==============================================*/
#ifndef _KAL_H
#define _KAL_H

/**
 * \addtogroup lepton_kernel
 * @{
 */

/**
 * \addtogroup kal couche d'abstraction pour les operations primitives sur le micro noyau temps rel
 * @{
 *
 */

/*============================================
| Includes
==============================================*/
#include "kernel/core/kernelconf.h"
#include "kernel/core/types.h"

/*============================================
| Declaration
==============================================*/
//for eCos
#if defined(CPU_GNU32) && defined(USE_KERNEL_STATIC)

   #define __va_list_copy(__dest_va_list__,__src_va_list__) __dest_va_list__ = __src_va_list__

//	typedef cyg_thread tcb_t;
//   typedef cyg_handle_t thr_id_t;
//   //contexte défini lors de l'entrée dans le handler de signaux (au sens Linux)
//   typedef cyg_hal_sys_ucontext_t k_handler_context_t;
//   //contexte nécessaire pour la sauvegarde de contexte dans le thread
//   typedef hal_gregset_t context_t;
//   typedef void (*_pthreadstart_routine_t)(void);
//   typedef _pthreadstart_routine_t pthreadstart_routine_t;
typedef struct
{
   unsigned int esp;
   unsigned int next_context;
   unsigned int ebp;
   unsigned int ebx;
   unsigned int esi;
   unsigned int edi;
   short interrupts;
} context_t;

typedef int tcb_t;
typedef int thr_id_t;
//macro de profilage (voir kal.h sur CVS)
/**
        début du zone de code atomique (non préemptible)
 *
 * \hideinitializer
 */
   #define __atomic_in()

/**
 * début du zone de code atomique (non préemptible)
 *
 * \hideinitializer
 */
   #define __atomic_out()

   #define __begin_pthread(pthread_name) \
   void pthread_name(void){

   #define __end_pthread() \
   return; }

   #define _macro_stack_addr
   #define __is_thread_self(__pthread_ptr__)    0

//uninterruptible section in
   #define __disable_interrupt_section_in()
//uninterruptible section out
   #define __disable_interrupt_section_out()

   #define __stop_sched()

   #define __restart_sched()    //cyg_scheduler_start();

   #define __set_active_pthread(__pthread_ptr__)
//énumération de registres pour la sauvegarde de contexte (à mettre éventuellement dans le hal_io.h de synthetic)
enum enum_synth_regs {
   S_REG_GS = 0,
   S_REG_FS,
   S_REG_ES,
   S_REG_DS,
   S_REG_EDI,
   S_REG_ESI,
   S_REG_EBP,
   S_REG_ESP,
   S_REG_EBX,
   S_REG_EDX,
   S_REG_ECX,
   S_REG_EAX,
   S_REG_TRAPNO,
   S_REG_ERR,
   S_REG_EIP,
   S_REG_CS,
   S_REG_EFL,
   S_REG_UESP,
   S_REG_SS
};

//get the 5 registers defined on HAL_SavedRegisters (esp, ebp, ebx, esi, edi)
//and store it at the right place
   #define __get_thread_context(__context__)
//get the main registers at the beginning of the thread
   #define __inline_bckup_thread_start_context(__context__,__pthread_ptr__)

//save the stack of current thread
   #define __inline_bckup_stack(__pthread_ptr__)

//restore the stack content
   #define __inline_rstr_stack(__pthread_ptr__)

//save the current tcb
   #define __inline_bckup_context(__context__,__pthread_ptr__)

//restore the previous tcb
   #define __inline_rstr_context(__context__,__pthread_ptr__)

   #define __inline_swap_signal_handler(__pthread_ptr__,__sig_handler__)

//restaure le contexte du programme interropu par un signal
   #define __inline_exit_signal_handler(__pthread_ptr__)

   #define __bckup_thread_start_context(__context__,__pthread_ptr__) \
   __inline_bckup_thread_start_context(__context__,__pthread_ptr__)

   #define __bckup_context(__context__,__pthread_ptr__) \
   __inline_bckup_context(__context__,__pthread_ptr__)

   #define __bckup_stack(__pthread_ptr__) \
   __inline_bckup_stack(__pthread_ptr__)

   #define __rstr_stack(__pthread_ptr__) \
   __inline_rstr_stack(__pthread_ptr__)

   #define __rstr_context(__context__,__pthread_ptr__) \
   __inline_rstr_context(__context__,__pthread_ptr__)

   #define __swap_signal_handler(__pthread_ptr__,sig_handler) \
   __inline_swap_signal_handler(__pthread_ptr__,sig_handler)

   #define __exit_signal_handler(__pthread_ptr__) \
   __inline_exit_signal_handler(__pthread_ptr__)

   #define __kernel_profiler_start()
   #define __kernel_profiler_stop(__pid__)
   #define __kernel_profiler_get_counter(__pid__)
   #define __io_profiler_init()
   #define __io_profiler_start(__desc__)
   #define __io_profiler_stop(__desc__)
   #define __io_profiler_get_counter(__desc__)
//
#elif defined (__KERNEL_UCORE_EMBOS)\
 &&((__tauon_cpu_core__ == __tauon_cpu_core_arm_arm7tdmi__)\
 || (__tauon_cpu_core__ == __tauon_cpu_core_arm_arm926ejs__)\
 || (__tauon_cpu_core__ == __tauon_cpu_core_arm_cortexM3__)\
 || (__tauon_cpu_core__ == __tauon_cpu_core_arm_cortexM4__)\
 || (__tauon_cpu_core__ == __tauon_cpu_core_arm_cortexM7__))

   #include "RTOS.h"
   //OS_MakeTaskReady : exportee par la bibliotheque embOS, non declaree par RTOS.h (ecart E4).
   //HYPOTHESE A VALIDER : signature void OS_MakeTaskReady(OS_TASK*) deduite des usages Lepton
   //(desassemblage interdit par la licence SFL ; a confirmer aupres de Segger).
   void OS_MakeTaskReady(OS_TASK* pTask);
/*modif for segger version 3.28h */
//#include "OSKern.H"
   //GD-TODO lm3S: improve? 
   #if   (__tauon_cpu_core__ != __tauon_cpu_core_arm_cortexM3__)\
       &&(__tauon_cpu_core__ != __tauon_cpu_core_arm_cortexM4__)\
       &&(__tauon_cpu_core__ != __tauon_cpu_core_arm_cortexM7__)
      #if OS_VERSION_GENERIC != (38607) 
        // #include "OS_Priv.h"
      #endif
   #endif
/*modif for segger version 3.52e */
//#include "OSint.h"

//must be set to 1 for segger version 3.28n and set to 0 for 3.32e
   #if 0
//#include "OSint.h"
   #endif

   #include "stdlib.h"

   #if (__tauon_cpu_device__ == __tauon_cpu_device_arm7_at91m55800a__)
      #include <ioat91m55800.h>
   #endif

   #if (__tauon_cpu_device__ == __tauon_cpu_device_arm7_at91sam7x__)
      #include <ioat91sam7x256.h>
   #endif

   #if (__tauon_cpu_device__ == __tauon_cpu_device_arm9_at91sam9261__)
      #include <atmel/ioat91sam9261.h>
   #endif

   #define __va_list_copy(__dest_va_list__,__src_va_list__) memcpy(&__dest_va_list__,&__src_va_list__,sizeof(__dest_va_list__))
//for compatibility with m16c 3.06h
//

typedef OS_TASK tcb_t;
typedef void (*_pthreadstart_routine_t)(void);
typedef _pthreadstart_routine_t pthreadstart_routine_t;
typedef int thr_id_t;

   #define __begin_pthread(pthread_name) \
   void pthread_name(void){

   #define __end_pthread() \
   return; }

   #define __is_thread_self(__tcb__) \
   (__tcb__ == OS_pCurrentTask)

   #define _macro_stack_addr OS_STACKPTR

   #if(OS_VERSION_GENERIC > (38000))
      #define OS_REGS_GENERIC    OS_REGS_BASE
   #else
      #define OS_REGS_GENERIC    OS_REGS
   #endif
    #if(OS_VERSION_GENERIC >= 38400)
    #else
        #define OS_REG_PC   PC
    #endif

   //Ecart E3 (embos-iar-vs-gcc.md) : avec FPU (OS_CPU_HAS_VFP, defini par RTOS.h), embOS sauvegarde
   //un cadre etendu OS_REGS_BASE_FPU (S16-S31 avant R0, S0-S15 et FPSCR apres xPSR) pour une tache
   //ayant un contexte FPU actif, signale par le bit 4 d'EXC_RETURN a 0 (meme position dans les deux
   //cadres). Le contexte copie le cadre effectif ; sans FPU, cadre OS_REGS_GENERIC inchange.
   #if defined(OS_CPU_HAS_VFP) && (OS_CPU_HAS_VFP == 1)
      #define OS_REGS_CONTEXT    OS_REGS
      #define __os_regs_is_fpu(__regs__) \
         ((((OS_REGS_BASE OS_STACKPTR *)(__regs__))->OS_REG_EXC_RETURN & 0x10u) == 0u)
      #define __os_regs_size(__regs__) \
         (__os_regs_is_fpu(__regs__) ? sizeof(OS_REGS_BASE_FPU) : sizeof(OS_REGS_BASE))
      #define __os_regs_pc(__regs__) \
         (*(__os_regs_is_fpu(__regs__) \
            ? &((OS_REGS OS_STACKPTR *)(__regs__))->Base_FPU.OS_REG_PC \
            : &((OS_REGS OS_STACKPTR *)(__regs__))->Base.OS_REG_PC))
   #else
      #define OS_REGS_CONTEXT    OS_REGS_GENERIC
      #define __os_regs_size(__regs__) sizeof(OS_REGS_GENERIC)
      #define __os_regs_pc(__regs__) (((OS_REGS_GENERIC OS_STACKPTR *)(__regs__))->OS_REG_PC)
   #endif
typedef struct {
   OS_TASK os_task;
      OS_REGS_CONTEXT  os_regs;
}context_t;

   #define __inline_bckup_thread_start_context(__context__,__pthread_ptr__){ \
      memcpy(&__context__.os_task,__pthread_ptr__->tcb,sizeof(OS_TASK)); \
      memcpy(&__context__.os_regs,((OS_REGS_CONTEXT OS_STACKPTR *)__pthread_ptr__->tcb->pStack),__os_regs_size(__pthread_ptr__->tcb->pStack));\
}

   #define __inline_bckup_context(__context__,__pthread_ptr__){ \
      memcpy(&__context__.os_task,__pthread_ptr__->tcb,sizeof(OS_TASK)); \
      memcpy(&__context__.os_regs,((OS_REGS_CONTEXT OS_STACKPTR *)__pthread_ptr__->tcb->pStack),__os_regs_size(__pthread_ptr__->tcb->pStack));\
}

   #define __inline_rstr_context(__context__,__pthread_ptr__){ \
      OS_TASK   *pPrev; \
      OS_TASK   *pNext; \
      pPrev= __pthread_ptr__->tcb->pPrev; \
      pNext= __pthread_ptr__->tcb->pNext; \
      memcpy(__pthread_ptr__->tcb,&__context__.os_task,sizeof(OS_TASK)); \
      memcpy(((OS_REGS_CONTEXT OS_STACKPTR *)__pthread_ptr__->tcb->pStack),&__context__.os_regs,__os_regs_size(&__context__.os_regs));\
      __pthread_ptr__->tcb->pNext = pNext; \
      __pthread_ptr__->tcb->pPrev = pPrev; \
}

//Use Dynamic Allocation!!!
   #define __inline_bckup_stack(__pthread_ptr__){ \
      int __stack_size__; \
      void* __src_stack_ptr__; \
      __stack_size__ = ((int)(__pthread_ptr__->bckup_context.os_task.pStack) - (int)(__pthread_ptr__->start_context.os_task.pStack));\
      __src_stack_ptr__ = (void*)(((uint8_t*)__pthread_ptr__->start_context.os_task.pStack)+__stack_size__);\
      __pthread_ptr__->bckup_stack = (char*)_sys_malloc( abs(__stack_size__) ); \
      if(!__pthread_ptr__->bckup_stack) \
         return -ENOMEM; \
      memcpy(__pthread_ptr__->bckup_stack,__src_stack_ptr__,abs(__stack_size__)); \
}

   #define __inline_rstr_stack(__pthread_ptr__){ \
      int __stack_size__; \
      void* __src_stack_ptr__; \
      __stack_size__ = ((int)(__pthread_ptr__->bckup_context.os_task.pStack)- (int)(__pthread_ptr__->start_context.os_task.pStack));\
      __src_stack_ptr__ = (void*)(((uint8_t*)__pthread_ptr__->start_context.os_task.pStack)+__stack_size__);\
      memcpy(__src_stack_ptr__,__pthread_ptr__->bckup_stack,abs(__stack_size__)); \
      _sys_free(__pthread_ptr__->bckup_stack); \
}

#if   (__tauon_cpu_core__ == __tauon_cpu_core_arm_cortexM3__)\
       || (__tauon_cpu_core__ == __tauon_cpu_core_arm_cortexM4__)\
       || (__tauon_cpu_core__ == __tauon_cpu_core_arm_cortexM7__)
         
   #if (OS_VERSION_GENERIC>=51800u)
      #define __inline_swap_signal_handler(__pthread_ptr__,__sig_handler__){ \
         /*modif for 3.06h version*/ \
         /*((OS_REGS OS_STACKPTR *)process_lst[pid]->pthread_ptr->tcb->pStack)->RetAdr4    = OS_MakeIntAdr(sig_handler);*/ \
         /* First return adr (see OS_Private.h)*/ \
         /*modif for 3.32 replace RetAdr4 by PC0 */ \
         /*((OS_REGS OS_STACKPTR *)__pthread_ptr__->tcb->pStack)->PC0= (OS_U32)(__sig_handler__);*/ \
         /*modif for 3.52e and 3.60 replace PC0 by PC */ \
         /*((OS_REGS_GENERIC OS_STACKPTR *)__pthread_ptr__->tcb->pStack)->PC= (OS_U32)(__sig_handler__);\*/\
         /* GD - modif for 3.84, "PC" from OS_REGS_BASE struct changed to "OS_REG_PC" since embOS 3.84. for cotrex M3/M4 core */\
         /* E3 : PC du cadre effectif (etendu si FPU) ; OS_Global_Counters en tete des deux cadres */\
         __os_regs_pc(__pthread_ptr__->tcb->pStack)= (OS_U32)(__sig_handler__);\
         ((OS_REGS_GENERIC OS_STACKPTR *)__pthread_ptr__->tcb->pStack)->OS_Global_Counters= 0;\
         __pthread_ptr__->tcb->Timeout=0; \
         __pthread_ptr__->tcb->Stat=0; \
         OS_MakeTaskReady(__pthread_ptr__->tcb); \
      }
   #else
      #define __inline_swap_signal_handler(__pthread_ptr__,__sig_handler__){ \
         /*modif for 3.06h version*/ \
         /*((OS_REGS OS_STACKPTR *)process_lst[pid]->pthread_ptr->tcb->pStack)->RetAdr4    = OS_MakeIntAdr(sig_handler);*/ \
         /* First return adr (see OS_Private.h)*/ \
         /*modif for 3.32 replace RetAdr4 by PC0 */ \
         /*((OS_REGS OS_STACKPTR *)__pthread_ptr__->tcb->pStack)->PC0= (OS_U32)(__sig_handler__);*/ \
         /*modif for 3.52e and 3.60 replace PC0 by PC */ \
         /*((OS_REGS_GENERIC OS_STACKPTR *)__pthread_ptr__->tcb->pStack)->PC= (OS_U32)(__sig_handler__);\*/\
         /* GD - modif for 3.84, "PC" from OS_REGS_BASE struct changed to "OS_REG_PC" since embOS 3.84. for cotrex M3/M4 core */\
         ((OS_REGS_GENERIC OS_STACKPTR *)__pthread_ptr__->tcb->pStack)->OS_REG_PC= (OS_U32)(__sig_handler__);\
         ((OS_REGS_GENERIC OS_STACKPTR *)__pthread_ptr__->tcb->pStack)->Counters= 0;\
         __pthread_ptr__->tcb->Timeout=0; \
         __pthread_ptr__->tcb->Stat=0; \
         OS_MakeTaskReady(__pthread_ptr__->tcb); \
      }
   #endif

#elif  (__tauon_cpu_core__ == __tauon_cpu_core_arm_arm926ejs__)
   #define __inline_swap_signal_handler(__pthread_ptr__,__sig_handler__){ \
      /* phlb - modif for 3.88, "PC" from OS_REGS_BASE for arm7/arm9  core*/\
      ((OS_REGS_GENERIC OS_STACKPTR *)__pthread_ptr__->tcb->pStack)->PC= (OS_U32)(__sig_handler__);\
      ((OS_REGS_GENERIC OS_STACKPTR *)__pthread_ptr__->tcb->pStack)->Counters= 0;\
      __pthread_ptr__->tcb->Timeout=0; \
      __pthread_ptr__->tcb->Stat=0; \
      OS_MakeTaskReady(__pthread_ptr__->tcb); \
   }
#endif

/*TS_WAIT_TIME*/
   #define __inline_exit_signal_handler(__pthread_ptr__){ \
      __rstr_context(__pthread_ptr__->bckup_context,__pthread_ptr__); \
      /*process_lst[pid]->pthread_ptr->tcb->TASK_Timeout=0;*/ \
      /*process_lst[pid]->pthread_ptr->tcb->Stat=0;*/ \
}

   #define __set_active_pthread(__pthread_ptr__) \
   if(__pthread_ptr__) OS_MakeTaskReady(__pthread_ptr__->tcb)

//stop task switching and software timer.
   #define __atomic_in() OS_EnterRegion(); //stop task switching not the scheduler, kernel timeslice must be set to 0 cooprative mode).
//restart task switching
   #define __atomic_out() OS_LeaveRegion(); //restart task switching

   #if (__tauon_cpu_device__ == __tauon_cpu_device_arm7_at91m55800a__)
//stop timer (TIMER A0) tick for scheduler.TABSR.0=0;
      #define __stop_sched() __TC_CCRC0 &= ~(1);
//restart timer (TIMER A0) tick for scheduler.TABSR.0=1;
      #define __restart_sched() __TC_CCRC0 |= 1;
   #endif

   #if (__tauon_cpu_device__ == __tauon_cpu_device_arm7_at91sam7x__)
      #define __LEPTON_KAL_PIT_BASE     (0xFFFFFD30)
      #define __LEPTON_KAL_PIT_MR       (*(volatile OS_U32*) (__LEPTON_KAL_PIT_BASE + 0x00))
//stop timer (PIT periodic interval timer) tick for scheduler.PITIEN=0;
      #define __stop_sched() __LEPTON_KAL_PIT_MR &= ~(1<<25);
//restart timer (PIT periodic interval timer) tick for scheduler.PITIEN=1;
      #define __restart_sched() __LEPTON_KAL_PIT_MR |= (1<<25);
   #endif

   //GD all Cortex-M3 and cortex M4 MCUs have the same systick registers
   #if   (__tauon_cpu_core__ == __tauon_cpu_core_arm_cortexM3__)\
       ||(__tauon_cpu_core__ == __tauon_cpu_core_arm_cortexM4__)\
       ||(__tauon_cpu_core__ == __tauon_cpu_core_arm_cortexM7__)
      #define __LEPTON_KAL_PIT_BASE    (0xE000E010)
      #define __LEPTON_KAL_PIT_MR      (*(volatile OS_U32*)(__LEPTON_KAL_PIT_BASE + 0x00))
      #define __stop_sched() __LEPTON_KAL_PIT_MR &= ~(1uL << (1));
      #define __restart_sched() __LEPTON_KAL_PIT_MR |= (1uL << (1));
   #endif

   #if (__tauon_cpu_device__ == __tauon_cpu_device_arm9_at91sam9261__)
      #define __LEPTON_KAL_PIT_BASE     (0xFFFFFD30)
      #define __LEPTON_KAL_PIT_MR       (*(volatile OS_U32*) (__LEPTON_KAL_PIT_BASE + 0x00))
//stop timer (PIT periodic interval timer) tick for scheduler.PITIEN=0;
      #define __stop_sched() __LEPTON_KAL_PIT_MR &= ~(1<<25);
//restart timer (PIT periodic interval timer) tick for scheduler.PITIEN=1;
      #define __restart_sched() __LEPTON_KAL_PIT_MR |= (1<<25);
   #endif


//uninterruptible section in
   #define __disable_interrupt_section_in() OS_IncDI()
//uninterruptible section out
   #define __disable_interrupt_section_out() OS_DecRI()

//profiler macros for arm7 (at91m55800a)
   #if (__tauon_cpu_device__ == __tauon_cpu_device_arm7_at91m55800a__) && defined(KERNEL_PROFILER)
//
      #define PROFILER_PERIOD (1024.0/32000000.0)
//
      #define PROFILER_START_COUNTER_VALUE 0xFFFF
//
      #define __kernel_profiler_start(){ \
      __APMC_PCER |= 0x200; \
      __TC_BMR1   = 0x02; \
      __TC_CCR1C0 = 2; \
      __TC_CMR1C0 = 0x00004004; \
      __TC_RC1C1 = PROFILER_START_COUNTER_VALUE;   /*0x0bdb;*//*32000000/2/1000;*/ \
      __TC_CCR1C0 = 1; \
      __TC_CCR1C0 = 5; \
}
//
      #define __kernel_profiler_stop(__pthread_ptr__){ \
      __TC_CCR1C0 = 2; \
      if(__pthread_ptr__) \
         __pthread_ptr__->_profile_counter=__TC_CV1C0; \
}
//
      #define __kernel_profiler_get_counter(__pthread_ptr__) (__pthread_ptr__?__pthread_ptr__->_profile_counter:0)

//io
      #define __io_profiler_init(){ \
      __APMC_PCER |= 0x400; \
      __TC_CCR1C1 = 2; \
      __TC_BMR1   = 0x02; \
      __TC_CMR1C1 = 0x00000004; \
      __TC_RC1C1 = PROFILER_START_COUNTER_VALUE; \
      __TC_CCR1C1 = 1; \
      __TC_CCR1C1 = 5; \
}
//
      #define __io_profiler_start(__desc__){ \
      ofile_lst[__desc__]._profile_counter=(__TC_CV1C1); \
}
//
      #define __io_profiler_stop(__desc__){ \
      unsigned short __counter__ = __TC_CV1C1; \
      if(__counter__ > ofile_lst[__desc__]._profile_counter ) \
         ofile_lst[__desc__]._profile_counter=(__counter__)-ofile_lst[__desc__]._profile_counter; \
      else \
            ofile_lst[__desc__]._profile_counter=(PROFILER_START_COUNTER_VALUE-ofile_lst[__desc__]._profile_counter)+__counter__;\
}
//
      #define __io_profiler_get_counter(__desc__) ofile_lst[__desc__]._profile_counter
//
   #endif //END KERNEL_PROFILER CPU_ARM7

//profiler macros for arm9 (at91sam9260 and at91sam9261)
   #if (__tauon_cpu_device__ == __tauon_cpu_device_arm9_at91sam9261__) && defined(KERNEL_PROFILER)

//
      #define PROFILER_PERIOD           (1.0/32000.0)
//
      #define PROFILER_START_COUNTER_VALUE 0xFFFF
//
//#define __kernel_profiler_start()
//#define __kernel_profiler_stop(__pid__)
//#define __kernel_profiler_get_counter(__pid__) (0)

      #define __kernel_profiler_start(){ \
      *AT91C_PMC_PCER |= 0x80000; \
      *AT91C_TCB0_BMR = 0x02; \
      *AT91C_TC2_CCR  = 2; \
      *AT91C_TC2_CMR  = 0x00004004; \
      *AT91C_TC2_RC   = PROFILER_START_COUNTER_VALUE; \
      *AT91C_TC2_CCR  = 1; \
      *AT91C_TC2_CCR  = 5; \
}

//
      #define __kernel_profiler_stop(__pthread_ptr__){ \
      *AT91C_TC2_CCR = 2; \
      if(__pthread_ptr__) \
         __pthread_ptr__->_profile_counter=*AT91C_TC2_CV; \
}
//
      #define __kernel_profiler_get_counter(__pthread_ptr__) (__pthread_ptr__?__pthread_ptr__->_profile_counter:0)

//
      #define __io_profiler_init(){ \
      *AT91C_PMC_PCER |= 0x40000; \
      *AT91C_TCB0_BMR = 0x01; \
      *AT91C_TC1_CCR  = 2; \
      *AT91C_TC1_CMR  = 0x00004004; \
      *AT91C_TC1_RC   = PROFILER_START_COUNTER_VALUE; \
      *AT91C_TC1_CCR  = 1; \
      *AT91C_TC1_CCR  = 5; \
}
//
      #define __io_profiler_start(__desc__){ \
      ofile_lst[__desc__]._profile_counter=(*AT91C_TC1_CV); \
}
//
      #define __io_profiler_stop(__desc__){ \
      unsigned short __counter__ = (*AT91C_TC1_CV); \
      if(__counter__ > ofile_lst[__desc__]._profile_counter ) \
         ofile_lst[__desc__]._profile_counter=(__counter__)-ofile_lst[__desc__]._profile_counter; \
      else \
            ofile_lst[__desc__]._profile_counter=(PROFILER_START_COUNTER_VALUE-ofile_lst[__desc__]._profile_counter)+__counter__;\
}
//
      #define __io_profiler_get_counter(__desc__) ofile_lst[__desc__]._profile_counter

//
   #endif //END KERNEL_PROFILER CPU_ARM9

//profiling option not enabled (see in sys/root/src/kernel/core/kernelconf.h)
   #if (!defined(KERNEL_PROFILER) || !defined(__kernel_profiler_start)) //GD trick for default/unkown CPU
      #define __kernel_profiler_start()
      #define __kernel_profiler_stop(__pid__)
      #define __kernel_profiler_get_counter(__pid__)
      #define __io_profiler_init()
      #define __io_profiler_start(__desc__)
      #define __io_profiler_stop(__desc__)
      #define __io_profiler_get_counter(__desc__)
   #endif

#elif ((__tauon_compiler__ == __compiler_keil_arm__) || (__tauon_compiler__ == __compiler_gnuc__)) && defined(__KERNEL_UCORE_FREERTOS) && ((__tauon_cpu_core__ == __tauon_cpu_core_arm_arm7tdmi__) || (__tauon_cpu_core__ == __tauon_cpu_core_arm_arm926ejs__) || (__tauon_cpu_core__ == __tauon_cpu_core_arm_cortexM0__) || (__tauon_cpu_core__ == __tauon_cpu_core_arm_cortexM3__) || (__tauon_cpu_core__ == __tauon_cpu_core_arm_cortexM4__) || (__tauon_cpu_core__ == __tauon_cpu_core_arm_cortexM7__))

   //#include <stdlib.h>
   //#include <string.h>   

   #include "FreeRTOS.h"
   #include "task.h"
   #include "semphr.h"
   #include "timers.h"
   #include "event_groups.h"

   #include "kernel/core/ucore/freeRTOS_8-0-0/source/include/kal_freertos.h"


   #if (__tauon_cpu_device__ == __tauon_cpu_device_arm7_at91m55800a__)
      #include <ioat91m55800.h>
   #endif

   #if (__tauon_cpu_device__ == __tauon_cpu_device_arm7_at91sam7x__)
      #include <ioat91sam7x256.h>
   #endif

   #if (__tauon_cpu_device__ == __tauon_cpu_device_arm9_at91sam9261__)
      #include <atmel/ioat91sam9261.h>
   #endif

   #define __va_list_copy(__dest_va_list__,__src_va_list__) memcpy(&__dest_va_list__,&__src_va_list__,sizeof(__dest_va_list__))

   #if ( (__tauon_cpu_core__ ==__tauon_cpu_core_arm_cortexM0__) )
      typedef struct cpu_regs_st {
           uint32_t  OS_REG_R4;
           uint32_t  OS_REG_R5;
           uint32_t  OS_REG_R6;
           uint32_t  OS_REG_R7;
           uint32_t  OS_REG_R8;
           uint32_t  OS_REG_R9;
           uint32_t  OS_REG_R10;
           uint32_t  OS_REG_R11;
           //uint32_t  OS_REG_LR;
           uint32_t  OS_REG_R0;
           uint32_t  OS_REG_R1;
           uint32_t  OS_REG_R2;
           uint32_t  OS_REG_R3;
           uint32_t  OS_REG_R12;
           uint32_t  OS_REG_R14;
           uint32_t  OS_REG_PC;
           uint32_t  OS_REG_XPSR;
         } cpu_regs_t;
   #elif (  (__tauon_cpu_core__ ==__tauon_cpu_core_arm_cortexM3__)\
         || (__tauon_cpu_core__ ==__tauon_cpu_core_arm_cortexM4__)\
         || (__tauon_cpu_core__ ==__tauon_cpu_core_arm_cortexM7__) )
      typedef struct cpu_regs_st {
           uint32_t  OS_REG_R4;
           uint32_t  OS_REG_R5;
           uint32_t  OS_REG_R6;
           uint32_t  OS_REG_R7;
           uint32_t  OS_REG_R8;
           uint32_t  OS_REG_R9;
           uint32_t  OS_REG_R10;
           uint32_t  OS_REG_R11;
           uint32_t  OS_REG_LR;
           uint32_t  OS_REG_R0;
           uint32_t  OS_REG_R1;
           uint32_t  OS_REG_R2;
           uint32_t  OS_REG_R3;
           uint32_t  OS_REG_R12;
           uint32_t  OS_REG_R14;
           uint32_t  OS_REG_PC;
           uint32_t  OS_REG_XPSR;
         } cpu_regs_t;
   #elif ( (__tauon_cpu_core__ ==__tauon_cpu_core_arm_arm7tdmi__) || (__tauon_cpu_core__ ==__tauon_cpu_core_arm_arm926ejs__) )
      typedef struct {
         uint32_t counters_critical_nesting;
         uint32_t SPSR;
         uint32_t R0;
         uint32_t R1;
         uint32_t R2;
         uint32_t R3;
         uint32_t R4;
         uint32_t R5;
         uint32_t R6;
         uint32_t R7;
         uint32_t R8;
         uint32_t R9;
         uint32_t R10;
         uint32_t R11;
         uint32_t R12;
         uint32_t R13;
         uint32_t R14;
         uint32_t OS_REG_PC;
      } cpu_regs_t;
   #endif
    
   
   typedef freertos_tcb_t tcb_t;
   typedef void (*_pthreadstart_routine_t)(void);
   typedef _pthreadstart_routine_t pthreadstart_routine_t;
   typedef int thr_id_t;

   #define __begin_pthread(pthread_name) \
   void pthread_name(void){

   #define __end_pthread() \
   return; }

   #define __is_thread_self(__tcb__) \
   ((xTaskHandle)__tcb__ == xTaskGetCurrentTaskHandle())

   #define _macro_stack_addr 
   /*portSTACK_TYPE*/


   typedef struct {
      tcb_t tcb;
      cpu_regs_t  os_regs;
   }context_t;

   #define __inline_bckup_thread_start_context(__context__,__pthread_ptr__){ \
      memcpy(&__context__.tcb,__pthread_ptr__->tcb,sizeof(tcb_t)); \
      memcpy(&__context__.os_regs,((cpu_regs_t *)__pthread_ptr__->tcb->pStack),sizeof(cpu_regs_t));\
   }

   #define __inline_bckup_context(__context__,__pthread_ptr__){ \
      memcpy(&__context__.tcb,__pthread_ptr__->tcb,sizeof(tcb_t)); \
      memcpy(&__context__.os_regs,((cpu_regs_t *)__pthread_ptr__->tcb->pStack),sizeof(cpu_regs_t));\
   }

   #define __inline_rstr_context(__context__,__pthread_ptr__){ \
      ListItem_t xGenericListItem=__pthread_ptr__->tcb->xGenericListItem;\
      ListItem_t xEventListItem=__pthread_ptr__->tcb->xEventListItem;\
      memcpy(__pthread_ptr__->tcb,&__context__.tcb,sizeof(tcb_t)); \
      memcpy(((cpu_regs_t *)__pthread_ptr__->tcb->pStack),&__context__.os_regs,sizeof(cpu_regs_t));\
      __pthread_ptr__->tcb->xGenericListItem=xGenericListItem;\
      __pthread_ptr__->tcb->xEventListItem=xEventListItem;\
   }

   //Use Dynamic Allocation!!!
   #define __inline_bckup_stack(__pthread_ptr__){ \
      int __stack_size__; \
      void* __src_stack_ptr__; \
      __stack_size__ = ((int)(__pthread_ptr__->bckup_context.tcb.pStack) - (int)(__pthread_ptr__->start_context.tcb.pStack));\
      __src_stack_ptr__ = (void*)(((uint8_t*)__pthread_ptr__->start_context.tcb.pStack)+__stack_size__);\
      __pthread_ptr__->bckup_stack = (char*)_sys_malloc( abs(__stack_size__) ); \
      if(!__pthread_ptr__->bckup_stack) \
         return -ENOMEM; \
      memcpy(__pthread_ptr__->bckup_stack,__src_stack_ptr__,abs(__stack_size__)); \
   }

   #define __inline_rstr_stack(__pthread_ptr__){ \
      int __stack_size__; \
      void* __src_stack_ptr__; \
      __stack_size__ = ((int)(__pthread_ptr__->bckup_context.tcb.pStack)- (int)(__pthread_ptr__->start_context.tcb.pStack));\
      __src_stack_ptr__ = (void*)(((uint8_t*)__pthread_ptr__->start_context.tcb.pStack)+__stack_size__);\
      memcpy(__src_stack_ptr__,__pthread_ptr__->bckup_stack,abs(__stack_size__)); \
      _sys_free(__pthread_ptr__->bckup_stack); \
   }

   
   #define __inline_swap_signal_handler(__pthread_ptr__,__sig_handler__){ \
      ((cpu_regs_t *)__pthread_ptr__->tcb->pStack)->OS_REG_PC= (uint32_t)(__sig_handler__);\
   }

 
   /*TS_WAIT_TIME*/
   #define __inline_exit_signal_handler(__pthread_ptr__){ \
      __rstr_context(__pthread_ptr__->bckup_context,__pthread_ptr__); \
   }

   #define __set_active_pthread(__pthread_ptr__) \
      if(__pthread_ptr__)vTaskResume((xTaskHandle)(__pthread_ptr__->tcb))

   //stop task switching and software timer.
   #define __atomic_in() vTaskSuspendAll()
   //restart task switching
   #define __atomic_out() xTaskResumeAll()

   #if (__tauon_cpu_device__ == __tauon_cpu_device_arm7_at91m55800a__)
      //stop timer (TIMER A0) tick for scheduler.TABSR.0=0;
      #define __stop_sched() __TC_CCRC0 &= ~(1);
      //restart timer (TIMER A0) tick for scheduler.TABSR.0=1;
      #define __restart_sched() __TC_CCRC0 |= 1;
   #endif

   #if (__tauon_cpu_device__ == __tauon_cpu_device_arm7_at91sam7x__)
      #define __LEPTON_KAL_PIT_BASE     (0xFFFFFD30)
      #define __LEPTON_KAL_PIT_MR       (*(volatile uint32_t*) (__LEPTON_KAL_PIT_BASE + 0x00))
      //stop timer (PIT periodic interval timer) tick for scheduler.PITIEN=0;
      #define __stop_sched() __LEPTON_KAL_PIT_MR &= ~(1<<25);
      //restart timer (PIT periodic interval timer) tick for scheduler.PITIEN=1;
      #define __restart_sched() __LEPTON_KAL_PIT_MR |= (1<<25);
   #endif

   //GD all Cortex-M3 and cortex M4 MCUs have the same systick registers
   #if   (__tauon_cpu_core__ == __tauon_cpu_core_arm_cortexM0__)\
       ||(__tauon_cpu_core__ == __tauon_cpu_core_arm_cortexM3__)\
       ||(__tauon_cpu_core__ == __tauon_cpu_core_arm_cortexM4__)\
       ||(__tauon_cpu_core__ == __tauon_cpu_core_arm_cortexM7__)
      #define __LEPTON_KAL_PIT_BASE    (0xE000E010)
      #define __LEPTON_KAL_PIT_MR      (*(volatile uint32_t*)(__LEPTON_KAL_PIT_BASE + 0x00))
      #define __stop_sched() __LEPTON_KAL_PIT_MR &= ~(1uL << (1));
      #define __restart_sched() __LEPTON_KAL_PIT_MR |= (1uL << (1));
   #endif

   #if (__tauon_cpu_device__ == __tauon_cpu_device_arm9_at91sam9261__)
      #define __LEPTON_KAL_PIT_BASE     (0xFFFFFD30)
      #define __LEPTON_KAL_PIT_MR       (*(volatile uint32_t*) (__LEPTON_KAL_PIT_BASE + 0x00))
      //stop timer (PIT periodic interval timer) tick for scheduler.PITIEN=0;
      #define __stop_sched() __LEPTON_KAL_PIT_MR &= ~(1<<25);
      //restart timer (PIT periodic interval timer) tick for scheduler.PITIEN=1;
      #define __restart_sched() __LEPTON_KAL_PIT_MR |= (1<<25);
   #endif

   //uninterruptible section in
   #define __disable_interrupt_section_in() taskENTER_CRITICAL()
   
   //uninterruptible section out
   #define __disable_interrupt_section_out() taskEXIT_CRITICAL()

   //profiler macros for arm7 (at91m55800a)
   #if (__tauon_cpu_device__ == __tauon_cpu_device_arm7_at91m55800a__) && defined(KERNEL_PROFILER)
      //
      #define PROFILER_PERIOD (1024.0/32000000.0)
      //
      #define PROFILER_START_COUNTER_VALUE 0xFFFF
      //
      #define __kernel_profiler_start(){ \
         __APMC_PCER |= 0x200; \
         __TC_BMR1   = 0x02; \
         __TC_CCR1C0 = 2; \
         __TC_CMR1C0 = 0x00004004; \
         __TC_RC1C1 = PROFILER_START_COUNTER_VALUE;   /*0x0bdb;*//*32000000/2/1000;*/ \
         __TC_CCR1C0 = 1; \
         __TC_CCR1C0 = 5; \
      }
      //
      #define __kernel_profiler_stop(__pthread_ptr__){ \
         __TC_CCR1C0 = 2; \
         if(__pthread_ptr__) \
            __pthread_ptr__->_profile_counter=__TC_CV1C0; \
      }
      //
      #define __kernel_profiler_get_counter(__pthread_ptr__) (__pthread_ptr__?__pthread_ptr__->_profile_counter:0)

      //io
      #define __io_profiler_init(){ \
         __APMC_PCER |= 0x400; \
         __TC_CCR1C1 = 2; \
         __TC_BMR1   = 0x02; \
         __TC_CMR1C1 = 0x00000004; \
         __TC_RC1C1 = PROFILER_START_COUNTER_VALUE; \
         __TC_CCR1C1 = 1; \
         __TC_CCR1C1 = 5;    
      }
      //
      #define __io_profiler_start(__desc__){ \
         ofile_lst[__desc__]._profile_counter=(__TC_CV1C1); \
      }
      //
      #define __io_profiler_stop(__desc__){ \
         unsigned short __counter__ = __TC_CV1C1; \
         if(__counter__ > ofile_lst[__desc__]._profile_counter ) \
               ofile_lst[__desc__]._profile_counter=(__counter__)-ofile_lst[__desc__]._profile_counter; \
            else \
                  ofile_lst[__desc__]._profile_counter=(PROFILER_START_COUNTER_VALUE-ofile_lst[__desc__]._profile_counter)+__counter__;\
         }
      //
      #define __io_profiler_get_counter(__desc__) ofile_lst[__desc__]._profile_counter
   //
   #endif //END KERNEL_PROFILER CPU_ARM7

   //profiler macros for arm9 (at91sam9260 and at91sam9261)
   #if (__tauon_cpu_device__ == __tauon_cpu_device_arm9_at91sam9261__) && defined(KERNEL_PROFILER)

      //
      #define PROFILER_PERIOD           (1.0/32000.0)
      //
      #define PROFILER_START_COUNTER_VALUE 0xFFFF
      //
      //#define __kernel_profiler_start()
      //#define __kernel_profiler_stop(__pid__)
      //#define __kernel_profiler_get_counter(__pid__) (0)

      #define __kernel_profiler_start(){ \
         *AT91C_PMC_PCER |= 0x80000; \
         *AT91C_TCB0_BMR = 0x02; \
         *AT91C_TC2_CCR  = 2; \
         *AT91C_TC2_CMR  = 0x00004004; \
         *AT91C_TC2_RC   = PROFILER_START_COUNTER_VALUE; \
         *AT91C_TC2_CCR  = 1; \
         *AT91C_TC2_CCR  = 5; \
      }

      //
      #define __kernel_profiler_stop(__pthread_ptr__){ \
         *AT91C_TC2_CCR = 2; \
         if(__pthread_ptr__) \
            __pthread_ptr__->_profile_counter=*AT91C_TC2_CV; \
      }  
      //
      #define __kernel_profiler_get_counter(__pthread_ptr__) (__pthread_ptr__?__pthread_ptr__->_profile_counter:0)

      //
      #define __io_profiler_init(){ \
         *AT91C_PMC_PCER |= 0x40000; \
         *AT91C_TCB0_BMR = 0x01; \
         *AT91C_TC1_CCR  = 2; \
         *AT91C_TC1_CMR  = 0x00004004; \
         *AT91C_TC1_RC   = PROFILER_START_COUNTER_VALUE; \
         *AT91C_TC1_CCR  = 1; \
         *AT91C_TC1_CCR  = 5; \
      }
      //
      #define __io_profiler_start(__desc__){ \
         ofile_lst[__desc__]._profile_counter=(*AT91C_TC1_CV); \
      }
      //
      #define __io_profiler_stop(__desc__){ \
         unsigned short __counter__ = (*AT91C_TC1_CV); \
         if(__counter__ > ofile_lst[__desc__]._profile_counter ) \
            ofile_lst[__desc__]._profile_counter=(__counter__)-ofile_lst[__desc__]._profile_counter; \
         else \
            ofile_lst[__desc__]._profile_counter=(PROFILER_START_COUNTER_VALUE-ofile_lst[__desc__]._profile_counter)+__counter__;\
      }
      //
      #define __io_profiler_get_counter(__desc__) ofile_lst[__desc__]._profile_counter

   //
   #endif //END KERNEL_PROFILER CPU_ARM9

   //profiling option not enabled (see in sys/root/src/kernel/core/kernelconf.h)
   #if (!defined(KERNEL_PROFILER) || !defined(__kernel_profiler_start)) //GD trick for default/unkown CPU
      #define __kernel_profiler_start()
      #define __kernel_profiler_stop(__pid__)
      #define __kernel_profiler_get_counter(__pid__)
      #define __io_profiler_init()
      #define __io_profiler_start(__desc__)
      #define __io_profiler_stop(__desc__)
      #define __io_profiler_get_counter(__desc__)
   #endif

#else
/**
* structure de contexte utilis par le micro-noyau\n
* cette structure contient gnralement toutes les informations ncessaire  la commutation de tache:\n
*  1) sauvegarde de certains registres du processeur comme le pointeur de pile par exemple.\n
*  2) sauvegarde de du compteur programme.\n
* \n
* ce sont les deux informations les plus importantes pour lepton. Elle permettent de raliser le vfork()
* et la gestion des signaux avec kill().
* \hideinitializer
*/
typedef CONTEXT context_t;

/**
 * definition du prototype de fonction de la tache gre par le micro-noyau
 *
 * \param pthread_name nom de la fonction
 *
 * \hideinitializer
 */
   #define __begin_pthread(pthread_name)

/**
 * definition de la sortie de fonction de la tache gre par le micro-noyau
 *
 * \hideinitializer
 */
   #define __end_pthread()

/**
 * permet de savoir si le task control block tcb est bien celui de la tache courante
 *
 * \param tcb task control block
 *
 * \hideinitializer
 */
   #define __is_thread_self(tcb)

/**
 * permet de sauvegarder le contexte du thread __pthread_ptr dans context.
 *
 * \param context variable de type context_t dans laquelle sera sauvegarde le contexte.
 * \param __pthread_ptr pointeur sur la structure pthread_t du pthread.
 *
 * \note voir les fonctions _sys_vfork(), _sys_krnl_exec(), _sys_exec().
 * \hideinitializer
 */
   #define __bckup_context(context,__pthread_ptr)

/**
 * permet de restaurer le contexte context dans celui du thread __pthread_ptr.
 *
 * \param context variable de type context_t dans laquelle est plac le contexte  restaurer.
 * \param __pthread_ptr pointeur sur la structure pthread_t du pthread.
 *
 * \note voir les fonctions _sys_vfork(), _sys_vfork_exit(), _sys_krnl_exec(), _sys_exec().
 * \hideinitializer
 */
   #define __rstr_context(context,__pthread_ptr)

/**
 * permet de sauvegarder la pile d'un process
 *
 * \param pid pid du process dont il faut sauvegarder la pile.
 *
 * \note voir les fonctions _sys_vfork(), _sys_krnl_exec(), _sys_exec().
 * \hideinitializer
 */
   #define __bckup_stack(pid)

/**
 * permet de restaurer la pile d'un process
 *
 * \param pid pid du process dont il faut restaurer la pile.
 *
 * \note voir les fonctions _sys_vfork(), _sys_krnl_exec(), _sys_exec().
 * \hideinitializer
 */
   #define __rstr_stack(pid)

/**
 * permet de drouter le flux d'excution d'un process par la fonction _sys_kill().
 *
 * \param pid pid du process dont le flux d'excution doit tre drouter.
 * \param sig_handler address de la fonction sighandler() (voir kernel/signal.c)
 *
 * \note le droutage du flux d'excution est obtenue en modifiant l'addresse de retour d'interruption du scheduler.
 * cette addresse de retour est gnralement place dans la structure qui permet de sauvegarder le contexte context_t de la tche
 * lors de l'appel de l'ordonanceur par l'interuption du timer qui contrle la premption (le tick).
 * cette structure de contexte context_t depend du micro-noyau utilis.
 *
 * \hideinitializer
 */
   #define __swap_signal_handler(pid,sig_handler)

/**
 * permet de restaurer, aprs le droutement par __swap_signal_handler(), le flux d'excution d'un process.
 *
 * \param pid pid du process dont le flux d'excution doit tre restaurer.
 */
   #define __exit_signal_handler(pid)

/**
 * permet de rveiller un process.
 *
 * \param pid pid du process que l'on doit rveiller.
 * \note voir _kernel_timer() dans kernel/kernel.c
 */
   #define __set_active_pid(pid)

/**
 * dbut du zone de code atomique (non premptible)
 *
 * \hideinitializer
 */
   #define __atomic_in() OS_EnterRegion(); //stop task switching and the scheduler, kernel timeslice must be set to 0 cooprative mode).

/**
 * dbut du zone de code atomique (non premptible)
 *
 * \hideinitializer
 */
   #define __atomic_out() OS_LeaveRegion(); //restart task switching and scheduler.

/**
 * arrte le timer qui appel rgulirement l'ordonanceur.
 *
 * \note voir _syscall_execve(), _syscall_vfork(), _syscall_kill(), _syscall_exit(), _syscall_atexit(), _syscall_sigexit().
 */
   #define __stop_sched()

/**
 *  redmarre le timer qui appel rgulirement l'ordonanceur.
 *
 * \note voir _syscall_execve(), _syscall_vfork(), _syscall_kill(), _syscall_exit(), _syscall_atexit(), _syscall_sigexit().
 */
   #define __restart_sched()

#endif


#define __bckup_thread_start_context(__context__,__pthread_ptr__) __inline_bckup_thread_start_context(__context__,__pthread_ptr__)
#define __bckup_context(__context__,__pthread_ptr__)              __inline_bckup_context(__context__,__pthread_ptr__)
#define __rstr_context(__context__,__pthread_ptr__)               __inline_rstr_context(__context__,__pthread_ptr__)
#define __bckup_stack(__pthread_ptr__)                            __inline_bckup_stack(__pthread_ptr__)
#define __rstr_stack(__pthread_ptr__)                             __inline_rstr_stack(__pthread_ptr__)
#define __swap_signal_handler(__pthread_ptr__,sig_handler)        __inline_swap_signal_handler(__pthread_ptr__,sig_handler)
#define __exit_signal_handler(__pthread_ptr__)                    __inline_exit_signal_handler(__pthread_ptr__)


/** @} */
/** @} */

#endif
