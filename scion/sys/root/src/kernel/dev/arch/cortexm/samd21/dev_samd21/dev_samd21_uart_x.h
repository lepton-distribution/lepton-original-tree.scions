/*
 * Lepton — pilote USART sur SERCOM, SAMD21 (étape 6). Licence : voir LICENSE (MPL 1.1).
 * Partie commune ; chaque instance (SERCOM, horloge, broches, interruption) est déclarée par le
 * BSP de la carte.
 */
#ifndef _DEV_SAMD21_UART_X_H_
#define _DEV_SAMD21_UART_X_H_

#include <stdint.h>

/* tampon de réception réduit : 32 Ko de RAM sur la SAMD21J18A */
#define DEV_SAMD21_UART_RX_BUFFER_SIZE 64

typedef struct dev_samd21_uart_info_st {
   uint32_t base;          /* adresse du bloc SERCOM (BSP) */
   uint32_t clock_hz;      /* horloge GCLK_SERCOMx_CORE */
   uint32_t baudrate;
   uint8_t rxpo;           /* CTRLA.RXPO : pad de réception */
   uint8_t txpo;           /* CTRLA.TXPO : pad d'émission */
   void (*irq_enable)(struct dev_samd21_uart_info_st* info, int enable); /* NVIC : BSP */
   /* état */
   desc_t desc_r;
   desc_t desc_w;
   volatile uint16_t rx_head;
   volatile uint16_t rx_tail;
   uint8_t rx_buffer[DEV_SAMD21_UART_RX_BUFFER_SIZE];
} dev_samd21_uart_info_t;

int dev_samd21_uart_x_load(dev_samd21_uart_info_t* info);
int dev_samd21_uart_x_open(desc_t desc, int o_flag, dev_samd21_uart_info_t* info);
int dev_samd21_uart_x_close(desc_t desc);
int dev_samd21_uart_x_isset_read(desc_t desc);
int dev_samd21_uart_x_isset_write(desc_t desc);
int dev_samd21_uart_x_read(desc_t desc, char* buf, int size);
int dev_samd21_uart_x_write(desc_t desc, const char* buf, int size);
int dev_samd21_uart_x_seek(desc_t desc, int offset, int origin);
int dev_samd21_uart_x_ioctl(desc_t desc, int request, va_list ap);
/* à appeler depuis le gestionnaire d'interruption de l'instance (BSP) */
void dev_samd21_uart_x_interrupt(dev_samd21_uart_info_t* info);

#endif
