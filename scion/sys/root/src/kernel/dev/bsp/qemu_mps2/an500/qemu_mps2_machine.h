/*
 * Lepton — BSP qemu_mps2, machine mps2-an500 (Cortex-M7). Licence : voir LICENSE (MPL 1.1).
 * Seul endroit (avec cmake/boards et ld/mem_qemu-mps2-an500.ld) où figurent les adresses et
 * interruptions de la carte (relevé QEMU 10.0.13 : info mtree ; ARM AN500).
 */
#ifndef _QEMU_MPS2_MACHINE_H_
#define _QEMU_MPS2_MACHINE_H_

/* périphériques du cœur (CMSIS-Core 5 : modèle ARMCM7 FPU double précision, fpv5-d16) */
#include "kernel/core/ucore/cmsis-5/Device/ARM/ARMCM7/Include/ARMCM7_DP.h"

/* horloge de 25 MHz (cpuclk, pclk : info qtree, QEMU 10.0.13), comme AN386 */
#define QEMU_MPS2_SYSCLK_HZ     25000000u

/* UART CMSDK : mêmes adresses que AN386 (info mtree, QEMU 10.0.13). */
#define QEMU_MPS2_UART0_BASE    0x40004000u
#define QEMU_MPS2_UART1_BASE    0x40005000u
#define QEMU_MPS2_UART_BAUDRATE 115200u

/* lignes d'interruption des UART comme AN385/AN386 (RX/TX sur IRQ 0..5, débordements combinés sur
 * IRQ 12 ; réception par interruption vérifiée par le test de fumée, ttys0 et ttys1, 2026-10-05) ;
 * un seul gestionnaire interroge chaque instance. */
#define QEMU_MPS2_UART_IRQ_FIRST   0
#define QEMU_MPS2_UART_IRQ_LAST    5
#define QEMU_MPS2_UART_IRQ_OVF     12
/* priorité NVIC dans la plage gérée par embOS (>= 0x80) */
#define QEMU_MPS2_UART_IRQ_PRIO    ((1u << __NVIC_PRIO_BITS) - 3u)

/* Ethernet LAN9118 : 0xA0000000 sur AN500 (info mtree ; ETAPE-6, pièges connus), ligne
 * d'interruption 13 (QEMU v10.0.0 hw/arm/mps2.c, hors AN511). */
#define QEMU_MPS2_ETH_BASE         0xA0000000u
#define QEMU_MPS2_ETH_IRQ          13
#define QEMU_MPS2_ETH_IRQ_PRIO     ((1u << __NVIC_PRIO_BITS) - 3u)

#endif
