/*
The contents of this file are subject to the Mozilla Public License Version 1.1
(the "License"); you may not use this file except in compliance with the License.
You may obtain a copy of the License at http://www.mozilla.org/MPL/

Software distributed under the License is distributed on an "AS IS" basis,
WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License for the
specific language governing rights and limitations under the License.

The Original Code is Lepton.

The Initial Developer of the Original Code is Philippe Le Boulanger.
Portions created by Philippe Le Boulanger are Copyright (C) 2020 <lepton.phlb@gmail.com>.
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
#include "kernel/core/interrupt.h"
#include "kernel/core/kernel.h"
#include "kernel/core/system.h"
#include "kernel/core/fcntl.h"
#include "kernel/core/stat.h"
#include "kernel/core/cpu.h"
#include "kernel/fs/vfs/vfstypes.h"
#include "kernel/core/ioctl_board.h"

#include "stm32wlxx_hal.h"

#include "kernel/dev/bsp/stm32wl55jci_nucleo/dev_stm32wl55jci_nucleo_board/dev_stm32wl55jci_nucleo_board.h"

/*===========================================
Global Declaration
=============================================*/

#define LED1_Pin GPIO_PIN_15
#define LED1_GPIO_Port GPIOB
#define LED2_Pin GPIO_PIN_9
#define LED2_GPIO_Port GPIOB
#define BUT1_Pin GPIO_PIN_0
#define BUT1_GPIO_Port GPIOA
#define PROB2_Pin GPIO_PIN_13
#define PROB2_GPIO_Port GPIOB
#define PROB1_Pin GPIO_PIN_12
#define PROB1_GPIO_Port GPIOB
#define BUT3_Pin GPIO_PIN_6
#define BUT3_GPIO_Port GPIOC
#define BUT2_Pin GPIO_PIN_1
#define BUT2_GPIO_Port GPIOA
#define LED3_Pin GPIO_PIN_11
#define LED3_GPIO_Port GPIOB



const char dev_stm32wl55jci_nucleo_board_name[]="board\0";

int dev_stm32wl55jci_nucleo_board_load(void);
int dev_stm32wl55jci_nucleo_board_open(desc_t desc, int o_flag);
int dev_stm32wl55jci_nucleo_board_close(desc_t desc);
int dev_stm32wl55jci_nucleo_board_isset_read(desc_t desc);
int dev_stm32wl55jci_nucleo_board_isset_write(desc_t desc);
int dev_stm32wl55jci_nucleo_board_read(desc_t desc, char* buf,int size);
int dev_stm32wl55jci_nucleo_board_write(desc_t desc, const char* buf,int size);
int dev_stm32wl55jci_nucleo_board_seek(desc_t desc,int offset,int origin);
int dev_stm32wl55jci_nucleo_board_ioctl(desc_t desc,int request,va_list ap);


dev_map_t dev_stm32wl55jci_nucleo_board_map={
   dev_stm32wl55jci_nucleo_board_name,
   S_IFBLK,
   dev_stm32wl55jci_nucleo_board_load,
   dev_stm32wl55jci_nucleo_board_open,
   dev_stm32wl55jci_nucleo_board_close,
   dev_stm32wl55jci_nucleo_board_isset_read,
   dev_stm32wl55jci_nucleo_board_isset_write,
   dev_stm32wl55jci_nucleo_board_read,
   dev_stm32wl55jci_nucleo_board_write,
   dev_stm32wl55jci_nucleo_board_seek,
   dev_stm32wl55jci_nucleo_board_ioctl //ioctl
};



/*===========================================
Implementation
=============================================*/


/*-------------------------------------------
| Name:dev_stm32wl55jci_nucleo_board_load
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
int dev_stm32wl55jci_nucleo_board_load(void){
  
   GPIO_InitTypeDef GPIO_InitStruct = {0};
  
   __HAL_RCC_GPIOB_CLK_ENABLE();
   __HAL_RCC_GPIOC_CLK_ENABLE();
   __HAL_RCC_GPIOA_CLK_ENABLE();
   
   
   /*Configure GPIO pin Output Level */
   HAL_GPIO_WritePin(GPIOB, LED1_Pin|LED2_Pin|PROB2_Pin|PROB1_Pin
                       |LED3_Pin, GPIO_PIN_RESET);

   /*Configure GPIO pins : PBPin PBPin PBPin */
   GPIO_InitStruct.Pin = LED1_Pin|LED2_Pin|LED3_Pin;
   GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
   GPIO_InitStruct.Pull = GPIO_NOPULL;
   GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
   HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

   /*Configure GPIO pins : PAPin PAPin */
   GPIO_InitStruct.Pin = BUT1_Pin|BUT2_Pin;
   GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
   GPIO_InitStruct.Pull = GPIO_PULLUP;
   HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

   /*Configure GPIO pins : PBPin PBPin */
   GPIO_InitStruct.Pin = PROB2_Pin|PROB1_Pin;
   GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
   GPIO_InitStruct.Pull = GPIO_NOPULL;
   GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
   HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

   /*Configure GPIO pin : PtPin */
   GPIO_InitStruct.Pin = BUT3_Pin;
   GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
   GPIO_InitStruct.Pull = GPIO_PULLUP;
   HAL_GPIO_Init(BUT3_GPIO_Port, &GPIO_InitStruct);

  
   return 0;
}

/*-------------------------------------------
| Name:dev_stm32wl55jci_nucleo_board_open
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
int dev_stm32wl55jci_nucleo_board_open(desc_t desc, int o_flag){

   //
   if(o_flag & O_RDONLY) {
   }

   if(o_flag & O_WRONLY) {
   }

   ofile_lst[desc].offset=0;

   return 0;
}

/*-------------------------------------------
| Name:dev_stm32wl55jci_nucleo_board_close
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
int dev_stm32wl55jci_nucleo_board_close(desc_t desc){
   return 0;
}

/*-------------------------------------------
| Name:dev_stm32wl55jci_nucleo_board_isset_read
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
int dev_stm32wl55jci_nucleo_board_isset_read(desc_t desc){
   return -1;
}

/*-------------------------------------------
| Name:dev_stm32wl55jci_nucleo_board_isset_write
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
int dev_stm32wl55jci_nucleo_board_isset_write(desc_t desc){
   return -1;
}
/*-------------------------------------------
| Name:dev_stm32wl55jci_nucleo_board_read
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
int dev_stm32wl55jci_nucleo_board_read(desc_t desc, char* buf,int size){
   return -1;
}

/*-------------------------------------------
| Name:dev_stm32wl55jci_nucleo_board_write
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
int dev_stm32wl55jci_nucleo_board_write(desc_t desc, const char* buf,int size){
   //read only mode
   return -1;
}

/*-------------------------------------------
| Name:dev_a0350_board_seek
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
int dev_stm32wl55jci_nucleo_board_seek(desc_t desc,int offset,int origin){

   switch(origin) {

   case SEEK_SET:
      ofile_lst[desc].offset=offset;
      break;

   case SEEK_CUR:
      ofile_lst[desc].offset+=offset;
      break;

   case SEEK_END:
      //to do: warning in SEEK_END (+ or -)????
      ofile_lst[desc].offset-=offset;
      break;
   }

   return ofile_lst[desc].offset;
}

/*-------------------------------------------
| Name:dev_stm32wl55jci_nucleo_board_ioctl
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
int dev_stm32wl55jci_nucleo_board_ioctl(desc_t desc,int request,va_list ap){

   switch(request) {
      //
      case BRDPWRDOWN:   //power down
      break;
      //
      case BRDRESET:  
         
      break;
      //
      case BRDWATCHDOG:   //start watchdog
      break;
      //
      case BRDCFGPORT: {
      }
      break;
      //
      case BRDBEEP: {
      }
      break;
      
      //
      default:
         return -1;

   }

   return 0;
}

/*============================================
| End of Source  : dev_stm32wl55jci_nucleo_board.c
==============================================*/
