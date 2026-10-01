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


//KAL, axe micro-noyau : noyau statique sans ordonnanceur (mklepton).
//Extrait de kal.h (kal_split.py, étape extraire-static) ; sélectionné par cmake/kal/static.cmake.
#ifndef _KAL_BACKEND_STATIC_H
#define _KAL_BACKEND_STATIC_H



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

#endif //_KAL_BACKEND_STATIC_H
