/*
 * Lepton — pilote Ethernet STM32F7 (MAC + DMA, HAL ETH V1.3.x ; famille STM32F7xx, étape 6).
 * Licence : voir LICENSE (MPL 1.1). Partie commune ; l'instance (PHY, interruption, broches par
 * HAL_ETH_MspInit) est déclarée par le BSP de la carte. Une seule instance (ETH du STM32F7).
 */
#ifndef _DEV_STM32F7XX_ETH_X_H_
#define _DEV_STM32F7XX_ETH_X_H_

#include <stdint.h>

typedef struct dev_stm32f7xx_eth_info_st {
   /* PHY (BSP) : adresse MDIO, registre spécial d'état et codes vitesse/duplex */
   uint32_t phy_addr;
   uint16_t phy_sr;
   uint16_t phy_speed_mask;
   uint16_t phy_100_full;
   uint16_t phy_100_half;
   uint16_t phy_10_full;
   uint16_t phy_10_half;
   void (*irq_enable)(struct dev_stm32f7xx_eth_info_st* info, int enable); /* NVIC : BSP */
   /* état */
   desc_t desc_r;
   desc_t desc_w;
   int started;
   uint8_t mac_addr[6];
   eth_stat_t eth_stat;
   uint32_t rx_errors;      /* trames en erreur ou sur plusieurs tampons */
   uint32_t rx_dropped;     /* trame plus grande que le tampon du lecteur */
   uint32_t rx_no_buffer;   /* tampon de réception indisponible (RBU) */
   uint32_t tx_errors;
} dev_stm32f7xx_eth_info_t;

int dev_stm32f7xx_eth_x_load(dev_stm32f7xx_eth_info_t* info);
int dev_stm32f7xx_eth_x_open(desc_t desc, int o_flag, dev_stm32f7xx_eth_info_t* info);
int dev_stm32f7xx_eth_x_close(desc_t desc);
int dev_stm32f7xx_eth_x_isset_read(desc_t desc);
int dev_stm32f7xx_eth_x_isset_write(desc_t desc);
int dev_stm32f7xx_eth_x_read(desc_t desc, char* buf, int size);
int dev_stm32f7xx_eth_x_write(desc_t desc, const char* buf, int size);
int dev_stm32f7xx_eth_x_seek(desc_t desc, int offset, int origin);
int dev_stm32f7xx_eth_x_ioctl(desc_t desc, int request, va_list ap);
/* à appeler depuis le gestionnaire d'interruption ETH (BSP) */
void dev_stm32f7xx_eth_x_interrupt(dev_stm32f7xx_eth_info_t* info);

#endif
