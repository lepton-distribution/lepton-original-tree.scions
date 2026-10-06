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


//KAL, axe micro-noyau : FreeRTOS (noyau V11.3.0, ucore/freeRTOS_11-3-0), étape 7.
//Sélectionné par cmake/kal/freertos.cmake ; s'appuie sur kal_arch.h (inclus avant lui par
//kal.h) et sur la trame du port, décrite par ISA (kal/backend/freertos/arch/<isa>).
//Écarts et accès internes : doc/migration/kal-freertos-ecarts.md. Accès internes limités à
//I1 (pxTopOfStack, premier membre du TCB, lu par le port) et I2 (trame du port) ; la structure
//privée tskTCB n'est plus recopiée (ancien kal_freertos.h, FreeRTOS 8.0.0).
#ifndef _KAL_BACKEND_FREERTOS_H
#define _KAL_BACKEND_FREERTOS_H
   #include <stdint.h>
   #include <string.h>
   #include <stdlib.h>
   #include "FreeRTOS.h"
   #include "task.h"
   #include "semphr.h"
   #include "timers.h"
   #include "event_groups.h"
   #include "kal_freertos_frame.h"

   //TCB fourni par Lepton (xTaskCreateStatic), alloué à part comme OS_TASK pour embOS (le noyau
   //copie des kernel_pthread_t entiers : vfork, exec) : tâche en tête (handle de tâche = adresse
   //du TCB, pxTopOfStack en premier mot), puis groupe d'événements des appels système.
   typedef struct {
      StaticTask_t task;
      StaticEventGroup_t events;
   }freertos_tcb_t; //nom utilisé par kernel_pthread.h
   typedef freertos_tcb_t tcb_t;
   typedef void (*_pthreadstart_routine_t)(void);
   typedef _pthreadstart_routine_t pthreadstart_routine_t;
   typedef int thr_id_t;

   #define __begin_pthread(pthread_name) \
   void pthread_name(void){

   #define __end_pthread() \
   return; }

   #define __is_thread_self(__tcb__) \
   ((TaskHandle_t)(__tcb__) == xTaskGetCurrentTaskHandle())

   #define _macro_stack_addr

   //Priorités : Lepton 0-255 et FreeRTOS vont dans le même sens (plus grand = plus prioritaire,
   //comme embOS) ; projection monotone sur [1, configMAX_PRIORITIES-2] : 0 reste à la tâche
   //idle, configMAX_PRIORITIES-1 à la tâche de service des temporisateurs (dont les rappels
   //s'exécutent dans le contexte du tick sous embOS). 150 (noyau) -> 18, 100 -> 12 sur 32 niveaux.
   #define __kal_priority(__prio__) \
      ((UBaseType_t)(1u + (((unsigned int)(__prio__) & 0xFFu) * (unsigned int)(configMAX_PRIORITIES-2)) / 256u))

   //I1 : pointeur de pile sauvegardé de la tâche (pxTopOfStack).
   #define __kal_frt_sp(__tcb__) (*(uint32_t**)(__tcb__))

   //Contexte : pile sauvegardée et copie de la trame du port (pas de copie du TCB : listes,
   //priorité, notifications et mutex détenus restent ceux de FreeRTOS, comme embOS préserve
   //pPrev/pNext).
   typedef struct {
      uint32_t* sp;
      uint32_t  os_regs[__KAL_FRT_FRAME_MAX_WORDS];
   }context_t;

   #define __inline_bckup_thread_start_context(__context__,__pthread_ptr__){ \
      (__context__).sp = __kal_frt_sp((__pthread_ptr__)->tcb); \
      memcpy((__context__).os_regs,(__context__).sp,__kal_frt_frame_words((__context__).sp)*sizeof(uint32_t)); \
   }

   #define __inline_bckup_context(__context__,__pthread_ptr__){ \
      (__context__).sp = __kal_frt_sp((__pthread_ptr__)->tcb); \
      memcpy((__context__).os_regs,(__context__).sp,__kal_frt_frame_words((__context__).sp)*sizeof(uint32_t)); \
   }

   #define __inline_rstr_context(__context__,__pthread_ptr__){ \
      __kal_frt_sp((__pthread_ptr__)->tcb) = (__context__).sp; \
      memcpy((__context__).sp,(__context__).os_regs,__kal_frt_frame_words((__context__).os_regs)*sizeof(uint32_t)); \
   }

   //Use Dynamic Allocation!!!
   #define __inline_bckup_stack(__pthread_ptr__){ \
      int __stack_size__; \
      void* __src_stack_ptr__; \
      __stack_size__ = ((int)(__pthread_ptr__->bckup_context.sp) - (int)(__pthread_ptr__->start_context.sp));\
      __src_stack_ptr__ = (void*)(((uint8_t*)__pthread_ptr__->start_context.sp)+__stack_size__);\
      __pthread_ptr__->bckup_stack = (char*)_sys_malloc( abs(__stack_size__) ); \
      if(!__pthread_ptr__->bckup_stack) \
         return -ENOMEM; \
      memcpy(__pthread_ptr__->bckup_stack,__src_stack_ptr__,abs(__stack_size__)); \
   }

   #define __inline_rstr_stack(__pthread_ptr__){ \
      int __stack_size__; \
      void* __src_stack_ptr__; \
      __stack_size__ = ((int)(__pthread_ptr__->bckup_context.sp)- (int)(__pthread_ptr__->start_context.sp));\
      __src_stack_ptr__ = (void*)(((uint8_t*)__pthread_ptr__->start_context.sp)+__stack_size__);\
      memcpy(__src_stack_ptr__,__pthread_ptr__->bckup_stack,abs(__stack_size__)); \
      _sys_free(__pthread_ptr__->bckup_stack); \
   }

   //Réveil d'une tâche, équivalent d'OS_MakeTaskReady : bloquée (attente finie ou infinie,
   //sémaphore, groupe d'événements, délai) -> xTaskAbortDelay ; suspendue -> vTaskResume.
   //HYPOTHÈSE À VALIDER (banc KAL T2/T3) : xTaskAbortDelay sur une attente infinie
   //(eTaskGetState rend eBlocked pour une tâche en attente d'événement, V10.x et suivantes).
   #define __kal_frt_make_ready(__tcb__){ \
      switch(eTaskGetState((TaskHandle_t)(__tcb__))) { \
      case eBlocked:   xTaskAbortDelay((TaskHandle_t)(__tcb__)); break; \
      case eSuspended: vTaskResume((TaskHandle_t)(__tcb__)); break; \
      default: break; \
      } \
   }

   //Déroutement vers le gestionnaire de signal : PC du cadre matériel (étendu si FPU, comme E3),
   //état ICI/IT du xPSR effacé (kal_arch.h, test TICI), puis réveil.
   #define __inline_swap_signal_handler(__pthread_ptr__,__sig_handler__){ \
      uint32_t* __hw__ = __kal_frt_hw(__kal_frt_sp((__pthread_ptr__)->tcb)); \
      __hw__[__KAL_FRT_HW_PC]   = (uint32_t)(__sig_handler__); \
      __hw__[__KAL_FRT_HW_XPSR] = __kal_arch_redirect_xpsr(__hw__[__KAL_FRT_HW_XPSR]); \
      __kal_frt_make_ready((__pthread_ptr__)->tcb); \
   }

   /*TS_WAIT_TIME*/
   #define __inline_exit_signal_handler(__pthread_ptr__){ \
      __rstr_context(__pthread_ptr__->bckup_context,__pthread_ptr__); \
   }

   #define __set_active_pthread(__pthread_ptr__) \
      if(__pthread_ptr__) __kal_frt_make_ready((__pthread_ptr__)->tcb)

   //Attente bloquante depuis une région atomique : la région est quittée pendant l'attente et
   //reprise au réveil, comme sous embOS (kal_freertos.c). Toute attente bloquante du backend
   //(sémaphores, mutex, événements, délais, sys_arch lwIP) passe par __kal_frt_block.
   UBaseType_t kal_freertos_region_leave(void);
   void kal_freertos_region_enter(UBaseType_t n);
   #define __kal_frt_block(__stmt__) do { \
      UBaseType_t __kal_region__ = kal_freertos_region_leave(); \
      __stmt__; \
      kal_freertos_region_enter(__kal_region__); \
   } while(0)

   //stop task switching (ordonnanceur suspendu, interruptions actives), comme OS_EnterRegion.
   #define __atomic_in() vTaskSuspendAll()
   //restart task switching
   #define __atomic_out() xTaskResumeAll()

   //__stop_sched / __restart_sched : SysTick, kal_arch.h (commun aux micro-noyaux).

   //uninterruptible section in (BASEPRI = configMAX_SYSCALL_INTERRUPT_PRIORITY en ARMv7-M)
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
