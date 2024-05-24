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
| Includes    
==============================================*/
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

#include "kernel/fs/vfs/vfstypes.h"

#include "stm32wlxx_hal.h"

/*============================================
| Global Declaration 
==============================================*/


/*============================================
| Implementation 
==============================================*/
const char dev_stm32wlxx_cpu_x_name[]="cpu0\0";

int dev_stm32wlxx_cpu_x_load(void);
int dev_stm32wlxx_cpu_x_open(desc_t desc, int o_flag);
int dev_stm32wlxx_cpu_x_close(desc_t desc);
int dev_stm32wlxx_cpu_x_isset_read(desc_t desc);
int dev_stm32wlxx_cpu_x_isset_write(desc_t desc);
int dev_stm32wlxx_cpu_x_read(desc_t desc, char* buf,int size);
int dev_stm32wlxx_cpu_x_write(desc_t desc, const char* buf,int size);
int dev_stm32wlxx_cpu_x_seek(desc_t desc,int offset,int origin);
int dev_stm32wlxx_cpu_x_ioctl(desc_t desc,int request,va_list ap);


dev_map_t dev_stm32wlxx_cpu_x_map={
   dev_stm32wlxx_cpu_x_name,
   S_IFBLK,
   dev_stm32wlxx_cpu_x_load,
   dev_stm32wlxx_cpu_x_open,
   dev_stm32wlxx_cpu_x_close,
   dev_stm32wlxx_cpu_x_isset_read,
   dev_stm32wlxx_cpu_x_isset_write,
   dev_stm32wlxx_cpu_x_read,
   dev_stm32wlxx_cpu_x_write,
   dev_stm32wlxx_cpu_x_seek,
   dev_stm32wlxx_cpu_x_ioctl //ioctl
};

//
static uint32_t g_multiplier;

/*===========================================
Implementation
=============================================*/

//
void UsageFault_Handler(void){
   NVIC_SystemReset();
}

//
void BusFault_Handler(void){
   NVIC_SystemReset();
}

//
void MemMang_Handler(void){
   NVIC_SystemReset();
}

//
void HardFault_Handler(void){
   NVIC_SystemReset();
}


// 
void TM_Delay_Init(void) {
    /* While loop takes 4 cycles */
    /* For 1 us delay, we need to divide with 4M */
    g_multiplier = HAL_RCC_GetHCLKFreq() / 4000000;
}
//
void TM_DelayMicros(uint32_t micros) {
    /* Multiply micros with multipler */
    /* Substract 10 */
    micros = micros * g_multiplier - 10;
    /* 4 cycles for one loop */
    while (micros--);
}
//
void TM_DelayMillis(uint32_t millis) {
    /* Multiply millis with multipler */
    /* Substract 10 */
    millis = 1000 * millis * g_multiplier - 10;
    /* 4 cycles for one loop */
    while (millis--);
}

#if 1
//
void HAL_Delay(__IO uint32_t Delay)
{
   __kernel_usleep(Delay);
  //TM_DelayMicros(Delay);
}

//
uint32_t HAL_GetTick(void)
{
  return __kernel_get_timer_ticks();
}
#endif

//stm32wlxx
/**
  * @brief System Clock Configuration
  * @retval None
  */
static void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure LSE Drive Capability
  */
  HAL_PWR_EnableBkUpAccess();
  __HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_LOW);

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSE|RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = RCC_MSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_11;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    return;
  }

  /** Configure the SYSCLKSource, HCLK, PCLK1 and PCLK2 clocks dividers
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK3|RCC_CLOCKTYPE_HCLK
                              |RCC_CLOCKTYPE_SYSCLK|RCC_CLOCKTYPE_PCLK1
                              |RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_MSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.AHBCLK3Divider = RCC_SYSCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    return;
  }
}


 
/* Initializes the IWDG 
   Watchdog freq. is 32 kHz
   Prescaler: Min_Value = 4 and Max_Value = 256
   Reload: Min_Data = 0 and Max_Data = 0x0FFF
   TimeOut in seconds = (Reload * Prescaler) / Freq.
   MinTimeOut = (4 * 1) / 32000 = 0.000125 seconds (125 microseconds)
   MaxTimeOut = (256 * 4096) / 32000 = 32.768 seconds
  
lepton: prescaler 64 reload = 250 Timeout=0.5s
*/
void WatchdogInit(void) {
#if 0     
    hiwdg.Instance = IWDG;
    hiwdg.Init.Prescaler = IWDG_PRESCALER_64; 
    hiwdg.Init.Reload = 250;                
    HAL_IWDG_Init(&hiwdg);
#endif
}
 
/* Refreshes the IWDG. */
void WatchdogRefresh(void) {
#if 0 
    HAL_IWDG_Refresh(&hiwdg);
#endif
}

/*-------------------------------------------
| Name:dev_stm32wlxx_cpu_reset
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
void dev_stm32wlxx_cpu_reset(void){
   NVIC_SystemReset();
}

/*-------------------------------------------
| Name:dev_stm32wlxx_cpu_x_load
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
int dev_stm32wlxx_cpu_x_load(void){
   /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
   HAL_Init();
   /* Configure the system clock */
   SystemClock_Config();
   //
   TM_Delay_Init();
   //
 
   return 0;
}

/*-------------------------------------------
| Name:dev_stm32wlxx_cpu_x_open
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
int dev_stm32wlxx_cpu_x_open(desc_t desc, int o_flag){

   //
   if(o_flag & O_RDONLY){
   }

   if(o_flag & O_WRONLY){
   }

   ofile_lst[desc].offset=0;

   return 0;
}

/*-------------------------------------------
| Name:dev_stm32wlxx_cpu_x_close
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
int dev_stm32wlxx_cpu_x_close(desc_t desc){
   return 0;
}

/*-------------------------------------------
| Name:dev_stm32wlxx_cpu_x_isset_read
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
int dev_stm32wlxx_cpu_x_isset_read(desc_t desc){
  return -1;
}

/*-------------------------------------------
| Name:dev_stm32wlxx_cpu_x_isset_write
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
int dev_stm32wlxx_cpu_x_isset_write(desc_t desc){
      return -1;
}
/*-------------------------------------------
| Name:dev_stm32wlxx_cpu_x_read
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
int dev_stm32wlxx_cpu_x_read(desc_t desc, char* buf,int size){
   return -1;
}

/*-------------------------------------------
| Name:dev_stm32wlxx_cpu_x_write
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
int dev_stm32wlxx_cpu_x_write(desc_t desc, const char* buf,int size){
   //read only mode
   return -1;
}

/*-------------------------------------------
| Name:dev_stm32wlxx_cpu_x_seek
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
int dev_stm32wlxx_cpu_x_seek(desc_t desc,int offset,int origin){

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
| Name:dev_stm32wlxx_cpu_x_ioctl
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
int dev_stm32wlxx_cpu_x_ioctl(desc_t desc,int request,va_list ap){
   
    switch(request){
      //
      case CPUSRST:{
         NVIC_SystemReset();
      }
      break;
      
      //
      case CPUWDGDISABLE :{
      }      
      break;
      
      //
      case CPUWDGREFRESH:{
        WatchdogRefresh();
      }
      break;

      //
      case CPUWDGINIT:{
        WatchdogInit();
      }
      break;
      
      //
      default:
         return -1;

   }
   return 0;     
}

/*============================================
| End of Source  : dev_stm32wlxx_cpu_x.c
==============================================*/
