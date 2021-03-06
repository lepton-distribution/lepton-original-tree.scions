/*********************************************************************
*                     SEGGER Microcontroller GmbH                    *
*                        The Embedded Experts                        *
**********************************************************************
*                                                                    *
*       (c) 1995 - 2018 SEGGER Microcontroller GmbH                  *
*                                                                    *
*       Internet: segger.com  Support: support_embos@segger.com      *
*                                                                    *
**********************************************************************
*                                                                    *
*       embOS * Real time operating system for microcontrollers      *
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
*       OS version: 4.40                                             *
*                                                                    *
**********************************************************************

----------------------------------------------------------------------
File    : xmtx.c
Purpose : xmtx system interface -- thread locking and unlocking
          functions, adapted to embOS
--------- END-OF-HEADER ----------------------------------------------
*/

#include <yvals.h>
#include "RTOS.h"
#include "DLib_Threads.h"

#if _MULTI_THREAD  // Used in multi thread supported libraries only

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
  OS__iar_system_Mtxinit(m);
}

/*********************************************************************
*
*       __iar_system_Mtxdst()
*/
__ATTRIBUTES void __iar_system_Mtxdst(__iar_Rmtx* m) {
  OS__iar_system_Mtxdst(m);
}

/*********************************************************************
*
*       __iar_system_Mtxlock()
*/
__ATTRIBUTES void __iar_system_Mtxlock(__iar_Rmtx* m) {
  OS__iar_system_Mtxlock(m);
}

/*********************************************************************
*
*       __iar_system_Mtxunlock()
*/
__ATTRIBUTES void __iar_system_Mtxunlock(__iar_Rmtx* m) {
  OS__iar_system_Mtxunlock(m);
}

#endif // _MULTI_THREAD

/****** End Of File *************************************************/
