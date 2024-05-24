/*
The contents of this file are subject to the Mozilla Public License Version 1.1
(the "License"); you may not use this file except in compliance with the License.
You may obtain a copy of the License at http://www.mozilla.org/MPL/

Software distributed under the License is distributed on an "AS IS" basis,
WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License for the
specific language governing rights and limitations under the License.

The Original Code is Lepton.

The Initial Developer of the Original Code is Philippe Le Boulanger.
Portions created by Philippe Le Boulanger are Copyright (C) 2024 <lepton.phlb@gmail.com>.
All Rights Reserved.

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
#include "kernel/core/ioctl.h"
#include "kernel/core/ioctl_cpu.h"
#include "kernel/core/kernel_ring_buffer.h"

#include "kernel/fs/vfs/vfstypes.h"

#include "stm32wlxx_hal.h"

#include "kernel\dev\arch\cortexm\stm32wlxx\radio_subghz_phy\stm32_radio_target\radio_board_if.h"
#include "kernel\dev\arch\cortexm\stm32wlxx\dev_stm32wlxx\dev_stm32wlxx_bsp_radio_if.h"

#include "kernel\dev\arch\cortexm\stm32wlxx\radio_subghz_phy\radio.h"

/*===========================================
Global Declaration
=============================================*/

//
#define RF_FREQUENCY                                868000000 /* Hz */
//
#define TX_OUTPUT_POWER                             14        /* dBm */
//
#define FSK_FDEV                                    25000     /* Hz */
#define FSK_DATARATE                                50000     /* bps */
#define FSK_BANDWIDTH                               50000     /* Hz */
#define FSK_PREAMBLE_LENGTH                         5         /* Same for Tx and Rx */
#define FSK_FIX_LENGTH_PAYLOAD_ON                   false
//
#define PAYLOAD_LEN                    64
//
#define RX_TIMEOUT_VALUE               3000
#define TX_TIMEOUT_VALUE               3000
/* wait for remote to be in Rx, before sending a Tx frame*/
#define RX_TIME_MARGIN                 200
/* Afc bandwidth in Hz */
#define FSK_AFC_BANDWIDTH              83333

/* Radio events function pointer */
static RadioEvents_t RadioEvents;
/* Radio events function pointer */
static RadioEvents_t RadioEvents;

//
SUBGHZ_HandleTypeDef hsubghz;

//
static dev_stm32wlxx_bsp_radio_if_t* g_p_dev_stm32wlxx_bsp_radio_if;
//
static volatile desc_t g_dev_stm32wlxx_bsp_radio_if_desc_r=INVALID_DESC;
static volatile desc_t g_dev_stm32wlxx_bsp_radio_if_desc_w=INVALID_DESC;

#define RADIO_RX_BUFFER_SIZE            255
#if (PAYLOAD_LEN > RADIO_RX_BUFFER_SIZE)
   #error PAYLOAD_LEN must be less or equal than RADIO_RX_BUFFER_SIZE
#endif /* (PAYLOAD_LEN > RADIO_RX_BUFFER_SIZE) */

#define RADIO_TX_BUFFER_SIZE            255
#if (PAYLOAD_LEN > RADIO_TX_BUFFER_SIZE)
   #error PAYLOAD_LEN must be less or equal than RADIO_TX_BUFFER_SIZE
#endif /* (PAYLOAD_LEN > RADIO_TX_BUFFER_SIZE) */


static uint8_t radio_rx_buffer[RADIO_RX_BUFFER_SIZE]={0};
static uint8_t radio_tx_buffer[RADIO_TX_BUFFER_SIZE]={0};

static kernel_ring_buffer_t krb_usb_radio_rx;


/*===========================================
Implementation
=============================================*/

/**
  * @brief This function handles SUBGHZ Radio Interrupt.
  */
void SUBGHZ_Radio_IRQHandler(void){
  HAL_SUBGHZ_IRQHandler(&hsubghz);
}

/* SUBGHZ init function */
//
void MX_SUBGHZ_Init(void){
  hsubghz.Init.BaudratePrescaler = SUBGHZSPI_BAUDRATEPRESCALER_4;
  if (HAL_SUBGHZ_Init(&hsubghz) != HAL_OK){
    return;
  }
}

//
void HAL_SUBGHZ_MspInit(SUBGHZ_HandleTypeDef* subghzHandle){
    /* SUBGHZ clock enable */
    __HAL_RCC_SUBGHZSPI_CLK_ENABLE();
    /* SUBGHZ interrupt Init */
    //HAL_NVIC_SetPriority(SUBGHZ_Radio_IRQn, 0, 0);
    NVIC_SetPriority(SUBGHZ_Radio_IRQn, (1 << __NVIC_PRIO_BITS) -4); // priority 12
    HAL_NVIC_EnableIRQ(SUBGHZ_Radio_IRQn);
}

//
void HAL_SUBGHZ_MspDeInit(SUBGHZ_HandleTypeDef* subghzHandle){
    /* Peripheral clock disable */
    __HAL_RCC_SUBGHZSPI_CLK_DISABLE();
    /* SUBGHZ interrupt Deinit */
    HAL_NVIC_DisableIRQ(SUBGHZ_Radio_IRQn);
}

//
static void OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t LoraSnr_FskCfo){
   uint8_t flag_do_fire_interrupt=0;
   
   //
   if(g_dev_stm32wlxx_bsp_radio_if_desc_r==-1){
      return;
   }

   //
   __hw_enter_interrupt();
   //
   if (__kernel_ring_buffer_is_empty(krb_usb_radio_rx)) {
      flag_do_fire_interrupt = 1;
   }
   //
   kernel_ring_buffer_write(&krb_usb_radio_rx, (uint8_t*)payload, size);

   /* Record Received Signal Strength*/
   //RssiValue = rssi; 

   //
   if(flag_do_fire_interrupt==1){
      __fire_io_int(ofile_lst[g_dev_stm32wlxx_bsp_radio_if_desc_r].owner_pthread_ptr_read);
   }
   //
   __hw_leave_interrupt();
}

//
static void OnTxDone(void){
   //
   if(g_dev_stm32wlxx_bsp_radio_if_desc_w==-1){
      return;
   }
   //
   __hw_enter_interrupt();
   //
   __fire_io_int(ofile_lst[g_dev_stm32wlxx_bsp_radio_if_desc_w].owner_pthread_ptr_write);
   //
   __hw_leave_interrupt();
}


//
static void OnTxTimeout(void){
}
//
static void OnRxTimeout(void){
}
//
static void OnRxError(void){
}


/*-------------------------------------------
| Name:get_stm32wlxx_bsp_radio_if
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
dev_stm32wlxx_bsp_radio_if_t* get_stm32wlxx_bsp_radio_if(void){
  return g_p_dev_stm32wlxx_bsp_radio_if;
}

/*-------------------------------------------
| Name:dev_stm32wl55jci_nucleo_radio_if_load
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
int dev_stm32wlxx_bsp_radio_if_load(dev_stm32wlxx_bsp_radio_if_t* p_dev_stm32wlxx_bsp_radio_if) {
   //
   g_p_dev_stm32wlxx_bsp_radio_if = p_dev_stm32wlxx_bsp_radio_if;
   
   /* Ensure that MSI is wake-up system clock */
   //__HAL_RCC_WAKEUPSTOP_CLK_CONFIG(RCC_STOP_WAKEUPCLOCK_MSI);

   
   /* Radio initialization */
   RadioEvents.TxDone = OnTxDone;
   RadioEvents.RxDone = OnRxDone;
   RadioEvents.TxTimeout = OnTxTimeout;
   RadioEvents.RxTimeout = OnRxTimeout;
   RadioEvents.RxError = OnRxError;
   //
   Radio.Init(&RadioEvents);
   /*calculate random delay for synchronization*/
   //random_delay = (Radio.Random()) >> 22; /*10bits random e.g. from 0 to 1023 ms*/
   /* Radio Set frequency */
   Radio.SetChannel(RF_FREQUENCY);
   //
   Radio.SetTxConfig(MODEM_FSK, TX_OUTPUT_POWER, FSK_FDEV, 0,
                    FSK_DATARATE, 0,
                    FSK_PREAMBLE_LENGTH, FSK_FIX_LENGTH_PAYLOAD_ON,
                    true, 0, 0, 0, TX_TIMEOUT_VALUE);

   Radio.SetRxConfig(MODEM_FSK, FSK_BANDWIDTH, FSK_DATARATE,
                    0, FSK_AFC_BANDWIDTH, FSK_PREAMBLE_LENGTH,
                    0, FSK_FIX_LENGTH_PAYLOAD_ON, 0, true,
                    0, 0, false, true);

   Radio.SetMaxPayloadLength(MODEM_FSK, RADIO_RX_BUFFER_SIZE);
   
   //kernel ring buffer for radio rx
   kernel_ring_buffer_init(&krb_usb_radio_rx,radio_rx_buffer,RADIO_RX_BUFFER_SIZE);
    
   //
   return 0;
}
   

/*-------------------------------------------
| Name: dev_stm32wlxx_bsp_radio_if_open
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
int dev_stm32wlxx_bsp_radio_if_open(desc_t desc, int o_flag){
   //
   if(o_flag & O_RDONLY) {
      if(g_dev_stm32wlxx_bsp_radio_if_desc_r<0) {
         //
         kernel_ring_buffer_reset(&krb_usb_radio_rx);
         //
         g_dev_stm32wlxx_bsp_radio_if_desc_r=desc;
         //start in continuous rx mode
         Radio.Rx(RX_TIMEOUT_VALUE);
      }else{
         return -1; 
      }
   }
   //
   if(o_flag & O_WRONLY) {
      if(g_dev_stm32wlxx_bsp_radio_if_desc_w<0) {
         //
         g_dev_stm32wlxx_bsp_radio_if_desc_w=desc;
         //
      }else{
         return -1; 
      }
   }
   //
   ofile_lst[desc].offset=0;
   //
   return 0;
}

/*-------------------------------------------
| Name: dev_stm32wlxx_bsp_radio_if_close
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
int dev_stm32wlxx_bsp_radio_if_close(desc_t desc){
  
   // 
   if(ofile_lst[desc].oflag & O_RDONLY) {
      if(!ofile_lst[desc].nb_reader) {
         g_dev_stm32wlxx_bsp_radio_if_desc_r = -1;
      }
   }
   //
   if(ofile_lst[desc].oflag & O_WRONLY) {
      if(!ofile_lst[desc].nb_writer) {
         g_dev_stm32wlxx_bsp_radio_if_desc_w = -1;
      }
   }
   //
   if(g_dev_stm32wlxx_bsp_radio_if_desc_r<0 && g_dev_stm32wlxx_bsp_radio_if_desc_w<0) {
      //
   }
  
    return 0;
}

/*-------------------------------------------
| Name:dev_stm32wlxx_bsp_radio_if_isset_read
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
int dev_stm32wlxx_bsp_radio_if_isset_read(desc_t desc){
   //
   if(__kernel_ring_buffer_is_not_empty(krb_usb_radio_rx)){
      return 0;
   }
   //
   return -1;
}

/*-------------------------------------------
| Name:dev_stm32wlxx_bsp_radio_if_isset_write
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
int dev_stm32wlxx_bsp_radio_if_isset_write(desc_t desc){
   //
   if(g_dev_stm32wlxx_bsp_radio_if_desc_r!=INVALID_DESC) {
      //return in continuous rx mode
      Radio.Rx(RX_TIMEOUT_VALUE);
   }
   //
   return 0;
}
/*-------------------------------------------
| Name:dev_stm32wlxx_bsp_radio_if_read
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
int dev_stm32wlxx_bsp_radio_if_read(desc_t desc, char* buf,int size){
   int cb;
   cb = kernel_ring_buffer_read(&krb_usb_radio_rx,buf,size);
   return cb;
}

/*-------------------------------------------
| Name:dev_stm32wlxx_bsp_radio_if_write
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
int dev_stm32wlxx_bsp_radio_if_write(desc_t desc, const char* buf,int size){
   int cb=size;
   //
   if(size>PAYLOAD_LEN){
      cb=PAYLOAD_LEN;
   }
   //
   memset(radio_tx_buffer,0,sizeof(radio_tx_buffer));
   memcpy(radio_tx_buffer,buf,cb);
   //fixed payload length (to do variable length)
   Radio.Send(radio_tx_buffer, PAYLOAD_LEN);
   //
   return cb;
}

/*-------------------------------------------
| Name:dev_a0350_board_seek
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
int dev_stm32wlxx_bsp_radio_if_seek(desc_t desc,int offset,int origin){

   switch(origin) {

   case SEEK_SET:
      ofile_lst[desc].offset=offset;
      break;

   case SEEK_CUR:
      ofile_lst[desc].offset+=offset;
      break;

   case SEEK_END:
      //to do: warning in SEEK_END (+ or -)????
      ofile_lst[desc].offset-=offset;
      break;
   }

   return ofile_lst[desc].offset;
}

/*-------------------------------------------
| Name:dev_stm32wlxx_bsp_radio_if_ioctl
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
int dev_stm32wlxx_bsp_radio_if_ioctl(desc_t desc,int request,va_list ap){

   switch(request) {
      
      //
      default:
         return -1;

   }

   return 0;
}

/*============================================
| End of Source  : dev_stm32wlxx_bsp_radio_if.c
==============================================*/
