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


/*============================================
| Compiler Directive
==============================================*/
#ifndef __DEV_STM32WLXX_BSP_RADIO_IF_H__
#define __DEV_STM32WLXX_BSP_RADIO_IF_H__


/*============================================
| Includes
==============================================*/


/*============================================
| Declaration
==============================================*/

typedef int32_t (*pfn_bsp_radio_init_t)(void);
typedef int32_t (*pfn_bsp_radio_deinit_t)(void);
typedef int32_t (*pfn_bsp_radio_config_RF_switch_t)(RBI_Switch_TypeDef Config);
typedef int32_t (*pfn_bsp_radio_init_get_tx_config_t)(void);
typedef int32_t (*pfn_bsp_radio_is_TCXO_t)(void);
typedef int32_t (*pfn_bsp_radio_is_DCDC_t)(void);
typedef int32_t (*pfn_bsp_radio_is_get_RFO_max_power_config_t)(RBI_RFOMaxPowerConfig_TypeDef Config);

//
typedef struct dev_stm32wlxx_bsp_radio_if_st {
  pfn_bsp_radio_init_t                          pfn_bsp_radio_init;
  pfn_bsp_radio_deinit_t                        pfn_bsp_radio_deinit;
  pfn_bsp_radio_config_RF_switch_t              pfn_bsp_radio_config_RF_switch;
  pfn_bsp_radio_init_get_tx_config_t            pfn_bsp_radio_init_get_tx_config;
  pfn_bsp_radio_is_TCXO_t                       pfn_bsp_radio_is_TCXO;
  pfn_bsp_radio_is_DCDC_t                       pfn_bsp_radio_is_DCDC;
  pfn_bsp_radio_is_get_RFO_max_power_config_t   pfn_bsp_radio_is_get_RFO_max_power_config;
} dev_stm32wlxx_bsp_radio_if_t;

//
dev_stm32wlxx_bsp_radio_if_t* get_stm32wlxx_bsp_radio_if(void);

#endif //__DEV_STM32WLXX_BSP_RADIO_IF_H__
