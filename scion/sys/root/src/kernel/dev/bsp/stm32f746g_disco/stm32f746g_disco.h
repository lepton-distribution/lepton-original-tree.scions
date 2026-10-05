/*
 * Lepton — BSP de la carte STM32F746G-DISCO (STM32F746NG, Cortex-M7 r0p1, étape 6).
 * Licence : voir LICENSE (MPL 1.1).
 *
 * Seul endroit (avec cmake/boards et ld/mem_stm32f746g-disco.ld) où figurent fréquences et
 * réglages de la carte. Références : STM32CubeF7 v1.17.4 (Projects/STM32746G-Discovery/Templates :
 * SystemClock_Config ; BSP 32f746gdiscovery : COM1), RM0385. Inclus par user_kernel_mkconf.h :
 * aucun en-tête ST ici (inclus par kernelconf.h dans tout le noyau).
 */
#ifndef _STM32F746G_DISCO_H_
#define _STM32F746G_DISCO_H_

/* --- horloges : HSE 25 MHz (quartz X2), PLL → 216 MHz avec over-drive (exemple ST) ----------- */
#define STM32F746G_DISCO_HSE_HZ        25000000u
#define STM32F746G_DISCO_SYSCLK_HZ     216000000u
#define STM32F746G_DISCO_PLL_M         25u      /* entrée PLL 1 MHz */
#define STM32F746G_DISCO_PLL_N         432u     /* VCO 432 MHz */
#define STM32F746G_DISCO_PLL_P         2u       /* SYSCLK 216 MHz */
#define STM32F746G_DISCO_PLL_Q         9u       /* 48 MHz (USB, SDMMC, RNG) */
#define STM32F746G_DISCO_FLASH_LATENCY 7u       /* 7 états d'attente à 216 MHz, 2,7-3,6 V */
#define STM32F746G_DISCO_APB1_DIV      4u       /* PCLK1 54 MHz */
#define STM32F746G_DISCO_APB2_DIV      2u       /* PCLK2 108 MHz (USART1) */
#define STM32F746G_DISCO_PCLK2_HZ      (STM32F746G_DISCO_SYSCLK_HZ / STM32F746G_DISCO_APB2_DIV)

/* --- console : USART1, TX PA9, RX PB7 (AF7), port série virtuel du ST-LINK (COM1 ST) ---------- */
#define STM32F746G_DISCO_CONSOLE_BAUDRATE 115200u

/* priorité NVIC des périphériques : dans la plage gérée par embOS (>= 0x80) */
#define STM32F746G_DISCO_IRQ_PRIO      ((1u << __NVIC_PRIO_BITS) - 4u)

#endif
