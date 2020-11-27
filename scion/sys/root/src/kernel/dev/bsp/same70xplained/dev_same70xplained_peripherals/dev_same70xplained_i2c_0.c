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

#include "kernel/core/kernelconf.h"
#include "kernel/core/types.h"
#include "kernel/core/dirent.h"
#include "kernel/core/stat.h"
#include "kernel/core/interrupt.h"
#include "kernel/core/kernel.h"
#include "kernel/core/system.h"
#include "kernel/core/ioctl.h"
#include "kernel/core/fcntl.h"
#include "kernel/core/cpu.h"
#include "kernel/fs/vfs/vfstypes.h"



#include "board.h"

#include "kernel/dev/arch/cortexm/at91samv7x/dev_samv7x/dev_samv7x_cpu_x.h"
#include "kernel/dev/arch/cortexm/at91samv7x/dev_samv7x/dev_samv7x_i2c_x.h"

/*===========================================
Global Declaration
=============================================*/

static const char dev_same70xplained_i2c_0_name[]="i2c0\0";

static int dev_same70xplained_i2c_0_load(void);
static int dev_same70xplained_i2c_0_open(desc_t desc, int o_flag);


dev_map_t dev_same70xplained_i2c_0_map={
   dev_same70xplained_i2c_0_name,
   S_IFBLK,
   dev_same70xplained_i2c_0_load,
   dev_same70xplained_i2c_0_open,
   dev_samv71x_i2c_x_close,
   dev_samv71x_i2c_x_isset_read,
   dev_samv71x_i2c_x_isset_write,
   dev_samv71x_i2c_x_read,
   dev_samv71x_i2c_x_write,
   dev_samv71x_i2c_x_seek,
   dev_samv71x_i2c_x_ioctl
};

/** TWI0 data pin */
#define SAME70XPLAINED_PINS_TWD0 {PIO_PA3A_TWD0, PIOA, ID_PIOA, PIO_PERIPH_A, PIO_DEFAULT}
/** TWI0 clock pin */
#define SAME70XPLAINED_PINS_TWCK0 {PIO_PA4A_TWCK0, PIOA, ID_PIOA, PIO_PERIPH_A, PIO_DEFAULT}
/** TWI0 pins */
#define SAME70XPLAINED_PINS_TWI0  SAME70XPLAINED_PINS_TWD0, SAME70XPLAINED_PINS_TWCK0

   
// i2c 0 configuration for same70 xplained dev kit.
static const Pin base_i2c0_pins[] = {SAME70XPLAINED_PINS_TWI0};
static samv71x_i2c_info_t same70xplained_i2c_info_0; 

COMPILER_SECTION(".ram_nocache")
COMPILER_ALIGNED(32) uint8_t dma_data_buffer_i2c_0[512];

/*===========================================
Implementation
=============================================*/
/*----------------------------------------------------------------------------
 *        Local functions
 *----------------------------------------------------------------------------*/
/**
 * \brief TWI interrupt handler. Forwards the interrupt to the TWI driver handler.
 */
void TWIHS0_Handler(void)
{
	TWID_Handler(&same70xplained_i2c_info_0.twi_id);
}

/*-------------------------------------------
| Name:dev_same70xplained_i2c_0_load
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
static int dev_same70xplained_i2c_0_load(void) {
   
   same70xplained_i2c_info_0.dwBaseId = ID_TWIHS0;
   same70xplained_i2c_info_0.p_twi_hs=TWIHS0;
   
   //
   same70xplained_i2c_info_0.twi_data_buffer_sz = sizeof(dma_data_buffer_i2c_0);
   same70xplained_i2c_info_0.p_twi_data_buffer = dma_data_buffer_i2c_0;
   
   //
   PIO_Configure(base_i2c0_pins, PIO_LISTSIZE(base_i2c0_pins));
   
   return dev_samv71x_i2c_x_load(&same70xplained_i2c_info_0);
}
   
/*-------------------------------------------
| Name:dev_same70xplained_i2c_0_open
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
static int dev_same70xplained_i2c_0_open(desc_t desc, int o_flag) {
   return dev_samv71x_i2c_x_open(desc, o_flag, &same70xplained_i2c_info_0);
}

/*============================================
| End of Source  : dev_same70xplained_i2c_0.c
==============================================*/