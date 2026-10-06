/*
The contents of this file are subject to the Mozilla Public License Version 1.1
(the "License"); you may not use this file except in compliance with the License.
You may obtain a copy of the License at http://www.mozilla.org/MPL/

Software distributed under the License is distributed on an "AS IS" basis,
WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License for the
specific language governing rights and limitations under the License.

The Original Code is Lepton.

The Initial Developer of the Original Code is Philippe Le Boulanger.
Portions created by Philippe Le Boulanger are Copyright (C) 2011 <lepton.phlb@gmail.com>.
All Rights Reserved.

Contributor(s): Jean-Jacques Pitrolle <lepton.jjp@gmail.com>.

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
| Includes
==============================================*/
#include <stdint.h>

#include "kernel/core/errno.h"
#include "kernel/core/kernel_pthread.h"
#include "kernel/core/kernel_sem.h"

#include "kernel/core/time.h"
#include "kernel/core/kernel_clock.h"
#include "kernel/core/interrupt.h"   //kernel_in_interrupt (FreeRTOS, contexte d'interruption)


/*============================================
| Global Declaration
==============================================*/


/*============================================
| Implementation
==============================================*/

/*--------------------------------------------
| Name:        kernel_sem_init
| Description:
| Parameters:  none
| Return Type: none
| Comments:
| See:
----------------------------------------------*/
int kernel_sem_init(kernel_sem_t* kernel_sem, int pshared, unsigned int value){
   if(!kernel_sem)
      return -1;
#ifdef __KERNEL_UCORE_FREERTOS
   kernel_sem->xHigherPriorityTaskWoken = pdFALSE;
   kernel_sem->sem = xSemaphoreCreateCountingStatic( (UBaseType_t)(-1), (UBaseType_t)value, &kernel_sem->sem_static);
   if(kernel_sem->sem==(SemaphoreHandle_t)0)
      return -1;
#endif
   return 0;
}

/*--------------------------------------------
| Name:        kernel_sem_destroy
| Description:
| Parameters:  none
| Return Type: none
| Comments:
| See:
----------------------------------------------*/
int kernel_sem_destroy(kernel_sem_t* kernel_sem){
   if(!kernel_sem)
      return -1;
#ifdef __KERNEL_UCORE_FREERTOS
   vSemaphoreDelete(kernel_sem->sem);
#endif
   return 0;
}

/*--------------------------------------------
| Name:        kernel_sem_getvalue
| Description:
| Parameters:  none
| Return Type: none
| Comments:
| See:
----------------------------------------------*/
int kernel_sem_getvalue(kernel_sem_t* kernel_sem, int *value){
   if(!kernel_sem)
      return -1;
#ifdef __KERNEL_UCORE_FREERTOS
   *value = (int)uxSemaphoreGetCount(kernel_sem->sem);
#endif
   return 0;
}

/*--------------------------------------------
| Name:        kernel_sem_post
| Description:
| Parameters:  none
| Return Type: none
| Comments:
| See:
----------------------------------------------*/
int kernel_sem_post(kernel_sem_t* kernel_sem){
   if(!kernel_sem)
      return -1;
#ifdef __KERNEL_UCORE_FREERTOS
   if(!kernel_in_interrupt) {
      xSemaphoreGive(kernel_sem->sem);
   }else if(kernel_in_interrupt>0) {
      //etape 7 : xSemaphoreGiveFromISR ne remet jamais l'indicateur a pdFALSE ; il est donc
      //initialise a chaque appel, puis cumule dans celui de l'interruption (interrupt.h).
      kernel_sem->xHigherPriorityTaskWoken = pdFALSE;
      xSemaphoreGiveFromISR(kernel_sem->sem,&kernel_sem->xHigherPriorityTaskWoken);
      if(kernel_sem->xHigherPriorityTaskWoken!=pdFALSE)
         kernel_in_interrupt_higher_priority_task_woken = pdTRUE;
   }else{
     return -1;
   }
#endif
   return 0;
}

/*--------------------------------------------
| Name:        kernel_sem_timedwait
| Description:
| Parameters:  none
| Return Type: none
| Comments:
| See:
----------------------------------------------*/
int kernel_sem_timedwait(kernel_sem_t* kernel_sem, int flag, const struct timespec * abs_timeout){

   int timeout=0;
   int ret=0;
   UBaseType_t region;

   //
   if(!kernel_sem)
      return -1;
   //etape 7 : region atomique quittee pendant l'attente (kal_freertos.c), avant de masquer
   //les interruptions (xTaskResumeAll les demasque).
   region = kal_freertos_region_leave();
   //
   if(flag==TIMER_ABSTIME && abs_timeout) { //warning: on 16bit architecture use ldiv instead '/' for division with long type.
     //
     //comme OS_DI() d'embOS : interruptions masquees jusqu'a l'attente (taskEXIT_CRITICAL de
     //xSemaphoreTake les demasque, imbrication critique a 0 hors section critique).
     taskDISABLE_INTERRUPTS();
     //
     timeout = kernel_clock_timeout(CLOCK_REALTIME,abs_timeout);
   } if(!flag && abs_timeout) {
      timeout = __time_s_to_ms(abs_timeout->tv_sec)+__time_ns_to_ms(abs_timeout->tv_nsec);
   }
   //
#ifdef __KERNEL_UCORE_FREERTOS
   if(abs_timeout && timeout) {
      if(xSemaphoreTake(kernel_sem->sem, pdMS_TO_TICKS(timeout))!=pdPASS)
         ret = -1;
   }else if(abs_timeout && !timeout) {
      if(xSemaphoreTake(kernel_sem->sem, (TickType_t)0)!=pdPASS)   //try to get sem
         ret = -EBUSY;
   }else{
      while(xSemaphoreTake(kernel_sem->sem, portMAX_DELAY)!=pdPASS);
   }

#endif
   kal_freertos_region_enter(region);
   return ret;
}

/*--------------------------------------------
| Name:        kernel_sem_trywait*
| Description:
| Parameters:  none
| Return Type: none
| Comments:
| See:
----------------------------------------------*/
int kernel_sem_trywait(kernel_sem_t* kernel_sem){
   if(!kernel_sem)
      return -1;
#ifdef __KERNEL_UCORE_FREERTOS
   if(xSemaphoreTake(kernel_sem->sem, (TickType_t)0)!=pdPASS)
      return -EBUSY;
#endif
   return 0;
}

/*--------------------------------------------
| Name:        kernel_sem_wait
| Description:
| Parameters:  none
| Return Type: none
| Comments:
| See:
----------------------------------------------*/
int kernel_sem_wait(kernel_sem_t* kernel_sem){
   if(!kernel_sem)
      return -1;
#ifdef __KERNEL_UCORE_FREERTOS
   __kal_frt_block(while(xSemaphoreTake(kernel_sem->sem, portMAX_DELAY)!=pdPASS));
#endif
   return 0;
}

//
#if 0

// the following function is not used at kernel level

/*--------------------------------------------
| Name:        kernel_sem_open
| Description:
| Parameters:  none
| Return Type: none
| Comments: named sem not used at kernel level
| See:
----------------------------------------------*/
sem_t *kernel_sem_open(const char * name, , ...){
   return (sem_t*)0;
}

/*--------------------------------------------
| Name:        kernel_sem_close
| Description:
| Parameters:  none
| Return Type: none
| Comments: named sem not used at kernel level
| See:
----------------------------------------------*/
int kernel_sem_close(kernel_sem_t* kernel_sem){
   return 0;
}

/*--------------------------------------------
| Name:        kernel_sem_unlink
| Description:
| Parameters:  none
| Return Type: none
| Comments: named sem not used at kernel level
| See:
----------------------------------------------*/
int kernel_sem_unlink(const char*){
   return 0;
}
#endif
/*============================================
| End of Source  : kernel_sem.c
==============================================*/
