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
| Compiler Directive
==============================================*/
#ifndef __DEV_STM32F469I_EVAL_H__
#define __DEV_STM32F469I_EVAL_H__


/*============================================
| Includes
==============================================*/


/*============================================
| Declaration
==============================================*/
#define _GPIO_DEFAULT_SPEED   GPIO_Speed_50MHz

typedef enum
{
  GPIO_ID_TXD1,
  GPIO_ID_RXD1,
  GPIO_NB
} _GPIO_LIST;

#define GPIO_TXD1  (&Gpio_Descriptor[GPIO_ID_TXD1])
#define GPIO_RXD1  (&Gpio_Descriptor[GPIO_ID_RXD1])


/*******************************************************************************
* UART
*******************************************************************************/
//#define _UART_OS_SUPPORT

typedef enum
{
  UART_ID_1,
  UART_NB
} _UART_LIST;

#define UART_1  (&Uart_Descriptor[UART_ID_1])

#endif /* __DEV_STM32F469I_EVAL_H__ */