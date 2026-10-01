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


//Configuration du noyau dépendant de l'ISA : ARMv6-M (Cortex-M0/M0+).
//Inclus par kernel/core/kernelconf.h (chemin d'inclusion LEPTON_KAL_ARCH_DIR) ; valeurs reprises
//des branches __tauon_cpu_core__ de kernelconf.h (KAL-2, 2026-10-01).
//HYPOTHÈSE À VALIDER : valeurs M0 de la configuration IAR, jamais compilées sous GCC ; non
//compilé avant l'étape 6 (kal/arch/armv6m/kal_arch.h à créer, aucun preset armv6m).
#ifndef _KAL_ARCH_ARMV6M_CONF_H
#define _KAL_ARCH_ARMV6M_CONF_H

#define __KERNEL_COMPILER_SUPPORT_TYPE __KERNEL_COMPILER_SUPPORT_64_BITS_TYPE
#define __KERNEL_CPU_ARCH CPU_ARCH_32
//profil : celui de kernel_mkconf.h (mklepton), sinon minimal (repli de kernelconf.h)
//ni signaux temps réel (__KERNEL_POSIX_REALTIME_SIGNALS) ni verrous de fichiers sur Cortex-M0

#endif //_KAL_ARCH_ARMV6M_CONF_H
