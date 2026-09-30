/*
 * Lepton — pilote Ethernet SMSC LAN9118 (MPS2, QEMU mps2-an38x). Licence : voir LICENSE (MPL 1.1).
 *
 * Nouveau (étape 3b, absent de l'arbre). Référence des registres : modèle QEMU
 * hw/net/lan9118.c (v10.0.0), seule cible de ce pilote à ce jour (pas de fiche technique SMSC
 * dans l'arbre) ; accès 32 bits uniquement.
 *   0x00-0x1C RX_DATA_FIFO, 0x20-0x3C TX_DATA_FIFO, 0x40 RX_STATUS_FIFO, 0x48 TX_STATUS_FIFO,
 *   0x54 IRQ_CFG, 0x58 INT_STS (écrire 1 efface), 0x5C INT_EN, 0x64 BYTE_TEST (0x87654321),
 *   0x6C RX_CFG, 0x70 TX_CFG, 0x74 HW_CFG (b0 SRST), 0x7C RX_FIFO_INF (b23:16 statuts RX),
 *   0x80 TX_FIFO_INF (b23:16 statuts TX, b15:0 place libre), 0x84 PMT_CTRL (b0 READY),
 *   0xA4 MAC_CSR_CMD (b31 occupé, b30 lecture, b3:0 registre MAC), 0xA8 MAC_CSR_DATA.
 *   Registres MAC : 1 MAC_CR, 2 ADDRH, 3 ADDRL, 6 MII_ACC, 7 MII_DATA.
 * Contrat Lepton (dev_stm32f4xx_eth, ethif_core) : /dev/eth0 ouvert deux fois (lecture, écriture),
 * une trame par read/write (sans CRC), ioctl ETHGETHWADDRESS/ETHSETHWADDRESS/ETHSTAT/ETHRESET.
 * Émission synchrone (QEMU transmet la trame à l'écriture de son dernier mot) : isset_write
 * toujours prêt. Réception : interruption RSFL (statut RX disponible) → __fire_io_int du lecteur.
 */
#include <stdint.h>
#include <stdarg.h>
#include <string.h>

#include "kernel/core/kernelconf.h"
#include "kernel/core/types.h"
#include "kernel/core/interrupt.h"
#include "kernel/core/kernel.h"
#include "kernel/core/system.h"
#include "kernel/core/fcntl.h"
#include "kernel/core/stat.h"
#include "kernel/core/ioctl_eth.h"
#include "kernel/fs/vfs/vfstypes.h"

#include "dev_eth_lan9118_x.h"

#define REG(info, off)          (*(volatile uint32_t*)((info)->base + (off)))
#define LAN_RX_DATA_FIFO        0x00u
#define LAN_TX_DATA_FIFO        0x20u
#define LAN_RX_STATUS_FIFO      0x40u
#define LAN_TX_STATUS_FIFO      0x48u
#define LAN_ID_REV              0x50u
#define LAN_IRQ_CFG             0x54u
#define LAN_INT_STS             0x58u
#define LAN_INT_EN              0x5Cu
#define LAN_BYTE_TEST           0x64u
#define LAN_RX_CFG              0x6Cu
#define LAN_TX_CFG              0x70u
#define LAN_HW_CFG              0x74u
#define LAN_RX_FIFO_INF         0x7Cu
#define LAN_TX_FIFO_INF         0x80u
#define LAN_PMT_CTRL            0x84u
#define LAN_MAC_CSR_CMD         0xA4u
#define LAN_MAC_CSR_DATA        0xA8u

#define BYTE_TEST_VALUE         0x87654321u
#define HW_CFG_SRST             (1u << 0)
#define PMT_CTRL_READY          (1u << 0)
/* sortie d'interruption active à l'état haut, push-pull (sinon active à l'état bas) */
#define IRQ_CFG_TYPE            (1u << 0)
#define IRQ_CFG_POL             (1u << 4)
#define IRQ_CFG_EN              (1u << 8)
#define INT_RSFL                (1u << 3)     /* statut RX disponible */
#define INT_TXE                 (1u << 13)
#define INT_RXE                 (1u << 14)
#define TX_CFG_TX_ON            (1u << 1)
#define MAC_CSR_BUSY            (1u << 31)
#define MAC_CSR_READ            (1u << 30)

#define MAC_CR                  1u
#define MAC_ADDRH               2u
#define MAC_ADDRL               3u
#define MAC_MII_ACC             6u
#define MAC_MII_DATA            7u
#define MAC_CR_FDPX             (1u << 20)
#define MAC_CR_TXEN             (1u << 3)
#define MAC_CR_RXEN             (1u << 2)
#define MII_ACC_PHY_INTERNAL    (1u << 11)    /* PHY interne, adresse 1 */
#define MII_ACC_REG(r)          ((uint32_t)(r) << 6)
#define MII_BMSR                1u            /* IEEE 802.3 : registre d'état */
#define MII_BMSR_LINK           (1u << 2)

/* commandes TX (mots A et B) */
#define TX_CMD_A_FIRST_SEG      (1u << 13)
#define TX_CMD_A_LAST_SEG       (1u << 12)
#define TX_STS_ERROR            (1u << 15)
/* statut RX */
#define RX_STS_LENGTH(s)        (((s) >> 16) & 0x3FFFu)   /* CRC compris */
#define RX_STS_ERROR            (1u << 15)

#define ETH_FRAME_MIN           14
#define ETH_FRAME_MAX           1514          /* sans CRC */
#define ETH_CRC_SIZE            4
#define LAN_SPIN_MAX            100000

static int _lan_wait_clear(dev_eth_lan9118_info_t* info, uint32_t off, uint32_t mask){
   int n;
   for(n = 0; n < LAN_SPIN_MAX; n++)
      if(!(REG(info, off) & mask))
         return 0;
   return -1;
}

static uint32_t _lan_mac_read(dev_eth_lan9118_info_t* info, uint32_t reg){
   _lan_wait_clear(info, LAN_MAC_CSR_CMD, MAC_CSR_BUSY);
   REG(info, LAN_MAC_CSR_CMD) = MAC_CSR_BUSY | MAC_CSR_READ | reg;
   _lan_wait_clear(info, LAN_MAC_CSR_CMD, MAC_CSR_BUSY);
   return REG(info, LAN_MAC_CSR_DATA);
}

static void _lan_mac_write(dev_eth_lan9118_info_t* info, uint32_t reg, uint32_t val){
   _lan_wait_clear(info, LAN_MAC_CSR_CMD, MAC_CSR_BUSY);
   REG(info, LAN_MAC_CSR_DATA) = val;
   REG(info, LAN_MAC_CSR_CMD) = MAC_CSR_BUSY | reg;
   _lan_wait_clear(info, LAN_MAC_CSR_CMD, MAC_CSR_BUSY);
}

static void _lan_get_mac(dev_eth_lan9118_info_t* info){
   uint32_t lo = _lan_mac_read(info, MAC_ADDRL);
   uint32_t hi = _lan_mac_read(info, MAC_ADDRH);
   info->mac_addr[0] = (uint8_t)lo;
   info->mac_addr[1] = (uint8_t)(lo >> 8);
   info->mac_addr[2] = (uint8_t)(lo >> 16);
   info->mac_addr[3] = (uint8_t)(lo >> 24);
   info->mac_addr[4] = (uint8_t)hi;
   info->mac_addr[5] = (uint8_t)(hi >> 8);
}

static void _lan_set_mac(dev_eth_lan9118_info_t* info){
   _lan_mac_write(info, MAC_ADDRL, (uint32_t)info->mac_addr[0] | ((uint32_t)info->mac_addr[1] << 8)
                  | ((uint32_t)info->mac_addr[2] << 16) | ((uint32_t)info->mac_addr[3] << 24));
   _lan_mac_write(info, MAC_ADDRH, (uint32_t)info->mac_addr[4] | ((uint32_t)info->mac_addr[5] << 8));
}

static uint32_t _lan_phy_read(dev_eth_lan9118_info_t* info, uint32_t reg){
   _lan_mac_write(info, MAC_MII_ACC, MII_ACC_PHY_INTERNAL | MII_ACC_REG(reg));
   return _lan_mac_read(info, MAC_MII_DATA);
}

static int _lan_start(dev_eth_lan9118_info_t* info){
   if(REG(info, LAN_BYTE_TEST) != BYTE_TEST_VALUE)
      return -1;
   if(info->irq_enable)
      info->irq_enable(info, 0);
   REG(info, LAN_HW_CFG) = HW_CFG_SRST;
   if(_lan_wait_clear(info, LAN_HW_CFG, HW_CFG_SRST) < 0)
      return -1;
   {
      int n;
      for(n = 0; n < LAN_SPIN_MAX && !(REG(info, LAN_PMT_CTRL) & PMT_CTRL_READY); n++) {
      }
      if(!(REG(info, LAN_PMT_CTRL) & PMT_CTRL_READY))
         return -1;
   }
   /* adresse MAC chargée depuis l'EEPROM au reset (QEMU : -nic …,mac=) */
   _lan_get_mac(info);
   REG(info, LAN_INT_EN) = 0;
   REG(info, LAN_INT_STS) = 0xFFFFFFFFu;
   REG(info, LAN_IRQ_CFG) = IRQ_CFG_EN | IRQ_CFG_POL | IRQ_CFG_TYPE;
   REG(info, LAN_RX_CFG) = 0;                /* sans décalage ni alignement de fin */
   REG(info, LAN_TX_CFG) = TX_CFG_TX_ON;
   _lan_mac_write(info, MAC_CR, MAC_CR_FDPX | MAC_CR_TXEN | MAC_CR_RXEN);
   REG(info, LAN_INT_EN) = INT_RSFL | INT_RXE | INT_TXE;
   if(info->irq_enable)
      info->irq_enable(info, 1);
   info->started = 1;
   return 0;
}

static void _lan_stop(dev_eth_lan9118_info_t* info){
   if(info->irq_enable)
      info->irq_enable(info, 0);
   REG(info, LAN_INT_EN) = 0;
   _lan_mac_write(info, MAC_CR, 0);
   info->started = 0;
}

static int _lan_rx_pending(dev_eth_lan9118_info_t* info){
   return (int)((REG(info, LAN_RX_FIFO_INF) >> 16) & 0xFFu);
}

int dev_eth_lan9118_x_load(dev_eth_lan9118_info_t* info){
   info->desc_r = -1;
   info->desc_w = -1;
   info->started = 0;
   info->eth_stat = ETH_STAT_LINK_DOWN;
   info->rx_errors = info->rx_dropped = info->tx_errors = 0;
   return 0;
}

int dev_eth_lan9118_x_open(desc_t desc, int o_flag, dev_eth_lan9118_info_t* info){
   if(!info->started && _lan_start(info) < 0)
      return -1;
   if(o_flag & O_RDONLY) {
      if(info->desc_r >= 0)
         return -1;
      info->desc_r = desc;
   }
   if(o_flag & O_WRONLY) {
      if(info->desc_w >= 0)
         return -1;
      info->desc_w = desc;
   }
   ofile_lst[desc].p = info;
   return 0;
}

int dev_eth_lan9118_x_close(desc_t desc){
   dev_eth_lan9118_info_t* info = (dev_eth_lan9118_info_t*)ofile_lst[desc].p;
   if(!info)
      return -1;
   if((ofile_lst[desc].oflag & O_RDONLY) && !ofile_lst[desc].nb_reader)
      info->desc_r = -1;
   if((ofile_lst[desc].oflag & O_WRONLY) && !ofile_lst[desc].nb_writer)
      info->desc_w = -1;
   if(info->desc_r < 0 && info->desc_w < 0)
      _lan_stop(info);
   return 0;
}

int dev_eth_lan9118_x_isset_read(desc_t desc){
   dev_eth_lan9118_info_t* info = (dev_eth_lan9118_info_t*)ofile_lst[desc].p;
   if(!info || !info->started)
      return -1;
   return _lan_rx_pending(info) ? 0 : -1;
}

int dev_eth_lan9118_x_isset_write(desc_t desc){
   dev_eth_lan9118_info_t* info = (dev_eth_lan9118_info_t*)ofile_lst[desc].p;
   if(!info || !info->started)
      return -1;
   return 0; /* émission synchrone : toujours prêt */
}

/* une trame par appel, sans CRC ; 0 si aucune trame valide n'est disponible */
int dev_eth_lan9118_x_read(desc_t desc, char* buf, int size){
   dev_eth_lan9118_info_t* info = (dev_eth_lan9118_info_t*)ofile_lst[desc].p;
   if(!info || !info->started)
      return -1;
   while(_lan_rx_pending(info)) {
      uint32_t status = REG(info, LAN_RX_STATUS_FIFO);
      int len = (int)RX_STS_LENGTH(status);
      int words = (len + 3) / 4;
      int frame = len - ETH_CRC_SIZE;
      int keep = !(status & RX_STS_ERROR) && frame >= ETH_FRAME_MIN && frame <= size;
      int i;
      if(status & RX_STS_ERROR)
         info->rx_errors++;
      else if(!keep)
         info->rx_dropped++;
      for(i = 0; i < words; i++) {
         uint32_t w = REG(info, LAN_RX_DATA_FIFO);
         int off = i * 4;
         if(keep && off < frame) {
            int n = frame - off < 4 ? frame - off : 4;
            memcpy(buf + off, &w, (size_t)n);   /* octets de la trame en petit-boutiste */
         }
      }
      if(keep)
         return frame;
   }
   return 0;
}

int dev_eth_lan9118_x_write(desc_t desc, const char* buf, int size){
   dev_eth_lan9118_info_t* info = (dev_eth_lan9118_info_t*)ofile_lst[desc].p;
   int words, i, n;
   if(!info || !info->started || size < ETH_FRAME_MIN || size > ETH_FRAME_MAX)
      return -1;
   words = (size + 3) / 4;
   /* place libre (b15:0) : QEMU la compte en mots ; l'exiger en octets est plus strict */
   for(n = 0; n < LAN_SPIN_MAX
       && (int)(REG(info, LAN_TX_FIFO_INF) & 0xFFFFu) < (words + 2) * 4; n++) {
   }
   if(n == LAN_SPIN_MAX)
      return -1;
   REG(info, LAN_TX_DATA_FIFO) = TX_CMD_A_FIRST_SEG | TX_CMD_A_LAST_SEG | (uint32_t)size;
   REG(info, LAN_TX_DATA_FIFO) = ((uint32_t)size << 16) | (uint32_t)size;   /* étiquette = longueur */
   for(i = 0; i < words; i++) {
      uint32_t w = 0;
      int off = i * 4;
      memcpy(&w, buf + off, (size_t)(size - off < 4 ? size - off : 4));
      REG(info, LAN_TX_DATA_FIFO) = w;
   }
   /* statuts TX : vidés à chaque trame (file de 512 entrées) */
   while((REG(info, LAN_TX_FIFO_INF) >> 16) & 0xFFu) {
      if(REG(info, LAN_TX_STATUS_FIFO) & TX_STS_ERROR)
         info->tx_errors++;
   }
   return size;
}

int dev_eth_lan9118_x_seek(desc_t desc, int offset, int origin){
   return -1;
}

int dev_eth_lan9118_x_ioctl(desc_t desc, int request, va_list ap){
   dev_eth_lan9118_info_t* info = (dev_eth_lan9118_info_t*)ofile_lst[desc].p;
   if(!info)
      return -1;
   switch(request) {
   case ETHRESET:
      /* réinitialisation demandée par ethif_core (aucun lecteur, délai d'émission) : le
         descripteur qui la demande redevient lecteur ou écrivain de l'interface */
      if((ofile_lst[desc].oflag & O_RDONLY) && info->desc_r < 0)
         info->desc_r = desc;
      if((ofile_lst[desc].oflag & O_WRONLY) && info->desc_w < 0)
         info->desc_w = desc;
      return _lan_start(info);

   case ETHSTAT: {
      eth_stat_t* p_eth_stat = va_arg(ap, eth_stat_t*);
      if(!p_eth_stat)
         return -1;
      info->eth_stat = (_lan_phy_read(info, MII_BMSR) & MII_BMSR_LINK) ? ETH_STAT_LINK_100
                                                                       : ETH_STAT_LINK_DOWN;
      if(info->rx_errors || info->tx_errors)
         info->eth_stat |= ETH_STAT_ERROR;
      *p_eth_stat = info->eth_stat;
   }
   break;

   case ETHSETHWADDRESS: {
      unsigned char* p_eth_hwaddr = va_arg(ap, unsigned char*);
      if(!p_eth_hwaddr)
         return -1;
      memcpy(info->mac_addr, p_eth_hwaddr, sizeof(info->mac_addr));
      _lan_set_mac(info);
   }
   break;

   case ETHGETHWADDRESS: {
      unsigned char* p_eth_hwaddr = va_arg(ap, unsigned char*);
      if(!p_eth_hwaddr)
         return -1;
      memcpy(p_eth_hwaddr, info->mac_addr, sizeof(info->mac_addr));
   }
   break;

   default:
      return -1;
   }
   return 0;
}

void dev_eth_lan9118_x_interrupt(dev_eth_lan9118_info_t* info){
   uint32_t sts;
   __hw_enter_interrupt();
   sts = REG(info, LAN_INT_STS) & REG(info, LAN_INT_EN);
   REG(info, LAN_INT_STS) = sts;
   if(sts & INT_RXE)
      info->rx_errors++;
   if(sts & INT_TXE)
      info->tx_errors++;
   if((sts & INT_RSFL) && info->desc_r >= 0)
      __fire_io_int(ofile_lst[info->desc_r].owner_pthread_ptr_read);
   __hw_leave_interrupt();
}
