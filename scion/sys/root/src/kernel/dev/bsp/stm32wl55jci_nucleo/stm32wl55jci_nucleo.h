/*
 * Lepton — BSP de la carte NUCLEO-WL55JC1 (STM32WL55JC, Cortex-M4 du CPU1, étape 6).
 * Licence : voir LICENSE (MPL 1.1).
 *
 * Seul endroit (avec cmake/boards et ld/mem_nucleo-wl55jc1.ld) où figurent fréquences et réglages
 * de la carte. Références : SystemClock_Config du portage IAR (dev_stm32wlxx_cpu_x.c : MSI range
 * 11, sans PLL, AHB/APB /1, latence flash 2, échelle de tension 1), RM0453. Inclus par
 * user_kernel_mkconf.h : aucun en-tête ST ici.
 */
#ifndef _STM32WL55JCI_NUCLEO_H_
#define _STM32WL55JCI_NUCLEO_H_

/* --- horloges : MSI range 11 = 48 MHz, sans PLL, HCLK = HCLK3 = PCLK1 = PCLK2 = 48 MHz -------- */
#define STM32WL55JCI_NUCLEO_SYSCLK_HZ     48000000u
#define STM32WL55JCI_NUCLEO_FLASH_LATENCY 2u     /* 2 états d'attente à 48 MHz, échelle 1 */

/* --- console : USART2, TX PA2, RX PA3 (AF7), port série virtuel du STLINK-V3 ------------------ */
#define STM32WL55JCI_NUCLEO_CONSOLE_BAUDRATE 115200u

/* priorité NVIC des périphériques : dans la plage gérée par embOS (>= 0x80) */
#define STM32WL55JCI_NUCLEO_IRQ_PRIO      ((1u << __NVIC_PRIO_BITS) - 4u)

#endif
