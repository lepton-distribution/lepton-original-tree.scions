/*
The contents of this file are subject to the Mozilla Public License Version 1.1
(the "License"); you may not use this file except in compliance with the License.
You may obtain a copy of the License at http://www.mozilla.org/MPL/

Software distributed under the License is distributed on an "AS IS" basis,
WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License for the
specific language governing rights and limitations under the License.

The Original Code is Lepton.

Alternatively, the contents of this file may be used under the terms of the eCos GPL license
(the  [eCos GPL] License), in which case the provisions of [eCos GPL] License are applicable
instead of those above. If you do not delete the provisions above, a recipient may use your
version of this file under either the MPL or the [eCos GPL] License."
*/

//KAL, axe micro-noyau FreeRTOS × ISA ARMv6-M (étape 7, module 7.3) : trame de contexte sauvegardée
//par le port FreeRTOS (accès interne I2, kal-freertos-ecarts.md §4), vue depuis pxTopOfStack (I1).
//Sélectionné par cmake/kal/freertos.cmake (kal/backend/freertos/arch/<isa>).
//
//Port ARM_CM0 de FreeRTOS V11.3.0, sans MPU (configENABLE_MPU = 0) — PendSV_Handler de portasm.c :
//   [EXC_RETURN, R4-R7, R8-R11] [R0-R3, R12, LR, PC, xPSR]
//pas de FPU en ARMv6-M. Le mot de bourrage d'alignement (xPSR bit 9) est au-dessus du cadre :
//non copié.
#ifndef _KAL_FREERTOS_FRAME_H
#define _KAL_FREERTOS_FRAME_H

#include <stdint.h>

#define __KAL_FRT_HW_WORDS          8u    //R0-R3, R12, LR, PC, xPSR
#define __KAL_FRT_HW_PC             6u
#define __KAL_FRT_HW_XPSR           7u

#define __KAL_FRT_SW_WORDS          9u    //EXC_RETURN, R4-R7, R8-R11
#define __KAL_FRT_SW_EXC_RETURN     0u
#define __KAL_FRT_FPU_HI_WORDS      0u
#define __KAL_FRT_FPU_LO_WORDS      0u
#define __kal_frt_is_fpu(__sp__)    (0)

#define __KAL_FRT_FRAME_MAX_WORDS   (__KAL_FRT_SW_WORDS+__KAL_FRT_HW_WORDS)

//cadre matériel (R0) d'une trame sauvegardée commençant à __sp__
#define __kal_frt_hw(__sp__)        (((uint32_t*)(__sp__))+__KAL_FRT_SW_WORDS)

//taille en mots de la trame sauvegardée commençant à __sp__
#define __kal_frt_frame_words(__sp__) ((void)(__sp__), __KAL_FRT_FRAME_MAX_WORDS)

#endif //_KAL_FREERTOS_FRAME_H
