/*
 * Lepton — BSP de la carte SAMD21 Xplained Pro (ATSAMD21J18A, Cortex-M0+ r0p1, étape 6).
 * Licence : voir LICENSE (MPL 1.1).
 *
 * Seul endroit (avec cmake/boards et ld/mem_samd21-xplained-pro.ld) où figurent fréquences et
 * réglages de la carte. Références : fiche technique SAMD21 (SYSCTRL, GCLK, NVMCTRL, SERCOM),
 * en-tête de carte ASF samd21_xplained_pro.h (quartz 32,768 kHz, console EDBG). Inclus par
 * user_kernel_mkconf.h : aucun en-tête Microchip ici (inclus par kernelconf.h dans tout le noyau).
 */
#ifndef _SAMD21_XPLAINED_PRO_H_
#define _SAMD21_XPLAINED_PRO_H_

/* --- horloges : DFLL48M en boucle fermée sur XOSC32K (quartz 32,768 kHz de la carte, ASF :
 *     BOARD_FREQ_SLCK_XTAL) par GCLK1 ; GCLK0 (CPU, bus, SERCOM) = DFLL48M ---------------------- */
#define SAMD21_XPLAINED_PRO_XOSC32K_HZ    32768u
#define SAMD21_XPLAINED_PRO_SYSCLK_HZ     48000000u
/* multiplicateur de la DFLL : 48 MHz / 32 768 Hz = 1464,8 → 1465 (47,99 MHz… 48,005 MHz) */
#define SAMD21_XPLAINED_PRO_DFLL_MUL      1465u
/* 1 état d'attente de la flash au-delà de 24 MHz (2,7-3,63 V) */
#define SAMD21_XPLAINED_PRO_NVM_RWS       1u

/* --- console : SERCOM3, TX PA22 (PAD0), RX PA23 (PAD1), fonction C : port série virtuel de
 *     l'EDBG (ASF : EDBG_CDC_MODULE, USART_RX_1_TX_0_XCK_1) -------------------------------------- */
#define SAMD21_XPLAINED_PRO_CONSOLE_BAUDRATE 115200u

/* priorité NVIC des périphériques : 2 bits implémentés ; niveau 2 (comme SysTick, au-dessus de
 * PendSV, embos_init_hw.c armv6m) */
#define SAMD21_XPLAINED_PRO_IRQ_PRIO      2u

#endif
