/*
The contents of this file are subject to the Mozilla Public License Version 1.1
(the "License"); you may not use this file except in compliance with the License.
You may obtain a copy of the License at http://www.mozilla.org/MPL/

Software distributed under the License is distributed on an "AS IS" basis,
WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License for the
specific language governing rights and limitations under the License.

The Original Code is Lepton.

The Initial Developer of the Original Code is Philippe Le Boulanger.
Portions created by Philippe Le Boulanger are Copyright (C) 2015 <lepton.phlb@gmail.com>.
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


/*============================================
| Includes    
==============================================*/
#include <stdint.h>

#include "kernel/core/kernelconf.h"
#include "kernel/core/types.h"
#include "kernel/core/dirent.h"
#include "kernel/core/stat.h"
#include "kernel/core/interrupt.h"
#include "kernel/core/kernel.h"
#include "kernel/core/system.h"
#include "kernel/core/fcntl.h"
#include "kernel/core/ioctl.h"
#include "kernel/core/ioctl_board.h"

#include "kernel/core/kernel_printk.h"

#include "kernel/fs/vfs/vfstypes.h"

#include "board.h"

#include "kernel/dev/arch/cortexm/at91samv7x/dev_samv7x/dev_samv7x_cpu_x.h"
#include "kernel/dev/arch/cortexm/at91samv7x/dev_samv7x/dev_samv7x_adc0.h"

/*============================================
| Global Declaration 
==============================================*/

/** SAMPLES per cycle*/
#define SAMPLES (100)

/** Reference voltage for AFEC in mv. */
#define VOLT_REF        (3300)

/** The maximal digital value */
#define MAX_DIGITAL_12_BIT     (4095UL)
//#define AFEC_TEMPERATURE_SENSOR  11
#define AFEC_ADC_CHANNEL_0   0
#define AFEC_ADC_CHANNEL_8   8
#define AFEC_ADC_CHANNEL_11  11 //temperature sensor

#define AFEC_ACR_PGA0_ON     (0x1u << 2)
#define AFEC_ACR_PGA1_ON     (0x1u << 3)

/** Global AFE DMA instance */
static AfeDma Afed;

/** AFE command instance */
static AfeCmd AfeCommand;

#define ADC_CHANNEL_MAX 12
/** AFE output value */
COMPILER_ALIGNED(32) uint32_t afeOutput[ADC_CHANNEL_MAX];

enum afec_interrupt_source {
	AFEC_INTERRUPT_EOC_0 = 0,
	AFEC_INTERRUPT_EOC_1,
	AFEC_INTERRUPT_EOC_2,
	AFEC_INTERRUPT_EOC_3,
	AFEC_INTERRUPT_EOC_4,
	AFEC_INTERRUPT_EOC_5,
	AFEC_INTERRUPT_EOC_6,
	AFEC_INTERRUPT_EOC_7,
	AFEC_INTERRUPT_EOC_8,
	AFEC_INTERRUPT_EOC_9,
	AFEC_INTERRUPT_EOC_10,
	AFEC_INTERRUPT_EOC_11,
	AFEC_INTERRUPT_DATA_READY,
	AFEC_INTERRUPT_OVERRUN_ERROR,
	AFEC_INTERRUPT_COMP_ERROR,
	AFEC_INTERRUPT_TEMP_CHANGE,
	_AFEC_NUM_OF_INTERRUPT_SOURCE,
	AFEC_INTERRUPT_ALL = 0x47000FFF,
};

//
typedef struct adc_channel_enable_st{
   uint32_t channel_0:1;
   uint32_t channel_1:1;
   uint32_t channel_2:1;
   uint32_t channel_3:1;
   uint32_t channel_4:1;
   uint32_t channel_5:1;
   uint32_t channel_6:1;
   uint32_t channel_7:1;
   uint32_t channel_8:1;
   uint32_t channel_9:1;
   uint32_t channel_10:1;
   uint32_t channel_11:1;   
}adc_channel_enable_t;


const char dev_samv7x_adc0_name[]="adc0\0";

static int dev_samv7x_adc0_load(void);
static int dev_samv7x_adc0_open(desc_t desc, int o_flag);
static int dev_samv7x_adc0_close(desc_t desc);
static int dev_samv7x_adc0_isset_read(desc_t desc);
static int dev_samv7x_adc0_isset_write(desc_t desc);
static int dev_samv7x_adc0_read(desc_t desc, char* buf,int size);
static int dev_samv7x_adc0_write(desc_t desc, const char* buf,int size);
static int dev_samv7x_adc0_seek(desc_t desc,int offset,int origin);
static int dev_samv7x_adc0_ioctl(desc_t desc,int request,va_list ap);


dev_map_t dev_samv7x_adc0_map={
   dev_samv7x_adc0_name,
   S_IFBLK,
   dev_samv7x_adc0_load,
   dev_samv7x_adc0_open,
   dev_samv7x_adc0_close,
   dev_samv7x_adc0_isset_read,
   dev_samv7x_adc0_isset_write,
   dev_samv7x_adc0_read,
   dev_samv7x_adc0_write,
   dev_samv7x_adc0_seek,
   dev_samv7x_adc0_ioctl //ioctl
};

//
typedef struct adc_info_st{
   desc_t desc_r;
   //
   kernel_sem_t kernel_sem_adc_conversion_done;
}adc_info_t;
//
static adc_info_t same70_adc0_info;

/*===========================================
Implementation
=============================================*/
/**
 *  \brief Initialize AFE.
 *
 */
static void _afe_initialization(void) {
	AFEC_Initialize( AFEC0, ID_AFEC0 );

	AFEC_SetModeReg(AFEC0, 0
			| AFEC_MR_FREERUN_ON
			| (1 << AFEC_MR_TRANSFER_Pos)
			| (2 << AFEC_MR_TRACKTIM_Pos)
			| AFEC_MR_ONE
			| AFEC_MR_SETTLING_AST3
			| AFEC_MR_STARTUP_SUT64);

	AFEC_SetClock(AFEC0, 6000000, BOARD_MCK);

	AFEC_SetExtModeReg(AFEC0, 0
         | AFEC_EMR_SIGNMODE_ALL_UNSIGNED
			| AFEC_EMR_RES_NO_AVERAGE
			| AFEC_EMR_TAG
			| AFEC_EMR_STM );

   AFEC0->AFEC_ACR = AFEC_ACR_IBCTL(2)
		| (1 << 4)
		| AFEC_ACR_PGA0_ON
		| AFEC_ACR_PGA1_ON;
   
   //
	AFEC_EnableChannel(AFEC0, AFEC_ADC_CHANNEL_0);
	AFEC_SetChannelGain(AFEC0, AFEC_CGR_GAIN0(AFEC_ADC_CHANNEL_0));
   //
   AFEC_EnableChannel(AFEC0, AFEC_ADC_CHANNEL_8);
	AFEC_SetChannelGain(AFEC0, AFEC_CGR_GAIN0(AFEC_ADC_CHANNEL_8));
	//
   AFEC_EnableChannel(AFEC0, AFEC_ADC_CHANNEL_11);
	AFEC_SetChannelGain(AFEC0, AFEC_CGR_GAIN6(AFEC_ADC_CHANNEL_11));
   AFEC_SetAnalogOffset(AFEC0, AFEC_ADC_CHANNEL_11, 0x200);
}



void AFEC0_Handler(void){
   //
   __hw_enter_interrupt();
   //
   uint32_t status = AFEC_GetStatus(AFEC0);
   //
   uint32_t msk = AFEC_ISR_EOC8|AFEC_ISR_EOC8;
   //
   if((status&msk)==msk){
      AFEC_DisableIt(AFEC0,AFEC_IDR_EOC0|AFEC_IDR_EOC8);
      //
      kernel_sem_post(&same70_adc0_info.kernel_sem_adc_conversion_done);
   }
   //
   __hw_leave_interrupt();
}


/**
 *  \brief Callback function for AFE interrupt
 *
 */
static void _afe_Callback(int dummy, void* pArg)
{
   //
   __hw_enter_interrupt();
   //
	dummy = dummy;
	pArg = pArg;
   //
   kernel_sem_post(&same70_adc0_info.kernel_sem_adc_conversion_done);
   //
   
	//afeValue = (afeOutput & AFEC_LCDR_LDATA_Msk) * VOLT_REF / MAX_DIGITAL_12_BIT;
	/* According to datasheet, The output voltage VT = 0.72V at 27C and the
		temperature slope dVT/dT = 2.33 mV/C */
	//printf("\n\r Temperature is: %4d ", (int) (afeValue - 720) * 100 / 233 + 27);
   
   //
   __hw_leave_interrupt();
   //
}

/**
 *  \brief Configure AFE DMA and start DMA transfer.
 *
 */
static void _afe_dmaTransfer(void)
{
	AfeCommand.RxSize= 2;//ADC_CHANNEL_MAX;
	AfeCommand.pRxBuff = afeOutput;
	AfeCommand.callback = (AfeCallback)_afe_Callback;
	Afe_ConfigureDma(&Afed, AFEC0, ID_AFEC0, &g_samv7x_dmad);
	Afe_SendData(&Afed, &AfeCommand);
}


/*-------------------------------------------
| Name:dev_samv7x_adc0_load
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
static int dev_samv7x_adc0_load(void){  
   //
   same70_adc0_info.desc_r = INVALID_DESC;
   //
   _afe_initialization();
   //
   return 0;
}

/*-------------------------------------------
| Name:dev_samv7x_adc0_open
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
static int dev_samv7x_adc0_open(desc_t desc, int o_flag){

   //
   if(o_flag & O_RDONLY){
      if(same70_adc0_info.desc_r<0) { 
         //
         same70_adc0_info.desc_r = desc;
         //
         kernel_sem_init(&same70_adc0_info.kernel_sem_adc_conversion_done,0,0);
         //
         NVIC_SetPriority(AFEC0_IRQn, (1 << __NVIC_PRIO_BITS) -3u);
         NVIC_ClearPendingIRQ(AFEC0_IRQn);
         NVIC_EnableIRQ(AFEC0_IRQn);
      }else{
         return -1;
      }           
   }
   //
   if(o_flag & O_WRONLY){
      return -1;
   }
   //
   ofile_lst[desc].offset=0;
   //
   return 0;
}

/*-------------------------------------------
| Name:dev_samv7x_adc0_close
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
static int dev_samv7x_adc0_close(desc_t desc){
   return 0;
}

/*-------------------------------------------
| Name:dev_samv7x_adc0_isset_read
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
static int dev_samv7x_adc0_isset_read(desc_t desc){
  return -1;
}

/*-------------------------------------------
| Name:dev_samv7x_adc0_isset_write
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
static int dev_samv7x_adc0_isset_write(desc_t desc){
      return -1;
}
/*-------------------------------------------
| Name:dev_samv7x_adc0_read
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
static int dev_samv7x_adc0_read(desc_t desc, char* buf,int size){
   // start conversion and transfert
   //_afe_dmaTransfer();
   
   //
   AFEC_EnableIt(AFEC0,AFEC_IER_EOC0|AFEC_IER_EOC8);
   AFEC_StartConversion(AFEC0);
   
   //wait end of conversion
   kernel_sem_wait(&same70_adc0_info.kernel_sem_adc_conversion_done);
   //
   if( size>(ADC_CHANNEL_MAX*sizeof(uint32_t)) ){
      size=(ADC_CHANNEL_MAX*sizeof(uint32_t));
   }
   //
   if(ofile_lst[desc].offset+size>=ADC_CHANNEL_MAX*sizeof(uint32_t)){
      size=(ADC_CHANNEL_MAX*sizeof(uint32_t))-(ofile_lst[desc].offset);
      if(size<0){
         return -1;
      }
      if(size==0){
         return 0;
      }
   }
   //
   for(int channel=0;channel<ADC_CHANNEL_MAX;channel++){
      if(channel==11){
         afeOutput[channel] =(AFEC_GetConvertedData(AFEC0,channel)& AFEC_LCDR_LDATA_Msk) * VOLT_REF / MAX_DIGITAL_12_BIT;
      }else{
         uint32_t v = AFEC_GetConvertedData(AFEC0,channel);
         if(v<=2048){
            afeOutput[channel] = 0;
         }else{
            afeOutput[channel] = (AFEC_GetConvertedData(AFEC0,channel)& AFEC_LCDR_LDATA_Msk) * VOLT_REF / MAX_DIGITAL_12_BIT - (VOLT_REF/2);
         }
                
      }
      
   }
   //
   memcpy(buf,&afeOutput[ofile_lst[desc].offset],size);
   //
   return size;
}

/*-------------------------------------------
| Name:dev_samv7x_adc0_write
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
static int dev_samv7x_adc0_write(desc_t desc, const char* buf,int size){
   //read only mode
   return -1;
}

/*-------------------------------------------
| Name:dev_samv7x_adc0_seek
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
static int dev_samv7x_adc0_seek(desc_t desc,int offset,int origin){

   switch(origin){

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
| Name:dev_samv7x_adc0_ioctl
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
static int dev_samv7x_adc0_ioctl(desc_t desc,int request,va_list ap){      
   return 0;     
}

/*============================================
| End of Source  : dev_samv7x_adc0.c
==============================================*/
