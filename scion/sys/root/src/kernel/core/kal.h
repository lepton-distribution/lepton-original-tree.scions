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
   #include "kernel/core/kal/arch/host/kal_arch.h"
   #include "kernel/core/kal/backend/static/kal_backend.h"
#elif defined (__KERNEL_UCORE_EMBOS)\
 &&((__tauon_cpu_core__ == __tauon_cpu_core_arm_arm7tdmi__)\
 || (__tauon_cpu_core__ == __tauon_cpu_core_arm_arm926ejs__)\
 || (__tauon_cpu_core__ == __tauon_cpu_core_arm_cortexM3__)\
 || (__tauon_cpu_core__ == __tauon_cpu_core_arm_cortexM4__)\
 || (__tauon_cpu_core__ == __tauon_cpu_core_arm_cortexM7__))
   #include "kernel/core/kal/arch/armv7m/kal_arch.h"
   #include "kernel/core/kal/backend/embos/kal_backend.h"
#elif ((__tauon_compiler__ == __compiler_keil_arm__) || (__tauon_compiler__ == __compiler_gnuc__)) && defined(__KERNEL_UCORE_FREERTOS) && ((__tauon_cpu_core__ == __tauon_cpu_core_arm_arm7tdmi__) || (__tauon_cpu_core__ == __tauon_cpu_core_arm_arm926ejs__) || (__tauon_cpu_core__ == __tauon_cpu_core_arm_cortexM0__) || (__tauon_cpu_core__ == __tauon_cpu_core_arm_cortexM3__) || (__tauon_cpu_core__ == __tauon_cpu_core_arm_cortexM4__) || (__tauon_cpu_core__ == __tauon_cpu_core_arm_cortexM7__))
   #include "kernel/core/kal/backend/freertos/kal_backend.h"
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
