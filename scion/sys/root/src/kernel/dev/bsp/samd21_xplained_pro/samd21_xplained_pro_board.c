/*
 * Lepton — BSP de la carte SAMD21 Xplained Pro (ATSAMD21J18A, Cortex-M0+ r0p1, étape 6).
 * Licence : voir LICENSE (MPL 1.1).
 *
 * Horloges (SystemInit, appelée par le démarrage générique kernel/core/arch/cortexm après
 * l'initialisation de .data et .bss), console /dev/ttys3 (SERCOM3, port série virtuel de l'EDBG)
 * sur le pilote USART SAMD21 de Lepton. Valeurs : samd21_xplained_pro.h.
 * Interruptions : IRQ<n>_Handler (n < 32) de la table générique ARMv6-M ; SERCOM3_IRQn = 12.
 */
#include <stdint.h>
#include <stdarg.h>

#include "kernel/core/kernelconf.h"
#include "kernel/core/types.h"
#include "kernel/core/interrupt.h"
#include "kernel/core/kernel.h"
#include "kernel/core/system.h"
#include "kernel/core/fcntl.h"
#include "kernel/core/stat.h"
#include "kernel/fs/vfs/vfsdev.h"
#include "kernel/fs/vfs/vfstypes.h"

#include "samd21j18a.h"
#include "dev_samd21_uart_x.h"
#include "samd21_xplained_pro.h"

/* --- horloges --------------------------------------------------------------------------------- */
uint32_t SystemCoreClock = 1000000u;   /* OSC8M / 8 après reset */
/* étape de SystemInit en échec (lisible au débogueur, palier 1) : 0 aucune, 1 XOSC32K,
   2 DFLL prête, 3 verrouillage de la DFLL */
volatile uint32_t samd21_xplained_pro_clock_error;

#define SAMD21_XPLAINED_PRO_WAIT_LOOPS 2000000u

static void _samd21_xplained_pro_wait(volatile const uint32_t* reg, uint32_t mask, uint32_t step){
   uint32_t n;
   for(n = 0; n < SAMD21_XPLAINED_PRO_WAIT_LOOPS; n++) {
      if((*reg & mask) == mask)
         return;
   }
   /* horloge défaillante : arrêt visible (le débit série serait faux sur OSC8M) */
   samd21_xplained_pro_clock_error = step;
   for(;;) {
   }
}

static void _samd21_xplained_pro_gclk_sync(void){
   while(GCLK_REGS->GCLK_STATUS & GCLK_STATUS_SYNCBUSY_Msk) {
   }
}

static void _samd21_xplained_pro_dfll_sync(void){
   _samd21_xplained_pro_wait(&SYSCTRL_REGS->SYSCTRL_PCLKSR, SYSCTRL_PCLKSR_DFLLRDY_Msk, 2);
}

void SystemInit(void){
   /* états d'attente avant d'augmenter la fréquence */
   NVMCTRL_REGS->NVMCTRL_CTRLB = (NVMCTRL_REGS->NVMCTRL_CTRLB & ~NVMCTRL_CTRLB_RWS_Msk)
                               | NVMCTRL_CTRLB_RWS(SAMD21_XPLAINED_PRO_NVM_RWS);

   /* XOSC32K : quartz, sortie 32 kHz, démarrage 6 (65 536 cycles), puis activation */
   SYSCTRL_REGS->SYSCTRL_XOSC32K = SYSCTRL_XOSC32K_STARTUP(6u) | SYSCTRL_XOSC32K_XTALEN_Msk
                                 | SYSCTRL_XOSC32K_EN32K_Msk;
   SYSCTRL_REGS->SYSCTRL_XOSC32K |= SYSCTRL_XOSC32K_ENABLE_Msk;
   _samd21_xplained_pro_wait(&SYSCTRL_REGS->SYSCTRL_PCLKSR, SYSCTRL_PCLKSR_XOSC32KRDY_Msk, 1);

   /* GCLK1 = XOSC32K, référence de la DFLL */
   GCLK_REGS->GCLK_GENDIV = GCLK_GENDIV_ID(1u) | GCLK_GENDIV_DIV(0u);
   _samd21_xplained_pro_gclk_sync();
   GCLK_REGS->GCLK_GENCTRL = GCLK_GENCTRL_ID(1u) | GCLK_GENCTRL_SRC_XOSC32K | GCLK_GENCTRL_GENEN_Msk;
   _samd21_xplained_pro_gclk_sync();
   GCLK_REGS->GCLK_CLKCTRL = GCLK_CLKCTRL_ID_DFLL48 | GCLK_CLKCTRL_GEN_GCLK1 | GCLK_CLKCTRL_CLKEN_Msk;
   _samd21_xplained_pro_gclk_sync();

   /* DFLL48M, séquence de l'ASF (clock_samd21_r21_da_ha1/clock.c,
      _system_clock_source_dfll_set_config_errata_9905) : ONDEMAND levé pendant l'écriture de la
      configuration, multiplicateur et pas maximaux de recherche, DFLLCTRL remis à 0 puis écrit
      en entier (boucle fermée, attente du verrouillage, verrouillage rapide désactivé, activée) */
   SYSCTRL_REGS->SYSCTRL_DFLLCTRL = SYSCTRL_DFLLCTRL_ENABLE_Msk;
   _samd21_xplained_pro_dfll_sync();
   SYSCTRL_REGS->SYSCTRL_DFLLMUL = SYSCTRL_DFLLMUL_CSTEP(31u) | SYSCTRL_DFLLMUL_FSTEP(511u)
                                 | SYSCTRL_DFLLMUL_MUL(SAMD21_XPLAINED_PRO_DFLL_MUL);
   SYSCTRL_REGS->SYSCTRL_DFLLCTRL = 0;
   _samd21_xplained_pro_dfll_sync();
   SYSCTRL_REGS->SYSCTRL_DFLLCTRL = SYSCTRL_DFLLCTRL_MODE_Msk | SYSCTRL_DFLLCTRL_WAITLOCK_Msk
                                  | SYSCTRL_DFLLCTRL_QLDIS_Msk | SYSCTRL_DFLLCTRL_ENABLE_Msk;
   _samd21_xplained_pro_wait(&SYSCTRL_REGS->SYSCTRL_PCLKSR,
                             SYSCTRL_PCLKSR_DFLLLCKC_Msk | SYSCTRL_PCLKSR_DFLLLCKF_Msk, 3);
   _samd21_xplained_pro_dfll_sync();

   /* GCLK0 (CPU, bus) = DFLL48M ; diviseurs CPU et APB du PM à 1 (valeurs de reset) */
   GCLK_REGS->GCLK_GENDIV = GCLK_GENDIV_ID(0u) | GCLK_GENDIV_DIV(0u);
   _samd21_xplained_pro_gclk_sync();
   GCLK_REGS->GCLK_GENCTRL = GCLK_GENCTRL_ID(0u) | GCLK_GENCTRL_SRC_DFLL48M | GCLK_GENCTRL_IDC_Msk
                           | GCLK_GENCTRL_GENEN_Msk;
   _samd21_xplained_pro_gclk_sync();
   SystemCoreClock = SAMD21_XPLAINED_PRO_SYSCLK_HZ;
}

void SystemCoreClockUpdate(void){
   /* seules sources possibles ici : DFLL48M réglée par SystemInit, ou OSC8M / 8 (reset) */
   SystemCoreClock = samd21_xplained_pro_clock_error == 0u
                     && (SYSCTRL_REGS->SYSCTRL_DFLLCTRL & SYSCTRL_DFLLCTRL_ENABLE_Msk)
                     ? SAMD21_XPLAINED_PRO_SYSCLK_HZ : 1000000u;
}

/* --- console : SERCOM3 (PA22 TX PAD0, PA23 RX PAD1, fonction C) ------------------------------- */
static void samd21_xplained_pro_uart_irq_enable(dev_samd21_uart_info_t* info, int enable){
   (void)info;
   if(enable) {
      NVIC_SetPriority(SERCOM3_IRQn, SAMD21_XPLAINED_PRO_IRQ_PRIO);
      NVIC_ClearPendingIRQ(SERCOM3_IRQn);
      NVIC_EnableIRQ(SERCOM3_IRQn);
   } else {
      NVIC_DisableIRQ(SERCOM3_IRQn);
   }
}

static dev_samd21_uart_info_t samd21_xplained_pro_uart3 = {
   (uint32_t)SERCOM3_REGS, SAMD21_XPLAINED_PRO_SYSCLK_HZ, SAMD21_XPLAINED_PRO_CONSOLE_BAUDRATE,
   1u /* RXPO : PAD1 */, 0u /* TXPO : PAD0 */,
   samd21_xplained_pro_uart_irq_enable
};

void IRQ12_Handler(void) { dev_samd21_uart_x_interrupt(&samd21_xplained_pro_uart3); }

static int dev_samd21_xplained_pro_uart_3_load(void){
   /* horloge du bus (PM) et horloge du noyau SERCOM3 (GCLK0) */
   PM_REGS->PM_APBCMASK |= PM_APBCMASK_SERCOM3_Msk;
   GCLK_REGS->GCLK_CLKCTRL = GCLK_CLKCTRL_ID_SERCOM3_CORE | GCLK_CLKCTRL_GEN_GCLK0
                           | GCLK_CLKCTRL_CLKEN_Msk;
   _samd21_xplained_pro_gclk_sync();
   /* PA22 (pair : PMUXE) et PA23 (impair : PMUXO) sur la fonction C */
   PORT_REGS->GROUP[0].PORT_PMUX[22u / 2u] = PORT_PMUX_PMUXE(MUX_PA22C_SERCOM3_PAD0)
                                           | PORT_PMUX_PMUXO(MUX_PA23C_SERCOM3_PAD1);
   PORT_REGS->GROUP[0].PORT_PINCFG[22] = PORT_PINCFG_PMUXEN_Msk;
   PORT_REGS->GROUP[0].PORT_PINCFG[23] = PORT_PINCFG_PMUXEN_Msk | PORT_PINCFG_INEN_Msk;
   return dev_samd21_uart_x_load(&samd21_xplained_pro_uart3);
}

static int dev_samd21_xplained_pro_uart_3_open(desc_t desc, int o_flag){
   return dev_samd21_uart_x_open(desc, o_flag, &samd21_xplained_pro_uart3);
}

dev_map_t dev_samd21_xplained_pro_uart_3_map = {
   "ttys3\0",
   S_IFCHR,
   dev_samd21_xplained_pro_uart_3_load,
   dev_samd21_xplained_pro_uart_3_open,
   dev_samd21_uart_x_close,
   dev_samd21_uart_x_isset_read,
   dev_samd21_uart_x_isset_write,
   dev_samd21_uart_x_read,
   dev_samd21_uart_x_write,
   dev_samd21_uart_x_seek,
   dev_samd21_uart_x_ioctl
};
