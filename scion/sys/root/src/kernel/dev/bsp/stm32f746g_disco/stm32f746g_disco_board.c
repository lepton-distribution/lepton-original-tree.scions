/*
 * Lepton — BSP de la carte STM32F746G-DISCO (STM32F746NG, Cortex-M7 r0p1, étape 6).
 * Licence : voir LICENSE (MPL 1.1).
 *
 * Horloges et caches (SystemInit, appelée par le démarrage générique kernel/core/arch/cortexm
 * après l'initialisation de .data et .bss), console /dev/ttys1 (USART1, port série virtuel du
 * ST-LINK) sur le pilote USART STM32F7 de Lepton, /dev/eth0 (Ethernet RMII, PHY LAN8742A) sur le
 * pilote Ethernet STM32F7. Valeurs : stm32f746g_disco.h.
 * Interruptions : IRQ<n>_Handler (n < 64) de la table générique ; USART1_IRQn = 37,
 * ETH_IRQn = 61 (RM0385).
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

#include "stm32f7xx_hal.h"
#include "kernel/core/ioctl_eth.h"
#include "dev_stm32f7xx_uart_x.h"
#include "dev_stm32f7xx_eth_x.h"
#include "stm32f746g_disco.h"

/* --- horloges et caches ----------------------------------------------------------------------- */
uint32_t SystemCoreClock = 16000000u;   /* HSI après reset */
/* étape de SystemInit en échec (lisible au débogueur, palier 1) : 0 aucune, 1 HSE, 2 PLL,
   3 over-drive, 4 commutation over-drive, 5 latence flash, 6 commutation sur la PLL */
volatile uint32_t stm32f746g_disco_clock_error;

#define STM32F746G_DISCO_WAIT_LOOPS 2000000u

static void _stm32f746g_disco_wait(volatile uint32_t* reg, uint32_t mask, uint32_t value,
                                   uint32_t step){
   uint32_t n;
   for(n = 0; n < STM32F746G_DISCO_WAIT_LOOPS; n++) {
      if((*reg & mask) == value)
         return;
   }
   /* horloge défaillante : arrêt visible (le débit série serait faux sur HSI) */
   stm32f746g_disco_clock_error = step;
   for(;;) {
   }
}

void SystemInit(void){
   /* tension du régulateur : échelle 1 (requise pour 216 MHz et l'over-drive) */
   __HAL_RCC_PWR_CLK_ENABLE();
   PWR->CR1 |= PWR_CR1_VOS;

   RCC->CR |= RCC_CR_HSEON;
   _stm32f746g_disco_wait(&RCC->CR, RCC_CR_HSERDY, RCC_CR_HSERDY, 1);

   RCC->CR &= ~RCC_CR_PLLON;
   _stm32f746g_disco_wait(&RCC->CR, RCC_CR_PLLRDY, 0, 2);
   RCC->PLLCFGR = (STM32F746G_DISCO_PLL_M << RCC_PLLCFGR_PLLM_Pos)
                | (STM32F746G_DISCO_PLL_N << RCC_PLLCFGR_PLLN_Pos)
                | (((STM32F746G_DISCO_PLL_P / 2u) - 1u) << RCC_PLLCFGR_PLLP_Pos)
                | RCC_PLLCFGR_PLLSRC_HSE
                | (STM32F746G_DISCO_PLL_Q << RCC_PLLCFGR_PLLQ_Pos);
   RCC->CR |= RCC_CR_PLLON;
   _stm32f746g_disco_wait(&RCC->CR, RCC_CR_PLLRDY, RCC_CR_PLLRDY, 2);

   /* over-drive (RM0385 §4.1.4) : activation, puis commutation */
   PWR->CR1 |= PWR_CR1_ODEN;
   _stm32f746g_disco_wait(&PWR->CSR1, PWR_CSR1_ODRDY, PWR_CSR1_ODRDY, 3);
   PWR->CR1 |= PWR_CR1_ODSWEN;
   _stm32f746g_disco_wait(&PWR->CSR1, PWR_CSR1_ODSWRDY, PWR_CSR1_ODSWRDY, 4);

   /* états d'attente avant d'augmenter la fréquence ; accélérateur ART et prefetch (ITCM) */
   FLASH->ACR = (STM32F746G_DISCO_FLASH_LATENCY << FLASH_ACR_LATENCY_Pos)
              | FLASH_ACR_PRFTEN | FLASH_ACR_ARTEN;
   _stm32f746g_disco_wait(&FLASH->ACR, FLASH_ACR_LATENCY,
                          STM32F746G_DISCO_FLASH_LATENCY << FLASH_ACR_LATENCY_Pos, 5);

   RCC->CFGR = (RCC->CFGR & ~(RCC_CFGR_HPRE | RCC_CFGR_PPRE1 | RCC_CFGR_PPRE2 | RCC_CFGR_SW))
             | RCC_CFGR_HPRE_DIV1 | RCC_CFGR_PPRE1_DIV4 | RCC_CFGR_PPRE2_DIV2 | RCC_CFGR_SW_PLL;
   _stm32f746g_disco_wait(&RCC->CFGR, RCC_CFGR_SWS, RCC_CFGR_SWS_PLL, 6);
   SystemCoreClock = STM32F746G_DISCO_SYSCLK_HZ;

   /* caches L1 du Cortex-M7 (CMSIS-Core) ; aucune DMA à ce palier (Ethernet : session suivante,
      zone non cachée par la MPU) */
   SCB_EnableICache();
   SCB_EnableDCache();
}

void SystemCoreClockUpdate(void){
   /* seules sources possibles ici : PLL réglée par SystemInit, ou HSI (reset) */
   SystemCoreClock = ((RCC->CFGR & RCC_CFGR_SWS) == RCC_CFGR_SWS_PLL) ? STM32F746G_DISCO_SYSCLK_HZ
                                                                      : 16000000u;
}

/* --- console : USART1 (PA9 TX, PB7 RX, AF7) --------------------------------------------------- */
static void stm32f746g_disco_uart_irq_enable(dev_stm32f7xx_uart_info_t* info, int enable){
   (void)info;
   if(enable) {
      NVIC_SetPriority(USART1_IRQn, STM32F746G_DISCO_IRQ_PRIO);
      NVIC_ClearPendingIRQ(USART1_IRQn);
      NVIC_EnableIRQ(USART1_IRQn);
   } else {
      NVIC_DisableIRQ(USART1_IRQn);
   }
}

static dev_stm32f7xx_uart_info_t stm32f746g_disco_uart1 = {
   USART1_BASE, STM32F746G_DISCO_PCLK2_HZ, STM32F746G_DISCO_CONSOLE_BAUDRATE,
   stm32f746g_disco_uart_irq_enable
};

void IRQ37_Handler(void) { dev_stm32f7xx_uart_x_interrupt(&stm32f746g_disco_uart1); }

static int dev_stm32f746g_disco_uart_1_load(void){
   GPIO_InitTypeDef gpio;
   __HAL_RCC_GPIOA_CLK_ENABLE();
   __HAL_RCC_GPIOB_CLK_ENABLE();
   __HAL_RCC_USART1_CLK_ENABLE();   /* noyau USART1 sur PCLK2 (RCC_DCKCFGR2, valeur de reset) */
   gpio.Mode = GPIO_MODE_AF_PP;
   gpio.Pull = GPIO_PULLUP;
   gpio.Speed = GPIO_SPEED_FREQ_HIGH;
   gpio.Alternate = GPIO_AF7_USART1;
   gpio.Pin = GPIO_PIN_9;
   HAL_GPIO_Init(GPIOA, &gpio);
   gpio.Pin = GPIO_PIN_7;
   HAL_GPIO_Init(GPIOB, &gpio);
   return dev_stm32f7xx_uart_x_load(&stm32f746g_disco_uart1);
}

static int dev_stm32f746g_disco_uart_1_open(desc_t desc, int o_flag){
   return dev_stm32f7xx_uart_x_open(desc, o_flag, &stm32f746g_disco_uart1);
}

dev_map_t dev_stm32f746g_disco_uart_1_map = {
   "ttys1\0",
   S_IFCHR,
   dev_stm32f746g_disco_uart_1_load,
   dev_stm32f746g_disco_uart_1_open,
   dev_stm32f7xx_uart_x_close,
   dev_stm32f7xx_uart_x_isset_read,
   dev_stm32f7xx_uart_x_isset_write,
   dev_stm32f7xx_uart_x_read,
   dev_stm32f7xx_uart_x_write,
   dev_stm32f7xx_uart_x_seek,
   dev_stm32f7xx_uart_x_ioctl
};

/* --- Ethernet : eth0 (RMII, PHY LAN8742A) ------------------------------------------------------ */
/* Broches RMII (exemple LwIP STM32CubeF7 v1.17.4 de la carte, AF11) : PA1 REF_CLK, PA2 MDIO,
   PA7 CRS_DV, PC1 MDC, PC4 RXD0, PC5 RXD1, PG2 RXER, PG11 TX_EN, PG13 TXD0, PG14 TXD1 */
void HAL_ETH_MspInit(ETH_HandleTypeDef* heth){
   GPIO_InitTypeDef gpio;
   (void)heth;
   __HAL_RCC_GPIOA_CLK_ENABLE();
   __HAL_RCC_GPIOC_CLK_ENABLE();
   __HAL_RCC_GPIOG_CLK_ENABLE();
   gpio.Mode = GPIO_MODE_AF_PP;
   gpio.Pull = GPIO_NOPULL;
   gpio.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
   gpio.Alternate = GPIO_AF11_ETH;
   gpio.Pin = GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_7;
   HAL_GPIO_Init(GPIOA, &gpio);
   gpio.Pin = GPIO_PIN_1 | GPIO_PIN_4 | GPIO_PIN_5;
   HAL_GPIO_Init(GPIOC, &gpio);
   gpio.Pin = GPIO_PIN_2 | GPIO_PIN_11 | GPIO_PIN_13 | GPIO_PIN_14;
   HAL_GPIO_Init(GPIOG, &gpio);
   __HAL_RCC_ETHMAC_CLK_ENABLE();
   __HAL_RCC_ETHMACTX_CLK_ENABLE();
   __HAL_RCC_ETHMACRX_CLK_ENABLE();
}

static void stm32f746g_disco_eth_irq_enable(dev_stm32f7xx_eth_info_t* info, int enable){
   (void)info;
   if(enable) {
      NVIC_SetPriority(ETH_IRQn, STM32F746G_DISCO_IRQ_PRIO);
      NVIC_ClearPendingIRQ(ETH_IRQn);
      NVIC_EnableIRQ(ETH_IRQn);
   } else {
      NVIC_DisableIRQ(ETH_IRQn);
   }
}

static dev_stm32f7xx_eth_info_t stm32f746g_disco_eth = {
   BOARD_ETH_PHY_ADDR, BOARD_ETH_PHY_SR, BOARD_ETH_PHY_DUPLEX_SPEED_MASK,
   BOARD_ETH_PHY_100BTX_FULL, BOARD_ETH_PHY_100BTX_HALF, BOARD_ETH_PHY_10M_FULL,
   BOARD_ETH_PHY_10M_HALF, stm32f746g_disco_eth_irq_enable
};

void IRQ61_Handler(void) { dev_stm32f7xx_eth_x_interrupt(&stm32f746g_disco_eth); }

static int dev_stm32f746g_disco_eth_0_load(void){
   return dev_stm32f7xx_eth_x_load(&stm32f746g_disco_eth);
}

static int dev_stm32f746g_disco_eth_0_open(desc_t desc, int o_flag){
   return dev_stm32f7xx_eth_x_open(desc, o_flag, &stm32f746g_disco_eth);
}

dev_map_t dev_stm32f746g_disco_eth_0_map = {
   "eth0\0",
   S_IFCHR,
   dev_stm32f746g_disco_eth_0_load,
   dev_stm32f746g_disco_eth_0_open,
   dev_stm32f7xx_eth_x_close,
   dev_stm32f7xx_eth_x_isset_read,
   dev_stm32f7xx_eth_x_isset_write,
   dev_stm32f7xx_eth_x_read,
   dev_stm32f7xx_eth_x_write,
   dev_stm32f7xx_eth_x_seek,
   dev_stm32f7xx_eth_x_ioctl
};
