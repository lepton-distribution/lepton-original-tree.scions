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
#include "kernel/fs/vfs/vfstypes.h"

#include "stm32wlxx_hal.h"

#include "kernel\dev\arch\cortexm\stm32wlxx\radio_subghz_phy\stm32_radio_target\radio_board_if.h"
#include "kernel\dev\arch\cortexm\stm32wlxx\dev_stm32wlxx\dev_stm32wlxx_bsp_radio_if.h"

#include "kernel\dev\bsp\stm32wl55jci_nucleo\dev_stm32wl55jci_nucleo_radio/stm32wlxx_nucleo_radio.h"

/*===========================================
Global Declaration
=============================================*/

//
static const char dev_stm32wl55jci_nucleo_radio_name[]="radio\0";
//
/*static*/ int dev_stm32wl55jci_nucleo_radio_load(void);
//
extern int dev_stm32wlxx_bsp_radio_if_load(dev_stm32wlxx_bsp_radio_if_t* p_dev_stm32wlxx_bsp_radio_if);
extern int dev_stm32wlxx_bsp_radio_if_open(desc_t desc, int o_flag);
extern int dev_stm32wlxx_bsp_radio_if_close(desc_t desc);
extern int dev_stm32wlxx_bsp_radio_if_read(desc_t desc, char* buf,int cb);
extern int dev_stm32wlxx_bsp_radio_if_write(desc_t desc, const char* buf,int cb);
extern int dev_stm32wlxx_bsp_radio_if_ioctl(desc_t desc,int request,va_list ap);
extern int dev_stm32wlxx_bsp_radio_if_isset_read(desc_t desc);
extern int dev_stm32wlxx_bsp_radio_if_isset_write(desc_t desc);
extern int dev_stm32wlxx_bsp_radio_if_seek(desc_t desc,int offset,int origin);

dev_map_t dev_stm32wl55jci_nucleo_radio_map={
   dev_stm32wl55jci_nucleo_radio_name,
   S_IFCHR,
   dev_stm32wl55jci_nucleo_radio_load,
   dev_stm32wlxx_bsp_radio_if_open,
   dev_stm32wlxx_bsp_radio_if_close,
   dev_stm32wlxx_bsp_radio_if_isset_read,
   dev_stm32wlxx_bsp_radio_if_isset_write,
   dev_stm32wlxx_bsp_radio_if_read,
   dev_stm32wlxx_bsp_radio_if_write,
   dev_stm32wlxx_bsp_radio_if_seek,
   dev_stm32wlxx_bsp_radio_if_ioctl
};


//
static dev_stm32wlxx_bsp_radio_if_t g_dev_stm32wlxx_bsp_radio_if;


/*===========================================
Implementation
=============================================*/


/*-------------------------------------------
| Name:dev_stm32wl55jci_nucleo_radio_load
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
/*static*/ int dev_stm32wl55jci_nucleo_radio_load(void) {
   //
   g_dev_stm32wlxx_bsp_radio_if.pfn_bsp_radio_init                         = (pfn_bsp_radio_init_t)BSP_RADIO_Init;
   g_dev_stm32wlxx_bsp_radio_if.pfn_bsp_radio_deinit                       = (pfn_bsp_radio_deinit_t)BSP_RADIO_DeInit;
   g_dev_stm32wlxx_bsp_radio_if.pfn_bsp_radio_config_RF_switch             = (pfn_bsp_radio_config_RF_switch_t)BSP_RADIO_ConfigRFSwitch;
   g_dev_stm32wlxx_bsp_radio_if.pfn_bsp_radio_init_get_tx_config           = (pfn_bsp_radio_init_get_tx_config_t)BSP_RADIO_GetTxConfig;
   g_dev_stm32wlxx_bsp_radio_if.pfn_bsp_radio_is_TCXO                      = (pfn_bsp_radio_is_TCXO_t)BSP_RADIO_IsTCXO;
   g_dev_stm32wlxx_bsp_radio_if.pfn_bsp_radio_is_DCDC                      = (pfn_bsp_radio_is_DCDC_t)BSP_RADIO_IsDCDC;
   g_dev_stm32wlxx_bsp_radio_if.pfn_bsp_radio_is_get_RFO_max_power_config  = (pfn_bsp_radio_is_get_RFO_max_power_config_t)BSP_RADIO_GetRFOMaxPowerConfig;
   //
   dev_stm32wlxx_bsp_radio_if_load(&g_dev_stm32wlxx_bsp_radio_if);
   //
   return 0;
}
   


/*============================================
| End of Source  : dev_stm32wl55jci_nucleo_radio.c
==============================================*/