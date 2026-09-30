/*
 * Lepton — pilote Ethernet SMSC LAN9118 (MPS2, QEMU mps2-an38x). Licence : voir LICENSE (MPL 1.1).
 * Partie commune ; chaque instance (adresse, interruption) est déclarée par le BSP de la carte.
 */
#ifndef _DEV_ETH_LAN9118_X_H_
#define _DEV_ETH_LAN9118_X_H_

#include <stdint.h>

#include "kernel/core/ioctl_eth.h"

typedef struct dev_eth_lan9118_info_st {
   uint32_t base;          /* adresse du bloc LAN9118 (BSP) */
   void (*irq_enable)(struct dev_eth_lan9118_info_st* info, int enable); /* NVIC : BSP */
   /* état */
   desc_t desc_r;
   desc_t desc_w;
   int started;
   uint8_t mac_addr[6];
   eth_stat_t eth_stat;
   uint32_t rx_errors;
   uint32_t rx_dropped;     /* trame plus grande que le tampon du lecteur */
   uint32_t tx_errors;
} dev_eth_lan9118_info_t;

int dev_eth_lan9118_x_load(dev_eth_lan9118_info_t* info);
int dev_eth_lan9118_x_open(desc_t desc, int o_flag, dev_eth_lan9118_info_t* info);
int dev_eth_lan9118_x_close(desc_t desc);
int dev_eth_lan9118_x_isset_read(desc_t desc);
int dev_eth_lan9118_x_isset_write(desc_t desc);
int dev_eth_lan9118_x_read(desc_t desc, char* buf, int size);
int dev_eth_lan9118_x_write(desc_t desc, const char* buf, int size);
int dev_eth_lan9118_x_seek(desc_t desc, int offset, int origin);
int dev_eth_lan9118_x_ioctl(desc_t desc, int request, va_list ap);
/* à appeler depuis le gestionnaire d'interruption de l'instance (BSP) */
void dev_eth_lan9118_x_interrupt(dev_eth_lan9118_info_t* info);

#endif
