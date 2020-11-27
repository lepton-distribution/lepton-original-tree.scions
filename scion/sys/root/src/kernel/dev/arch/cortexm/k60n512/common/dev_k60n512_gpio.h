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
#ifndef __DEV_CORTEXM_K60N512_GPIO_H__
#define __DEV_CORTEXM_K60N512_GPIO_H__


/*============================================
| Includes
==============================================*/

/*============================================
| Declaration
==============================================*/
#define REG_GPIO_PDOR      (0x0) //Port Data Output Register
#define REG_GPIO_PSOR      (0x4) //Port Set Output Register
#define REG_GPIO_PCOR      (0x8) //Port Clear Output Register
#define REG_GPIO_PTOR      (0xc) //Port Toggle Output Register
#define REG_GPIO_PDIR      (0x10) //Port Data Input Register
#define REG_GPIO_PDDR      (0x14) //Port Data Direction Register

#endif //__DEV_CORTEXM_K60N512_GPIO_H__
