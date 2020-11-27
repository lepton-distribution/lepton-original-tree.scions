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

/*===========================================
Includes
=============================================*/
#include <stdint.h>

#include "kernel/core/kernelconf.h"
#include "kernel/core/types.h"
#include "kernel/core/dirent.h"
#include "kernel/core/interrupt.h"
#include "kernel/core/kernel.h"
#include "kernel/core/system.h"
#include "kernel/core/fcntl.h"
#include "kernel/core/stat.h"
#include "kernel/core/ioctl.h"
#include "kernel/core/ioctl_hd.h"

#include "kernel/fs/vfs/vfstypes.h"

#include "kernel/dev/arch/cortexm/stm32f4xx/driverlib/stm32f4xx.h"
#include "kernel/dev/arch/cortexm/stm32f4xx/types.h"
#include "kernel/dev/arch/cortexm/stm32f4xx/gpio.h"


#include "kernel/dev/arch/all/i2c/nxp-nfc/pn7150/dev_nxp_nfc_pn7150_x.h"

/*===========================================
Global Declaration
=============================================*/
const char dev_discovery_f4_nfc_pn7150_name[]="pn7150\0";

int dev_discovery_f4_nfc_pn7150_load(void);
int dev_discovery_f4_nfc_pn7150_open(desc_t desc, int o_flag);


dev_map_t dev_discovery_f4_nfc_pn7150_map={
   dev_discovery_f4_nfc_pn7150_name,
   S_IFCHR,
   dev_discovery_f4_nfc_pn7150_load,
   dev_discovery_f4_nfc_pn7150_open,
   dev_nxp_nfc_pn7150_x_close,
   dev_nxp_nfc_pn7150_x_isset_read,
   dev_nxp_nfc_pn7150_x_isset_write,
   dev_nxp_nfc_pn7150_x_read,
   dev_nxp_nfc_pn7150_x_write,
   dev_nxp_nfc_pn7150_x_seek,
   dev_nxp_nfc_pn7150_x_ioctl
};


// uart 3 configuration. see in board device driver of current bsp
static dev_nxp_nfc_pn7150_info_t discovery_f4_nfc_pn7150={0}; 


/*NXP NFC PN7150 IRQ Pin definition */
#define NXP_NFC_PN7150_IRQ_PIN                       GPIO_Pin_5
#define NXP_NFC_PN7150_IRQ_GPIO                      GPIOE

/*NXP NFC PN7150 VEN Pin definition */
#define NXP_NFC_PN7150_VEN_PIN                       GPIO_Pin_8
#define NXP_NFC_PN7150_VEN_GPIO                      GPIOA

/*===========================================
Implementation
=============================================*/
static int pn7150_previous_level=0;
#if 0
void EXTI9_5_IRQHandler(void){
    static int pn7150_current_level;
    
    /* Make sure that interrupt flag is set */
    if (EXTI_GetITStatus(EXTI_Line5) != RESET) {
        /* Do your stuff when PC6 is changed */
         if((pn7150_current_level=GPIO_ReadInputDataBit(NXP_NFC_PN7150_IRQ_GPIO, NXP_NFC_PN7150_IRQ_PIN))!=pn7150_previous_level){
            if(pn7150_current_level==1){
               dev_nxp_nfc_pn7150_x_recv_interrupt(&discovery_f4_nfc_pn7150);
            }
            pn7150_previous_level=pn7150_current_level;
         }
        /* Clear interrupt flag */
        EXTI_ClearITPendingBit(EXTI_Line5);
    }
}
#endif

/*-------------------------------------------
| Name:dev_discovery_f4_nfc_pn7150_load
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
static int dev_discovery_f4_nfc_pn7150_load(void) {
   GPIO_InitTypeDef  GPIO_InitStruct;
   EXTI_InitTypeDef EXTI_InitStruct;
   NVIC_InitTypeDef NVIC_InitStruct;
  
   //
   discovery_f4_nfc_pn7150.i2c_addr = (0x28<<1);
   //
   discovery_f4_nfc_pn7150.desc_r=-1;
   discovery_f4_nfc_pn7150.desc_w=-1;
   //
   
   /* PC6 EXT IRQ Set pin as input */
   GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
   GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
   GPIO_InitStruct.GPIO_Pin = NXP_NFC_PN7150_IRQ_PIN;
   GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
   GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
   GPIO_Init(NXP_NFC_PN7150_IRQ_GPIO, &GPIO_InitStruct);

   /* Tell system that you will use PE5 for EXTI_Line6 */
   SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOE, EXTI_PinSource5);

   /* PD0 is connected to EXTI_Line6 */
   EXTI_InitStruct.EXTI_Line = EXTI_Line5;
   /* Enable interrupt */
   EXTI_InitStruct.EXTI_LineCmd = ENABLE;
   /* Interrupt mode */
   EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
   /* Triggers on rising and falling edge */
   EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
   /* Add to EXTI */
   EXTI_Init(&EXTI_InitStruct);

   /* Add IRQ vector to NVIC */
   /* PD0 is connected to EXTI_Line6, which has EXTI9_5_IRQn vector */
   NVIC_InitStruct.NVIC_IRQChannel = EXTI9_5_IRQn;
   /* Set priority */
   NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0x0F;
   /* Set sub priority */
   NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0x0F;
   /* Enable interrupt */
   NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
   /* Add to NVIC */
   NVIC_Init(&NVIC_InitStruct);
  
   /* nxp nfc PN7150 VEN pin configuration */
   GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
   GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
   GPIO_InitStruct.GPIO_Pin = GPIO_Pin_8;
   GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
   GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
   GPIO_Init(NXP_NFC_PN7150_VEN_GPIO, &GPIO_InitStruct);    
         
   //
   return dev_nxp_nfc_pn7150_x_load(&discovery_f4_nfc_pn7150);
}
   
/*-------------------------------------------
| Name:dev_discovery_f4_nfc_pn7150_open
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
static int dev_discovery_f4_nfc_pn7150_open(desc_t desc, int o_flag) {
   //
   if(dev_nxp_nfc_pn7150_x_open(desc, o_flag, &discovery_f4_nfc_pn7150)<0){
      return -1;
   }
   // VEN
   GPIO_ResetBits(NXP_NFC_PN7150_VEN_GPIO, GPIO_Pin_8);
	__kernel_usleep(10000);
	GPIO_SetBits(NXP_NFC_PN7150_VEN_GPIO, GPIO_Pin_8);
	__kernel_usleep(10000);
   //
	return 0;
}

/*============================================
| End of Source  : dev_discovery_f4_uart_3.c
==============================================*/