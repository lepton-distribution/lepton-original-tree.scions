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


//KAL, axe micro-noyau : FreeRTOS. Branche de kal.h extraite telle quelle (kal_split.py),
//hors ARM7/ARM9 ; non compilée avant l'étape 7 (aucun cmake/kal/freertos.cmake).
//Ses #if de cœur (cpu_regs_t M0 / M3-M7, SysTick) sont à répartir dans kal/arch/ à
//l'étape 7 ; __va_list_copy y double celui de kal/arch/armv7m/kal_arch.h.
//Condition d'origine : ((__tauon_compiler__ == __compiler_keil_arm__) || (__tauon_compiler__ == __compiler_gnuc__)) && defined(__KERNEL_UCORE_FREERTOS) && ((__tauon_cpu_core__ == __tauon_cpu_core_arm_arm7tdmi__) || (__tauon_cpu_core__ == __tauon_cpu_core_arm_arm926ejs__) || (__tauon_cpu_core__ == __tauon_cpu_core_arm_cortexM0__) || (__tauon_cpu_core__ == __tauon_cpu_core_arm_cortexM3__) || (__tauon_cpu_core__ == __tauon_cpu_core_arm_cortexM4__) || (__tauon_cpu_core__ == __tauon_cpu_core_arm_cortexM7__))
#ifndef _KAL_BACKEND_FREERTOS_H
#define _KAL_BACKEND_FREERTOS_H


   //#include <stdlib.h>
   //#include <string.h>   

   #include "FreeRTOS.h"
   #include "task.h"
   #include "semphr.h"
   #include "timers.h"
   #include "event_groups.h"

   #include "kernel/core/ucore/freeRTOS_8-0-0/source/include/kal_freertos.h"





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


   //uninterruptible section in
   #define __disable_interrupt_section_in() taskENTER_CRITICAL()
   
   //uninterruptible section out
   #define __disable_interrupt_section_out() taskEXIT_CRITICAL()

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


#endif //_KAL_BACKEND_FREERTOS_H
