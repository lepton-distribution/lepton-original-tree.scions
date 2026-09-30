/*
 * Lepton — pilote UART ARM CMSDK APB (MPS2, QEMU mps2-an38x). Licence : voir LICENSE (MPL 1.1).
 * Partie commune ; chaque instance (adresse, interruption) est déclarée par le BSP de la carte.
 */
#ifndef _DEV_CMSDK_UART_X_H_
#define _DEV_CMSDK_UART_X_H_

#include <stdint.h>

#define DEV_CMSDK_UART_RX_BUFFER_SIZE 256

typedef struct dev_cmsdk_uart_info_st {
   uint32_t base;          /* adresse du bloc APB UART (BSP) */
   uint32_t clock_hz;      /* horloge APB */
   uint32_t baudrate;
   void (*irq_enable)(struct dev_cmsdk_uart_info_st* info, int enable); /* NVIC : BSP */
   /* état */
   desc_t desc_r;
   desc_t desc_w;
   volatile uint16_t rx_head;
   volatile uint16_t rx_tail;
   uint8_t rx_buffer[DEV_CMSDK_UART_RX_BUFFER_SIZE];
} dev_cmsdk_uart_info_t;

int dev_cmsdk_uart_x_load(dev_cmsdk_uart_info_t* info);
int dev_cmsdk_uart_x_open(desc_t desc, int o_flag, dev_cmsdk_uart_info_t* info);
int dev_cmsdk_uart_x_close(desc_t desc);
int dev_cmsdk_uart_x_isset_read(desc_t desc);
int dev_cmsdk_uart_x_isset_write(desc_t desc);
int dev_cmsdk_uart_x_read(desc_t desc, char* buf, int size);
int dev_cmsdk_uart_x_write(desc_t desc, const char* buf, int size);
int dev_cmsdk_uart_x_seek(desc_t desc, int offset, int origin);
int dev_cmsdk_uart_x_ioctl(desc_t desc, int request, va_list ap);
/* à appeler depuis le gestionnaire d'interruption RX de l'instance (BSP) */
void dev_cmsdk_uart_x_rx_interrupt(dev_cmsdk_uart_info_t* info);

#endif
