/*
 * Lepton — BSP de la carte NUCLEO-F439ZI (STM32F439ZI ; validée sur NUCLEO-F429ZI, étape 5).
 * Licence : voir LICENSE (MPL 1.1).
 *
 * Horloges (SystemInit, appelée par le démarrage générique kernel/core/arch/cortexm), GPIO,
 * périphériques /dev/board, /dev/ttys3 (USART3, console), /dev/ttys6 (USART6), descripteur
 * Ethernet de la couche eth.c (/dev/eth0 : dev_stm32f4xx_eth_map). Interruptions :
 * nucleo_f439zi_vectors.c. Écrit d'après le BSP Olimex STM32-P407 (même couche STM32F4).
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
#include "kernel/core/cpu.h"
#include "kernel/fs/vfs/vfstypes.h"
#include "kernel/core/ioctl_board.h"
#include "kernel/core/ioctl_eth.h"
#include "kernel/dev/arch/cortexm/stm32f4xx/driverlib/stm32f4xx.h"
#include "kernel/dev/arch/cortexm/stm32f4xx/types.h"
#include "kernel/dev/arch/cortexm/stm32f4xx/gpio.h"
#include "kernel/dev/arch/cortexm/stm32f4xx/dma.h"
#include "kernel/dev/arch/cortexm/stm32f4xx/uart.h"
#include "kernel/dev/arch/cortexm/stm32f4xx/eth.h"
#include "kernel/dev/arch/cortexm/stm32f4xx/dev_stm32f4xx/dev_stm32f4xx_uart_x.h"

#include "nucleo_f439zi.h"

/* --- horloges --------------------------------------------------------------------------------- */
uint32_t SystemCoreClock = 16000000u;   /* HSI après reset */
/* source de la PLL retenue par SystemInit (lisible au débogueur, palier 1) : 1 HSE, 0 HSI (repli
   si le MCO du ST-LINK est absent : pont SB148 ouvert, ST-LINK non alimenté) */
volatile uint32_t nucleo_f439zi_clock_hse;

#define NUCLEO_F439ZI_HSE_TIMEOUT 100000u
/* positions des champs (RM0090 6.3.2, 3.9.1 ; absentes du stm32f429xx.h de l'arbre) */
#define NUCLEO_F439ZI_PLLN_POS     6
#define NUCLEO_F439ZI_PLLP_POS     16
#define NUCLEO_F439ZI_PLLQ_POS     24
#define NUCLEO_F439ZI_LATENCY_POS  0

void SystemInit(void){
   uint32_t timeout;
   uint32_t pllm = NUCLEO_F439ZI_PLL_M;
   uint32_t pllsrc = RCC_PLLCFGR_PLLSRC_HSE;

   /* HSE en bypass : horloge 8 MHz fournie par le MCO du ST-LINK (UM1974, 6.7.1) */
   RCC->CR |= RCC_CR_HSEBYP;
   RCC->CR |= RCC_CR_HSEON;
   for(timeout = 0; !(RCC->CR & RCC_CR_HSERDY) && timeout < NUCLEO_F439ZI_HSE_TIMEOUT; timeout++) {
   }
   if(RCC->CR & RCC_CR_HSERDY) {
      nucleo_f439zi_clock_hse = 1;
   } else {
      RCC->CR &= ~RCC_CR_HSEON;
      RCC->CR &= ~RCC_CR_HSEBYP;
      pllm = 16u;                         /* HSI 16 MHz : même entrée PLL de 1 MHz */
      pllsrc = RCC_PLLCFGR_PLLSRC_HSI;
      nucleo_f439zi_clock_hse = 0;
   }

   /* régulateur en échelle 1 (168 MHz) */
   RCC->APB1ENR |= RCC_APB1ENR_PWREN;
   (void)RCC->APB1ENR;
   PWR->CR |= PWR_CR_VOS;

   /* AHB 168 MHz, APB1 42 MHz (/4), APB2 84 MHz (/2) */
   RCC->CFGR = (RCC->CFGR & ~(RCC_CFGR_HPRE | RCC_CFGR_PPRE1 | RCC_CFGR_PPRE2))
               | RCC_CFGR_HPRE_DIV1 | RCC_CFGR_PPRE1_DIV4 | RCC_CFGR_PPRE2_DIV2;

   RCC->PLLCFGR = pllm
                  | (NUCLEO_F439ZI_PLL_N << NUCLEO_F439ZI_PLLN_POS)
                  | (((NUCLEO_F439ZI_PLL_P >> 1) - 1u) << NUCLEO_F439ZI_PLLP_POS)
                  | pllsrc
                  | (NUCLEO_F439ZI_PLL_Q << NUCLEO_F439ZI_PLLQ_POS);
   RCC->CR |= RCC_CR_PLLON;
   while(!(RCC->CR & RCC_CR_PLLRDY)) {
   }

   /* flash : états d'attente avant de passer sur la PLL, prélecture et caches */
   FLASH->ACR = FLASH_ACR_PRFTEN | FLASH_ACR_ICEN | FLASH_ACR_DCEN
                | (NUCLEO_F439ZI_FLASH_LATENCY << NUCLEO_F439ZI_LATENCY_POS);

   RCC->CFGR = (RCC->CFGR & ~RCC_CFGR_SW) | RCC_CFGR_SW_PLL;
   while((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL) {
   }

   SystemCoreClock = NUCLEO_F439ZI_SYSCLK_HZ;
}

void SystemCoreClockUpdate(void){
   RCC_ClocksTypeDef clocks;
   RCC_GetClocksFreq(&clocks);
   SystemCoreClock = clocks.SYSCLK_Frequency;
}

/* --- GPIO (couche gpio.c ; ordre de _GPIO_LIST) ------------------------------------------------- */
const _Gpio_Descriptor Gpio_Descriptor[] = {
   {GPIO_TYPE_STD, GPIOD, GPIO_Pin_8,  0, GPIO_MODE_IN,  0},   /* GPIO_TXD3 (uart.c : AF) */
   {GPIO_TYPE_STD, GPIOD, GPIO_Pin_9,  0, GPIO_MODE_IN,  0},   /* GPIO_RXD3 */
   {GPIO_TYPE_STD, GPIOG, GPIO_Pin_14, 0, GPIO_MODE_IN,  0},   /* GPIO_TXD6 */
   {GPIO_TYPE_STD, GPIOG, GPIO_Pin_9,  0, GPIO_MODE_IN,  0},   /* GPIO_RXD6 */
   {GPIO_TYPE_STD, GPIOB, GPIO_Pin_0,  1, GPIO_MODE_OUT, 0},   /* GPIO_LED1 (LD1 verte) */
   {GPIO_TYPE_STD, GPIOB, GPIO_Pin_7,  1, GPIO_MODE_OUT, 0},   /* GPIO_LED2 (LD2 bleue) */
   {GPIO_TYPE_STD, GPIOB, GPIO_Pin_14, 1, GPIO_MODE_OUT, 0},   /* GPIO_LED3 (LD3 rouge) */
   {GPIO_TYPE_STD, GPIOC, GPIO_Pin_13, 1, GPIO_MODE_IN,  0}    /* GPIO_BUTTON (B1) */
};

/* --- UART : descripteurs (uart.c), DMA de réception (RM0090, tables 42 et 43) ----------------- */
board_stm32f4xx_uart_info_t nucleo_f439zi_uart_3 = {
   .uart_descriptor = {USART3, RCC_APB1PeriphClockCmd, RCC_APB1Periph_USART3, USART3_IRQn,
                       DMA1_Stream1, DMA_Channel_4, DMA1_Stream1_IRQn,
                       GPIO_TXD3, GPIO_RXD3, GPIO_AF_USART3, &Uart_Ctrl[UART_ID_3]}
};
board_stm32f4xx_uart_info_t nucleo_f439zi_uart_6 = {
   .uart_descriptor = {USART6, RCC_APB2PeriphClockCmd, RCC_APB2Periph_USART6, USART6_IRQn,
                       DMA2_Stream1, DMA_Channel_5, DMA2_Stream1_IRQn,
                       GPIO_TXD6, GPIO_RXD6, GPIO_AF_USART6, &Uart_Ctrl[UART_ID_6]}
};

/* gestionnaires, appelés par nucleo_f439zi_vectors.c */
void USART3_IRQHandler(void)       { uart_irq_handler(&nucleo_f439zi_uart_3.uart_descriptor); }
void DMA1_Stream1_IRQHandler(void) { uart_dma_irq_handler(&nucleo_f439zi_uart_3.uart_descriptor); }
void USART6_IRQHandler(void)       { uart_irq_handler(&nucleo_f439zi_uart_6.uart_descriptor); }
void DMA2_Stream1_IRQHandler(void) { uart_dma_irq_handler(&nucleo_f439zi_uart_6.uart_descriptor); }

extern int dev_stm32f4xx_uart_x_load(board_stm32f4xx_uart_info_t* uart_info);
extern int dev_stm32f4xx_uart_x_open(desc_t desc, int o_flag, board_stm32f4xx_uart_info_t* uart_info);
extern int dev_stm32f4xx_uart_x_close(desc_t desc);
extern int dev_stm32f4xx_uart_x_read(desc_t desc, char* buf, int cb);
extern int dev_stm32f4xx_uart_x_write(desc_t desc, const char* buf, int cb);
extern int dev_stm32f4xx_uart_x_ioctl(desc_t desc, int request, va_list ap);
extern int dev_stm32f4xx_uart_x_isset_read(desc_t desc);
extern int dev_stm32f4xx_uart_x_isset_write(desc_t desc);
extern int dev_stm32f4xx_uart_x_seek(desc_t desc, int offset, int origin);

/* instances : ttys3, ttys6 */
#define NUCLEO_F439ZI_UART_INSTANCE(n) \
   static int dev_nucleo_f439zi_uart_##n##_load(void){ \
      nucleo_f439zi_uart_##n.uart_descriptor.board_uart_info = &nucleo_f439zi_uart_##n; \
      nucleo_f439zi_uart_##n.desc_r = -1; \
      nucleo_f439zi_uart_##n.desc_w = -1; \
      return dev_stm32f4xx_uart_x_load(&nucleo_f439zi_uart_##n); } \
   static int dev_nucleo_f439zi_uart_##n##_open(desc_t desc, int o_flag){ \
      return dev_stm32f4xx_uart_x_open(desc, o_flag, &nucleo_f439zi_uart_##n); } \
   dev_map_t dev_nucleo_f439zi_uart_##n##_map = { \
      "ttys" #n "\0", \
      S_IFCHR, \
      dev_nucleo_f439zi_uart_##n##_load, \
      dev_nucleo_f439zi_uart_##n##_open, \
      dev_stm32f4xx_uart_x_close, \
      dev_stm32f4xx_uart_x_isset_read, \
      dev_stm32f4xx_uart_x_isset_write, \
      dev_stm32f4xx_uart_x_read, \
      dev_stm32f4xx_uart_x_write, \
      dev_stm32f4xx_uart_x_seek, \
      dev_stm32f4xx_uart_x_ioctl \
   };

NUCLEO_F439ZI_UART_INSTANCE(3)
NUCLEO_F439ZI_UART_INSTANCE(6)

/* --- Ethernet RMII, PHY LAN8742A (eth.c ; UM1974 6.11, broches fixées par la carte) ----------- */
static const eth_stm32f4x7_bsp_pin_t nucleo_f439zi_eth_pins[] = {
   {GPIOA, GPIO_PinSource1},  {GPIOA, GPIO_PinSource2},  {GPIOA, GPIO_PinSource7},  /* REF_CLK, MDIO, CRS_DV */
   {GPIOB, GPIO_PinSource13},                                                        /* TXD1 */
   {GPIOC, GPIO_PinSource1},  {GPIOC, GPIO_PinSource4},  {GPIOC, GPIO_PinSource5},  /* MDC, RXD0, RXD1 */
   {GPIOG, GPIO_PinSource11}, {GPIOG, GPIO_PinSource13}                             /* TX_EN, TXD0 */
};

const eth_stm32f4x7_bsp_t eth_stm32f4x7_bsp = {
   RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_GPIOG,
   GPIO_Speed_100MHz,
   nucleo_f439zi_eth_pins, sizeof(nucleo_f439zi_eth_pins) / sizeof(nucleo_f439zi_eth_pins[0]),
   BOARD_ETH_PHY_ID1, BOARD_ETH_PHY_ID2, BOARD_ETH_PHY_ID2_MASK,
   BOARD_ETH_PHY_ADDR_FIRST
};

/* --- /dev/board : GPIO et DMA, chargé avant les UART (ordre du mkconf) ------------------------ */
static int dev_nucleo_f439zi_board_load(void){
   unsigned int i;
   RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOC
                          | RCC_AHB1Periph_GPIOD | RCC_AHB1Periph_GPIOG, ENABLE);
   for(i = 0; i < GPIO_NB; i++) {
      if(Gpio_Descriptor[i].Init)
         gpio_init(&Gpio_Descriptor[i]);
   }
   dma_startup_init();
   return 0;
}

static int dev_nucleo_f439zi_board_open(desc_t desc, int o_flag){
   (void)o_flag;
   ofile_lst[desc].offset = 0;
   return 0;
}

static int dev_nucleo_f439zi_board_close(desc_t desc){
   (void)desc;
   return 0;
}

static int dev_nucleo_f439zi_board_isset(desc_t desc){
   (void)desc;
   return -1;
}

static int dev_nucleo_f439zi_board_read(desc_t desc, char* buf, int size){
   (void)desc; (void)buf; (void)size;
   return -1;
}

static int dev_nucleo_f439zi_board_write(desc_t desc, const char* buf, int size){
   (void)desc; (void)buf; (void)size;
   return -1;
}

static int dev_nucleo_f439zi_board_seek(desc_t desc, int offset, int origin){
   (void)desc; (void)offset; (void)origin;
   return -1;
}

static int dev_nucleo_f439zi_board_ioctl(desc_t desc, int request, va_list ap){
   (void)desc; (void)ap;
   switch(request) {
   case BRDPWRDOWN:
   case BRDRESET:
   case BRDWATCHDOG:
   case BRDCFGPORT:
   case BRDBEEP:
      return 0;
   default:
      return -1;
   }
}

dev_map_t dev_nucleo_f439zi_board_map = {
   "board\0",
   S_IFBLK,
   dev_nucleo_f439zi_board_load,
   dev_nucleo_f439zi_board_open,
   dev_nucleo_f439zi_board_close,
   dev_nucleo_f439zi_board_isset,
   dev_nucleo_f439zi_board_isset,
   dev_nucleo_f439zi_board_read,
   dev_nucleo_f439zi_board_write,
   dev_nucleo_f439zi_board_seek,
   dev_nucleo_f439zi_board_ioctl
};
