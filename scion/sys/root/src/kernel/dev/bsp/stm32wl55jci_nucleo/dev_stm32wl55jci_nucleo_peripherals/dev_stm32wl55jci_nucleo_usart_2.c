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
#include "kernel/core/interrupt.h"
#include "kernel/core/kernel.h"
#include "kernel/core/system.h"
#include "kernel/core/fcntl.h"
#include "kernel/core/stat.h"
#include "kernel/core/cpu.h"
#include "kernel/fs/vfs/vfstypes.h"

#include "stm32wlxx_hal.h"

#include "kernel/dev/arch/cortexm/stm32wlxx/dev_stm32wlxx/dev_stm32wlxx_definitions.h"
#include "kernel/dev/arch/cortexm/stm32wlxx/dev_stm32wlxx/dev_stm32wlxx_uart_x.h"

/*===========================================
Global Declaration
=============================================*/
static const char dev_stm32wl55jci_nucleo_usart_2_name[]="ttys2\0";

static int dev_stm32wl55jci_nucleo_usart_2_load(void);
static int dev_stm32wl55jci_nucleo_usart_2_open(desc_t desc, int o_flag);

extern int dev_stm32wlxx_uart_x_load(board_stm32wlxx_uart_info_t * uart_info);
extern int dev_stm32wlxx_uart_x_open(desc_t desc, int o_flag, board_stm32wlxx_uart_info_t * uart_info);
extern int dev_stm32wlxx_uart_x_close(desc_t desc);
extern int dev_stm32wlxx_uart_x_read(desc_t desc, char* buf,int cb);
extern int dev_stm32wlxx_uart_x_write(desc_t desc, const char* buf,int cb);
extern int dev_stm32wlxx_uart_x_ioctl(desc_t desc,int request,va_list ap);
extern int dev_stm32wlxx_uart_x_isset_read(desc_t desc);
extern int dev_stm32wlxx_uart_x_isset_write(desc_t desc);
extern int dev_stm32wlxx_uart_x_seek(desc_t desc,int offset,int origin);

dev_map_t dev_stm32wl55jci_nucleo_usart_2_map={
   dev_stm32wl55jci_nucleo_usart_2_name,
   S_IFCHR,
   dev_stm32wl55jci_nucleo_usart_2_load,
   dev_stm32wl55jci_nucleo_usart_2_open,
   dev_stm32wlxx_uart_x_close,
   dev_stm32wlxx_uart_x_isset_read,
   dev_stm32wlxx_uart_x_isset_write,
   dev_stm32wlxx_uart_x_read,
   dev_stm32wlxx_uart_x_write,
   dev_stm32wlxx_uart_x_seek,
   dev_stm32wlxx_uart_x_ioctl
};

//
#define UART2_RX_BUFFER_SZ 255
static uint8_t UART2_RX_BUFFER[UART2_RX_BUFFER_SZ];
//
#define UART2_TX_BUFFER_SZ 255
static uint8_t UART2_TX_BUFFER[UART2_TX_BUFFER_SZ];

// uart 2 configuration. see in board device driver of current bsp
/*static*/ board_stm32wlxx_uart_info_t stm32wl55jci_nucleo_usart_2; 

/*===========================================
Implementation
=============================================*/

/**
  * @brief This function handles DMA1 Channel 1 Interrupt.
  */
void DMA1_Channel1_IRQHandler(void){
  HAL_DMA_IRQHandler((DMA_HandleTypeDef*)&stm32wl55jci_nucleo_usart_2.dev_hdma_usart_tx);
}

/**
  * @brief This function handles DMA1 Channel 2 Interrupt.
  */
void DMA1_Channel2_IRQHandler(void){
  HAL_DMA_IRQHandler((DMA_HandleTypeDef*)&stm32wl55jci_nucleo_usart_2.dev_hdma_usart_rx);
}
  

/**
  * @brief This function handles USART2 Interrupt.
  */
void USART2_IRQHandler(void){
  HAL_UART_IRQHandler((UART_HandleTypeDef*)&stm32wl55jci_nucleo_usart_2.dev_huart);
}

/*-------------------------------------------
| Name:dev_stm32wl55jci_nucleo_usart_2_load
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
static int dev_stm32wl55jci_nucleo_usart_2_load(void) {
   // cube mx hal 
   // USART2 DMA Init
   // USART2 RX Init
   GPIO_InitTypeDef GPIO_InitStruct = {0};
   RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};
   
   /** 
      Initializes the peripherals clocks
   */
   PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_USART2;
   PeriphClkInitStruct.Usart2ClockSelection = RCC_USART2CLKSOURCE_PCLK1;
   if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK){
      return -1;
   }

   /* Peripheral clock enable */
   __HAL_RCC_USART2_CLK_ENABLE();

   __HAL_RCC_GPIOA_CLK_ENABLE();
   /**
      USART2 NUCLEO GPIO Configuration
      PA3     ------> USART2_RX
      PA2     ------> USART2_TX
   */
   GPIO_InitStruct.Pin = GPIO_PIN_3|GPIO_PIN_2;
   GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
   GPIO_InitStruct.Pull = GPIO_PULLUP;
   GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
   GPIO_InitStruct.Alternate = GPIO_AF7_USART2;
   HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
   
   // DMA controller clock enable
   __HAL_RCC_DMAMUX1_CLK_ENABLE();
   __HAL_RCC_DMA1_CLK_ENABLE();
   
   // USART2
   ((UART_HandleTypeDef*)(&stm32wl55jci_nucleo_usart_2.dev_huart))->Instance = USART2;
   // rx
   ((DMA_HandleTypeDef*)(&stm32wl55jci_nucleo_usart_2.dev_hdma_usart_rx))->Instance = DMA1_Channel2;
   ((DMA_HandleTypeDef*)(&stm32wl55jci_nucleo_usart_2.dev_hdma_usart_rx))->Init.Request = DMA_REQUEST_USART2_RX;
   // tx
   ((DMA_HandleTypeDef*)(&stm32wl55jci_nucleo_usart_2.dev_hdma_usart_tx))->Instance = DMA1_Channel1;
   ((DMA_HandleTypeDef*)(&stm32wl55jci_nucleo_usart_2.dev_hdma_usart_tx))->Init.Request = DMA_REQUEST_USART2_TX;
   //
   stm32wl55jci_nucleo_usart_2.uart_irq_nb = USART2_IRQn;
   stm32wl55jci_nucleo_usart_2.dma_rx_irq_nb  = DMA1_Channel2_IRQn;
   stm32wl55jci_nucleo_usart_2.dma_tx_irq_nb  = DMA1_Channel1_IRQn;
   
   //
   stm32wl55jci_nucleo_usart_2.dev_huart.p_dev_information=&stm32wl55jci_nucleo_usart_2;
   stm32wl55jci_nucleo_usart_2.dev_hdma_usart_rx.p_dev_information=&stm32wl55jci_nucleo_usart_2;
   stm32wl55jci_nucleo_usart_2.dev_hdma_usart_tx.p_dev_information=&stm32wl55jci_nucleo_usart_2;
   //
   stm32wl55jci_nucleo_usart_2.desc_r=-1;
   stm32wl55jci_nucleo_usart_2.desc_w=-1;
   stm32wl55jci_nucleo_usart_2.baudrate=115200; //ublox ble NINA-B1:115200 //ublox modem: 19200  / ublox gnss: 9600
   //
   stm32wl55jci_nucleo_usart_2.p_rx_buffer=UART2_RX_BUFFER;
   stm32wl55jci_nucleo_usart_2.rx_buffer_sz=UART2_RX_BUFFER_SZ;
   //
   stm32wl55jci_nucleo_usart_2.p_tx_buffer=UART2_TX_BUFFER;
   stm32wl55jci_nucleo_usart_2.tx_buffer_sz=UART2_TX_BUFFER_SZ;
   
   //
   return dev_stm32wlxx_uart_x_load(&stm32wl55jci_nucleo_usart_2);
}
   
/*-------------------------------------------
| Name:dev_stm32wl55jci_nucleo_usart_2_open
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
static int dev_stm32wl55jci_nucleo_usart_2_open(desc_t desc, int o_flag) {
   return dev_stm32wlxx_uart_x_open(desc, o_flag, &stm32wl55jci_nucleo_usart_2);
}

/*============================================
| End of Source  : dev_stm32wl55jci_nucleo_usart_2.c
==============================================*/