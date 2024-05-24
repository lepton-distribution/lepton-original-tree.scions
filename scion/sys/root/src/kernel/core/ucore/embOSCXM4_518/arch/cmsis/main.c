/*********************************************************************
*                     SEGGER Microcontroller GmbH                    *
*                        The Embedded Experts                        *
**********************************************************************
*                                                                    *
*       (c) 1995 - 2023 SEGGER Microcontroller GmbH                  *
*                                                                    *
*       Internet: segger.com  Support: support_embos@segger.com      *
*                                                                    *
**********************************************************************
*                                                                    *
*       embOS * Real time operating system                           *
*                                                                    *
*       Please note:                                                 *
*                                                                    *
*       Knowledge of this file may under no circumstances            *
*       be used to write a similar product or a real-time            *
*       operating system for in-house use.                           *
*                                                                    *
*       Thank you for your fairness !                                *
*                                                                    *
**********************************************************************
*                                                                    *
*       OS version: V5.18.3.1                                        *
*                                                                    *
**********************************************************************

-------------------------- END-OF-HEADER -----------------------------
File    : OS_StartLEDBlink.c
Purpose : embOS sample program running two simple tasks, each toggling
          an LED of the target hardware (as configured in BSP.c).
*/


#include <stdint.h>

#include "RTOS.h"
#include "kernel/core/kernel.h"

  
#ifdef TEST_BUG_RADIO
//
#include "stm32wlxx_hal.h"
//extern int dev_stm32wl55jci_nucleo_board_load(void)
extern int dev_stm32wl55jci_nucleo_radio_load(void);
extern void test_send_tx(void);


static OS_STACKPTR int StackHP[128], StackLP[128];  // Task stacks
static OS_TASK         TCBHP, TCBLP;                // Task control blocks



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

/**
  * @note This function overwrites the __weak one from HAL
  */
uint32_t HAL_GetTick(void){
  
  return OS_TIME_GetTicks32();
}

/**
  * @note This function overwrites the __weak one from HAL
  */
void HAL_Delay(__IO uint32_t Delay)
{
  OS_Delay(Delay);
}


/*********************************************************************
*
*       HPTask()
*/
static void HPTask(void* pContext) {
  
   /* Initialize all configured peripherals */
   __HAL_RCC_GPIOB_CLK_ENABLE();
   __HAL_RCC_GPIOC_CLK_ENABLE();
   __HAL_RCC_GPIOA_CLK_ENABLE();
  
   //MX_GPIO_Init();
   //MX_SubGHz_Phy_Init();
   dev_stm32wl55jci_nucleo_radio_load();

   //
  while (1) {
    //
    test_send_tx();
    //
    OS_TASK_Delay((OS_TIME)pContext);
    
    //
  }
}

/*********************************************************************
*
*       LPTask()
*/
static void LPTask(void* pContext) {
  while (1) {
    OS_TASK_Delay((OS_TIME)pContext);
  }
}

#endif

/*********************************************************************
*
*       main
*
*********************************************************************/
int main(void) {
  OS_IncDI();                      /* Initially disable interrupts  */
  OS_InitKern();                   /* Initialize OS                 */
  OS_InitHW();                     /* Initialize Hardware for OS    */
  
#ifdef TEST_BUG_RADIO
  //
  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();
  
  //
  SystemClock_Config();


  OS_TASK_CREATEEX(&TCBHP, "HP Task", 100, HPTask, StackHP, (void*)5000);   // Create HPTask
  OS_TASK_CREATEEX(&TCBLP, "LP Task",  50, LPTask, StackLP, (void*)5000);  // Create LPTask
  
#endif
  
  _start_kernel(0);
  
  OS_Start();                      /* Start multitasking            */
  return 0;
}
