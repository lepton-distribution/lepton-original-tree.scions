/*
 * Lepton — pilote Ethernet STM32F7 (MAC + DMA, HAL ETH V1.3.x ; famille STM32F7xx, étape 6).
 * Licence : voir LICENSE (MPL 1.1).
 *
 * Nouveau : la HAL ETH V1.3.x (STM32CubeF7 v1.17.4) a changé d'API par rapport à la SPL du
 * pilote STM32F4 de Lepton (eth.c) ; écrit sur le modèle du pilote LAN9118 (étape 3b).
 * Contrat Lepton (ethif_core) : /dev/eth0 ouvert deux fois (lecture, écriture), une trame par
 * read/write (sans CRC), ioctl ETHGETHWADDRESS/ETHSETHWADDRESS/ETHSTAT/ETHRESET.
 *
 * Mémoire du DMA (descripteurs, tampons) : un bloc de 16 Ko aligné, déclaré non cachable par une
 * région MPU (cache D du Cortex-M7 actif, SystemInit du BSP) ; aucune maintenance de cache.
 * Réception : tampons d'un réservoir prêtés à la HAL (HAL_ETH_RxAllocateCallback), une trame par
 * tampon (RxBuffLen 1536) ; l'interruption RI réveille le lecteur, seuls les processus
 * manipulent les descripteurs (HAL_ETH_ReadData). isset_read garde d'avance la trame qu'il a
 * lue (aucune perte, défaut de l'étape 5 sur la F4). Émission synchrone (HAL_ETH_Transmit par
 * scrutation, tampon unique, libéré aussitôt). PHY IEEE 802.3 clause 22 ; vitesse et duplex lus
 * dans le registre spécial du PHY décrit par le BSP.
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

#include "stm32f7xx_hal.h"
#include "dev_stm32f7xx_eth_x.h"

#define ETH_FRAME_MIN        14
#define ETH_FRAME_MAX        1514          /* sans CRC */
#define ETH_BUF_LEN          1536u         /* multiple de 4 ; une trame par tampon */
#define ETH_RX_POOL          (ETH_RX_DESC_CNT + 4u)
#define ETH_TX_TIMEOUT_MS    20u
#define ETH_AUTONEG_MS       3000u

/* registres PHY standard (IEEE 802.3 clause 22) */
#define MII_BMCR             0u
#define MII_BMSR             1u
#define MII_PHYID1           2u
#define MII_BMCR_RESET       (1u << 15)
#define MII_BMCR_ANENABLE    (1u << 12)
#define MII_BMCR_ANRESTART   (1u << 9)
#define MII_BMSR_ANCOMPLETE  (1u << 5)
#define MII_BMSR_LINK        (1u << 2)

/* mémoire du DMA : un seul bloc, région MPU non cachable */
#define ETH_DMA_REGION_SIZE  16384u
typedef struct {
   ETH_DMADescTypeDef rx_desc[ETH_RX_DESC_CNT];
   ETH_DMADescTypeDef tx_desc[ETH_TX_DESC_CNT];
   uint8_t rx_buf[ETH_RX_POOL][ETH_BUF_LEN];
   uint8_t tx_buf[ETH_BUF_LEN];
} eth_dma_t;
_Static_assert(sizeof(eth_dma_t) <= ETH_DMA_REGION_SIZE, "mémoire DMA Ethernet > région MPU");

static eth_dma_t eth_dma __attribute__((aligned(ETH_DMA_REGION_SIZE)));
static ETH_HandleTypeDef eth_handle;
static dev_stm32f7xx_eth_info_t* eth_info;      /* instance unique, pour les callbacks de la HAL */
static volatile uint8_t eth_rx_used[ETH_RX_POOL];
static uint16_t eth_rx_len[ETH_RX_POOL];       /* 0 : trame invalide (sur plusieurs tampons) */
static uint8_t* eth_rx_pending;                 /* trame lue d'avance par isset_read */
static int eth_mpu_done;

static int _eth_rx_index(const uint8_t* buff){
   return (int)((buff - &eth_dma.rx_buf[0][0]) / (int)ETH_BUF_LEN);
}

static void _eth_rx_free(uint8_t* buff){
   eth_rx_used[_eth_rx_index(buff)] = 0;
}

/* --- callbacks de la HAL (instance unique) ------------------------------------------------ */
void HAL_ETH_RxAllocateCallback(uint8_t** buff){
   unsigned int i;
   *buff = NULL;
   for(i = 0; i < ETH_RX_POOL; i++) {
      if(!eth_rx_used[i]) {
         eth_rx_used[i] = 1;
         *buff = eth_dma.rx_buf[i];
         return;
      }
   }
}

void HAL_ETH_RxLinkCallback(void** pStart, void** pEnd, uint8_t* buff, uint16_t Length){
   if(*pStart == NULL) {
      *pStart = buff;
      eth_rx_len[_eth_rx_index(buff)] = Length;
   } else {
      /* trame sur plusieurs tampons : impossible avec RxBuffLen 1536 ; trame rejetée */
      eth_rx_len[_eth_rx_index((uint8_t*)*pStart)] = 0;
      _eth_rx_free(buff);
   }
   *pEnd = buff;
}

void HAL_ETH_TxFreeCallback(uint32_t* buff){
   (void)buff;   /* tampon d'émission unique et statique */
}

void HAL_ETH_RxCpltCallback(ETH_HandleTypeDef* heth){
   (void)heth;
   if(eth_info && eth_info->desc_r >= 0)
      __fire_io_int(ofile_lst[eth_info->desc_r].owner_pthread_ptr_read);
}

void HAL_ETH_ErrorCallback(ETH_HandleTypeDef* heth){
   if(!eth_info)
      return;
   if(heth->DMAErrorCode & ETH_DMASR_RBUS) {
      /* plus de descripteur libre : le lecteur les rend en lisant (HAL_ETH_ReadData) */
      eth_info->rx_no_buffer++;
      if(eth_info->desc_r >= 0)
         __fire_io_int(ofile_lst[eth_info->desc_r].owner_pthread_ptr_read);
   } else {
      eth_info->rx_errors++;
   }
}

/* --- PHY ------------------------------------------------------------------------------------ */
static uint32_t _eth_phy_read(dev_stm32f7xx_eth_info_t* info, uint32_t reg){
   uint32_t val = 0;
   if(HAL_ETH_ReadPHYRegister(&eth_handle, info->phy_addr, reg, &val) != HAL_OK)
      return 0xFFFFu;
   return val;
}

static void _eth_phy_write(dev_stm32f7xx_eth_info_t* info, uint32_t reg, uint32_t val){
   (void)HAL_ETH_WritePHYRegister(&eth_handle, info->phy_addr, reg, val);
}

static int _eth_phy_link(dev_stm32f7xx_eth_info_t* info){
   (void)_eth_phy_read(info, MII_BMSR);   /* bit de lien verrouillé à 0 : seconde lecture */
   return (_eth_phy_read(info, MII_BMSR) & MII_BMSR_LINK) ? 1 : 0;
}

/* auto-négociation puis réglage du MAC ; 100 Mb/s en duplex intégral si aucun lien */
static void _eth_phy_negotiate(dev_stm32f7xx_eth_info_t* info){
   ETH_MACConfigTypeDef mac;
   uint32_t start, sr, speed = ETH_SPEED_100M, duplex = ETH_FULLDUPLEX_MODE;
   _eth_phy_write(info, MII_BMCR, MII_BMCR_RESET);
   start = HAL_GetTick();
   while((_eth_phy_read(info, MII_BMCR) & MII_BMCR_RESET) && HAL_GetTick() - start < 500u) {
   }
   _eth_phy_write(info, MII_BMCR, MII_BMCR_ANENABLE | MII_BMCR_ANRESTART);
   start = HAL_GetTick();
   while(HAL_GetTick() - start < ETH_AUTONEG_MS) {
      if((_eth_phy_read(info, MII_BMSR) & (MII_BMSR_ANCOMPLETE | MII_BMSR_LINK))
         == (MII_BMSR_ANCOMPLETE | MII_BMSR_LINK))
         break;
   }
   if(_eth_phy_link(info)) {
      sr = _eth_phy_read(info, info->phy_sr) & info->phy_speed_mask;
      if(sr == info->phy_100_half) {
         duplex = ETH_HALFDUPLEX_MODE;
      } else if(sr == info->phy_10_full) {
         speed = ETH_SPEED_10M;
      } else if(sr == info->phy_10_half) {
         speed = ETH_SPEED_10M;
         duplex = ETH_HALFDUPLEX_MODE;
      }
      info->eth_stat = (speed == ETH_SPEED_100M) ? ETH_STAT_LINK_100 : ETH_STAT_LINK_10;
   } else {
      info->eth_stat = ETH_STAT_LINK_DOWN;
   }
   HAL_ETH_GetMACConfig(&eth_handle, &mac);
   mac.Speed = speed;
   mac.DuplexMode = duplex;
   HAL_ETH_SetMACConfig(&eth_handle, &mac);
}

/* --- démarrage ------------------------------------------------------------------------------ */
/* bloc DMA en mémoire normale non cachable, partageable (TEX 1, C 0, B 0) ; carte par défaut
   conservée pour le reste (PRIVDEFENA) */
static void _eth_mpu_nocache(void){
   MPU_Region_InitTypeDef r;
   SCB_CleanInvalidateDCache();
   HAL_MPU_Disable();
   memset(&r, 0, sizeof(r));
   r.Enable = MPU_REGION_ENABLE;
   r.Number = MPU_REGION_NUMBER0;
   r.BaseAddress = (uint32_t)&eth_dma;
   r.Size = MPU_REGION_SIZE_16KB;
   r.SubRegionDisable = 0x00;
   r.TypeExtField = MPU_TEX_LEVEL1;
   r.AccessPermission = MPU_REGION_FULL_ACCESS;
   r.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;
   r.IsShareable = MPU_ACCESS_SHAREABLE;
   r.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
   r.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;
   HAL_MPU_ConfigRegion(&r);
   HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);
   eth_mpu_done = 1;
}

/* adresse MAC par défaut : administrée localement (0x02), dérivée de l'identifiant unique de
   96 bits de la puce (UID_BASE) : stable et propre à chaque carte */
static void _eth_default_mac(dev_stm32f7xx_eth_info_t* info){
   const uint8_t* uid = (const uint8_t*)UID_BASE;
   unsigned int i;
   info->mac_addr[0] = 0x02;
   for(i = 1; i < 6; i++)
      info->mac_addr[i] = 0;
   for(i = 0; i < 12; i++)
      info->mac_addr[1 + (i % 5)] ^= uid[i];
}

static void _eth_set_mac(dev_stm32f7xx_eth_info_t* info){
   eth_handle.Instance->MACA0HR = ((uint32_t)info->mac_addr[5] << 8) | info->mac_addr[4];
   eth_handle.Instance->MACA0LR = ((uint32_t)info->mac_addr[3] << 24)
                                | ((uint32_t)info->mac_addr[2] << 16)
                                | ((uint32_t)info->mac_addr[1] << 8) | info->mac_addr[0];
}

static void _eth_stop(dev_stm32f7xx_eth_info_t* info){
   if(info->irq_enable)
      info->irq_enable(info, 0);
   if(info->started) {
      (void)HAL_ETH_Stop_IT(&eth_handle);
      (void)HAL_ETH_DeInit(&eth_handle);
   }
   memset((void*)eth_rx_used, 0, sizeof(eth_rx_used));
   eth_rx_pending = NULL;
   info->started = 0;
}

static int _eth_start(dev_stm32f7xx_eth_info_t* info){
   _eth_stop(info);
   if(!eth_mpu_done)
      _eth_mpu_nocache();
   memset(&eth_dma.rx_desc, 0, sizeof(eth_dma.rx_desc) + sizeof(eth_dma.tx_desc));
   eth_info = info;
   eth_handle.Instance = ETH;
   eth_handle.Init.MACAddr = info->mac_addr;
   eth_handle.Init.MediaInterface = HAL_ETH_RMII_MODE;
   eth_handle.Init.RxDesc = eth_dma.rx_desc;
   eth_handle.Init.TxDesc = eth_dma.tx_desc;
   eth_handle.Init.RxBuffLen = ETH_BUF_LEN;
   /* HAL_ETH_MspInit (BSP) : horloges, broches RMII ; échec si le PHY ne fournit pas REF_CLK */
   if(HAL_ETH_Init(&eth_handle) != HAL_OK)
      return -1;
   _eth_phy_negotiate(info);
   if(HAL_ETH_Start_IT(&eth_handle) != HAL_OK)
      return -1;
   info->started = 1;
   if(info->irq_enable)
      info->irq_enable(info, 1);
   return 0;
}

/* --- interface Lepton ----------------------------------------------------------------------- */
int dev_stm32f7xx_eth_x_load(dev_stm32f7xx_eth_info_t* info){
   info->desc_r = -1;
   info->desc_w = -1;
   info->started = 0;
   info->eth_stat = ETH_STAT_LINK_DOWN;
   info->rx_errors = info->rx_dropped = info->rx_no_buffer = info->tx_errors = 0;
   _eth_default_mac(info);
   return 0;
}

int dev_stm32f7xx_eth_x_open(desc_t desc, int o_flag, dev_stm32f7xx_eth_info_t* info){
   /* démarrage à l'ouverture (contexte de processus : la HAL a besoin du tick) */
   if(!info->started && _eth_start(info) < 0)
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

int dev_stm32f7xx_eth_x_close(desc_t desc){
   dev_stm32f7xx_eth_info_t* info = (dev_stm32f7xx_eth_info_t*)ofile_lst[desc].p;
   if(!info)
      return -1;
   if((ofile_lst[desc].oflag & O_RDONLY) && !ofile_lst[desc].nb_reader)
      info->desc_r = -1;
   if((ofile_lst[desc].oflag & O_WRONLY) && !ofile_lst[desc].nb_writer)
      info->desc_w = -1;
   if(info->desc_r < 0 && info->desc_w < 0)
      _eth_stop(info);
   return 0;
}

/* trame suivante, gardée d'avance (NULL si aucune) */
static uint8_t* _eth_rx_peek(dev_stm32f7xx_eth_info_t* info){
   void* buff = NULL;
   if(!eth_rx_pending && info->started && HAL_ETH_ReadData(&eth_handle, &buff) == HAL_OK)
      eth_rx_pending = (uint8_t*)buff;
   return eth_rx_pending;
}

int dev_stm32f7xx_eth_x_isset_read(desc_t desc){
   dev_stm32f7xx_eth_info_t* info = (dev_stm32f7xx_eth_info_t*)ofile_lst[desc].p;
   if(!info || !info->started)
      return -1;
   return _eth_rx_peek(info) ? 0 : -1;
}

int dev_stm32f7xx_eth_x_isset_write(desc_t desc){
   dev_stm32f7xx_eth_info_t* info = (dev_stm32f7xx_eth_info_t*)ofile_lst[desc].p;
   if(!info || !info->started)
      return -1;
   return 0; /* émission synchrone : toujours prêt */
}

/* une trame par appel, sans CRC ; 0 si aucune trame valide n'est disponible */
int dev_stm32f7xx_eth_x_read(desc_t desc, char* buf, int size){
   dev_stm32f7xx_eth_info_t* info = (dev_stm32f7xx_eth_info_t*)ofile_lst[desc].p;
   uint8_t* frame;
   if(!info || !info->started)
      return -1;
   while((frame = _eth_rx_peek(info)) != NULL) {
      int len = eth_rx_len[_eth_rx_index(frame)];
      int keep = len >= ETH_FRAME_MIN && len <= size;
      if(keep)
         memcpy(buf, frame, (size_t)len);
      else if(len == 0)
         info->rx_errors++;
      else
         info->rx_dropped++;
      eth_rx_pending = NULL;
      _eth_rx_free(frame);
      if(keep)
         return len;
   }
   return 0;
}

int dev_stm32f7xx_eth_x_write(desc_t desc, const char* buf, int size){
   dev_stm32f7xx_eth_info_t* info = (dev_stm32f7xx_eth_info_t*)ofile_lst[desc].p;
   ETH_BufferTypeDef tx;
   ETH_TxPacketConfigTypeDef cfg;
   if(!info || !info->started || size < ETH_FRAME_MIN || size > ETH_FRAME_MAX)
      return -1;
   memcpy(eth_dma.tx_buf, buf, (size_t)size);
   tx.buffer = eth_dma.tx_buf;
   tx.len = (uint32_t)size;
   tx.next = NULL;
   memset(&cfg, 0, sizeof(cfg));
   /* sommes de contrôle calculées par lwIP : insertion matérielle contournée explicitement (la
      HAL initialise chaque descripteur en insertion complète, ETH_DMATXDESC_CHECKSUMTCPUDPICMPFULL,
      et n'applique ChecksumCtrl qu'avec l'attribut CSUM ; sinon le MAC recalcule la somme ICMP
      par-dessus celle de lwIP : réponses rejetées par l'hôte, IcmpInCsumErrors) */
   cfg.Attributes = ETH_TX_PACKETS_FEATURES_CRCPAD | ETH_TX_PACKETS_FEATURES_CSUM;
   cfg.CRCPadCtrl = ETH_CRC_PAD_INSERT;
   cfg.ChecksumCtrl = ETH_CHECKSUM_DISABLE;
   cfg.Length = (uint32_t)size;
   cfg.TxBuffer = &tx;
   if(HAL_ETH_Transmit(&eth_handle, &cfg, ETH_TX_TIMEOUT_MS) != HAL_OK)
      info->tx_errors++;
   (void)HAL_ETH_ReleaseTxPacket(&eth_handle);
   return size;
}

int dev_stm32f7xx_eth_x_seek(desc_t desc, int offset, int origin){
   return -1;
}

int dev_stm32f7xx_eth_x_ioctl(desc_t desc, int request, va_list ap){
   dev_stm32f7xx_eth_info_t* info = (dev_stm32f7xx_eth_info_t*)ofile_lst[desc].p;
   if(!info)
      return -1;
   switch(request) {
   case ETHRESET:
      /* réinitialisation demandée par ethif_core : le descripteur qui la demande redevient
         lecteur ou écrivain de l'interface */
      if((ofile_lst[desc].oflag & O_RDONLY) && info->desc_r < 0)
         info->desc_r = desc;
      if((ofile_lst[desc].oflag & O_WRONLY) && info->desc_w < 0)
         info->desc_w = desc;
      return _eth_start(info);

   case ETHSTAT: {
      eth_stat_t* p_eth_stat = va_arg(ap, eth_stat_t*);
      if(!p_eth_stat)
         return -1;
      if(!info->started || !_eth_phy_link(info))
         info->eth_stat = ETH_STAT_LINK_DOWN;
      else if(info->eth_stat == ETH_STAT_LINK_DOWN)
         info->eth_stat = ETH_STAT_LINK_100;   /* lien revenu : vitesse non renégociée */
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
      if(info->started)
         _eth_set_mac(info);
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

void dev_stm32f7xx_eth_x_interrupt(dev_stm32f7xx_eth_info_t* info){
   (void)info;
   __hw_enter_interrupt();
   HAL_ETH_IRQHandler(&eth_handle);
   __hw_leave_interrupt();
}
