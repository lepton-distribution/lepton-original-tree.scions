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


//KAL, axe ISA : ARMv7-M (Cortex-M3, M4, M7), indépendant du micro-noyau.
//Extrait de kal.h (kal_split.py, étape extraire-embos) ; sélectionné par cmake/isa/armv7m.cmake.
#ifndef _KAL_ARCH_ARMV7M_H
#define _KAL_ARCH_ARMV7M_H

   #define __va_list_copy(__dest_va_list__,__src_va_list__) memcpy(&__dest_va_list__,&__src_va_list__,sizeof(__dest_va_list__))

   //SysTick (CTRL.TICKINT, bit 1), commun à tous les cœurs ARMv7-M : coupe / relance l'IT du tick
   //de l'ordonnanceur. Type uint32_t (et non OS_U32 d'embOS) : indépendant du micro-noyau.
   #define __LEPTON_KAL_PIT_BASE    (0xE000E010)
   #define __LEPTON_KAL_PIT_MR      (*(volatile uint32_t*)(__LEPTON_KAL_PIT_BASE + 0x00))
   #define __stop_sched() __LEPTON_KAL_PIT_MR &= ~(1uL << (1));
   #define __restart_sched() __LEPTON_KAL_PIT_MR |= (1uL << (1));

#endif //_KAL_ARCH_ARMV7M_H
