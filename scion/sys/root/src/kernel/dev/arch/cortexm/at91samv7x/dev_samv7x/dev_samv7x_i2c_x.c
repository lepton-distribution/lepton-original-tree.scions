/*
The contents of this file are subject to the Mozilla Public License Version 1.1
(the "License"); you may not use this file except in compliance with the License.
You may obtain a copy of the License at http://www.mozilla.org/MPL/

Software distributed under the License is distributed on an "AS IS" basis,
WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License for the
specific language governing rights and limitations under the License.

The Original Code is Lepton.

The Initial Developer of the Original Code is Philippe Le Boulanger.
Portions created by Philippe Le Boulanger are Copyright (C) 2017 <lepton.phlb@gmail.com>.
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
#include "kernel/core/dirent.h"
#include "kernel/core/stat.h"
#include "kernel/core/interrupt.h"
#include "kernel/core/kernel.h"
#include "kernel/core/system.h"
#include "kernel/core/malloc.h"
#include "kernel/core/ioctl.h"
#include "kernel/core/fcntl.h"
#include "kernel/core/cpu.h"
#include "kernel/core/ioctl_i2c.h"
#include "kernel/fs/vfs/vfstypes.h"



#include "board.h"

#include "kernel/dev/arch/cortexm/at91samv7x/dev_samv7x/dev_samv7x_cpu_x.h"
#include "kernel/dev/arch/cortexm/at91samv7x/dev_samv7x/dev_samv7x_i2c_x.h"

/*===========================================
Global Declaration
=============================================*/

//#define DMA_USED_THRESHOLD ((int)(16))
#define DMA_USED_THRESHOLD ((int)(1024)) // dma not used, problem with i2c dma and sdio dma.


/*===========================================
Implementation
=============================================*/



/*-------------------------------------------
| Name:dev_samv71x_i2c_x_load
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
int dev_samv71x_i2c_x_load(samv71x_i2c_info_t * i2c_info){
   //
   pthread_mutexattr_t mutex_attr=0;
   kernel_pthread_mutex_init(&i2c_info->mutex,&mutex_attr);
   
   //
   if(i2c_info->speed==0){
      i2c_info->speed=100000;
   }
   //
   i2c_info->twi_hs_dma.pTwid = &i2c_info->twi_id_dma;
   i2c_info->twi_hs_dma.pTwiDma = &g_samv7x_dmad;
   //
   PMC_EnablePeripheral(i2c_info->dwBaseId);
   //
   TWI_ConfigureMaster(i2c_info->p_twi_hs, i2c_info->speed, BOARD_MCK);
   TWID_Initialize(&i2c_info->twi_id, i2c_info->p_twi_hs);
   TWID_DmaInitialize(&i2c_info->twi_hs_dma, i2c_info->p_twi_hs, 0);
   //
   return 0;
}

/*-------------------------------------------
| Name:dev_samv71x_i2c_x_open
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
int dev_samv71x_i2c_x_open(desc_t desc, int o_flag, samv71x_i2c_info_t* i2c_info){


   if(!ofile_lst[desc].p)
      ofile_lst[desc].p=i2c_info;

   //
   return 0;
}

/*-------------------------------------------
| Name:dev_samv71x_i2c_x_close
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
int dev_samv71x_i2c_x_close(desc_t desc){
   //
   samv71x_i2c_info_t* p_i2c_info = (samv71x_i2c_info_t*)ofile_lst[desc].p;   
   //
   return 0;
}

/*-------------------------------------------
| Name:dev_samv71x_i2c_x_isset_read
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
int dev_samv71x_i2c_x_isset_read(desc_t desc){
   return -1;
}

/*-------------------------------------------
| Name:dev_samv71x_i2c_x_isset_write
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
int dev_samv71x_i2c_x_isset_write(desc_t desc){
   return -1;
}
/*-------------------------------------------
| Name:dev_samv71x_i2c_x_read
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
int dev_samv71x_i2c_x_read(desc_t desc, char* buf,int size){
   samv71x_i2c_info_t* p_i2c_info = (samv71x_i2c_info_t*)ofile_lst[desc].p;
   
   uint8_t  i2c_slave_address;
   int16_t  data_length_r=size;
   int error;

   //
   if(!p_i2c_info){
      return -1;
   }
   
   //
   kernel_pthread_mutex_lock(&p_i2c_info->mutex);
   //
   __atomic_in();
   //
   i2c_slave_address =  p_i2c_info->current_slave_address;
   
   //
   if(size>p_i2c_info->twi_data_buffer_sz){
      data_length_r=p_i2c_info->twi_data_buffer_sz;
   }
   
   //
   if(size>=DMA_USED_THRESHOLD){
      error = TWID_DmaRead(&p_i2c_info->twi_hs_dma, i2c_slave_address, 0, 0, p_i2c_info->p_twi_data_buffer, data_length_r, 0);
   }else{
      error = TWID_Read(&p_i2c_info->twi_id, i2c_slave_address, 0, 0, p_i2c_info->p_twi_data_buffer, data_length_r, 0);
   }
     
   //
   if(error==0){
      //
      //SCB_InvalidateDCache_by_Addr((uint32_t *) p_i2c_info->p_twi_data_buffer,data_length_r);
      //
      memcpy(buf, p_i2c_info->p_twi_data_buffer,data_length_r);
   }else{
      data_length_r=-1;
   }
   //
   __atomic_out();
   //
   kernel_pthread_mutex_unlock(&p_i2c_info->mutex);
   //
   return data_length_r;
}

/*-------------------------------------------
| Name:dev_samv71x_i2c_x_write
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
int dev_samv71x_i2c_x_write(desc_t desc, const char* buf,int size){
   samv71x_i2c_info_t* p_i2c_info = (samv71x_i2c_info_t*)ofile_lst[desc].p;
   
   uint8_t  i2c_slave_address;
   int16_t  data_length_w=size;

   //
   if(!p_i2c_info){
      return -1;
   }
   
   //
   kernel_pthread_mutex_lock(&p_i2c_info->mutex);
   //
   __atomic_in();
   //
   i2c_slave_address =  p_i2c_info->current_slave_address;
   
   //
   if(size>p_i2c_info->twi_data_buffer_sz){
      data_length_w=p_i2c_info->twi_data_buffer_sz;
   }
     
   //
   memcpy(p_i2c_info->p_twi_data_buffer,buf,data_length_w);
   //
   //SCB_InvalidateDCache_by_Addr((uint32_t *) p_i2c_info->p_twi_data_buffer,data_length_w);
   //
   if(size>=DMA_USED_THRESHOLD){
      if(TWID_DmaWrite(&p_i2c_info->twi_hs_dma, i2c_slave_address, 0, 0, p_i2c_info->p_twi_data_buffer, data_length_w, 0)!=0){
         data_length_w=-1;
      }
   }else{
       if(TWID_Write(&p_i2c_info->twi_id, i2c_slave_address, 0, 0, p_i2c_info->p_twi_data_buffer, data_length_w, 0)!=0){
         data_length_w=-1;
      }
   }
   //
   __atomic_out();
   //
   kernel_pthread_mutex_unlock(&p_i2c_info->mutex);
   //
   return data_length_w;
}

/*-------------------------------------------
| Name:dev_a0350_board_seek
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
int dev_samv71x_i2c_x_seek(desc_t desc,int offset,int origin){
   return -1;
}

/*-------------------------------------------
| Name:dev_samv71x_i2c_x_ioctl
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
int dev_samv71x_i2c_x_ioctl(desc_t desc,int request,va_list ap){
   samv71x_i2c_info_t* p_i2c_info = (samv71x_i2c_info_t*)ofile_lst[desc].p;
   switch (request){
      //
      case I2C_SLAVEADDR:{
         uint8_t slave_addr;
         
         kernel_pthread_mutex_lock(&p_i2c_info->mutex);
         //
         slave_addr= va_arg( ap, uint8_t);
         p_i2c_info->current_slave_address=slave_addr;
         //
         kernel_pthread_mutex_unlock(&p_i2c_info->mutex);
      }
      break;
      // 
      case I2C_LOCK: 
         kernel_pthread_mutex_lock(&p_i2c_info->mutex);
      break;
      //
      case I2C_UNLOCK:
         kernel_pthread_mutex_unlock(&p_i2c_info->mutex);
      break;
      
      case I2C_RESET:{
         //
         kernel_pthread_mutex_lock(&p_i2c_info->mutex);
         //
         if(p_i2c_info->speed==0){
            p_i2c_info->speed=100000;
         }
         //
         p_i2c_info->twi_hs_dma.pTwid = &p_i2c_info->twi_id_dma;
         p_i2c_info->twi_hs_dma.pTwiDma = &g_samv7x_dmad;
         //
         PMC_EnablePeripheral(p_i2c_info->dwBaseId);
         //
         TWI_ConfigureMaster(p_i2c_info->p_twi_hs, p_i2c_info->speed, BOARD_MCK);
         TWID_Initialize(&p_i2c_info->twi_id, p_i2c_info->p_twi_hs);
         TWID_DmaInitialize(&p_i2c_info->twi_hs_dma, p_i2c_info->p_twi_hs, 0);
         //
         __kernel_usleep(100000);
         //
         kernel_pthread_mutex_unlock(&p_i2c_info->mutex);
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
| End of Source  : dev_samv71x_i2c_x.c
==============================================*/