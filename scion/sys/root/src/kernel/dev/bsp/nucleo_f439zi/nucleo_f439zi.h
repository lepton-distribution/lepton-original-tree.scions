/*
 * Lepton — BSP de la carte NUCLEO-F439ZI (STM32F439ZI ; validée sur NUCLEO-F429ZI, étape 5).
 * Licence : voir LICENSE (MPL 1.1).
 *
 * Configuration de carte attendue par la couche STM32F4 de Lepton (kernel/dev/arch/cortexm/
 * stm32f4xx : gpio.c, uart.c, eth.c), incluse par user_kernel_mkconf.h de l'application comme
 * pour l'Olimex STM32-P407 : liste des GPIO, des UART, réglages du PHY Ethernet.
 * Seul endroit (avec cmake/boards et ld/mem_nucleo-f439zi.ld) où figurent broches, fréquences et
 * interruptions de la carte. Référence : UM1974 (cartes NUCLEO-144), RM0090, fiche LAN8742A.
 * Ne pas inclure d'en-tête ST ici (inclus par kernelconf.h dans tout le noyau) : les descripteurs
 * ne sont que nommés.
 */
#ifndef _NUCLEO_F439ZI_H_
#define _NUCLEO_F439ZI_H_

/* --- horloges : HSE 8 MHz (MCO du ST-LINK, bypass), PLL → 168 MHz (sans over-drive) ----------- */
#define NUCLEO_F439ZI_SYSCLK_HZ    168000000u
#define NUCLEO_F439ZI_PLL_M        8u      /* entrée PLL 1 MHz (HSE 8 MHz ; HSI 16 MHz : M = 16) */
#define NUCLEO_F439ZI_PLL_N        336u    /* VCO 336 MHz */
#define NUCLEO_F439ZI_PLL_P        2u      /* SYSCLK 168 MHz */
#define NUCLEO_F439ZI_PLL_Q        7u      /* 48 MHz (USB OTG, SDIO, RNG) */
#define NUCLEO_F439ZI_FLASH_LATENCY 5u     /* 5 états d'attente, 2,7-3,6 V, 150-168 MHz (RM0090) */

/* --- GPIO (couche gpio.c) ------------------------------------------------------------------ */
#define _GPIO_DEFAULT_SPEED   GPIO_Speed_50MHz

typedef enum
{
  GPIO_ID_TXD3,      /* PD8, USART3_TX → ST-LINK (port série virtuel) */
  GPIO_ID_RXD3,      /* PD9, USART3_RX */
  GPIO_ID_TXD6,      /* PG14, USART6_TX, CN10 D1 */
  GPIO_ID_RXD6,      /* PG9, USART6_RX, CN10 D0 */
  GPIO_ID_LED1,      /* PB0, LD1 verte */
  GPIO_ID_LED2,      /* PB7, LD2 bleue */
  GPIO_ID_LED3,      /* PB14, LD3 rouge */
  GPIO_ID_BUTTON,    /* PC13, B1 (USER) */
  GPIO_NB
} _GPIO_LIST;

#define GPIO_TXD3     (&Gpio_Descriptor[GPIO_ID_TXD3])
#define GPIO_RXD3     (&Gpio_Descriptor[GPIO_ID_RXD3])
#define GPIO_TXD6     (&Gpio_Descriptor[GPIO_ID_TXD6])
#define GPIO_RXD6     (&Gpio_Descriptor[GPIO_ID_RXD6])
#define GPIO_LED1     (&Gpio_Descriptor[GPIO_ID_LED1])
#define GPIO_LED2     (&Gpio_Descriptor[GPIO_ID_LED2])
#define GPIO_LED3     (&Gpio_Descriptor[GPIO_ID_LED3])
#define GPIO_BUTTON   (&Gpio_Descriptor[GPIO_ID_BUTTON])

/* --- UART (couche uart.c) ------------------------------------------------------------------ */
typedef enum
{
  UART_ID_3,         /* ttys3 : console lsh (__KERNEL_DEV_TTY) */
  UART_ID_6,         /* ttys6 : second port, connecteur */
  UART_NB
} _UART_LIST;

/* priorité NVIC des UART et de l'Ethernet : dans la plage gérée par embOS (>= 0x80) */
#define NUCLEO_F439ZI_IRQ_PRIO     ((1u << __NVIC_PRIO_BITS) - 4u)

/* --- PHY Ethernet LAN8742A (RMII, adresse 0) : couche eth.c et stm32f4x7_eth_conf.h ---------- */
#define BOARD_ETH_PHY_ID1                 0x0007u   /* registre 2 */
#define BOARD_ETH_PHY_ID2                 0xC130u   /* registre 3, révision masquée */
#define BOARD_ETH_PHY_ID2_MASK            0xFFF0u
#define BOARD_ETH_PHY_ADDR_FIRST          0u
/* registre spécial d'état (31), champ « speed indication » (bits 4:2) */
#define BOARD_ETH_PHY_SR                  ((uint16_t)31)
#define BOARD_ETH_PHY_DUPLEX_SPEED_MASK   ((uint16_t)0x001C)
#define BOARD_ETH_PHY_100BTX_FULL         ((uint16_t)0x0018)
#define BOARD_ETH_PHY_100BTX_HALF         ((uint16_t)0x0008)
#define BOARD_ETH_PHY_10M_FULL            ((uint16_t)0x0014)
#define BOARD_ETH_PHY_10M_HALF            ((uint16_t)0x0004)

#endif
