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


//KAL, axe micro-noyau : embOS (Segger), TCB et trame de pile embOS.
//Extrait de kal.h (kal_split.py, étape extraire-embos) ; sélectionné par cmake/kal/embos.cmake.
//S'appuie sur kal_arch.h (inclus avant lui par kal.h).
#ifndef _KAL_BACKEND_EMBOS_H
#define _KAL_BACKEND_EMBOS_H


   #include "RTOS.h"
   //OS_MakeTaskReady : exportee par la bibliotheque embOS, non declaree par RTOS.h (ecart E4).
   //HYPOTHESE A VALIDER : signature void OS_MakeTaskReady(OS_TASK*) deduite des usages Lepton
   //(desassemblage interdit par la licence SFL ; a confirmer aupres de Segger).
   void OS_MakeTaskReady(OS_TASK* pTask);
/*modif for segger version 3.28h */
//#include "OSKern.H"
   //GD-TODO lm3S: improve? 
/*modif for segger version 3.52e */
//#include "OSint.h"

//must be set to 1 for segger version 3.28n and set to 0 for 3.32e
   #if 0
//#include "OSint.h"
   #endif

   #include "stdlib.h"




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

//Cadre de pile ARMv7-M (M3, M4, M7) : seul cas du backend embOS (kal/arch/armv7m) ; ARMv6-M
//(M0/M0+) à l'étape 6.
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






//uninterruptible section in
   #define __disable_interrupt_section_in() OS_IncDI()
//uninterruptible section out
   #define __disable_interrupt_section_out() OS_DecRI()

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


#endif //_KAL_BACKEND_EMBOS_H
