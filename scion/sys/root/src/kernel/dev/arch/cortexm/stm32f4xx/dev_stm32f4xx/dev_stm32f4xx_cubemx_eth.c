/*
The contents of this file are subject to the Mozilla Public License Version 1.1
(the "License"); you may not use this file except in compliance with the License.
You may obtain a copy of the License at http://www.mozilla.org/MPL/

Software distributed under the License is distributed on an "AS IS" basis,
WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License for the
specific language governing rights and limitations under the License.

The Original Code is Lepton.

The Initial Developer of the Original Code is Philippe Le Boulanger.
Portions created by Philippe Le Boulanger are Copyright (C) 2011 <lepton.phlb@gmail.com>.
All Rights Reserved.

Contributor(s): Jean-Jacques Pitrolle <lepton.jjp@gmail.com>.

Alternatively, the contents of this file may be used under the terms of the eCos GPL license
(the  [eCos GPL] License), in which case the provisions of [eCos GPL] License are applicable
instead of those above. If you wish to allow use of your version of this file only under the
terms of the [eCos GPL] License and not to allow others to use your version of this file under
the MPL, indicate your decision by deleting  the provisions above and replace
them with the notice and other provisions required by the [eCos GPL] License.
If you do not delete the provisions above, a recipient may use your version of this file under
either the MPL or the [eCos GPL] License."
*/


/*===========================================
Includes
=============================================*/
#include <stdint.h>

#include "kernel/core/kernelconf.h"
#include "kernel/core/types.h"
#include "kernel/core/interrupt.h"
#include "kernel/core/kernel.h"
#include "kernel/core/system.h"
#include "kernel/core/fcntl.h"
#include "kernel/core/stat.h"
#include "kernel/core/cpu.h"
#include "kernel/core/core_rttimer.h"
#include "kernel/fs/vfs/vfstypes.h"
#include "kernel/core/ioctl_eth.h"

#include "stm32f4xx_hal.h"


/*===========================================
Global Declaration
=============================================*/

#if defined ( __ICCARM__ ) /*!< IAR Compiler */
  #pragma data_alignment=4   
#endif
__ALIGN_BEGIN static ETH_DMADescTypeDef  DMARxDscrTab[ETH_RXBUFNB] __ALIGN_END;/* Ethernet Rx MA Descriptor */

#if defined ( __ICCARM__ ) /*!< IAR Compiler */
  #pragma data_alignment=4   
#endif
__ALIGN_BEGIN static ETH_DMADescTypeDef  DMATxDscrTab[ETH_TXBUFNB] __ALIGN_END;/* Ethernet Tx DMA Descriptor */

#if defined ( __ICCARM__ ) /*!< IAR Compiler */
  #pragma data_alignment=4   
#endif
__ALIGN_BEGIN static uint8_t Rx_Buff[ETH_RXBUFNB][ETH_RX_BUF_SIZE] __ALIGN_END; /* Ethernet Receive Buffer */

#if defined ( __ICCARM__ ) /*!< IAR Compiler */
  #pragma data_alignment=4   
#endif
__ALIGN_BEGIN static uint8_t Tx_Buff[ETH_TXBUFNB][ETH_TX_BUF_SIZE] __ALIGN_END; /* Ethernet Transmit Buffer */

/* Global Ethernet handle */
static ETH_HandleTypeDef heth;

//
static const char dev_stm32f4xx_cubemx_eth_name[]="eth0\0";

static int dev_stm32f4xx_cubemx_eth_load(void);
static int dev_stm32f4xx_cubemx_eth_open(desc_t desc, int o_flag);
//
static int dev_stm32f4xx_cubemx_eth_isset_read(desc_t desc);
static int dev_stm32f4xx_cubemx_eth_isset_write(desc_t desc);
static int dev_stm32f4xx_cubemx_eth_close(desc_t desc);
static int dev_stm32f4xx_cubemx_eth_seek(desc_t desc,int offset,int origin);
static int dev_stm32f4xx_cubemx_eth_read(desc_t desc, char* buf,int cb);
static int dev_stm32f4xx_cubemx_eth_write(desc_t desc, const char* buf,int cb);
static int dev_stm32f4xx_cubemx_eth_ioctl(desc_t desc,int request,va_list ap);
static int dev_stm32f4xx_cubemx_eth_seek(desc_t desc,int offset,int origin);

dev_map_t dev_stm32f4xx_cubemx_eth_map={
   dev_stm32f4xx_cubemx_eth_name,
   S_IFCHR,
   dev_stm32f4xx_cubemx_eth_load,
   dev_stm32f4xx_cubemx_eth_open,
   dev_stm32f4xx_cubemx_eth_close,
   dev_stm32f4xx_cubemx_eth_isset_read,
   dev_stm32f4xx_cubemx_eth_isset_write,
   dev_stm32f4xx_cubemx_eth_read,
   dev_stm32f4xx_cubemx_eth_write,
   dev_stm32f4xx_cubemx_eth_seek,
   dev_stm32f4xx_cubemx_eth_ioctl
};

//
static desc_t desc_eth_r = -1;
static desc_t desc_eth_w = -1;
//
static unsigned int eth_packet_recv_w;
static unsigned int eth_packet_recv_r;

static unsigned int eth_packet_send_w;
static unsigned int eth_packet_send_r;

#define ETH_STM32F4XX_DEFAULT_MAC_ADDR0   0x00
#define ETH_STM32F4XX_DEFAULT_MAC_ADDR1   0xbd
#define ETH_STM32F4XX_DEFAULT_MAC_ADDR2   0x3b
#define ETH_STM32F4XX_DEFAULT_MAC_ADDR3   0x33
#define ETH_STM32F4XX_DEFAULT_MAC_ADDR4   0x05
#define ETH_STM32F4XX_DEFAULT_MAC_ADDR5   0x71

#if 0
   #define ETH_STM32F4XX_DEFAULT_MAC_ADDR0 0x00;
   #define ETH_STM32F4XX_DEFAULT_MAC_ADDR0 0x80;
   #define ETH_STM32F4XX_DEFAULT_MAC_ADDR0 0xE1;
   #define ETH_STM32F4XX_DEFAULT_MAC_ADDR0 0x00;
   #define ETH_STM32F4XX_DEFAULT_MAC_ADDR0 0x00;
   #define ETH_STM32F4XX_DEFAULT_MAC_ADDR0 0x00;
#endif
 

//
typedef struct eth_stm32f4xx_cubemx_info_st {
   //
   uint8_t mac_addr[6];
   //
   uint8_t link_up;
   //ethernet status
   eth_stat_t eth_stat;
}eth_stm32f4xx_cubemx_info_t;

//
static eth_stm32f4xx_cubemx_info_t eth_stm32f4xx_cubemx_info;
/*===========================================
Implementation
=============================================*/



/* interrupt service routine */
void ETH_IRQHandler(void){
   //
   __hw_enter_interrupt();
   //
   HAL_ETH_IRQHandler(&heth);
   //
   __hw_leave_interrupt();
   //   
}

// see HAL_ETH_IRQHandler(ETH_HandleTypeDef *heth) in stm32f4xx_hal_eth.c
/**
  * @brief  Rx Transfer completed callbacks.
  * @param  heth pointer to a ETH_HandleTypeDef structure that contains
  *         the configuration information for ETHERNET module
  * @retval None
  */
void HAL_ETH_RxCpltCallback(ETH_HandleTypeDef *heth){   
   if(desc_eth_r!=-1 /*&& eth_packet_recv_r==eth_packet_recv_w*/){
      eth_packet_recv_w++;
      __fire_io_int(ofile_lst[desc_eth_r].owner_pthread_ptr_read);         
   }     
}
/**
  * @brief  Tx Transfer completed callbacks.
  * @param  heth pointer to a ETH_HandleTypeDef structure that contains
  *         the configuration information for ETHERNET module
  * @retval None
 */
void HAL_ETH_TxCpltCallback(ETH_HandleTypeDef *heth){   
   eth_packet_send_r++;
   //lepton: optimzation nt empty -> to empty
   if(desc_eth_w!=-1 && eth_packet_send_r==eth_packet_send_w){
      __fire_io_int(ofile_lst[desc_eth_w].owner_pthread_ptr_write);
   }     
}

/*-------------------------------------------
| Name: stm32f4xx_ethernet_nvic_configuration
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
static void stm32f4xx_ethernet_nvic_configuration(void){         
   //
   NVIC_SetPriority((IRQn_Type)ETH_IRQn, (1 << __NVIC_PRIO_BITS) -4);
   NVIC_EnableIRQ(ETH_IRQn);
}

/*-------------------------------------------
| Name: stm32f4xx_ethernet_low_level_init
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
static int stm32f4xx_ethernet_low_level_init(void)
{ 
   uint32_t regvalue = 0;
   HAL_StatusTypeDef hal_eth_init_status;
   
   //
   heth.Instance = ETH;
   heth.Init.AutoNegotiation = ETH_AUTONEGOTIATION_DISABLE;
   heth.Init.Speed = ETH_SPEED_100M;
   heth.Init.DuplexMode = ETH_MODE_FULLDUPLEX;
   heth.Init.PhyAddress = DP83848_PHY_ADDRESS;   
   //   
   heth.Init.MACAddr = &eth_stm32f4xx_cubemx_info.mac_addr[0];
   //
   heth.Init.RxMode = ETH_RXINTERRUPT_MODE;
   heth.Init.ChecksumMode = ETH_CHECKSUM_BY_SOFTWARE; //ETH_CHECKSUM_BY_HARDWARE;
   heth.Init.MediaInterface = ETH_MEDIA_INTERFACE_MII;
   //
   hal_eth_init_status = HAL_ETH_Init(&heth);
   if (hal_eth_init_status != HAL_OK){
     /* Set netif link flag */  
     //netif->flags |= NETIF_FLAG_LINK_UP;
     //nothing to do
   }
   
   //
   __HAL_ETH_DMA_ENABLE_IT((&heth), ETH_DMA_IT_NIS | ETH_DMA_IT_R| ETH_DMA_IT_T);
   
   /* Initialize Tx Descriptors list: Chain Mode */
   HAL_ETH_DMATxDescListInit(&heth, DMATxDscrTab, &Tx_Buff[0][0], ETH_TXBUFNB);
     
   /* Initialize Rx Descriptors list: Chain Mode  */
   HAL_ETH_DMARxDescListInit(&heth, DMARxDscrTab, &Rx_Buff[0][0], ETH_RXBUFNB);

   //interrupt vector an priority configuration
   stm32f4xx_ethernet_nvic_configuration();
   
   /* Enable MAC and DMA transmission and reception */
   HAL_ETH_Start(&heth);

   /* USER CODE BEGIN PHY_PRE_CONFIG */ 
    
   /* USER CODE END PHY_PRE_CONFIG */

   /**** Configure PHY to generate an interrupt when Eth Link state changes ****/
   /* Read Register Configuration */
   HAL_ETH_ReadPHYRegister(&heth, PHY_MICR, &regvalue);

   regvalue |= (PHY_MICR_INT_EN | PHY_MICR_INT_OE);

   /* Enable Interrupts */
   HAL_ETH_WritePHYRegister(&heth, PHY_MICR, regvalue );

   /* Read Register Configuration */
   HAL_ETH_ReadPHYRegister(&heth, PHY_MISR, &regvalue);

   regvalue |= PHY_MISR_LINK_INT_EN;
    
   /* Enable Interrupt on change of link status */
   HAL_ETH_WritePHYRegister(&heth, PHY_MISR, regvalue);

   /* USER CODE BEGIN PHY_POST_CONFIG */ 
    
   /* USER CODE END PHY_POST_CONFIG */

   /* USER CODE BEGIN LOW_LEVEL_INIT */ 
    
   /* USER CODE END LOW_LEVEL_INIT */
   
   return 0;
}

/*-------------------------------------------
| Name:dev_stm32f4xx_cubemx_eth_load
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
int dev_stm32f4xx_cubemx_eth_load(void){
   
   /*Set default MAC Address*/
   eth_stm32f4xx_cubemx_info.mac_addr[0]=ETH_STM32F4XX_DEFAULT_MAC_ADDR0;
   eth_stm32f4xx_cubemx_info.mac_addr[1]=ETH_STM32F4XX_DEFAULT_MAC_ADDR1;
   eth_stm32f4xx_cubemx_info.mac_addr[2]=ETH_STM32F4XX_DEFAULT_MAC_ADDR2;
   eth_stm32f4xx_cubemx_info.mac_addr[3]=ETH_STM32F4XX_DEFAULT_MAC_ADDR3;
   eth_stm32f4xx_cubemx_info.mac_addr[4]=ETH_STM32F4XX_DEFAULT_MAC_ADDR4;
   eth_stm32f4xx_cubemx_info.mac_addr[5]=ETH_STM32F4XX_DEFAULT_MAC_ADDR5;
   
   //
   GPIO_InitTypeDef GPIO_InitStruct = {0};  
   /* Enable Peripheral clock */
   __HAL_RCC_ETH_CLK_ENABLE();

   __HAL_RCC_GPIOE_CLK_ENABLE();
   __HAL_RCC_GPIOG_CLK_ENABLE();
   __HAL_RCC_GPIOC_CLK_ENABLE();
   __HAL_RCC_GPIOA_CLK_ENABLE();
   __HAL_RCC_GPIOH_CLK_ENABLE();
   /**ETH GPIO Configuration    
   PE2     ------> ETH_TXD3
   PG14     ------> ETH_TXD1
   PG13     ------> ETH_TXD0
   PG11     ------> ETH_TX_EN
   PC3     ------> ETH_TX_CLK
   PC1     ------> ETH_MDC
   PC2     ------> ETH_TXD2
   PA1     ------> ETH_RX_CLK
   PC4     ------> ETH_RXD0
   PH7     ------> ETH_RXD3
   PA2     ------> ETH_MDIO
   PC5     ------> ETH_RXD1
   PH6     ------> ETH_RXD2
   PA7     ------> ETH_RX_DV 
   */
   GPIO_InitStruct.Pin = MII_TXD3_Pin;
   GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
   GPIO_InitStruct.Pull = GPIO_NOPULL;
   GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
   GPIO_InitStruct.Alternate = GPIO_AF11_ETH;
   HAL_GPIO_Init(MII_TXD3_GPIO_Port, &GPIO_InitStruct);

   GPIO_InitStruct.Pin = MII_TXD1_Pin|MII_TXD0_Pin|MII_TX_EN_Pin;
   GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
   GPIO_InitStruct.Pull = GPIO_NOPULL;
   GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
   GPIO_InitStruct.Alternate = GPIO_AF11_ETH;
   HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

   GPIO_InitStruct.Pin = MII_TX_CLK_Pin|MII_MDC_Pin|MII_TXD2_Pin|MII_RXD0_Pin 
                        |MII_RXD1_Pin;
   GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
   GPIO_InitStruct.Pull = GPIO_NOPULL;
   GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
   GPIO_InitStruct.Alternate = GPIO_AF11_ETH;
   HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

   GPIO_InitStruct.Pin = MII_RX_CLK_Pin|MII_MDIO_Pin|MII_RX_DV_Pin;
   GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
   GPIO_InitStruct.Pull = GPIO_NOPULL;
   GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
   GPIO_InitStruct.Alternate = GPIO_AF11_ETH;
   HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

   GPIO_InitStruct.Pin = MII_RXD3_Pin|MII_RXD2_Pin;
   GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
   GPIO_InitStruct.Pull = GPIO_NOPULL;
   GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
   GPIO_InitStruct.Alternate = GPIO_AF11_ETH;
   HAL_GPIO_Init(GPIOH, &GPIO_InitStruct);
   //
   return 0;
}

/*-------------------------------------------
| Name:dev_stm32f4xx_cubemx_eth_open
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
int dev_stm32f4xx_cubemx_eth_open(desc_t desc, int o_flag){
   //
   if(desc_eth_r<0 && desc_eth_w<0) {
     //first startup
     // start ethernet periheral  
     stm32f4xx_ethernet_low_level_init();
   }
   //
   if(o_flag & O_RDONLY) {
      if(desc_eth_r<0) {
         desc_eth_r = desc;
         //
         eth_packet_recv_r = 0;
         eth_packet_recv_w = 0;
         //
         if(!ofile_lst[desc].p)
            ofile_lst[desc].p=&eth_stm32f4xx_cubemx_info;
         //
         HAL_ETH_RegisterCallback(&heth,HAL_ETH_RX_COMPLETE_CB_ID,HAL_ETH_RxCpltCallback);
      }
      else
         return -1;                //already open
   }

   if(o_flag & O_WRONLY) {
      if(desc_eth_w<0) {
         desc_eth_w = desc;
         //
         eth_packet_send_w = 0;
         eth_packet_send_r = 0;
         //
         if(!ofile_lst[desc].p)
            ofile_lst[desc].p=&eth_stm32f4xx_cubemx_info;
         //
         HAL_ETH_RegisterCallback(&heth,HAL_ETH_TX_COMPLETE_CB_ID,HAL_ETH_TxCpltCallback);
      }
      else
         return -1;                //already open
   }

   //unmask IRQ
   if(desc_eth_r>=0 && desc_eth_w>=0) {
     
   }
   return 0;
}

/*-------------------------------------------
| Name:dev_stm32f4xx_cubemx_eth_close
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
int dev_stm32f4xx_cubemx_eth_close(desc_t desc){
  // 
  if(ofile_lst[desc].oflag & O_RDONLY) {
      if(!ofile_lst[desc].nb_reader) {
         desc_eth_r = -1;
      }
   }
   //
   if(ofile_lst[desc].oflag & O_WRONLY) {
      if(!ofile_lst[desc].nb_writer) {
         desc_eth_w = -1;
      }
   }
   //
   if(desc_eth_r<0 && desc_eth_w<0) {
     //to do:stop ethernet periheral
   }

   return 0;
}

/*-------------------------------------------
| Name:dev_stm32f4xx_cubemx_eth_isset_read
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
int dev_stm32f4xx_cubemx_eth_isset_read(desc_t desc){
   if(desc_eth_r!=-1 && HAL_ETH_GetReceivedFrame_IT(&heth) == HAL_OK){
      return 0;//packet available
   }  
   return -1;
}

/*-------------------------------------------
| Name:dev_stm32f4xx_cubemx_eth_isset_write
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
int dev_stm32f4xx_cubemx_eth_isset_write(desc_t desc){
   if(desc_eth_w != -1 && eth_packet_send_r==eth_packet_send_w){
      return 0;//packet was sent
   }
   return -1;
}

/*-------------------------------------------
| Name:dev_stm32f4xx_cubemx_eth_read
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
int dev_stm32f4xx_cubemx_eth_read(desc_t desc, char* p_to_user_buffer,int size){
   uint16_t cb = 0;
   uint8_t *buffer;
   __IO ETH_DMADescTypeDef *dmarxdesc;  
   uint32_t i=0;
  
   /* get received frame */
   /*
   if (HAL_ETH_GetReceivedFrame_IT(&heth) != HAL_OK){  
      return 0;
   }*/
   
   if(heth.RxFrameInfos.SegCount==0 || heth.RxFrameInfos.length==0){
      return 0;
   }
  
   /* Obtain the size of the packet and put it into the "len" variable. */
   cb = heth.RxFrameInfos.length;
   buffer = (uint8_t *)heth.RxFrameInfos.buffer;
   //
   if(cb>size){
      //to do: drop packet
      //
      eth_packet_recv_r++;
      //
      return 0;
   }  
   
   //first segment
   dmarxdesc = heth.RxFrameInfos.FSRxDesc;
   
   //
   memcpy(p_to_user_buffer,buffer,cb);
   
   /* Release descriptors to DMA */
   /* Point to first descriptor */
   dmarxdesc = heth.RxFrameInfos.FSRxDesc;
   /* Set Own bit in Rx descriptors: gives the buffers back to DMA */
   for (i=0; i< heth.RxFrameInfos.SegCount; i++){  
      dmarxdesc->Status |= ETH_DMARXDESC_OWN;
      dmarxdesc = (ETH_DMADescTypeDef *)(dmarxdesc->Buffer2NextDescAddr);
   }
    
   /* Clear Segment_Count */
   heth.RxFrameInfos.SegCount =0;  
  
   /* When Rx Buffer unavailable flag is set: clear it and resume reception */
   if ((heth.Instance->DMASR & ETH_DMASR_RBUS) != (uint32_t)RESET){
      /* Clear RBUS ETHERNET DMA flag */
      heth.Instance->DMASR = ETH_DMASR_RBUS;
      /* Resume DMA reception */
      heth.Instance->DMARPDR = 0;
   }    
   //
   eth_packet_recv_r++;
   //
   return cb;
}

/*-------------------------------------------
| Name:dev_stm32f4xx_cubemx_eth_write
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
int dev_stm32f4xx_cubemx_eth_write(desc_t desc, const char* p_from_user_buffer,int size){
  
   uint8_t *buffer = (uint8_t *)(heth.TxDesc->Buffer1Addr);
  __IO ETH_DMADescTypeDef *DmaTxDesc;
  DmaTxDesc = heth.TxDesc;  
   
   //
   DmaTxDesc = heth.TxDesc;
      
   /* Is this buffer available? If not, goto error */
   if((DmaTxDesc->Status & ETH_DMATXDESC_OWN) != (uint32_t)RESET){
      //errval = ERR_USE;
      //
      size = -1;
      //
      goto error;
   }

   //
   if(size>ETH_TX_BUF_SIZE){
      return -1;
   }

   //
   memcpy((uint8_t*)buffer, (uint8_t*)p_from_user_buffer, size);

   // 
   eth_packet_send_w++;

   /* Prepare transmit descriptors to give to DMA */ 
   HAL_ETH_TransmitFrame(&heth, size);
   
   eth_packet_send_r++;
   //temp code grouik (DMA TX interrupt issue, will be fix soon)
   if(desc_eth_w!=-1){
      __fire_io_int(ofile_lst[desc_eth_w].owner_pthread_ptr_write);
   }
     
   //
error:  
   /* When Transmit Underflow flag is set, clear it and issue a Transmit Poll Demand to resume transmission */
   if ((heth.Instance->DMASR & ETH_DMASR_TUS) != (uint32_t)RESET){
      /* Clear TUS ETHERNET DMA flag */
      heth.Instance->DMASR = ETH_DMASR_TUS;

      /* Resume DMA transmission*/
      heth.Instance->DMATPDR = 0;
   }
   //
   return size;
}

/*-------------------------------------------
| Name:dev_stm32f4xx_cubemx_eth_seek
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
int dev_stm32f4xx_cubemx_eth_seek(desc_t desc,int offset,int origin){
   return -1;
}

/*-------------------------------------------
| Name:dev_stm32f4xx_cubemx_eth_ioctl
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
int dev_stm32f4xx_cubemx_eth_ioctl(desc_t desc,int request,va_list ap) {
   switch(request) {

   //reset interface
   case ETHRESET: {
      eth_stm32f4xx_cubemx_info_t * p_eth_info=(eth_stm32f4xx_cubemx_info_t *)ofile_lst[desc].p;
      if(!p_eth_info)
         return -1;
   }
   break;

   //status interface
   case ETHSTAT: {
      eth_stm32f4xx_cubemx_info_t* p_eth_info=(eth_stm32f4xx_cubemx_info_t *)ofile_lst[desc].p;
      eth_stat_t* p_eth_stat = va_arg( ap, eth_stat_t*);
      if(!p_eth_info || !p_eth_stat)
         return -1;
      //
      *p_eth_stat = p_eth_info->eth_stat;
   }
   break;

   case ETHSETHWADDRESS: {
      eth_stm32f4xx_cubemx_info_t* p_eth_info=(eth_stm32f4xx_cubemx_info_t *)ofile_lst[desc].p;
      unsigned char* p_eth_hwaddr = va_arg( ap, unsigned char*);
      if(!p_eth_info || !p_eth_hwaddr)
         return -1;
      //stop ehternet interface
     
      // Disable all interrupt
     
      //set mac address
      p_eth_info->mac_addr[0] = p_eth_hwaddr[0];
      p_eth_info->mac_addr[1] = p_eth_hwaddr[1];
      p_eth_info->mac_addr[2] = p_eth_hwaddr[2];
      p_eth_info->mac_addr[3] = p_eth_hwaddr[3];
      p_eth_info->mac_addr[4] = p_eth_hwaddr[4];
      p_eth_info->mac_addr[5] = p_eth_hwaddr[5];

      //reopen and restart ethernet interface
      //dmfe_open(p_eth_info);
   }
   break;

   case ETHGETHWADDRESS: {
      eth_stm32f4xx_cubemx_info_t* p_eth_info=(eth_stm32f4xx_cubemx_info_t *)ofile_lst[desc].p;
      unsigned char* p_eth_hwaddr = va_arg( ap, unsigned char*);
      if(!p_eth_info || !p_eth_hwaddr)
         return -1;
      p_eth_hwaddr[0] = p_eth_info->mac_addr[0];
      p_eth_hwaddr[1] = p_eth_info->mac_addr[1];
      p_eth_hwaddr[2] = p_eth_info->mac_addr[2];
      p_eth_hwaddr[3] = p_eth_info->mac_addr[3];
      p_eth_hwaddr[4] = p_eth_info->mac_addr[4];
      p_eth_hwaddr[5] = p_eth_info->mac_addr[5];
   }
   break;

   //
   default:
      return -1;
   }

   //
   return 0;
}


/*============================================
| End of Source  : dev_stm32f4xx_cubemx_eth.c
==============================================*/