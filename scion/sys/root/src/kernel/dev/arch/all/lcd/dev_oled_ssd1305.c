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

/* note: https://github.com/olikraus/u8g2/tree/master/csrc */

/*============================================
| Includes
==============================================*/
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>

#include "kernel/core/types.h"
#include "kernel/core/interrupt.h"
#include "kernel/core/kernel.h"
#include "kernel/core/system.h"
#include "kernel/core/malloc.h"
#include "kernel/core/fcntl.h"
#include "kernel/fs/vfs/vfs.h"
#include "kernel/fs/vfs/vfstypes.h"
#include "kernel/core/stat.h"
#include "kernel/core/ioctl.h"
#include "kernel/core/ioctl_lcd.h"
#include "kernel/core/ioctl_spi.h"
#include "kernel/core/ioctl_fb.h"

#include "kernel/dev/arch/all/lcd/dev_oled_ssd1305.h"


/*============================================
| Global Declaration
==============================================*/
//
static const char dev_oled_ssd1305_name[]="lcd0\0";

int dev_oled_ssd1305_load(void);
int dev_oled_ssd1305_open(desc_t desc, int o_flag);
int dev_oled_ssd1305_close(desc_t desc);
int dev_oled_ssd1305_isset_read(desc_t desc);
int dev_oled_ssd1305_isset_write(desc_t desc);
int dev_oled_ssd1305_read(desc_t desc, char* buf,int size);
int dev_oled_ssd1305_write(desc_t desc, const char* buf,int size);
int dev_oled_ssd1305_seek(desc_t desc,int offset,int origin);
int dev_oled_ssd1305_ioctl(desc_t desc,int request,va_list ap);


dev_map_t dev_oled_ssd1305_map={
   dev_oled_ssd1305_name,
   S_IFBLK,
   dev_oled_ssd1305_load,
   dev_oled_ssd1305_open,
   dev_oled_ssd1305_close,
   __fdev_not_implemented,
   __fdev_not_implemented,
   dev_oled_ssd1305_read,
   dev_oled_ssd1305_write,
   dev_oled_ssd1305_seek,
   dev_oled_ssd1305_ioctl
};



#define SSD1305_SETLOWCOLUMN 0x00
#define SSD1305_SETHIGHCOLUMN 0x10
#define SSD1305_MEMORYMODE 0x20
#define SSD1305_SETCOLADDR 0x21
#define SSD1305_SETPAGEADDR 0x22
#define SSD1305_SETSTARTLINE 0x40
#define SSD1305_SETCONTRAST 0x81
#define SSD1305_SETBRIGHTNESS 0x82
#define SSD1305_SETLUT 0x91
#define SSD1305_SEGREMAP 0xA0
#define SSD1305_DISPLAYALLON_RESUME 0xA4
#define SSD1305_DISPLAYALLON 0xA5
#define SSD1305_NORMALDISPLAY 0xA6
#define SSD1305_INVERTDISPLAY 0xA7
#define SSD1305_SETMULTIPLEX 0xA8
#define SSD1305_DISPLAYDIM 0xAC
#define SSD1305_MASTERCONFIG 0xAD
#define SSD1305_DISPLAYOFF 0xAE
#define SSD1305_DISPLAYON 0xAF
#define SSD1305_SETPAGESTART 0xB0
#define SSD1305_COMSCANINC 0xC0
#define SSD1305_COMSCANDEC 0xC8
#define SSD1305_SETDISPLAYOFFSET 0xD3
#define SSD1305_SETDISPLAYCLOCKDIV 0xD5
#define SSD1305_SETAREACOLOR 0xD8
#define SSD1305_SETPRECHARGE 0xD9
#define SSD1305_SETCOMPINS 0xDA
#define SSD1305_SETVCOMLEVEL 0xDB

#define OLED_DISPLAY_X_SIZE  128
#define OLED_DISPLAY_Y_SIZE   32
#define OLED_DISPLAY_BPP       1  //(1bits/pixel)

#define OLED_DISPLAY_SIZE     (OLED_DISPLAY_X_SIZE*OLED_DISPLAY_Y_SIZE)
#define SSD1305_PAGE_NB 4

typedef struct link_info_st{
  desc_t desc_oled_io;
  desc_t desc_spi_w;
  desc_t desc_w;
  uint8_t * p_fbuf;
}link_info_t;

static link_info_t link_info_oled_ssd1305;
#define SSD1305_INTERNAL_RAM_X_SIZE  128
#define SSD1305_INTERNAL_RAM_Y_SIZE   64
#define SSD1305_INTERNAL_RAM_SIZE  ((SSD1305_INTERNAL_RAM_X_SIZE*SSD1305_INTERNAL_RAM_Y_SIZE)/8)
static uint8_t ssd1305_internal_buffer[SSD1305_INTERNAL_RAM_SIZE];

static void ssd1305_reset(desc_t desc_w){   
   uint8_t v8;  
   desc_t desc_oled_io = link_info_oled_ssd1305.desc_oled_io;
   va_list ap={0};
   //
   if(desc_oled_io <0){
      return;
   }
   // RESET
   ofile_lst[desc_oled_io].pfsop->fdev.fdev_ioctl(desc_oled_io,IOCTL_OLED_SSD1305_IO_RESET,ap);
}

static void ssd1305_write_command(desc_t desc_w, uint8_t cmd){   
   uint8_t v8;  
   desc_t desc_oled_io = link_info_oled_ssd1305.desc_oled_io;
   desc_t desc_spi_w = link_info_oled_ssd1305.desc_spi_w;
   va_list ap={0};
   
   //
   if(desc_oled_io <0 || desc_spi_w<0){
      return;
   }

	//CS=0;
   //DC=0;
   ofile_lst[desc_oled_io].pfsop->fdev.fdev_ioctl(desc_oled_io,IOCTL_OLED_SSD1305_IO_CMD_ENTER,ap);
	
   //send SPI data
   v8 = cmd;
   ofile_lst[desc_spi_w].pfsop->fdev.fdev_write(desc_spi_w,(const char *)&v8,sizeof(v8));
	
   //DC=1;
	//CS=1;
   ofile_lst[desc_oled_io].pfsop->fdev.fdev_ioctl(desc_oled_io,IOCTL_OLED_SSD1305_IO_LEAVE,ap);
}

//
static void ssd1305_write_data(desc_t desc_w, uint8_t *data, int len){
	
   desc_t desc_oled_io = link_info_oled_ssd1305.desc_oled_io;
   desc_t desc_spi_w = link_info_oled_ssd1305.desc_spi_w;
   va_list ap={0};
   
   //
   if(desc_oled_io <0 || desc_spi_w<0){
      return;
   }

	//CS=0;
   //DC=1;
   ofile_lst[desc_oled_io].pfsop->fdev.fdev_ioctl(desc_oled_io,IOCTL_OLED_SSD1305_IO_DATA_ENTER,ap);
	
   //send SPI data
   ofile_lst[desc_spi_w].pfsop->fdev.fdev_write(desc_spi_w,(const char *)data,len);
	
   //DC=1;
	//CS=1;
   ofile_lst[desc_oled_io].pfsop->fdev.fdev_ioctl(desc_oled_io,IOCTL_OLED_SSD1305_IO_LEAVE,ap);
}

//
static int ssd1305_write_buffer(desc_t desc_w, uint8_t* buf, int size){
   //buffer rotation
   for(int i=0;i<size;i++){
      //
      uint8_t byte = buf[i];
      //
      for(uint8_t b=0;b<8;b++){
         int j=(i*8+b);
         int x=j%SSD1305_INTERNAL_RAM_X_SIZE;
         int y=j/SSD1305_INTERNAL_RAM_X_SIZE;
         //
         uint8_t bitno=(7-(y%8));
         //(4-((i+(2*128+1))/128))+(i*4)
         uint32_t offset= (x*SSD1305_INTERNAL_RAM_Y_SIZE/(8));
         uint32_t byteno=(SSD1305_INTERNAL_RAM_Y_SIZE/(2*8) - (y/8)) + offset;
         byteno-=1;
         //
         if((byte&(1<<(7-b)))){
            ssd1305_internal_buffer[byteno]|=(1<<bitno);
         }else{
            ssd1305_internal_buffer[byteno]&=~(1<<bitno);
         }
         
      }
   }  
   //
   ssd1305_write_command(desc_w,0x04);
   ssd1305_write_command(desc_w,0x10);
   ssd1305_write_command(desc_w,0x40);
   //send data
   ssd1305_write_data(desc_w, ssd1305_internal_buffer,sizeof(ssd1305_internal_buffer)); 
   //
   return size;
}

//

static int ssd1305_init(desc_t desc_w)
{
	uint8_t data[3];
	int i;
   
   ssd1305_reset(desc_w);
#if 0   
   ssd1305_write_command(desc_w,SSD1305_DISPLAYOFF);                    // 0xAE
   ssd1305_write_command(desc_w,SSD1305_SETLOWCOLUMN | 0x0);  // low col = 0
   ssd1305_write_command(desc_w,SSD1305_SETHIGHCOLUMN | 0x0);  // hi col = 0
   ssd1305_write_command(desc_w,SSD1305_SETSTARTLINE | 0x0); // line #0
   ssd1305_write_command(desc_w,0x2E); //Deactivate scroll 
   ssd1305_write_command(desc_w,SSD1305_SETCONTRAST);                   // 0x81
   ssd1305_write_command(desc_w,0x40);
   ssd1305_write_command(desc_w,SSD1305_SETBRIGHTNESS);                 // 0x82
   ssd1305_write_command(desc_w,0x80);
   ssd1305_write_command(desc_w,SSD1305_SEGREMAP | 0x01);               // 0xA1
   ssd1305_write_command(desc_w,SSD1305_NORMALDISPLAY);                 // 0xA6
   ssd1305_write_command(desc_w,SSD1305_SETMULTIPLEX);                  // 0xA8
   ssd1305_write_command(desc_w,0x3F); // 1/64
   ssd1305_write_command(desc_w,SSD1305_MASTERCONFIG);                  // 0xAD
   ssd1305_write_command(desc_w,0x8e); /* external vcc supply */
   ssd1305_write_command(desc_w,SSD1305_COMSCANDEC);                    // 0xC8
   ssd1305_write_command(desc_w,SSD1305_SETDISPLAYOFFSET);              // 0xD3
   ssd1305_write_command(desc_w,0x40); 
   ssd1305_write_command(desc_w,SSD1305_SETDISPLAYCLOCKDIV);            // 0xD5
   ssd1305_write_command(desc_w,0xf0); 
   ssd1305_write_command(desc_w,SSD1305_SETAREACOLOR);  
   ssd1305_write_command(desc_w,0x05);
   ssd1305_write_command(desc_w,SSD1305_SETPRECHARGE);                  // 0xd9
   ssd1305_write_command(desc_w,0xF1);
   ssd1305_write_command(desc_w,SSD1305_SETCOMPINS);                    // 0xDA
   ssd1305_write_command(desc_w,0x12);
#endif
   
#if 0
   ssd1305_write_command(desc_w,SSD1305_DISPLAYOFF);                    // 0xAE
   ssd1305_write_command(desc_w,SSD1305_SETLOWCOLUMN | 0x0);            // low col = 0
   ssd1305_write_command(desc_w,SSD1305_SETHIGHCOLUMN | 0x0);           // hi col = 0
   ssd1305_write_command(desc_w,SSD1305_SETSTARTLINE | 0x0);            // 0x40 : line #0
   ssd1305_write_command(desc_w,SSD1305_MEMORYMODE);                    // 0x20
   ssd1305_write_command(desc_w,0x00);                                  // page addressing mode
   ssd1305_write_command(desc_w,0x2E); //Deactivate scroll 
   ssd1305_write_command(desc_w,SSD1305_SETCONTRAST);                   // 0x81
   ssd1305_write_command(desc_w,0x32);
   ssd1305_write_command(desc_w,SSD1305_SETBRIGHTNESS);                 // 0x82
   ssd1305_write_command(desc_w,0x80);
#if USE_SSD1305_FLIP_MODE
   ssd1305_write_command(desc_w,SSD1305_SEGREMAP);                      // 0xA0
#else
   ssd1305_write_command(desc_w,SSD1305_SEGREMAP | 0x01);               // 0xA1
#endif
   ssd1305_write_command(desc_w,SSD1305_NORMALDISPLAY);                 // 0xA6
   ssd1305_write_command(desc_w,SSD1305_SETMULTIPLEX);                  // 0xA8
   ssd1305_write_command(desc_w,0x1F); // 1/64
   ssd1305_write_command(desc_w,SSD1305_MASTERCONFIG);                  // 0xAD
   ssd1305_write_command(desc_w,0x8e); /* external vcc supply */
#if USE_SSD1305_FLIP_MODE
   ssd1305_write_command(desc_w,SSD1305_COMSCANINC);                    // 0xC0
#else
   ssd1305_write_command(desc_w,SSD1305_COMSCANDEC);                    // 0xC8
#endif
#if USE_SSD1305_FLIP_MODE
   ssd1305_write_command(desc_w,SSD1305_SETDISPLAYOFFSET);              // 0xD3
   ssd1305_write_command(desc_w,32); 
#else
   ssd1305_write_command(desc_w,SSD1305_SETDISPLAYOFFSET);              // 0xD3
   ssd1305_write_command(desc_w,0x00); 
#endif
   ssd1305_write_command(desc_w,SSD1305_SETDISPLAYCLOCKDIV);            // 0xD5
   ssd1305_write_command(desc_w,0x80); 
   ssd1305_write_command(desc_w,SSD1305_SETVCOMLEVEL);                  // 0xDB
   ssd1305_write_command(desc_w,0x40);                                  // vcomh deselect level 
   ssd1305_write_command(desc_w,SSD1305_SETAREACOLOR);  
   ssd1305_write_command(desc_w,0x05);
   ssd1305_write_command(desc_w,SSD1305_SETPRECHARGE);                  // 0xd9
   ssd1305_write_command(desc_w,0xF1);
   ssd1305_write_command(desc_w,SSD1305_SETCOMPINS);                    // 0xDA
   ssd1305_write_command(desc_w,0x02);
   ssd1305_write_command(desc_w,SSD1305_DISPLAYALLON_RESUME);           // 0xA4

   ssd1305_write_command(desc_w,SSD1305_SETLUT);
   ssd1305_write_command(desc_w,0x3F);
   ssd1305_write_command(desc_w,0x3F);
   ssd1305_write_command(desc_w,0x3F);
   ssd1305_write_command(desc_w,0x3F);
   
   ssd1305_write_command(desc_w,SSD1305_DISPLAYON);                     // 0xAF --turn on oled panel
#endif
   
   /* Set Display OFF */
	ssd1305_write_command(desc_w, 0xAE);
	/* Set Display Clock Divide Ratio/ Oscillator Frequency */
	ssd1305_write_command(desc_w, 0xD5);
	ssd1305_write_command(desc_w, 0x80);
	/* Set Multiplex Ratio */
	ssd1305_write_command(desc_w, 0xA8);
	ssd1305_write_command(desc_w, 0x1F);
	/* Set Display Offset */
	ssd1305_write_command(desc_w, 0xD3);
	ssd1305_write_command(desc_w, 0x0);
	/* Set Display Start Line */
	ssd1305_write_command(desc_w, 0x40 | 0x0);
	/* Charge Pump Setting */
	ssd1305_write_command(desc_w, 0x8D);
	/* A[2] = 1b, Enable charge pump during display on */
	ssd1305_write_command(desc_w, 0x14);
	/* Set Memory Addressing Mode */
	ssd1305_write_command(desc_w, 0x20);
	/* Vertical addressing mode  */
	ssd1305_write_command(desc_w, 0x01);
	/*
	 * Set Segment Re-map
	 * column address 127 is mapped to SEG0
	 */
	ssd1305_write_command(desc_w, 0xA0 | (0x1));
	/*
	 * Set COM Output Scan Direction
	 * remapped mode. Scan from COM[N-1] to COM0
	 */
	ssd1305_write_command(desc_w, (0xC0));
	/* Set COM Pins Hardware Configuration */
	ssd1305_write_command(desc_w, 0xDA);
   /* A[4]=0b, Sequential COM pin configuration */
   ssd1305_write_command(desc_w, 0x12);
	/* Set Pre-charge Period */
	ssd1305_write_command(desc_w, 0xD9);
	ssd1305_write_command(desc_w, 0xF1);
	/*
	 * Entire Display ON
	 * Resume to RAM content display. Output follows RAM content
	 */
	ssd1305_write_command(desc_w, 0xA4);
	/*
	 * Set Normal Display
	 *  0 in RAM: OFF in display panel
	 *  1 in RAM: ON in display panel
	 */
	ssd1305_write_command(desc_w, 0xA6);

	/* Set Display ON */
	ssd1305_write_command(desc_w, 0xAF);

   memset(ssd1305_internal_buffer,0x00,sizeof(ssd1305_internal_buffer));
   
   /*uint8_t page=4;
   for(; page<8; page++){
      ssd1305_write_command(desc_w,SSD1305_SETPAGESTART + page);
      ssd1305_write_command(desc_w,0x00);
      ssd1305_write_command(desc_w,0x10);
      
      //clean display 
      memset(link_info_oled_ssd1305.p_fbuf,0x00,OLED_DISPLAY_SIZE/8);
      ssd1305_write_data(desc_w, link_info_oled_ssd1305.p_fbuf, OLED_DISPLAY_SIZE/8);
   }
   */ 
  
   //clear buffer
   memset(ssd1305_internal_buffer,0x00,sizeof(ssd1305_internal_buffer));
   memset(link_info_oled_ssd1305.p_fbuf, 0x00,OLED_DISPLAY_SIZE/8);
   //
   ssd1305_write_buffer(desc_w,link_info_oled_ssd1305.p_fbuf,OLED_DISPLAY_SIZE/8);
   
   //
   __kernel_usleep(1000000);
   
   //
#if 0
   memset(link_info_oled_ssd1305.p_fbuf,0x11,1);
   memset(link_info_oled_ssd1305.p_fbuf+(256/2),0x11,1);
   memset(link_info_oled_ssd1305.p_fbuf+2*(256/2),0x11,1);
   memset(link_info_oled_ssd1305.p_fbuf+3*(256/2),0x11,1);
   memset(link_info_oled_ssd1305.p_fbuf+4*(256/2),0x11,1);
   ssd1305_write_data(desc_w, link_info_oled_ssd1305.p_fbuf, OLED_DISPLAY_SIZE/8); 
#endif

	return 0;
}




/*-------------------------------------------
| Name:dev_oled_ssd1305_load
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
int dev_oled_ssd1305_load(void){
   link_info_oled_ssd1305.desc_oled_io=-1;
   link_info_oled_ssd1305.desc_spi_w=-1;
   link_info_oled_ssd1305.p_fbuf= (void*)0;
     
   return 0;
}

/*-------------------------------------------
| Name:dev_oled_ssd1305_open
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
int dev_oled_ssd1305_open(desc_t desc, int o_flag){

   //
   if(o_flag & O_RDONLY) 
   {
   }

   if(o_flag & O_WRONLY) {
   }

   ofile_lst[desc].offset=0;

   return 0;
}

/*-------------------------------------------
| Name:dev_oled_ssd1305_close
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
int dev_oled_ssd1305_close(desc_t desc){

   if(ofile_lst[desc].oflag & O_WRONLY) {
      if(!ofile_lst[desc].nb_writer) {
      }
   }

   return 0;
}

/*-------------------------------------------
| Name:dev_oled_ssd1305_isset_read
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
int dev_oled_ssd1305_isset_read(desc_t desc){
   return -1;
}

/*-------------------------------------------
| Name:dev_oled_ssd1305_isset_write
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
int dev_oled_ssd1305_isset_write(desc_t desc){
   return -1;
}

/*-------------------------------------------
| Name:dev_oled_ssd1305_read
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
int dev_oled_ssd1305_read(desc_t desc, char* buf,int size){
   return 0;
}

/*-------------------------------------------
| Name:dev_oled_ssd1305_write
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
int dev_oled_ssd1305_write(desc_t desc, const char* buf,int size){
   int cb=size;
   
   //
   if(ofile_lst[desc].offset+size>=(OLED_DISPLAY_SIZE/8))
      cb=((OLED_DISPLAY_SIZE/8)-ofile_lst[desc].offset);
   //
   memcpy(link_info_oled_ssd1305.p_fbuf+ofile_lst[desc].offset,buf,cb);
   //
	ssd1305_write_data(link_info_oled_ssd1305.desc_w, link_info_oled_ssd1305.p_fbuf, cb);
   //
   ofile_lst[desc].offset+=cb;
   //
   return cb;
}

/*-------------------------------------------
| Name:dev_oled_ssd1305_seek
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
int dev_oled_ssd1305_seek(desc_t desc,int offset,int origin){
   switch(origin) {

   case SEEK_SET:
      ofile_lst[desc].offset=offset;
      break;

   case SEEK_CUR:
      ofile_lst[desc].offset+=offset;
      break;

   case SEEK_END:
      ofile_lst[desc].offset+=offset;
      break;
   }

   return ofile_lst[desc].offset;
}

/*--------------------------------------------
| Name:        dev_oled_ssd1305_ioctl
| Description:
| Parameters:  none
| Return Type: none
| Comments:
| See:
----------------------------------------------*/
int dev_oled_ssd1305_ioctl(desc_t desc,int request,va_list ap){

   switch(request) {
     
    case I_LINK: {
         desc_t desc_oled_io = INVALID_DESC;
         desc_t desc_spi_w = INVALID_DESC;

         //must be open in O_RDWR mode
         if((ofile_lst[desc].oflag&O_RDWR)!=O_RDWR)
            return -1;
         //
         desc_oled_io = ofile_lst[desc].desc_nxt[0];
         if(desc_oled_io==INVALID_DESC)
            return -1;
         //
         desc_spi_w = ofile_lst[desc_oled_io].desc_nxt[0];
         if(desc_spi_w==INVALID_DESC)
            return -1;
         //set info
         link_info_oled_ssd1305.desc_oled_io=desc_oled_io;
         link_info_oled_ssd1305.desc_spi_w=desc_spi_w;
         link_info_oled_ssd1305.desc_w=desc;
         //
         link_info_oled_ssd1305.p_fbuf = _sys_malloc(OLED_DISPLAY_X_SIZE*OLED_DISPLAY_Y_SIZE*2);
         //
         ofile_lst[desc].p=&link_info_oled_ssd1305;
         //
         ssd1305_init(desc);
         //test_lcd_main();
         //
         return 0;
      }
      break;

      //
      case I_UNLINK: {
      }
      break;
      
      //flush internal buffer of lcd device driver
      case LCDFLSBUF:{
         ssd1305_write_buffer(link_info_oled_ssd1305.desc_w,link_info_oled_ssd1305.p_fbuf,OLED_DISPLAY_SIZE/8);
      }
      break;

      //get physical video buffer address
      case LCDGETVADDR: {
         unsigned long* vaddr= va_arg( ap, unsigned long*);
         *vaddr=(unsigned long)link_info_oled_ssd1305.p_fbuf;
      }
      break;


      case LCDGETCONTRAST: {
         unsigned int* p_v= va_arg( ap, unsigned int*);
         *p_v = 0;
      }
      break;

      case LCDGETLUMINOSITY: {
         unsigned int* p_v= va_arg( ap, unsigned int*);
         *p_v = 0;
      }
      break;

      case LCDSETCONTRAST: {
         unsigned int p_v= va_arg( ap, int);
         //to do
      }
      break;

      case LCDSETLUMINOSITY: {
         unsigned int p_v= va_arg( ap, int);
         //to do
      }
      break;

      case LCDSETBACKLIGHTON: {
      }
      break;

      case LCDSETBACKLIGHTOFF: {
      }
      break;

      //color map (palette)
      case FBIOGETCMAP: {
         unsigned int * fbcmap_len = va_arg(ap, unsigned int *);
         fbcmap_t** fbcmap= va_arg( ap, fbcmap_t**);
         //same fields
         *fbcmap_len = 0;
         *fbcmap = (fbcmap_t*)0;
      }
      break;

      case FBIOPUTCMAP: {
         fbcmap_t* fbcmap= va_arg( ap, fbcmap_t*);
        // at91sam9261_lcd_set_lut((const PALETTEENTRY *)fbcmap);
      }
      break;

      case FBIOGET_DISPINFO: {
         fb_info_t * fb_info = va_arg(ap, fb_info_t*);
         if(!fb_info)
            return -1;
         //
         fb_info->x_res=OLED_DISPLAY_X_SIZE;    //x res
         fb_info->y_res=OLED_DISPLAY_Y_SIZE;    //y res
         //
         fb_info->bpp=OLED_DISPLAY_BPP;
         //
         fb_info->line_len=OLED_DISPLAY_X_SIZE;    //line length

         fb_info->smem_start=(unsigned long)link_info_oled_ssd1305.p_fbuf;    //addr of framebuffer
         fb_info->smem_len=OLED_DISPLAY_SIZE/8;    //size of framebuffer

         fb_info->cmap=(fbcmap_t*)0;    //color map from screen
         fb_info->cmap_len=0;   //color map length

         fb_info->desc_w=-1;
         fb_info->next=(void*)0;    //next framebuffer data
      }
      break;



      //
      default:
         return -1;

   }

   return 0;
}

/*============================================
| End of Source  : dev_oled_ssd1305.c
==============================================*/

