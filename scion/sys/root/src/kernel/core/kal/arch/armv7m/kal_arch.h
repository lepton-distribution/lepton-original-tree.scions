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

   //EXC_RETURN : bit 4 (FType) à 0 = cadre de pile étendu, contexte FPU sauvegardé (ARMv7-M avec
   //FPU : M4F, M7). Utilisé par le backend pour la taille et le PC du cadre sauvegardé (écart E3).
   #define __kal_arch_exc_return_fpu_frame(__exc_return__) (((__exc_return__) & 0x10u) == 0u)

   //xPSR d'un cadre sauvegardé dont le PC est redirigé (déroutement vers un gestionnaire de
   //signal) : état de reprise ICI/IT (EPSR, bits 26:25 et 15:10) effacé, bit T forcé. Une tâche
   //préemptée au milieu d'un LDM/STM/PUSH/POP ou d'un bloc IT garde cet état dans son cadre ; au
   //retour d'exception, le cœur le reprendrait sur la nouvelle instruction (UsageFault INVSTATE,
   //ou exécution conditionnelle du gestionnaire). Constaté sur carte à l'étape 5 (QEMU ne
   //modélise pas ICI) ; test TICI du banc KAL.
   #define __kal_arch_redirect_xpsr(__xpsr__) (((__xpsr__) & ~0x0600FC00u) | 0x01000000u)

#endif //_KAL_ARCH_ARMV7M_H
