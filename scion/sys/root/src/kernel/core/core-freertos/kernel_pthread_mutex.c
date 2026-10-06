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
#include "kernel/core/kal.h"
#include "kernel/core/kernel.h"
#include "kernel/core/kernel_pthread.h"
#include "kernel/core/kernel_pthread_mutex.h"


/*============================================
| Global Declaration
==============================================*/


/*============================================
| Implementation
==============================================*/
/*-------------------------------------------
| Name:pthread_mutex_init
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
int   kernel_pthread_mutex_init(kernel_pthread_mutex_t *mutex, const pthread_mutexattr_t *attr){
   //attr not used. preserved POSIX compatibility
#ifdef __KERNEL_UCORE_FREERTOS
   //etape 7 : mutex recursif a proprietaire et heritage de priorite, comme OS_RSEMA d'embOS
   //(l'ancienne version utilisait un semaphore binaire : ni recursif ni proprietaire).
   mutex->mutex = xSemaphoreCreateRecursiveMutexStatic(&mutex->mutex_static);
   if(mutex->mutex==(SemaphoreHandle_t)0)
      return -1;
#endif
   return 0;
}

/*-------------------------------------------
| Name:pthread_mutex_destroy
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
int   kernel_pthread_mutex_destroy(kernel_pthread_mutex_t *mutex){
   //
   __atomic_in();
   //
#ifdef __KERNEL_UCORE_FREERTOS
   {
      TaskHandle_t holder = xSemaphoreGetMutexHolder(mutex->mutex);
      if(holder!=(TaskHandle_t)0) {
         if(holder!=xTaskGetCurrentTaskHandle()) {
            __atomic_out();
            return -1;    //mutex is not owned by this thread. error it cannot be destroyed;
         }
         //mutex is owned by this thread: release all recursive locks.
         while(xSemaphoreGiveRecursive(mutex->mutex)==pdPASS);
      }
   }
   vSemaphoreDelete(mutex->mutex);
#endif
   //
   __atomic_out();
   //
   return 0; //mutex is not owned by any thread. it could be destroyed.
}

/*--------------------------------------------
| Name:        kernel_pthread_mutex_owner_destroy
| Description:
| Parameters:
| Return Type:
| Comments:    FreeRTOS : seul le proprietaire peut rendre un mutex (configASSERT) ; non utilise
|              (appel en commentaire dans process.c), comme dans l'ancienne version.
| See:
----------------------------------------------*/
int   kernel_pthread_mutex_owner_destroy(kernel_pthread_t* thread_ptr,kernel_pthread_mutex_t *mutex){
   //not used
   return -1;
}

/*-------------------------------------------
| Name:pthread_mutex_lock
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
int   kernel_pthread_mutex_lock(kernel_pthread_mutex_t *mutex){
   if(__kernel_is_in_static_mode())
      return 0;
#ifdef __KERNEL_UCORE_FREERTOS
   __kal_frt_block(while(xSemaphoreTakeRecursive(mutex->mutex, portMAX_DELAY)!=pdPASS));
#endif
   return 0;
}

/*-------------------------------------------
| Name:pthread_mutex_trylock
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
int   kernel_pthread_mutex_trylock(kernel_pthread_mutex_t *mutex){
   if(__kernel_is_in_static_mode())
      return 0;
#ifdef __KERNEL_UCORE_FREERTOS
   if(xSemaphoreTakeRecursive(mutex->mutex, (TickType_t)0)!=pdPASS)
      return -EBUSY;
#endif
   return 0;
}

/*-------------------------------------------
| Name:pthread_mutex_unlock
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
int   kernel_pthread_mutex_unlock(kernel_pthread_mutex_t *mutex){
   if(__kernel_is_in_static_mode())
      return 0;
#ifdef __KERNEL_UCORE_FREERTOS
   xSemaphoreGiveRecursive(mutex->mutex);
#endif
   return 0;
}

/*============================================
| End of Source  : kernel_pthread_mutex.c
==============================================*/
