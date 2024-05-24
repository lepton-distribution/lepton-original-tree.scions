/*
The contents of this file are subject to the Mozilla Public License Version 1.1
(the "License"); you may not use this file except in compliance with the License.
You may obtain a copy of the License at http://www.mozilla.org/MPL/

Software distributed under the License is distributed on an "AS IS" basis,
WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License for the
specific language governing rights and limitations under the License.

The Original Code is Lepton.

The Initial Developer of the Original Code is Philippe Le Boulanger.
Portions created by Philippe Le Boulanger are Copyright (C) 2024 <lepton.phlb@gmail.com>.
All Rights Reserved.

*/


/*============================================
| Compiler Directive
==============================================*/
#ifndef __DEV_STM32WLXX_UART_X_H__
#define __DEV_STM32WLXX_UART_X_H__


/*============================================
| Includes
==============================================*/


/*============================================
| Declaration
==============================================*/

//
typedef struct board_stm32wlxx_uart_info_st {
  
   //cubemx hal types
   IRQn_Type uart_irq_nb;
   IRQn_Type dma_rx_irq_nb;
   IRQn_Type dma_tx_irq_nb;
  
   //cubemx hal drivers structures
   dev_uart_handle_typedef_t dev_huart;
   dev_dma_handle_typedef_t  dev_hdma_usart_rx;
   dev_dma_handle_typedef_t  dev_hdma_usart_tx;
   //
   int desc_r;
   int desc_w;
   
   // rx
   uint16_t rx_index_w;
   uint16_t rx_index_r;
   uint16_t rx_buffer_sz;
   uint8_t *p_rx_buffer;
   // tx
   uint16_t tx_index_w;
   uint16_t tx_index_r;
   uint16_t tx_buffer_sz;
   uint8_t *p_tx_buffer;
   
   //
   unsigned long baudrate;
   
   //
   kernel_pthread_mutex_t mutex;
} board_stm32wlxx_uart_info_t;


#endif //__DEV_STM32WLXX_UART_X_H__
