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
Purpose : BSP for ST STM32WL55JC2 Nucleo
*/

#include "BSP.h"

/*********************************************************************
*
*       Defines
*
**********************************************************************
*/
#define LED0_BIT         (15)  // LED1, Blue
#define LED1_BIT         (9)   // LED2, Green
#define LED2_BIT         (11)  // LED3, Red

#define RCC_BASE_ADDR    (0x58000000u)
#define RCC_AHB2ENR      (*(volatile unsigned int*)(RCC_BASE_ADDR + 0x4Cu))    // AHB2 peripheral clock enable register

#define GPIOB_BASE_ADDR  (0x48000400u)
#define GPIOB_MODER      (*(volatile unsigned int*)(GPIOB_BASE_ADDR + 0x00u))  // GPIOB port mode register
#define GPIOB_IDR        (*(volatile unsigned int*)(GPIOB_BASE_ADDR + 0x10u))  // GPIOB input data register
#define GPIOB_BSRR       (*(volatile unsigned int*)(GPIOB_BASE_ADDR + 0x18u))  // GPIOB bit set/reset register


/*********************************************************************
*
*       Global functions
*
**********************************************************************
*/

/*********************************************************************
*
*       BSP_Init()
*/
void BSP_Init(void) {
  RCC_AHB2ENR |=  (1u << 1);               // Enable GPIOB clock
  GPIOB_MODER &= ~(3u << (LED0_BIT * 2))   // Clear mode bits
              &  ~(3u << (LED1_BIT * 2))
              &  ~(3u << (LED2_BIT * 2));
  GPIOB_MODER |=  (1u << (LED0_BIT * 2))   // Set mode to output
              |   (1u << (LED1_BIT * 2))
              |   (1u << (LED2_BIT * 2));
}

/*********************************************************************
*
*       BSP_SetLED()
*/
void BSP_SetLED(int Index) {
  switch (Index) {
  case 0:
    GPIOB_BSRR |= (1u << (LED0_BIT));
    break;
  case 1:
    GPIOB_BSRR |= (1u << (LED1_BIT));
    break;
  case 2:
    GPIOB_BSRR |= (1u << (LED2_BIT));
    break;
  default:
    break;
  }
}

/*********************************************************************
*
*       BSP_ClrLED()
*/
void BSP_ClrLED(int Index) {
  switch (Index) {
  case 0:
    GPIOB_BSRR |= (1u << (LED0_BIT + 16u));
    break;
  case 1:
    GPIOB_BSRR |= (1u << (LED1_BIT + 16u));
    break;
  case 2:
    GPIOB_BSRR |= (1u << (LED2_BIT + 16u));
    break;
  default:
    break;
  }
}

/*********************************************************************
*
*       BSP_ToggleLED()
*/
void BSP_ToggleLED(int Index) {
  switch (Index) {
  case 0:
    if (GPIOB_IDR & (1u << LED0_BIT)) {
      GPIOB_BSRR |= (1u << (LED0_BIT + 16u));
    } else {
      GPIOB_BSRR |= (1u << LED0_BIT);
    }
    break;
  case 1:
    if (GPIOB_IDR & (1u << LED1_BIT)) {
      GPIOB_BSRR |= (1u << (LED1_BIT + 16u));
    } else {
      GPIOB_BSRR |= (1u << LED1_BIT);
    }
    break;
  case 2:
    if (GPIOB_IDR & (1u << LED2_BIT)) {
      GPIOB_BSRR |= (1u << (LED2_BIT + 16u));
    } else {
      GPIOB_BSRR |= (1u << LED2_BIT);
    }
    break;
  default:
    break;
  }
}

/*************************** End of file ****************************/
