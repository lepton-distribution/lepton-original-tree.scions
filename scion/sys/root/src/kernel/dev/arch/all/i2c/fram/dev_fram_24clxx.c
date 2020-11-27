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
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>

#include "kernel/core/kernelconf.h"
#include "kernel/core/types.h"
#include "kernel/core/dirent.h"
#include "kernel/core/interrupt.h"
#include "kernel/core/kernel.h"
#include "kernel/core/system.h"
#include "kernel/core/fcntl.h"
#include "kernel/core/stat.h"
#include "kernel/core/ioctl.h"
#include "kernel/core/ioctl_i2c.h"
#include "kernel/core/kernel_io.h"

#include "kernel/fs/vfs/vfstypes.h"


/*===========================================
Global Declaration
=============================================*/
#define I2CADDR_ARGC    4
#define FRAMSZ_ARGC     5

static const char dev_fram_24clxx_addr   =  0x68; //already right shift 0x68=(rtc addr)>>1.

static const char dev_fram_24clxx_name[]="frm24cl\0fram_24clxx\0";

static int dev_fram_24clxx_load(void);
static int dev_fram_24clxx_open(desc_t desc, int o_flag);
static int dev_fram_24clxx_close(desc_t desc);
static int dev_fram_24clxx_seek(desc_t desc,int offset,int origin);
static int dev_fram_24clxx_read(desc_t desc, char* buf,int cb);
static int dev_fram_24clxx_write(desc_t desc, const char* buf,int cb);
static int dev_fram_24clxx_seek(desc_t desc,int offset,int origin);
static int dev_fram_24clxx_ioctl(desc_t desc,int request,va_list ap);


//
dev_map_t dev_fram_24clxx_map={
   dev_fram_24clxx_name,
   S_IFBLK,
   dev_fram_24clxx_load,
   dev_fram_24clxx_open,
   dev_fram_24clxx_close,
   __fdev_not_implemented,
   __fdev_not_implemented,
   dev_fram_24clxx_read,
   dev_fram_24clxx_write,
   dev_fram_24clxx_seek,
   dev_fram_24clxx_ioctl
};


typedef struct dev_fram_24clxx_info_st {
   uint8_t  i2c_addr;
   uint32_t size;
   //
   kernel_pthread_mutex_t mutex;
}dev_fram_24clxx_info_t;

static dev_fram_24clxx_info_t  g_dev_fram_24clxx_info;

//
#define FRAM_WRITE_BUFFER_ADDRESS_SIZE   (2)
#define FRAM_WRITE_BUFFER_DATA_SIZE_MAX  (32)
//
#define FRAM_WRITE_BUFFER_SIZE_MAX (FRAM_WRITE_BUFFER_ADDRESS_SIZE+FRAM_WRITE_BUFFER_DATA_SIZE_MAX)
static uint8_t fram_write_buffer[FRAM_WRITE_BUFFER_SIZE_MAX];

/*===========================================
Implementation
=============================================*/


/*-------------------------------------------
| Name: xtoi
| Description:
| Parameters:
| Return Type:
| Comments: 
| See:
---------------------------------------------*/
static unsigned htoi(unsigned u)
{
	if (isdigit(u))
		return (u - '0'); 
	else if (islower(u))
		return (10 + u - 'a'); 
	else if (isupper(u))
		return (10 + u - 'A'); 
	return (16);
}


/*-------------------------------------------
| Name: drv_i2c_fm24clxx_read
| Description:
| Parameters:
| Return Type:
| Comments: 
| See:
---------------------------------------------*/
static int drv_i2c_fm24clxx_read(desc_t desc,char* buf,int size){
   dev_fram_24clxx_info_t * p_dev_fram_24clxx_info = (dev_fram_24clxx_info_t*)ofile_lst[desc].p;
   desc_t desc_i2c_read;
   char bufr[2]={0};
   int cb;
  
   //
   if(!p_dev_fram_24clxx_info)
      return -1;
   //
   if((ofile_lst[desc].offset+size)>=p_dev_fram_24clxx_info->size){
      size = (p_dev_fram_24clxx_info->size - ofile_lst[desc].offset);
   }
   //
   desc_i2c_read=ofile_lst[desc].desc_nxt[0];
   
   //i2c access
   kernel_io_ll_ioctl(desc_i2c_read,I2C_LOCK);
   
   //i2c addr
   kernel_io_ll_ioctl(desc_i2c_read,I2C_SLAVEADDR,p_dev_fram_24clxx_info->i2c_addr);
   
   //
   bufr[0] = (ofile_lst[desc].offset>>8) & 0x00FF;   //msb address
   bufr[1] = (ofile_lst[desc].offset) & 0x00FF;      //lsb address
   //
   if(kernel_io_ll_write(desc_i2c_read,bufr,2)<0){
      //i2c access
      kernel_io_ll_ioctl(desc_i2c_read,I2C_UNLOCK);
      return -1;
   }
         
   //
   if((cb=kernel_io_ll_read(desc_i2c_read,buf,size))<0){
      //i2c access
      kernel_io_ll_ioctl(desc_i2c_read,I2C_UNLOCK);
      return -1;
   }
   
   //i2c access
   kernel_io_ll_ioctl(desc_i2c_read,I2C_UNLOCK);
   
   //
   ofile_lst[desc].offset+=cb;
   //
   return cb;
}

/*-------------------------------------------
| Name: drv_i2c_fm24clxx_write
| Description:
| Parameters:
| Return Type:
| Comments: 
| See:
---------------------------------------------*/
static int drv_i2c_fm24clxx_write(desc_t desc,char* buf,int size){
   dev_fram_24clxx_info_t * p_dev_fram_24clxx_info = (dev_fram_24clxx_info_t*)ofile_lst[desc].p;
   desc_t desc_i2c_write;
   //
   int cb;
  
   //
   if(!p_dev_fram_24clxx_info)
      return -1;
   //
   if(size>FRAM_WRITE_BUFFER_DATA_SIZE_MAX){
      size=FRAM_WRITE_BUFFER_DATA_SIZE_MAX;
   }
   //
   if((ofile_lst[desc].offset+size)>=p_dev_fram_24clxx_info->size){
      size = (p_dev_fram_24clxx_info->size - ofile_lst[desc].offset);
   }
   //
   desc_i2c_write=ofile_lst[desc].desc_nxt[1];
   
   //i2c access
   kernel_io_ll_ioctl(desc_i2c_write,I2C_LOCK);
   
   //i2c addr
   kernel_io_ll_ioctl(desc_i2c_write,I2C_SLAVEADDR,p_dev_fram_24clxx_info->i2c_addr);
   
   //
   fram_write_buffer[0] = (ofile_lst[desc].offset>>8) & 0x00FF;   //msb address
   fram_write_buffer[1] = (ofile_lst[desc].offset) & 0x00FF;      //lsb address
   //
   memcpy(&fram_write_buffer[FRAM_WRITE_BUFFER_ADDRESS_SIZE],buf,size);
   //
   if((cb=kernel_io_ll_write(desc_i2c_write,fram_write_buffer,FRAM_WRITE_BUFFER_ADDRESS_SIZE+size))<0){
      //i2c access
      kernel_io_ll_ioctl(desc_i2c_write,I2C_UNLOCK);
      return -1;
   }
   
   //i2c access
   kernel_io_ll_ioctl(desc_i2c_write,I2C_UNLOCK);
   //
   cb-=FRAM_WRITE_BUFFER_ADDRESS_SIZE;
   //
   ofile_lst[desc].offset+=cb;
   //
   return cb;
}

/*-------------------------------------------
| Name:dev_fram_24clxx_load
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
static int dev_fram_24clxx_load(void){
   //
   g_dev_fram_24clxx_info.i2c_addr = (0xA0>>1);
   g_dev_fram_24clxx_info.size = 8*1024; //8 KBytes
   //
   //
   pthread_mutexattr_t mutex_attr=0;
   kernel_pthread_mutex_init(&g_dev_fram_24clxx_info.mutex,&mutex_attr);
   //
   return 0;
}

/*-------------------------------------------
| Name:dev_fram_24clxx_isset_read
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
static int dev_fram_24clxx_isset_read(desc_t desc){
   return -1;
}

/*-------------------------------------------
| Name:dev_fram_24clxx_isset_write
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
static int dev_fram_24clxx_isset_write(desc_t desc){
   return -1;
}

/*-------------------------------------------
| Name:dev_fram_24clxx_open
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
static int dev_fram_24clxx_open(desc_t desc, int o_flag){
   //
   if(o_flag & O_RDONLY) {
   }

   if(o_flag & O_WRONLY) {
   }
   
   if(!ofile_lst[desc].p){
      ofile_lst[desc].p=&g_dev_fram_24clxx_info;
   }

   ofile_lst[desc].offset = 0;
   return 0;
}

/*-------------------------------------------
| Name:dev_fram_24clxx_close
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
static int dev_fram_24clxx_close(desc_t desc){
   return 0;
}

/*-------------------------------------------
| Name:dev_fram_24clxx_read
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
static int dev_fram_24clxx_read(desc_t desc, char* buf,int cb){
   //
   kernel_pthread_mutex_lock(&g_dev_fram_24clxx_info.mutex);
   //
   cb = drv_i2c_fm24clxx_read(desc,buf,cb);
   //
   kernel_pthread_mutex_unlock(&g_dev_fram_24clxx_info.mutex);
   //
   return cb;
}

/*-------------------------------------------
| Name:dev_fram_24clxx_write
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
static int dev_fram_24clxx_write(desc_t desc, const char* buf,int cb){
   //
   kernel_pthread_mutex_lock(&g_dev_fram_24clxx_info.mutex);
   //
   cb = drv_i2c_fm24clxx_write(desc,buf,cb);
   //
   kernel_pthread_mutex_unlock(&g_dev_fram_24clxx_info.mutex);
   //
   return cb;
}

/*-------------------------------------------
| Name:dev_fram_24clxx_seek
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
static int dev_fram_24clxx_seek(desc_t desc,int offset,int origin){
   dev_fram_24clxx_info_t * p_dev_fram_24clxx_info = (dev_fram_24clxx_info_t*)ofile_lst[desc].p;
   //
   if(p_dev_fram_24clxx_info==(dev_fram_24clxx_info_t*)0)
      return -1;
   //
   switch(origin) {

      case SEEK_SET:
         if(offset>=p_dev_fram_24clxx_info->size)
            return -1;
         ofile_lst[desc].offset=offset;
         break;

      case SEEK_CUR:
         if(ofile_lst[desc].offset+offset>=p_dev_fram_24clxx_info->size)
            return -1;
         ofile_lst[desc].offset+=offset;
         break;

      case SEEK_END:
         //to do: warning in SEEK_END (+ or -)????
         if(ofile_lst[desc].offset+offset>=p_dev_fram_24clxx_info->size)
            return -1;
         ofile_lst[desc].offset+=offset;
         break;
   }

   return ofile_lst[desc].offset;
}

/*-------------------------------------------
| Name:dev_fram_24clxx_ioctl
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
int dev_fram_24clxx_ioctl(desc_t desc,int request,va_list ap) {
    dev_fram_24clxx_info_t * p_dev_fram_24clxx_info = (dev_fram_24clxx_info_t*)ofile_lst[desc].p;

   //
   switch(request) {
      //
      case I_LINK:{
         int argc;
         char** argv;
         int i;
         int fd;
         uint8_t i2c_addr;
         uint16_t chip_id;
         //
         fd=va_arg(ap, int);//not used
         //
         argc=va_arg(ap, int);
         argv=va_arg(ap, char**);
         //
         if(argc<(I2CADDR_ARGC+1))
           return -1;
         //
        
         if(!p_dev_fram_24clxx_info)
            return -1;
   
         //
         for(i=I2CADDR_ARGC; i<argc; i++) {
           
            switch(i){
               //
               case I2CADDR_ARGC:{
                  if(argv[i][0]!='0' || argv[i][1]!='x')
                     return -1;
                  //
                  i2c_addr=htoi(argv[i][2])*16+htoi(argv[i][3]);
                  //
                  p_dev_fram_24clxx_info->i2c_addr=i2c_addr;
               }
               break;
               
               //
               case FRAMSZ_ARGC:{
                  int sz=atoi(argv[i]);
                  if(sz>0){
                     p_dev_fram_24clxx_info->size = atoi(argv[i]);
                  }
               }
               break;
               
               //
               default:
                  //nothing to do
               break;
            }
         }
      }
      return 0;
      
      //   
      case I_UNLINK:
        //nothing to do, see _vfs_ioctl2() 
        //ioctl of device driver is call with request I_LINK, I_UNLINK.
      return 0;   

      //
      default:
         return -1;

   }

   return 0;   
}


/** @} */
/** @} */
/** @} */

/*===========================================
End of Sourcedev_fram_24clxx.c
=============================================*/
