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

#include "stm32f4xx_hal.h"

#include "kernel/dev/arch/cortexm/stm32f4xx/dev_stm32f4xx/dev_stm32f4xx_i2c_x.h"

/*===========================================
Global Declaration
=============================================*/
const char dev_discovery_f4_i2c_1_name[]="i2c1\0";

int dev_discovery_f4_i2c_1_load(void);
int dev_discovery_f4_i2c_1_open(desc_t desc, int o_flag);

extern int dev_stm32f4xx_i2c_x_load(board_stm32f4xx_i2c_info_t * i2c_info);
extern int dev_stm32f4xx_i2c_x_open(desc_t desc, int o_flag, board_stm32f4xx_i2c_info_t * i2c_info);
extern int dev_stm32f4xx_i2c_x_close(desc_t desc);
extern int dev_stm32f4xx_i2c_x_read(desc_t desc, char* buf,int cb);
extern int dev_stm32f4xx_i2c_x_write(desc_t desc, const char* buf,int cb);
extern int dev_stm32f4xx_i2c_x_ioctl(desc_t desc,int request,va_list ap);
extern int dev_stm32f4xx_i2c_x_isset_read(desc_t desc);
extern int dev_stm32f4xx_i2c_x_isset_write(desc_t desc);
extern int dev_stm32f4xx_i2c_x_seek(desc_t desc,int offset,int origin);

dev_map_t dev_discovery_f4_i2c_1_map={
   dev_discovery_f4_i2c_1_name,
   S_IFBLK,
   dev_discovery_f4_i2c_1_load,
   dev_discovery_f4_i2c_1_open,
   dev_stm32f4xx_i2c_x_close,
   dev_stm32f4xx_i2c_x_isset_read,
   dev_stm32f4xx_i2c_x_isset_write,
   dev_stm32f4xx_i2c_x_read,
   dev_stm32f4xx_i2c_x_write,
   dev_stm32f4xx_i2c_x_seek,
   dev_stm32f4xx_i2c_x_ioctl
};


// uart 3 configuration. see in board device driver of current bsp
static board_stm32f4xx_i2c_info_t discovery_f4_i2c_1={0}; 

/* Definition for I2Cx Pins */
#define I2Cx_SDA_GPIO_CLK_ENABLE()       __HAL_RCC_GPIOB_CLK_ENABLE()
#define I2Cx_SCL_GPIO_CLK_ENABLE()       __HAL_RCC_GPIOB_CLK_ENABLE() 

#define I2Cx_SCL_PIN                    GPIO_PIN_6
#define I2Cx_SCL_GPIO_PORT              GPIOB
#define I2Cx_SCL_AF                     GPIO_AF4_I2C1
#define I2Cx_SDA_PIN                    GPIO_PIN_9
#define I2Cx_SDA_GPIO_PORT              GPIOB
#define I2Cx_SDA_AF                     GPIO_AF4_I2C1

/* Definition for I2Cx's DMA */
#define I2C1_TX_DMA_STREAM              DMA1_Stream6     
#define I2C1_DMA_TX_IRQn                DMA1_Stream6_IRQn
#define I2C1_TX_DMA_CHANNEL             DMA_CHANNEL_1

#define I2C1_RX_DMA_STREAM              DMA1_Stream5
#define I2C1_DMA_RX_IRQn                DMA1_Stream5_IRQn
#define I2C1_RX_DMA_CHANNEL             DMA_CHANNEL_1


/* Definition for I2Cx's DMA NVIC */
#define I2C1_DMA_TX_IRQHandler          DMA1_Stream6_IRQHandler
#define I2C1_DMA_RX_IRQHandler          DMA1_Stream5_IRQHandler


/* Audio Reset Pin definition */
#define AUDIO_RESET_PIN                       GPIO_PIN_4
#define AUDIO_RESET_GPIO                      GPIOD


/*===========================================
Implementation
=============================================*/
#define I2C1_RXTX_BUFFER_SIZE_MAX 512
static uint32_t align32_i2c1_rxtx_buffer[I2C1_RXTX_BUFFER_SIZE_MAX/sizeof(uint32_t)];
static uint8_t* i2c1_rxtx_buffer=(uint8_t*)&align32_i2c1_rxtx_buffer[0];


/**
  * @brief  This function handles DMA interrupt request.  
  * @param  None
  * @retval None
  * @Note   This function is redefined in "main.h" and related to DMA stream 
  *         used for I2C data transmission     
  */
void I2C1_DMA_RX_IRQHandler(void)
{
  HAL_DMA_IRQHandler(discovery_f4_i2c_1.i2c_handle.hdmarx);
}

/**
  * @brief  This function handles DMA interrupt request.
  * @param  None
  * @retval None
  * @Note   This function is redefined in "main.h" and related to DMA stream 
  *         used for I2C data reception    
  */
void I2C1_DMA_TX_IRQHandler(void)
{
  HAL_DMA_IRQHandler(discovery_f4_i2c_1.i2c_handle.hdmatx);
}



/*-------------------------------------------
| Name:dev_discovery_f4_i2c_1_load
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
static int dev_discovery_f4_i2c_1_load(void) {
   GPIO_InitTypeDef  GPIO_InitStruct;
  
   //
   discovery_f4_i2c_1.i2c_handle.Instance=I2C1;
   discovery_f4_i2c_1.bus_speed = 100000L;
   discovery_f4_i2c_1.own_address = 0xFE;
   //
   discovery_f4_i2c_1.desc_r=-1;
   discovery_f4_i2c_1.desc_w=-1;
   //
   
   discovery_f4_i2c_1.i2c_rxtx_buffer_size = I2C1_RXTX_BUFFER_SIZE_MAX;
   discovery_f4_i2c_1.p_i2c_rxtx_buffer = i2c1_rxtx_buffer;
  
   //to remove: just for i2c test     
   /* Audio reset pin configuration */
   GPIO_InitStruct.Pin = AUDIO_RESET_PIN; 
   GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
   GPIO_InitStruct.Speed = GPIO_SPEED_FAST;
   GPIO_InitStruct.Pull  = GPIO_NOPULL;
   HAL_GPIO_Init(AUDIO_RESET_GPIO, &GPIO_InitStruct);    


   /* Power Down the codec */
   HAL_GPIO_WritePin(AUDIO_RESET_GPIO, AUDIO_RESET_PIN, GPIO_PIN_RESET);

   /* Wait for a delay to insure registers erasing */
   HAL_Delay(5); 

   /* Power on the codec */
   HAL_GPIO_WritePin(AUDIO_RESET_GPIO, AUDIO_RESET_PIN, GPIO_PIN_SET);

   /* Wait for a delay to insure registers erasing */
   HAL_Delay(5); 
         
   
   
   /* Enable GPIO Clocks #################################################*/
   /* Enable GPIO TX/RX clock */
   I2Cx_SCL_GPIO_CLK_ENABLE();
   I2Cx_SDA_GPIO_CLK_ENABLE();

   /* Configure peripheral GPIO ##########################################*/  
   /* I2C TX GPIO pin configuration  */
   GPIO_InitStruct.Pin       = I2Cx_SCL_PIN;
   GPIO_InitStruct.Mode      = GPIO_MODE_AF_OD;
   GPIO_InitStruct.Pull      = GPIO_PULLUP;
   GPIO_InitStruct.Speed     = GPIO_SPEED_FAST;
   GPIO_InitStruct.Alternate = I2Cx_SCL_AF;
   HAL_GPIO_Init(I2Cx_SCL_GPIO_PORT, &GPIO_InitStruct);

   /* I2C RX GPIO pin configuration  */
   GPIO_InitStruct.Pin = I2Cx_SDA_PIN;
   GPIO_InitStruct.Alternate = I2Cx_SDA_AF;
   HAL_GPIO_Init(I2Cx_SDA_GPIO_PORT, &GPIO_InitStruct);

   //
   return dev_stm32f4xx_i2c_x_load(&discovery_f4_i2c_1);
}
   
/*-------------------------------------------
| Name:dev_discovery_f4_i2c_1_open
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
static int dev_discovery_f4_i2c_1_open(desc_t desc, int o_flag) {
   
   //DMA
   //
   discovery_f4_i2c_1.tx_dma_stream=I2C1_TX_DMA_STREAM  ;
   discovery_f4_i2c_1.tx_dma_stream_irq_no= I2C1_DMA_TX_IRQn ;
   discovery_f4_i2c_1.tx_dma_channel=I2C1_TX_DMA_CHANNEL;
   //
   discovery_f4_i2c_1.rx_dma_stream=I2C1_RX_DMA_STREAM;
   discovery_f4_i2c_1.rx_dma_stream_irq_no=I2C1_DMA_RX_IRQn;
   discovery_f4_i2c_1.rx_dma_channel=I2C1_RX_DMA_CHANNEL;
   //
   return dev_stm32f4xx_i2c_x_open(desc, o_flag, &discovery_f4_i2c_1);
}

/*============================================
| End of Source  : dev_discovery_f4_uart_3.c
==============================================*/