/*
 * Lepton — pilote USART STM32F7 (famille STM32F7xx, étape 6). Licence : voir LICENSE (MPL 1.1).
 *
 * Nouveau : l'USART du F7 (registres ISR/ICR/RDR/TDR, RM0385 §31) diffère de celui du F4
 * (SR/DR), le pilote STM32F4 de Lepton ne s'applique pas. Écrit sur le modèle du pilote CMSDK
 * (étape 3) : émission synchrone (attente de TXE) puis signalement de fin d'émission à
 * l'écrivain ; réception par interruption (RXNE) dans un tampon circulaire, réveil du lecteur
 * par __fire_io_int. 8 bits, sans parité, 1 stop, suréchantillonnage 16.
 */
#include <stdint.h>
#include <stdarg.h>
#include <string.h>

#include "kernel/core/kernelconf.h"
#include "kernel/core/types.h"
#include "kernel/core/interrupt.h"
#include "kernel/core/kernel.h"
#include "kernel/core/system.h"
#include "kernel/core/fcntl.h"
#include "kernel/core/stat.h"
#include "kernel/fs/vfs/vfstypes.h"

#include "stm32f7xx.h"
#include "dev_stm32f7xx_uart_x.h"

#define USART(info)     ((USART_TypeDef*)(info)->base)
#define USART_ERRORS    (USART_ISR_ORE | USART_ISR_FE | USART_ISR_NE | USART_ISR_PE)
#define USART_ERRORS_CF (USART_ICR_ORECF | USART_ICR_FECF | USART_ICR_NCF | USART_ICR_PECF)

int dev_stm32f7xx_uart_x_load(dev_stm32f7xx_uart_info_t* info){
   info->desc_r = -1;
   info->desc_w = -1;
   info->rx_head = 0;
   info->rx_tail = 0;
   return 0;
}

static void _dev_stm32f7xx_uart_start(dev_stm32f7xx_uart_info_t* info){
   USART_TypeDef* u = USART(info);
   u->CR1 = 0;
   u->CR2 = 0;
   u->CR3 = 0;
   /* suréchantillonnage 16 : USARTDIV = f / débit, arrondi */
   u->BRR = (info->clock_hz + info->baudrate / 2u) / info->baudrate;
   u->ICR = USART_ERRORS_CF;
   u->CR1 = USART_CR1_TE | USART_CR1_RE | USART_CR1_RXNEIE | USART_CR1_UE;
   if(info->irq_enable)
      info->irq_enable(info, 1);
}

int dev_stm32f7xx_uart_x_open(desc_t desc, int o_flag, dev_stm32f7xx_uart_info_t* info){
   if(info->desc_r < 0 && info->desc_w < 0)
      _dev_stm32f7xx_uart_start(info);
   if(o_flag & O_RDONLY) {
      if(info->desc_r >= 0)
         return -1;
      info->desc_r = desc;
   }
   if(o_flag & O_WRONLY) {
      if(info->desc_w >= 0)
         return -1;
      info->desc_w = desc;
   }
   ofile_lst[desc].p = info;
   return 0;
}

int dev_stm32f7xx_uart_x_close(desc_t desc){
   dev_stm32f7xx_uart_info_t* info = (dev_stm32f7xx_uart_info_t*)ofile_lst[desc].p;
   if(!info)
      return -1;
   if((ofile_lst[desc].oflag & O_RDONLY) && !ofile_lst[desc].nb_reader)
      info->desc_r = -1;
   if((ofile_lst[desc].oflag & O_WRONLY) && !ofile_lst[desc].nb_writer)
      info->desc_w = -1;
   if(info->desc_r < 0 && info->desc_w < 0) {
      if(info->irq_enable)
         info->irq_enable(info, 0);
      USART(info)->CR1 = 0;
   }
   return 0;
}

int dev_stm32f7xx_uart_x_isset_read(desc_t desc){
   dev_stm32f7xx_uart_info_t* info = (dev_stm32f7xx_uart_info_t*)ofile_lst[desc].p;
   if(!info)
      return -1;
   return (info->rx_head != info->rx_tail) ? 0 : -1;
}

int dev_stm32f7xx_uart_x_isset_write(desc_t desc){
   return 0; /* émission synchrone : toujours prêt */
}

int dev_stm32f7xx_uart_x_read(desc_t desc, char* buf, int size){
   dev_stm32f7xx_uart_info_t* info = (dev_stm32f7xx_uart_info_t*)ofile_lst[desc].p;
   int n = 0;
   if(!info)
      return -1;
   while(n < size && info->rx_tail != info->rx_head) {
      buf[n++] = (char)info->rx_buffer[info->rx_tail];
      info->rx_tail = (uint16_t)((info->rx_tail + 1u) % DEV_STM32F7XX_UART_RX_BUFFER_SIZE);
   }
   return n;
}

int dev_stm32f7xx_uart_x_write(desc_t desc, const char* buf, int size){
   dev_stm32f7xx_uart_info_t* info = (dev_stm32f7xx_uart_info_t*)ofile_lst[desc].p;
   USART_TypeDef* u;
   int n;
   if(!info)
      return -1;
   u = USART(info);
   for(n = 0; n < size; n++) {
      while(!(u->ISR & USART_ISR_TXE)) {
      }
      u->TDR = (uint8_t)buf[n];
   }
   /* Contrat kernel_io (écriture synchrone) : l'écrivain attend l'interruption de fin d'émission
      (__wait_io_int). Les octets sont confiés au registre d'émission : signalement immédiat. */
   if(info->desc_w >= 0)
      __fire_io_int(ofile_lst[info->desc_w].owner_pthread_ptr_write);
   return size;
}

int dev_stm32f7xx_uart_x_seek(desc_t desc, int offset, int origin){
   return 0;
}

int dev_stm32f7xx_uart_x_ioctl(desc_t desc, int request, va_list ap){
   return 0;
}

void dev_stm32f7xx_uart_x_interrupt(dev_stm32f7xx_uart_info_t* info){
   USART_TypeDef* u = USART(info);
   int woken = 0;
   __hw_enter_interrupt();
   while(u->ISR & USART_ISR_RXNE) {
      uint8_t c = (uint8_t)u->RDR;
      uint16_t next = (uint16_t)((info->rx_head + 1u) % DEV_STM32F7XX_UART_RX_BUFFER_SIZE);
      if(next != info->rx_tail) { /* tampon plein : octet perdu */
         info->rx_buffer[info->rx_head] = c;
         info->rx_head = next;
         woken = 1;
      }
   }
   /* un débordement (ORE) maintient l'interruption tant qu'il n'est pas acquitté */
   if(u->ISR & USART_ERRORS)
      u->ICR = USART_ERRORS_CF;
   if(woken && info->desc_r >= 0)
      __fire_io_int(ofile_lst[info->desc_r].owner_pthread_ptr_read);
   __hw_leave_interrupt();
}
