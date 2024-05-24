/*********************************************************************
*                     SEGGER Microcontroller GmbH                    *
*                        The Embedded Experts                        *
**********************************************************************
*                                                                    *
*       (c) 1995 - 2023 SEGGER Microcontroller GmbH                  *
*                                                                    *
*       Internet: segger.com  Support: support_embos@segger.com      *
*                                                                    *
**********************************************************************
*                                                                    *
*       embOS * Real time operating system                           *
*                                                                    *
*       Please note:                                                 *
*                                                                    *
*       Knowledge of this file may under no circumstances            *
*       be used to write a similar product or a real-time            *
*       operating system for in-house use.                           *
*                                                                    *
*       Thank you for your fairness !                                *
*                                                                    *
**********************************************************************
*                                                                    *
*       OS version: V5.18.3.1                                        *
*                                                                    *
**********************************************************************

-------------------------- END-OF-HEADER -----------------------------
File    : OS_ThreadSafe.c
Purpose : Thread safe library functions for IAR Embedded Workbench

Additional information:
  This module enables thread and/or interrupt safety for library functions.
  Per default it ensures thread and interrupt safety by disabling/restoring
  embOS interrupts. Zero latency interrupts are not affected and protected.
  If you need to call e.g. malloc() also from within a zero latency interrupt
  additional handling needs to be added.
  If you don't call such functions from within embOS interrupts you can use
  thread safety instead. This reduces the interrupt latency because a mutex
  is used instead of disabling embOS interrupts.
*/

#include <yvals.h>
#include "RTOS.h"

#if (_MULTI_THREAD != 0)

/*********************************************************************
*
*       Defines
*
**********************************************************************
*/
//
// When set to 1 thread and interrupt safety is guaranteed.
//
// When set to 0 only thread safety is guaranteed. In this case you
// must not call e.g. heap functions from ISRs, software timers or OS_Idle().
//
#ifndef   OS_INTERRUPT_SAFE
  #define OS_INTERRUPT_SAFE  1
#endif

/*********************************************************************
*
*       Global functions
*
**********************************************************************
*/

/*********************************************************************
*
*       __iar_system_Mtxinit()
*/
__ATTRIBUTES void __iar_system_Mtxinit(__iar_Rmtx* m) {
  OS_USE_PARA(m);
}

/*********************************************************************
*
*       __iar_system_Mtxdst()
*/
__ATTRIBUTES void __iar_system_Mtxdst(__iar_Rmtx* m) {
  OS_USE_PARA(m);
}

/*********************************************************************
*
*       __iar_system_Mtxlock()
*/
__ATTRIBUTES void __iar_system_Mtxlock(__iar_Rmtx* m) {
  OS_USE_PARA(m);
#if (OS_INTERRUPT_SAFE == 1)
  OS_InterruptSafe_Lock();
#else
  OS_ThreadSafe_Lock();
#endif
}

/*********************************************************************
*
*       __iar_system_Mtxunlock()
*/
__ATTRIBUTES void __iar_system_Mtxunlock(__iar_Rmtx* m) {
  OS_USE_PARA(m);
#if (OS_INTERRUPT_SAFE == 1)
  OS_InterruptSafe_Unlock();
#else
  OS_ThreadSafe_Unlock();
#endif
}

#if (_DLIB_FILE_DESCRIPTOR != 0)
/*********************************************************************
*
*       __iar_file_Mtxinit()
*/
__ATTRIBUTES void __iar_file_Mtxinit(__iar_Rmtx* m) {
  OS_USE_PARA(m);
}

/*********************************************************************
*
*       __iar_file_Mtxdst()
*/
__ATTRIBUTES void __iar_file_Mtxdst(__iar_Rmtx* m) {
  OS_USE_PARA(m);
}

/*********************************************************************
*
*       __iar_file_Mtxlock()
*/
__ATTRIBUTES void __iar_file_Mtxlock(__iar_Rmtx* m) {
  OS_USE_PARA(m);
#if (OS_INTERRUPT_SAFE == 1)
  OS_InterruptSafe_Lock();
#else
  OS_ThreadSafe_Lock();
#endif
}

/*********************************************************************
*
*       __iar_file_Mtxunlock()
*/
__ATTRIBUTES void __iar_file_Mtxunlock(__iar_Rmtx* m) {
  OS_USE_PARA(m);
#if (OS_INTERRUPT_SAFE == 1)
  OS_InterruptSafe_Unlock();
#else
  OS_ThreadSafe_Unlock();
#endif
}
#endif  // (_DLIB_FILE_DESCRIPTOR != 0)

#if (defined(__ICCARM__)   && (__ICCARM__   != 0) && (__VER__ >= 8000000)) \
 || (defined(__ICCRX__)    && (__ICCRX__    != 0) && (__VER__ >= 400))     \
 || (defined(__ICCRL78__)  && (__ICCRL78__  != 0) && (__VER__ >= 400))     \
 || (defined(__ICCRH850__) && (__ICCRH850__ != 0) && (__VER__ >= 200))     \
 || (defined(__ICCRISCV__) && (__ICCRISCV__ != 0) && (__VER__ >= 300))
/*********************************************************************
*
*       __iar_Initdynamiclock()
*/
__ATTRIBUTES void __iar_Initdynamiclock(__iar_Rmtx* m) {
  OS_USE_PARA(m);
}

/*********************************************************************
*
*       __iar_Dstdynamiclock()
*/
__ATTRIBUTES void __iar_Dstdynamiclock(__iar_Rmtx* m) {
  OS_USE_PARA(m);
}

/*********************************************************************
*
*       __iar_Lockdynamiclock()
*/
__ATTRIBUTES void __iar_Lockdynamiclock(__iar_Rmtx* m) {
  OS_USE_PARA(m);
#if (OS_INTERRUPT_SAFE == 1)
  OS_InterruptSafe_Lock();
#else
  OS_ThreadSafe_Lock();
#endif
}

/*********************************************************************
*
*       __iar_Unlockdynamiclock()
*/
__ATTRIBUTES void __iar_Unlockdynamiclock(__iar_Rmtx* m) {
  OS_USE_PARA(m);
#if (OS_INTERRUPT_SAFE == 1)
  OS_InterruptSafe_Unlock();
#else
  OS_ThreadSafe_Unlock();
#endif
}
#endif  // IAR Embedded Workbench version checks

#endif  // (_MULTI_THREAD != 0)

/*************************** End of file ****************************/
