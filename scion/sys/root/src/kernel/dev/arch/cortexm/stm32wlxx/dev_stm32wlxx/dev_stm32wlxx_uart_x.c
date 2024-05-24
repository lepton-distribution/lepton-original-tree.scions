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


/*
   note: for interrupt priority with embos see :https://wiki.segger.com/Interrupt_prioritization
   for stm32wlxx pririty coded on 4 bits: find __NVIC_PRIO_BITS definition
   cortex M4 : Drivers\CMSIS\Device\ST\STM32WLxx\Include\stm32wl55xx.h 206 #define __NVIC_PRIO_BITS          4U  STM32WLxx uses 4 Bits for the Priority Levels 

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
#include "kernel/core/ioctl.h"
#include "kernel/core/fcntl.h"
#include "kernel/core/cpu.h"
#include "kernel/core/core_rttimer.h"
#include "kernel/fs/vfs/vfstypes.h"


#include "lib/libc/termios/termios.h"

#include "stm32wlxx_hal.h"

#include "kernel/dev/arch/cortexm/stm32wlxx/dev_stm32wlxx/dev_stm32wlxx_definitions.h"
#include "kernel/dev/arch/cortexm/stm32wlxx/dev_stm32wlxx/dev_stm32wlxx_uart_x.h"


/*===========================================
Global Declaration
=============================================*/

//
typedef struct s2s {
   speed_t ts;
   long ns;
}s2s_t;

static s2s_t const s2s[] = {
   { B0,                0 },
   { B50,              50 },
   { B75,              75 },
   { B110,            110 },
   { B134,            134 },
   { B150,            150 },
   { B200,            200 },
   { B300,            300 },
   { B600,            600 },
   { B1200,          1200 },
   { B1800,          1800 },
   { B2400,          2400 },
   { B4800,          4800 },
   { B9600,          9600 },
   { B19200,        19200 },
   { B38400,        38400 },
   { B57600,        57600 },
   { B115200,      115200 },
   { B230400,      230400 },
   { B460800,      460800 }
};



int dev_stm32wlxx_uart_x_load(board_stm32wlxx_uart_info_t * p_board_stm32wlxx_uart_info);
int dev_stm32wlxx_uart_x_open(desc_t desc, int o_flag, board_stm32wlxx_uart_info_t * p_board_stm32wlxx_uart_info);

//
int dev_stm32wlxx_uart_x_isset_read(desc_t desc);
int dev_stm32wlxx_uart_x_isset_write(desc_t desc);
int dev_stm32wlxx_uart_x_close(desc_t desc);
int dev_stm32wlxx_uart_x_seek(desc_t desc,int offset,int origin);
int dev_stm32wlxx_uart_x_read(desc_t desc, char* buf,int cb);
int dev_stm32wlxx_uart_x_write(desc_t desc, const char* buf,int cb);
int dev_stm32wlxx_uart_x_ioctl(desc_t desc,int request,va_list ap);
int dev_stm32wlxx_uart_x_seek(desc_t desc,int offset,int origin);


/*===========================================
Implementation
=============================================*/

/**
  * @brief  User implementation of the Reception Event Callback
  *         (Rx event notification called after use of advanced reception service).
  * @param  huart UART handle
  * @param  Size  Number of data available in application reception buffer (indicates a position in
  *               reception buffer until which, data are available)
  * @retval None
  */
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size){
   //
   dev_uart_handle_typedef_t *dev_uart_handle =  (dev_uart_handle_typedef_t*)huart;
   board_stm32wlxx_uart_info_t * p_board_stm32wlxx_uart_info = (board_stm32wlxx_uart_info_t*)dev_uart_handle->p_dev_information;
    
   //
   __hw_enter_interrupt();
  
   //buffer from empty to not empty
   if(p_board_stm32wlxx_uart_info->rx_index_w==p_board_stm32wlxx_uart_info->rx_index_r){
      if( p_board_stm32wlxx_uart_info->desc_r!=-1){
         __fire_io_int(ofile_lst[p_board_stm32wlxx_uart_info->desc_r].owner_pthread_ptr_read);
      }
   }
  
   //
   p_board_stm32wlxx_uart_info->rx_index_w=Size;
   
   //
   __hw_leave_interrupt();
}


/**
  * @brief  Tx Transfer completed callback
  * @param  huart: UART handle.
  * @note   This example shows a simple way to report end of DMA Tx transfer, and
  *         you can add your own implementation.
  * @retval None
  */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart){
   //
   dev_uart_handle_typedef_t *dev_uart_handle =  (dev_uart_handle_typedef_t*)huart;
   board_stm32wlxx_uart_info_t * p_board_stm32wlxx_uart_info = (board_stm32wlxx_uart_info_t*)dev_uart_handle->p_dev_information;
    
   //
   __hw_enter_interrupt();
 
   //all data are sent
   p_board_stm32wlxx_uart_info->tx_index_r=p_board_stm32wlxx_uart_info->tx_index_w;
   
   if( p_board_stm32wlxx_uart_info->desc_w!=-1){
      __fire_io_int(ofile_lst[p_board_stm32wlxx_uart_info->desc_w].owner_pthread_ptr_write);
   }
  
   //
   __hw_leave_interrupt();
}


/*-------------------------------------------
| Name:termios2ttys
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
static int dev_stm32wlxx_uart_x_termios2ttys(struct termios* termios_p, board_stm32wlxx_uart_info_t * p_board_stm32wlxx_uart_info){
   const struct s2s *sp;
   long n_speed;
   speed_t speed;

   //speed
   speed = cfgetospeed(termios_p);
   for (sp = s2s; sp < s2s + (sizeof(s2s) / sizeof(s2s[0])); sp++){
      if (sp->ts == speed){
         n_speed = sp->ns;
         // Set baud rate
         //uart_set_baudrate(&p_board_stm32wlxx_uart_info->uart_descriptor,n_speed);         
         break;
      }
   }

   return 0;
}

/*-------------------------------------------
| Name:dev_stm32wlxx_uart_x_load
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
int dev_stm32wlxx_uart_x_load(board_stm32wlxx_uart_info_t * p_board_stm32wlxx_uart_info){
  
   pthread_mutexattr_t mutex_attr=0;

   kernel_pthread_mutex_init(&p_board_stm32wlxx_uart_info->mutex,&mutex_attr);

   return 0;
}

/*-------------------------------------------
| Name:dev_stm32wlxx_uart_x_open
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
int dev_stm32wlxx_uart_x_open(desc_t desc, int o_flag,
                            board_stm32wlxx_uart_info_t * p_board_stm32wlxx_uart_info){
                          
   //
   if(p_board_stm32wlxx_uart_info->desc_r<0 && p_board_stm32wlxx_uart_info->desc_w<0) {
      if(p_board_stm32wlxx_uart_info->baudrate==0){
         p_board_stm32wlxx_uart_info->baudrate=115200;
      }
      
      //
      ((UART_HandleTypeDef*)(&p_board_stm32wlxx_uart_info->dev_huart))->Init.BaudRate = p_board_stm32wlxx_uart_info->baudrate;
      ((UART_HandleTypeDef*)(&p_board_stm32wlxx_uart_info->dev_huart))->Init.WordLength = UART_WORDLENGTH_8B;
      ((UART_HandleTypeDef*)(&p_board_stm32wlxx_uart_info->dev_huart))->Init.StopBits = UART_STOPBITS_1;
      ((UART_HandleTypeDef*)(&p_board_stm32wlxx_uart_info->dev_huart))->Init.Parity = UART_PARITY_NONE;
      ((UART_HandleTypeDef*)(&p_board_stm32wlxx_uart_info->dev_huart))->Init.Mode = UART_MODE_TX_RX;
      ((UART_HandleTypeDef*)(&p_board_stm32wlxx_uart_info->dev_huart))->Init.HwFlowCtl = UART_HWCONTROL_NONE;
      ((UART_HandleTypeDef*)(&p_board_stm32wlxx_uart_info->dev_huart))->Init.OverSampling = UART_OVERSAMPLING_16;
      ((UART_HandleTypeDef*)(&p_board_stm32wlxx_uart_info->dev_huart))->Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
      ((UART_HandleTypeDef*)(&p_board_stm32wlxx_uart_info->dev_huart))->Init.ClockPrescaler = UART_PRESCALER_DIV1;
      ((UART_HandleTypeDef*)(&p_board_stm32wlxx_uart_info->dev_huart))->AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
      //
      if (HAL_UART_Init((UART_HandleTypeDef*)(&p_board_stm32wlxx_uart_info->dev_huart)) != HAL_OK){
         return -1;
      }
      if (HAL_UARTEx_SetTxFifoThreshold((UART_HandleTypeDef*)(&p_board_stm32wlxx_uart_info->dev_huart), UART_TXFIFO_THRESHOLD_1_8) != HAL_OK){
         return -1;
      }
      if (HAL_UARTEx_SetRxFifoThreshold((UART_HandleTypeDef*)(&p_board_stm32wlxx_uart_info->dev_huart), UART_RXFIFO_THRESHOLD_1_8) != HAL_OK){
         return -1;
      }
      if (HAL_UARTEx_DisableFifoMode((UART_HandleTypeDef*)(&p_board_stm32wlxx_uart_info->dev_huart)) != HAL_OK){
         return -1;
      }
      
   }
   //
   if(o_flag & O_RDONLY) {
      if(p_board_stm32wlxx_uart_info->desc_r<0) {
         //
         UART_HandleTypeDef* p_huart = (UART_HandleTypeDef*)(&p_board_stm32wlxx_uart_info->dev_huart);
         DMA_HandleTypeDef*  p_hdma_usart_rx = (DMA_HandleTypeDef*)(&p_board_stm32wlxx_uart_info->dev_hdma_usart_rx);
        
         //
         p_board_stm32wlxx_uart_info->desc_r = desc;
         //
         p_board_stm32wlxx_uart_info->rx_index_w=0;
         p_board_stm32wlxx_uart_info->rx_index_r=0;
        
         //
         p_hdma_usart_rx->Init.Direction = DMA_PERIPH_TO_MEMORY;
         p_hdma_usart_rx->Init.PeriphInc = DMA_PINC_DISABLE;
         p_hdma_usart_rx->Init.MemInc = DMA_MINC_ENABLE;
         p_hdma_usart_rx->Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
         p_hdma_usart_rx->Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
         p_hdma_usart_rx->Init.Mode = DMA_CIRCULAR;
         p_hdma_usart_rx->Init.Priority = DMA_PRIORITY_LOW;
         //
         if (HAL_DMA_Init(p_hdma_usart_rx) != HAL_OK){
            return -1;
         }
         //
         if (HAL_DMA_ConfigChannelAttributes(p_hdma_usart_rx, DMA_CHANNEL_NPRIV) != HAL_OK){
            return -1;
         }
         //
         __HAL_LINKDMA(p_huart, hdmarx, p_board_stm32wlxx_uart_info->dev_hdma_usart_rx.hdma);
           
         /* Initializes Rx sequence using Reception To Idle event API.
           As DMA channel associated to UART Rx is configured as Circular,
           reception is endless.
           If reception has to be stopped, call to HAL_UART_AbortReceive() could be used.

           Use of HAL_UARTEx_ReceiveToIdle_DMA service, will generate calls to
           user defined HAL_UARTEx_RxEventCallback callback for each occurrence of
           following events :
           - DMA RX Half Transfer event (HT)
           - DMA RX Transfer Complete event (TC)
           - IDLE event on UART Rx line (indicating a pause is UART reception flow)
         */
         if (HAL_OK != HAL_UARTEx_ReceiveToIdle_DMA(p_huart, p_board_stm32wlxx_uart_info->p_rx_buffer, p_board_stm32wlxx_uart_info->rx_buffer_sz)){
            return -1;
         }
         
         /* USART2 interrupt Init */
         //HAL_NVIC_SetPriority(p_board_stm32wlxx_uart_info->uart_irq_nb, 0, 0);
         NVIC_SetPriority(p_board_stm32wlxx_uart_info->uart_irq_nb, (1 << __NVIC_PRIO_BITS) -4); // priority 12
         HAL_NVIC_EnableIRQ(p_board_stm32wlxx_uart_info->uart_irq_nb);
         
         /* DMA rx interrupt init configuration*/
         //HAL_NVIC_SetPriority(p_board_stm32wlxx_uart_info->dma_rx_irq_nb, 0, 0);
         NVIC_SetPriority(p_board_stm32wlxx_uart_info->dma_rx_irq_nb, (1 << __NVIC_PRIO_BITS) -4);
         HAL_NVIC_EnableIRQ(p_board_stm32wlxx_uart_info->dma_rx_irq_nb);
         
      }else{
         return -1; //already open
      }
   }

   if(o_flag & O_WRONLY) {
      if(p_board_stm32wlxx_uart_info->desc_w<0) {
         //
         UART_HandleTypeDef* p_huart = (UART_HandleTypeDef*)(&p_board_stm32wlxx_uart_info->dev_huart);
         DMA_HandleTypeDef*  p_hdma_usart_tx = (DMA_HandleTypeDef*)(&p_board_stm32wlxx_uart_info->dev_hdma_usart_tx);
        
         //
         p_board_stm32wlxx_uart_info->desc_w = desc;
         //
         p_board_stm32wlxx_uart_info->tx_index_w=0;
         p_board_stm32wlxx_uart_info->tx_index_r=0;
         
         //
         /* USART2_TX Init */
         p_hdma_usart_tx->Init.Direction = DMA_MEMORY_TO_PERIPH;
         p_hdma_usart_tx->Init.PeriphInc = DMA_PINC_DISABLE;
         p_hdma_usart_tx->Init.MemInc = DMA_MINC_ENABLE;
         p_hdma_usart_tx->Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
         p_hdma_usart_tx->Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
         p_hdma_usart_tx->Init.Mode = DMA_NORMAL;
         p_hdma_usart_tx->Init.Priority = DMA_PRIORITY_LOW;
         //
         if (HAL_DMA_Init(p_hdma_usart_tx) != HAL_OK){
            return -1;
         }
         //
         if (HAL_DMA_ConfigChannelAttributes(p_hdma_usart_tx, DMA_CHANNEL_NPRIV) != HAL_OK){
            return -1;
         }
         //
         __HAL_LINKDMA(p_huart,hdmatx,p_board_stm32wlxx_uart_info->dev_hdma_usart_tx.hdma);

          /* USART2 interrupt Init */
         //HAL_NVIC_SetPriority(p_board_stm32wlxx_uart_info->uart_irq_nb, 0, 0);
         NVIC_SetPriority(p_board_stm32wlxx_uart_info->uart_irq_nb, (1 << __NVIC_PRIO_BITS) -4); // priority 12
         HAL_NVIC_EnableIRQ(p_board_stm32wlxx_uart_info->uart_irq_nb);
         
         /* DMA rx interrupt init configuration*/
        //HAL_NVIC_SetPriority(p_board_stm32wlxx_uart_info->dma_tx_irq_nb, 0, 0);
         NVIC_SetPriority(p_board_stm32wlxx_uart_info->dma_tx_irq_nb, (1 << __NVIC_PRIO_BITS) -4);
         HAL_NVIC_EnableIRQ(p_board_stm32wlxx_uart_info->dma_tx_irq_nb);
         
      }else{
         return -1; //already open
      }
   }

   //
   if(!ofile_lst[desc].p){
      ofile_lst[desc].p=p_board_stm32wlxx_uart_info;
   }

   //unmask IRQ
   if(p_board_stm32wlxx_uart_info->desc_r>=0 && p_board_stm32wlxx_uart_info->desc_w>=0) {
        
   }
   return 0;
}

/*-------------------------------------------
| Name:dev_stm32wlxx_uart_x_close
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
int dev_stm32wlxx_uart_x_close(desc_t desc){
   board_stm32wlxx_uart_info_t * p_board_stm32wlxx_uart_info = (board_stm32wlxx_uart_info_t*)ofile_lst[desc].p;
   //
   UART_HandleTypeDef* p_huart = (UART_HandleTypeDef*)(&p_board_stm32wlxx_uart_info->dev_huart);
   //
   if(!p_board_stm32wlxx_uart_info){
      return -1;
   }
   // 
   if(ofile_lst[desc].oflag & O_RDONLY) {
      if(!ofile_lst[desc].nb_reader) {
         p_board_stm32wlxx_uart_info->desc_r = -1;
         //
         // USART2 rx DMA DeInit
         HAL_DMA_DeInit(p_huart->hdmarx);
      }
   }
   //
   if(ofile_lst[desc].oflag & O_WRONLY) {
      if(!ofile_lst[desc].nb_writer) {
         p_board_stm32wlxx_uart_info->desc_w = -1;
         //
         // USART2 tx DMA DeInit
         HAL_DMA_DeInit(p_huart->hdmatx);
      }
   }
   //
   if(p_board_stm32wlxx_uart_info->desc_r<0 && p_board_stm32wlxx_uart_info->desc_w<0) {
      // USART2 interrupt DeInit
      HAL_NVIC_DisableIRQ(p_board_stm32wlxx_uart_info->uart_irq_nb); 
   }
   //
   return 0;
}

/*-------------------------------------------
| Name:dev_stm32wlxx_uart_x_isset_read
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
int dev_stm32wlxx_uart_x_isset_read(desc_t desc){
   board_stm32wlxx_uart_info_t * p_board_stm32wlxx_uart_info = (board_stm32wlxx_uart_info_t*)ofile_lst[desc].p;
   // 
   if(p_board_stm32wlxx_uart_info->rx_index_w!=p_board_stm32wlxx_uart_info->rx_index_r){
      return 0; //data are available in rx buffer
   }
   //
   return -1;
}

/*-------------------------------------------
| Name:dev_stm32wlxx_uart_x_isset_write
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
int dev_stm32wlxx_uart_x_isset_write(desc_t desc){
   board_stm32wlxx_uart_info_t * p_board_stm32wlxx_uart_info = (board_stm32wlxx_uart_info_t*)ofile_lst[desc].p;
   //
   if(p_board_stm32wlxx_uart_info->tx_index_r==p_board_stm32wlxx_uart_info->tx_index_w){
      return 0; //all data are sent.
   }
   //
   return -1;
}

/*-------------------------------------------
| Name:dev_stm32wlxx_uart_x_read
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
int dev_stm32wlxx_uart_x_read(desc_t desc, char* buf,int size){
   board_stm32wlxx_uart_info_t * p_board_stm32wlxx_uart_info = (board_stm32wlxx_uart_info_t*)ofile_lst[desc].p;
   
   //
   uint16_t r;
   uint16_t w;
   uint16_t sz;
   uint16_t available_data_sz;
   uint8_t* p_ring_buffer;
   
   //
   r=  p_board_stm32wlxx_uart_info->rx_index_r;
   w=  p_board_stm32wlxx_uart_info->rx_index_w;
   sz =  p_board_stm32wlxx_uart_info->rx_buffer_sz;
   
   p_ring_buffer =  p_board_stm32wlxx_uart_info->p_rx_buffer;
   
   //empty or overrun
   if(r==w){
      return 0;
   }
   
   //available data
   if(r<w)
      available_data_sz=(w-r);
   else
      available_data_sz=(sz-r)+w;

   //
   if (available_data_sz == 0)
      return 0;
   
   //
   if (available_data_sz<size) {
      //copy all available data in user buffer
      size = available_data_sz; 
   }
   
   //
   if( (r+size) <= sz ){
      memcpy(buf,p_ring_buffer+r,size);
      r = ((r+size)==sz)? 0 : r+size;
   }else{
      memcpy(buf,p_ring_buffer+r,sz-r);
      memcpy(buf+(sz-r),p_ring_buffer+(sz-r),size-(sz-r));
      r=size-(sz-r);
   }
   //
   p_board_stm32wlxx_uart_info->rx_index_r=r;
   
   //
   return size;
}

/*-------------------------------------------
| Name:dev_stm32wlxx_uart_x_write
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
int dev_stm32wlxx_uart_x_write(desc_t desc, const char* buf,int size){
   int cb;
   board_stm32wlxx_uart_info_t * p_board_stm32wlxx_uart_info = (board_stm32wlxx_uart_info_t*)ofile_lst[desc].p;
   UART_HandleTypeDef* p_huart = (UART_HandleTypeDef*)(&p_board_stm32wlxx_uart_info->dev_huart);
   //to do zero copy buffer or not
   /* User start transmission data through "TxBuffer" buffer */
   if(HAL_UART_Transmit_DMA(p_huart, (uint8_t*)buf, size)!= HAL_OK){
      /* Transfer error in transmission process */
      return -1;
   }
   //
   cb=size;
   //
   return cb;
}

/*-------------------------------------------
| Name:dev_stm32wlxx_uart_x_seek
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
int dev_stm32wlxx_uart_x_seek(desc_t desc,int offset,int origin){
   return 0;
}

/*-------------------------------------------
| Name:dev_stm32wlxx_uart_x_ioctl
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
int dev_stm32wlxx_uart_x_ioctl(desc_t desc,int request,va_list ap) {
   struct termios* termios_p = (struct termios*)0;

   switch(request) {
      case TCFLSH: {
         //
         int flush_io= va_arg( ap, int);
         board_stm32wlxx_uart_info_t * p_board_stm32wlxx_uart_info = (board_stm32wlxx_uart_info_t*)ofile_lst[desc].p;
         //
         if(flush_io>2)
            return -1;
         //TCIFLUSH 0 -> 1:01
         //TCIFLUSH 1 -> 2:10
         //TCOFLUSH 2 -> 3:11
         flush_io+=1;
         //
         if( (flush_io&(TCIFLUSH+1)) ) {
            if( !(ofile_lst[desc].oflag & O_RDONLY) )
               return -1;   //not compatible open mode
            //uart_flush_rx(p_uart_descriptor);
            uint8_t c;
            int cb;
            //while((cb=uart_read(&c,1,p_uart_descriptor))>0);
         }
         if( (flush_io&(TCOFLUSH+1)) ) {
            if( !(ofile_lst[desc].oflag & O_WRONLY) )
               return -1;   //not compatible open mode
            //
            //nothing to do
         }
      }
      break;
      
      
      case TCSETS:
      case TCSAFLUSH:
      case TCSADRAIN:
      case TCSANOW:{
         termios_p = va_arg( ap, struct termios*);
         //
         board_stm32wlxx_uart_info_t * p_board_stm32wlxx_uart_info = (board_stm32wlxx_uart_info_t*)ofile_lst[desc].p;
         if(!termios_p)
            return -1;
         //          
         dev_stm32wlxx_uart_x_termios2ttys(termios_p,p_board_stm32wlxx_uart_info);
      }
      break;
      
      
      //
      case TIOCSSERIAL: {
         unsigned long speed  = va_arg( ap, unsigned long);
         board_stm32wlxx_uart_info_t * p_board_stm32wlxx_uart_info = (board_stm32wlxx_uart_info_t*)ofile_lst[desc].p;
         if(speed==0){
            return -1;
         }
         //
         //uart_set_baudrate(&p_uart_info->uart_descriptor,speed);         
      } break;

        
      //
      case I_LINK:
      case I_UNLINK:
        //nothing to do, see _vfs_ioctl2() 
        //ioctl of device driver is call with request I_LINK, I_UNLINK.
      return 0;

      //
      default:
         return -1;

   }

   return 0;
}


/*============================================
| End of Source  : dev_stm32wlxx_uart_x.c
==============================================*/
