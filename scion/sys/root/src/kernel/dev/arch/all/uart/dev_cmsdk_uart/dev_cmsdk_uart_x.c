/*
 * Lepton — pilote UART ARM CMSDK APB (MPS2, QEMU mps2-an38x). Licence : voir LICENSE (MPL 1.1).
 *
 * Nouveau (étape 3, absent de l'arbre). Registres (ARM CMSDK, DDI 0479) :
 *   DATA 0x00, STATE 0x04 (b0 TX plein, b1 RX plein, b2/b3 débordements TX/RX),
 *   CTRL 0x08 (b0 TX, b1 RX, b2 int TX, b3 int RX, b5 int débordement RX),
 *   INTSTATUS/INTCLEAR 0x0C (b0 TX, b1 RX, b2/b3 débordements), BAUDDIV 0x10 (>= 16).
 * Émission synchrone (attente de TX non plein) puis signalement de fin d'émission à l'écrivain ;
 * réception par interruption dans un tampon circulaire, réveil du lecteur par __fire_io_int
 * (modèle des pilotes série Lepton).
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

#include "dev_cmsdk_uart_x.h"

#define REG(info, off)      (*(volatile uint32_t*)((info)->base + (off)))
#define UART_DATA           0x00u
#define UART_STATE          0x04u
#define UART_CTRL           0x08u
#define UART_INTSTATUS      0x0Cu
#define UART_BAUDDIV        0x10u

#define STATE_TX_FULL       (1u << 0)
#define STATE_RX_FULL       (1u << 1)
#define STATE_TX_OVERRUN    (1u << 2)
#define STATE_RX_OVERRUN    (1u << 3)
#define CTRL_TX_EN          (1u << 0)
#define CTRL_RX_EN          (1u << 1)
#define CTRL_RX_INT_EN      (1u << 3)
#define CTRL_RX_OVR_INT_EN  (1u << 5)
#define INT_RX              (1u << 1)
#define INT_RX_OVERRUN      (1u << 3)

int dev_cmsdk_uart_x_load(dev_cmsdk_uart_info_t* info){
   info->desc_r = -1;
   info->desc_w = -1;
   info->rx_head = 0;
   info->rx_tail = 0;
   return 0;
}

static void _dev_cmsdk_uart_start(dev_cmsdk_uart_info_t* info){
   uint32_t div = info->clock_hz / info->baudrate;
   if(div < 16u)
      div = 16u;
   REG(info, UART_CTRL) = 0;
   REG(info, UART_BAUDDIV) = div;
   REG(info, UART_INTSTATUS) = INT_RX | INT_RX_OVERRUN;
   REG(info, UART_STATE) = STATE_TX_OVERRUN | STATE_RX_OVERRUN;
   REG(info, UART_CTRL) = CTRL_TX_EN | CTRL_RX_EN | CTRL_RX_INT_EN | CTRL_RX_OVR_INT_EN;
   if(info->irq_enable)
      info->irq_enable(info, 1);
}

int dev_cmsdk_uart_x_open(desc_t desc, int o_flag, dev_cmsdk_uart_info_t* info){
   if(info->desc_r < 0 && info->desc_w < 0)
      _dev_cmsdk_uart_start(info);
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

int dev_cmsdk_uart_x_close(desc_t desc){
   dev_cmsdk_uart_info_t* info = (dev_cmsdk_uart_info_t*)ofile_lst[desc].p;
   if(!info)
      return -1;
   if((ofile_lst[desc].oflag & O_RDONLY) && !ofile_lst[desc].nb_reader)
      info->desc_r = -1;
   if((ofile_lst[desc].oflag & O_WRONLY) && !ofile_lst[desc].nb_writer)
      info->desc_w = -1;
   if(info->desc_r < 0 && info->desc_w < 0) {
      if(info->irq_enable)
         info->irq_enable(info, 0);
      REG(info, UART_CTRL) = 0;
   }
   return 0;
}

int dev_cmsdk_uart_x_isset_read(desc_t desc){
   dev_cmsdk_uart_info_t* info = (dev_cmsdk_uart_info_t*)ofile_lst[desc].p;
   if(!info)
      return -1;
   return (info->rx_head != info->rx_tail) ? 0 : -1;
}

int dev_cmsdk_uart_x_isset_write(desc_t desc){
   return 0; /* émission synchrone : toujours prêt */
}

int dev_cmsdk_uart_x_read(desc_t desc, char* buf, int size){
   dev_cmsdk_uart_info_t* info = (dev_cmsdk_uart_info_t*)ofile_lst[desc].p;
   int n = 0;
   if(!info)
      return -1;
   while(n < size && info->rx_tail != info->rx_head) {
      buf[n++] = (char)info->rx_buffer[info->rx_tail];
      info->rx_tail = (uint16_t)((info->rx_tail + 1u) % DEV_CMSDK_UART_RX_BUFFER_SIZE);
   }
   return n;
}

int dev_cmsdk_uart_x_write(desc_t desc, const char* buf, int size){
   dev_cmsdk_uart_info_t* info = (dev_cmsdk_uart_info_t*)ofile_lst[desc].p;
   int n;
   if(!info)
      return -1;
   for(n = 0; n < size; n++) {
      while(REG(info, UART_STATE) & STATE_TX_FULL) {
      }
      REG(info, UART_DATA) = (uint8_t)buf[n];
   }
   /* Contrat kernel_io (écriture synchrone) : l'écrivain attend l'interruption de fin d'émission
      (__wait_io_int). L'émission est déjà terminée : signalement immédiat. */
   if(info->desc_w >= 0)
      __fire_io_int(ofile_lst[info->desc_w].owner_pthread_ptr_write);
   return size;
}

int dev_cmsdk_uart_x_seek(desc_t desc, int offset, int origin){
   return 0;
}

int dev_cmsdk_uart_x_ioctl(desc_t desc, int request, va_list ap){
   return 0;
}

void dev_cmsdk_uart_x_rx_interrupt(dev_cmsdk_uart_info_t* info){
   int woken = 0;
   __hw_enter_interrupt();
   while(REG(info, UART_STATE) & STATE_RX_FULL) {
      uint8_t c = (uint8_t)REG(info, UART_DATA);
      uint16_t next = (uint16_t)((info->rx_head + 1u) % DEV_CMSDK_UART_RX_BUFFER_SIZE);
      if(next != info->rx_tail) { /* tampon plein : octet perdu */
         info->rx_buffer[info->rx_head] = c;
         info->rx_head = next;
         woken = 1;
      }
   }
   REG(info, UART_STATE) = STATE_RX_OVERRUN;
   REG(info, UART_INTSTATUS) = INT_RX | INT_RX_OVERRUN;
   if(woken && info->desc_r >= 0)
      __fire_io_int(ofile_lst[info->desc_r].owner_pthread_ptr_read);
   __hw_leave_interrupt();
}
