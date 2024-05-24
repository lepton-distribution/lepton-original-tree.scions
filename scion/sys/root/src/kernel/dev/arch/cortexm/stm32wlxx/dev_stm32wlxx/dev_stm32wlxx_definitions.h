#ifndef __DEV_STM32WLXX_DEFINITIONS_H__
#define __DEV_STM32WLXX_DEFINITIONS_H__


/*============================================
| Includes
==============================================*/


/*============================================
| Declaration
==============================================*/

typedef struct dev_uart_handle_typedef_st{
   UART_HandleTypeDef huart;
   void* p_dev_information;
}dev_uart_handle_typedef_t;

typedef struct dev_dma_handle_typedef_st{
   DMA_HandleTypeDef  hdma;
   void* p_dev_information;
}dev_dma_handle_typedef_t;


#endif //__DEV_STM32WLXX_DEFINITIONS_H__