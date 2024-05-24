/*********************************************************************
*                     SEGGER Microcontroller GmbH                    *
*                        The Embedded Experts                        *
**********************************************************************
*                                                                    *
*       (c) 1995 - 2022 SEGGER Microcontroller GmbH                  *
*                                                                    *
*       Internet: segger.com  Support: support_embos@segger.com      *
*                                                                    *
**********************************************************************

-------------------------- END-OF-HEADER -----------------------------
Purpose : Initializes and handles the hardware for embOS
*/

#include "RTOS.h"
#if (defined(CMSIS_device_header))
  #include CMSIS_device_header           // Include the specific device header
#endif
#if (defined(USE_HAL_DRIVER))
  #include "main.h"                      // Include the main.h that is provided by STM32CubeMX
  #include "SEGGER.I-CUBE-embOS_conf.h"  // Include embOS specific defines provided by STM32CubeMX
#endif

/*********************************************************************
*
*       Defines
*
**********************************************************************
*/

/*********************************************************************
*
*       System tick settings
*/
#ifndef   OS_TIMER_FREQ
  #define OS_TIMER_FREQ  (SystemCoreClock)
#endif
#ifndef   OS_TICK_FREQ
  #define OS_TICK_FREQ   (1000u)
#endif
#ifndef   OS_INT_FREQ
  #define OS_INT_FREQ    (OS_TICK_FREQ)
#endif

/*********************************************************************
*
*       embOSView settings
*/
#ifndef   OS_VIEW_IFSELECT
  #define OS_VIEW_IFSELECT  OS_VIEW_IF_JLINK
#endif

#if (OS_VIEW_IFSELECT == OS_VIEW_IF_JLINK)
  #include "JLINKMEM.h"
#endif

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/
#if (OS_VIEW_IFSELECT == OS_VIEW_IF_JLINK)
  const OS_U32 OS_JLINKMEM_BufferSize = 32u;  // Size of the communication buffer for JLINKMEM
#else
  const OS_U32 OS_JLINKMEM_BufferSize = 0u;   // Buffer not used
#endif

/*********************************************************************
*
*       Local functions
*
**********************************************************************
*/

/*********************************************************************
*
*       _OS_GetHWTimerCycles()
*
*  Function description
*    Returns the current hardware timer count value.
*
*  Return value
*    Current timer count value.
*/
static unsigned int _OS_GetHWTimerCycles(void) {
  return SysTick->VAL;
}

/*********************************************************************
*
*       _OS_GetHWTimer_IntPending()
*
*  Function description
*    Returns if the hardware timer interrupt pending flag is set.
*
*  Return value
*    == 0: Interrupt pending flag not set.
*    != 0: Interrupt pending flag set.
*/
static unsigned int _OS_GetHWTimer_IntPending(void) {
  return SCB->ICSR & SCB_ICSR_PENDSTSET_Msk;
}

/*********************************************************************
*
*       Global functions
*
**********************************************************************
*/

/*********************************************************************
*
*       SysTick_Handler()
*
*  Function description
*    This is the hardware timer exception handler.
*/
void SysTick_Handler(void) {
  OS_INT_EnterNestable();
  OS_TICK_Handle();
#if (OS_VIEW_IFSELECT == OS_VIEW_IF_JLINK)
  JLINKMEM_Process();
#endif
  OS_INT_LeaveNestable();
}

/*********************************************************************
*
*       OS_InitHW()
*
*  Function description
*    Initialize the hardware required for embOS to run.
*/
void OS_InitHW(void) {
  OS_INT_IncDI();
  //
  // We assume PLL and core clock were already set by the SystemInit() function,
  // which was called from the startup code. Therefore, we just ensure the system
  // clock variable is updated and then set the periodic system timer tick for embOS.
  //
  SystemCoreClockUpdate();                                        // Update the system clock variable (might not have been set before)
  SysTick_Config(OS_TIMER_FREQ / OS_INT_FREQ);                    // Setup SysTick Timer
  NVIC_SetPriority(SysTick_IRQn, (1u << __NVIC_PRIO_BITS) - 2u);  // Set the priority higher than the PendSV priority
  //
  // Inform embOS about the timer settings
  //
  {
    OS_SYSTIMER_CONFIG SysTimerConfig = {OS_TIMER_FREQ, OS_INT_FREQ, OS_TIMER_DOWNCOUNTING, _OS_GetHWTimerCycles, _OS_GetHWTimer_IntPending};
    OS_TIME_ConfigSysTimer(&SysTimerConfig);
  }
  //
  // Initialize communication for embOSView
  //
#if (OS_VIEW_IFSELECT == OS_VIEW_IF_JLINK)
  JLINKMEM_SetpfOnRx(OS_COM_OnRx);
  JLINKMEM_SetpfOnTx(OS_COM_OnTx);
  JLINKMEM_SetpfGetNextChar(OS_COM_GetNextChar);
#elif (OS_VIEW_IFSELECT == OS_VIEW_IF_UART)
  BSP_UART_Init(OS_UART, OS_BAUDRATE, BSP_UART_DATA_BITS_8, BSP_UART_PARITY_NONE, BSP_UART_STOP_BITS_1);
  BSP_UART_SetReadCallback(OS_UART, _OS_OnRX);
  BSP_UART_SetWriteCallback(OS_UART, _OS_OnTX);
#endif
  OS_INT_DecRI();
}

/*********************************************************************
*
*       OS_Idle()
*
*  Function description
*    This code is executed whenever no task, software timer, or
*    interrupt is ready for execution.
*
*  Additional information
*    The idle loop does not have a stack of its own, therefore no
*    functionality should be implemented that relies on the stack
*    to be preserved.
*/
void OS_Idle(void) {  // Idle loop: No task is ready to execute
  while (1) {         // Nothing to do ... wait for interrupt
    #if ((OS_VIEW_IFSELECT != OS_VIEW_IF_JLINK) && (OS_DEBUG == 0))
      //
      // When uncommenting this line, please be aware device
      // specific issues could occur.
      // Therefore, we do not call __WFI() by default.
      //
      //__WFI();        // Switch CPU into sleep mode
    #endif
  }
}

/*********************************************************************
*
*       Optional communication with embOSView
*
**********************************************************************
*/

/*********************************************************************
*
*       OS_COM_Send1()
*
*  Function description
*    Sends one character.
*/
void OS_COM_Send1(OS_U8 c) {
#if (OS_VIEW_IFSELECT == OS_VIEW_IF_JLINK)
  JLINKMEM_SendChar(c);
#elif (OS_VIEW_IFSELECT == OS_VIEW_DISABLED)
  OS_USEPARA(c);           // Avoid compiler warning
  OS_COM_ClearTxActive();  // Let embOS know that Tx is not busy
#endif
}

/*************************** End of file ****************************/
