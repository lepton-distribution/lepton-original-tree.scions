/*
 * Lepton — BSP commun des machines QEMU MPS2 (mps2-an386 Cortex-M4, mps2-an500 Cortex-M7).
 * Licence : voir LICENSE (MPL 1.1). Adresses et interruptions : <machine>/qemu_mps2_machine.h,
 * choisi par le chemin d'inclusion que pose cmake/boards/qemu-mps2-<machine>.cmake.
 * Horloge système (pas de PLL : QEMU), instances UART CMSDK ttys0 (console lsh) et ttys1,
 * Ethernet LAN9118 eth0, routage de leurs interruptions.
 */
#include <stdint.h>
#include <stdarg.h>

#include "kernel/core/kernelconf.h"
#include "kernel/core/types.h"
#include "kernel/core/interrupt.h"
#include "kernel/core/kernel.h"
#include "kernel/core/system.h"
#include "kernel/core/stat.h"
#include "kernel/fs/vfs/vfsdev.h"
#include "kernel/fs/vfs/vfstypes.h"

#include "kernel/dev/arch/all/uart/dev_cmsdk_uart/dev_cmsdk_uart_x.h"
#include "kernel/dev/arch/all/eth/dev_eth_lan9118/dev_eth_lan9118_x.h"
#include "qemu_mps2_machine.h"   /* en-tête de la machine : bsp/qemu_mps2/<machine>/ */

/* --- horloge (CMSIS system_ARMCM4.h) ------------------------------------------------------ */
uint32_t SystemCoreClock = QEMU_MPS2_SYSCLK_HZ;

void SystemInit(void){
}

void SystemCoreClockUpdate(void){
   SystemCoreClock = QEMU_MPS2_SYSCLK_HZ;
}

/* --- UART CMSDK ------------------------------------------------------------------------- */
static void qemu_mps2_uart_irq_enable(dev_cmsdk_uart_info_t* info, int enable);

static dev_cmsdk_uart_info_t qemu_mps2_uart[2] = {
   { QEMU_MPS2_UART0_BASE, QEMU_MPS2_SYSCLK_HZ, QEMU_MPS2_UART_BAUDRATE,
     qemu_mps2_uart_irq_enable },
   { QEMU_MPS2_UART1_BASE, QEMU_MPS2_SYSCLK_HZ, QEMU_MPS2_UART_BAUDRATE,
     qemu_mps2_uart_irq_enable },
};
static int qemu_mps2_uart_irq_users = 0;

static void qemu_mps2_uart_irq_enable(dev_cmsdk_uart_info_t* info, int enable){
   int irq;
   (void)info;
   if(enable) {
      if(qemu_mps2_uart_irq_users++ > 0)
         return;
   } else {
      if(--qemu_mps2_uart_irq_users > 0)
         return;
   }
   for(irq = QEMU_MPS2_UART_IRQ_FIRST; irq <= QEMU_MPS2_UART_IRQ_LAST + 1; irq++) {
      IRQn_Type n = (irq <= QEMU_MPS2_UART_IRQ_LAST) ? (IRQn_Type)irq
                                                           : (IRQn_Type)QEMU_MPS2_UART_IRQ_OVF;
      if(enable) {
         NVIC_SetPriority(n, QEMU_MPS2_UART_IRQ_PRIO);
         NVIC_ClearPendingIRQ(n);
         NVIC_EnableIRQ(n);
      } else {
         NVIC_DisableIRQ(n);
      }
   }
}

static void qemu_mps2_uart_irq(void){
   dev_cmsdk_uart_x_rx_interrupt(&qemu_mps2_uart[0]);
   dev_cmsdk_uart_x_rx_interrupt(&qemu_mps2_uart[1]);
}

void IRQ0_Handler(void)  { qemu_mps2_uart_irq(); }
void IRQ1_Handler(void)  { qemu_mps2_uart_irq(); }
void IRQ2_Handler(void)  { qemu_mps2_uart_irq(); }
void IRQ3_Handler(void)  { qemu_mps2_uart_irq(); }
void IRQ4_Handler(void)  { qemu_mps2_uart_irq(); }
void IRQ5_Handler(void)  { qemu_mps2_uart_irq(); }
void IRQ12_Handler(void) { qemu_mps2_uart_irq(); }

/* instances : ttys0, ttys1 */
#define QEMU_MPS2_UART_INSTANCE(n) \
   static int dev_cmsdk_uart_##n##_load(void){ return dev_cmsdk_uart_x_load(&qemu_mps2_uart[n]); } \
   static int dev_cmsdk_uart_##n##_open(desc_t desc, int o_flag){ \
      return dev_cmsdk_uart_x_open(desc, o_flag, &qemu_mps2_uart[n]); } \
   dev_map_t dev_cmsdk_uart_##n##_map = { \
      "ttys" #n "\0", \
      S_IFCHR, \
      dev_cmsdk_uart_##n##_load, \
      dev_cmsdk_uart_##n##_open, \
      dev_cmsdk_uart_x_close, \
      dev_cmsdk_uart_x_isset_read, \
      dev_cmsdk_uart_x_isset_write, \
      dev_cmsdk_uart_x_read, \
      dev_cmsdk_uart_x_write, \
      dev_cmsdk_uart_x_seek, \
      dev_cmsdk_uart_x_ioctl \
   };

QEMU_MPS2_UART_INSTANCE(0)
QEMU_MPS2_UART_INSTANCE(1)

/* --- Ethernet LAN9118 : eth0 ------------------------------------------------------------ */
static void qemu_mps2_eth_irq_enable(dev_eth_lan9118_info_t* info, int enable){
   IRQn_Type n = (IRQn_Type)QEMU_MPS2_ETH_IRQ;
   (void)info;
   if(enable) {
      NVIC_SetPriority(n, QEMU_MPS2_ETH_IRQ_PRIO);
      NVIC_ClearPendingIRQ(n);
      NVIC_EnableIRQ(n);
   } else {
      NVIC_DisableIRQ(n);
   }
}

static dev_eth_lan9118_info_t qemu_mps2_eth = {
   QEMU_MPS2_ETH_BASE, qemu_mps2_eth_irq_enable
};

void IRQ13_Handler(void) { dev_eth_lan9118_x_interrupt(&qemu_mps2_eth); }

static int dev_eth_lan9118_0_load(void){ return dev_eth_lan9118_x_load(&qemu_mps2_eth); }
static int dev_eth_lan9118_0_open(desc_t desc, int o_flag){
   return dev_eth_lan9118_x_open(desc, o_flag, &qemu_mps2_eth);
}

dev_map_t dev_eth_lan9118_0_map = {
   "eth0\0",
   S_IFCHR,
   dev_eth_lan9118_0_load,
   dev_eth_lan9118_0_open,
   dev_eth_lan9118_x_close,
   dev_eth_lan9118_x_isset_read,
   dev_eth_lan9118_x_isset_write,
   dev_eth_lan9118_x_read,
   dev_eth_lan9118_x_write,
   dev_eth_lan9118_x_seek,
   dev_eth_lan9118_x_ioctl
};
