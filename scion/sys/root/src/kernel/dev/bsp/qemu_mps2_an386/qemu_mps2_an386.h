/*
 * Lepton — BSP de la carte QEMU mps2-an386 (Cortex-M4). Licence : voir LICENSE (MPL 1.1).
 * Seul endroit (avec cmake/boards et ld/mem_qemu-mps2-an386.ld) où figurent les adresses et
 * interruptions de la carte (relevé QEMU 8.2 : info mtree ; ARM AN386).
 */
#ifndef _QEMU_MPS2_AN386_H_
#define _QEMU_MPS2_AN386_H_

/* périphériques du cœur (CMSIS-Core V3.30 de l'arbre, modèle ARMCM4) */
#include "kernel/core/ucore/cmsis/Device/ARM/ARMCM4/Include/ARMCM4.h"

#define QEMU_MPS2_AN386_SYSCLK_HZ     25000000u

#define QEMU_MPS2_AN386_UART0_BASE    0x40004000u
#define QEMU_MPS2_AN386_UART1_BASE    0x40005000u
#define QEMU_MPS2_AN386_UART_BAUDRATE 115200u

/* HYPOTHÈSE À VALIDER (test de fumée) : UART0..2 RX/TX sur IRQ 0..5, débordements combinés sur
 * IRQ 12 (AN385/AN386). Toutes ces lignes sont routées vers le même gestionnaire, qui interroge
 * chaque instance : correct quelle que soit la répartition exacte. */
#define QEMU_MPS2_AN386_UART_IRQ_FIRST   0
#define QEMU_MPS2_AN386_UART_IRQ_LAST    5
#define QEMU_MPS2_AN386_UART_IRQ_OVF     12
/* priorité NVIC dans la plage gérée par embOS (>= 0x80) */
#define QEMU_MPS2_AN386_UART_IRQ_PRIO    ((1u << __NVIC_PRIO_BITS) - 3u)

/* Ethernet LAN9118 (LAN9220 sur la carte réelle) : QEMU v10.0.0 hw/arm/mps2.c, lan9118_init
 * (ethernet_base 0x40200000, ligne d'interruption 13 hors AN511). */
#define QEMU_MPS2_AN386_ETH_BASE         0x40200000u
#define QEMU_MPS2_AN386_ETH_IRQ          13
#define QEMU_MPS2_AN386_ETH_IRQ_PRIO     ((1u << __NVIC_PRIO_BITS) - 3u)

#endif
