/*
 * Lepton — pilote USART sur SERCOM, SAMD21 (étape 6). Licence : voir LICENSE (MPL 1.1).
 *
 * Nouveau : le pilote SAMD20 de l'arbre repose sur l'ASF (SAMD20 seulement) ; celui-ci est écrit
 * par registres (DFP Microchip SAMD21, fiche technique SAMD21 §25-26) sur le modèle du pilote
 * USART STM32F7 : émission synchrone (attente de DRE) puis signalement de fin d'émission à
 * l'écrivain ; réception par interruption (RXC) dans un tampon circulaire, réveil du lecteur
 * par __fire_io_int. 8 bits, sans parité, 1 stop, suréchantillonnage 16 arithmétique.
 * Horloges (PM, GCLK) et broches (PORT) : BSP de la carte.
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

#include "samd21j18a.h"
#include "dev_samd21_uart_x.h"

#define USART(info)     (&((sercom_registers_t*)(info)->base)->USART_INT)
#define USART_ERRORS    (SERCOM_USART_INT_STATUS_PERR_Msk | SERCOM_USART_INT_STATUS_FERR_Msk \
                         | SERCOM_USART_INT_STATUS_BUFOVF_Msk)

int dev_samd21_uart_x_load(dev_samd21_uart_info_t* info){
   info->desc_r = -1;
   info->desc_w = -1;
   info->rx_head = 0;
   info->rx_tail = 0;
   return 0;
}

static void _dev_samd21_uart_start(dev_samd21_uart_info_t* info){
   sercom_usart_int_registers_t* u = USART(info);
   /* réinitialisation logicielle (registres synchronisés : SYNCBUSY) */
   u->SERCOM_CTRLA = SERCOM_USART_INT_CTRLA_SWRST_Msk;
   while(u->SERCOM_SYNCBUSY & SERCOM_USART_INT_SYNCBUSY_SWRST_Msk) {
   }
   /* horloge interne, LSB en premier, suréchantillonnage 16 arithmétique (SAMPR = 0) */
   u->SERCOM_CTRLA = SERCOM_USART_INT_CTRLA_MODE_USART_INT_CLK | SERCOM_USART_INT_CTRLA_DORD_Msk
                   | SERCOM_USART_INT_CTRLA_RXPO(info->rxpo) | SERCOM_USART_INT_CTRLA_TXPO(info->txpo);
   /* BAUD = 65536 * (1 - 16 * débit / f) (fiche technique SAMD21, §25.6.2.3) */
   u->SERCOM_BAUD = (uint16_t)(65536u
                    - (uint32_t)(((uint64_t)65536u * 16u * info->baudrate) / info->clock_hz));
   /* 8 bits, 1 stop, sans parité */
   u->SERCOM_CTRLB = SERCOM_USART_INT_CTRLB_TXEN_Msk | SERCOM_USART_INT_CTRLB_RXEN_Msk;
   while(u->SERCOM_SYNCBUSY & SERCOM_USART_INT_SYNCBUSY_CTRLB_Msk) {
   }
   u->SERCOM_INTENSET = SERCOM_USART_INT_INTENSET_RXC_Msk;
   u->SERCOM_CTRLA |= SERCOM_USART_INT_CTRLA_ENABLE_Msk;
   while(u->SERCOM_SYNCBUSY & SERCOM_USART_INT_SYNCBUSY_ENABLE_Msk) {
   }
   if(info->irq_enable)
      info->irq_enable(info, 1);
}

int dev_samd21_uart_x_open(desc_t desc, int o_flag, dev_samd21_uart_info_t* info){
   if(info->desc_r < 0 && info->desc_w < 0)
      _dev_samd21_uart_start(info);
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

int dev_samd21_uart_x_close(desc_t desc){
   dev_samd21_uart_info_t* info = (dev_samd21_uart_info_t*)ofile_lst[desc].p;
   if(!info)
      return -1;
   if((ofile_lst[desc].oflag & O_RDONLY) && !ofile_lst[desc].nb_reader)
      info->desc_r = -1;
   if((ofile_lst[desc].oflag & O_WRONLY) && !ofile_lst[desc].nb_writer)
      info->desc_w = -1;
   if(info->desc_r < 0 && info->desc_w < 0) {
      sercom_usart_int_registers_t* u = USART(info);
      if(info->irq_enable)
         info->irq_enable(info, 0);
      u->SERCOM_CTRLA &= ~SERCOM_USART_INT_CTRLA_ENABLE_Msk;
      while(u->SERCOM_SYNCBUSY & SERCOM_USART_INT_SYNCBUSY_ENABLE_Msk) {
      }
   }
   return 0;
}

int dev_samd21_uart_x_isset_read(desc_t desc){
   dev_samd21_uart_info_t* info = (dev_samd21_uart_info_t*)ofile_lst[desc].p;
   if(!info)
      return -1;
   return (info->rx_head != info->rx_tail) ? 0 : -1;
}

int dev_samd21_uart_x_isset_write(desc_t desc){
   return 0; /* émission synchrone : toujours prêt */
}

int dev_samd21_uart_x_read(desc_t desc, char* buf, int size){
   dev_samd21_uart_info_t* info = (dev_samd21_uart_info_t*)ofile_lst[desc].p;
   int n = 0;
   if(!info)
      return -1;
   while(n < size && info->rx_tail != info->rx_head) {
      buf[n++] = (char)info->rx_buffer[info->rx_tail];
      info->rx_tail = (uint16_t)((info->rx_tail + 1u) % DEV_SAMD21_UART_RX_BUFFER_SIZE);
   }
   return n;
}

int dev_samd21_uart_x_write(desc_t desc, const char* buf, int size){
   dev_samd21_uart_info_t* info = (dev_samd21_uart_info_t*)ofile_lst[desc].p;
   sercom_usart_int_registers_t* u;
   int n;
   if(!info)
      return -1;
   u = USART(info);
   for(n = 0; n < size; n++) {
      while(!(u->SERCOM_INTFLAG & SERCOM_USART_INT_INTFLAG_DRE_Msk)) {
      }
      u->SERCOM_DATA = (uint8_t)buf[n];
   }
   /* Contrat kernel_io (écriture synchrone) : l'écrivain attend l'interruption de fin d'émission
      (__wait_io_int). Les octets sont confiés au registre d'émission : signalement immédiat. */
   if(info->desc_w >= 0)
      __fire_io_int(ofile_lst[info->desc_w].owner_pthread_ptr_write);
   return size;
}

int dev_samd21_uart_x_seek(desc_t desc, int offset, int origin){
   return 0;
}

int dev_samd21_uart_x_ioctl(desc_t desc, int request, va_list ap){
   return 0;
}

void dev_samd21_uart_x_interrupt(dev_samd21_uart_info_t* info){
   sercom_usart_int_registers_t* u = USART(info);
   int woken = 0;
   __hw_enter_interrupt();
   while(u->SERCOM_INTFLAG & SERCOM_USART_INT_INTFLAG_RXC_Msk) {
      /* erreurs de l'octet en tête (STATUS) acquittées par écriture de 1 ; la lecture de DATA
         efface RXC */
      uint16_t status = u->SERCOM_STATUS;
      uint8_t c = (uint8_t)u->SERCOM_DATA;
      uint16_t next = (uint16_t)((info->rx_head + 1u) % DEV_SAMD21_UART_RX_BUFFER_SIZE);
      if(status & USART_ERRORS)
         u->SERCOM_STATUS = (uint16_t)(status & USART_ERRORS);
      if(next != info->rx_tail) { /* tampon plein : octet perdu */
         info->rx_buffer[info->rx_head] = c;
         info->rx_head = next;
         woken = 1;
      }
   }
   if(woken && info->desc_r >= 0)
      __fire_io_int(ofile_lst[info->desc_r].owner_pthread_ptr_read);
   __hw_leave_interrupt();
}
