/*===========================================
Compiler Directive
=============================================*/
#ifndef _USER_KERNEL_MKCONF_H_
#define _USER_KERNEL_MKCONF_H_


//force definition of realtime kernel
//#define __KERNEL_UCORE_FREERTOS
#define __KERNEL_UCORE_EMBOS

//see kernel/core/kernelconf.h
#ifdef __KERNEL_UCORE_FREERTOS
   #include "kernel/core/ucore/freeRTOS_8-0-0/source/arch/cortex-m4/stm32f4/FreeRTOSConfig.h"
#endif 

//see in kernel\dev\arch\cortexm\stm32f4xx\cubemx_hal_driver\inc\legacy\stm32f4xx_hal_conf.h
//force definition ST cpu target type
#ifndef STM32F469xx
   #define STM32F469xx
#endif
//
#ifndef HAL_ETH_MODULE_ENABLED
   #define HAL_ETH_MODULE_ENABLED
#endif
// use callbacks for ethernet 
#define  USE_HAL_ETH_REGISTER_CALLBACKS         1U /* ETH register callback disabled       */

//force definition of external clock (25MHz) on STM32f469I eval board
#define HSE_VALUE    ((uint32_t)25000000U)
//force definition of cpu device
#define __tauon_cpu_device__ __tauon_cpu_device_cortexM4_stm32f4__

//use pipe
#define __tauon_kernel_profile__ __tauon_kernel_profile_classic__
#define __KERNEL_PIPE_SIZE 1024

//rootfs 
#define __KERNEL_RTFS_NODETBL_SIZE 60
#define __KERNEL_RTFS_NODE_BLOCK_NB_MAX 64
#define __KERNEL_RTFS_BLOCK_SIZE 16
#define __KERNEL_RTFS_MAX_FILENAME 8

//force EFFS for stm32f407 on olimex-stm32-p407 board
#define __file_system_profile__  __file_system_profile_classic__
#define __KERNEL_VFS_SUPPORT_EFFS   0
#define __KERNEL_VFS_SUPPORT_FATFS  0


//kernel printk on /dev/console
#define __KERNEL_PRINTK
//kernel trace_printk on /dev/trace
#define __KERNEL_TRACE_PRINTK
//kernel console for initd and printk dev output on /dev/console stream
#define __KERNEL_DEV_TTY "/dev/ttys1"


//ip stack
//#define USE_UIP 
//#define USE_LWIP
//#define USE_IF_ETHERNET
//specific target include for pinout definition
#include "kernel/dev/bsp/stm32f469i-eval/dev_stm32f469i_eval_board/dev_stm32f469i_eval_board.h"

//#define __USER_MONGOOSE_PTHREAD_STACK_SIZE   (8*1024)
//#define __USER_MONGOOSE_CGI_ENVIRONMENT_SIZE (2*1024)
//#define __USER_MONGOOSE_MAX_REQUEST_SIZE     (2*1024)

//#define USE_LWIP
//#define USE_IF_ETHERNET

//
/* Section 4: Extended PHY Registers */
#define PHY_SR                          ((uint16_t)0x1FU)    /*!< PHY status register Offset                      */
#define PHY_MICR                        ((uint16_t)0x11U)    /*!< MII Interrupt Control Register                  */
#define PHY_MISR                        ((uint16_t)0x12U)    /*!< MII Interrupt Status and Misc. Control Register */

#define PHY_LINK_STATUS                 ((uint16_t)0x0001U)  /*!< PHY Link mask                                   */
#define PHY_SPEED_STATUS                ((uint16_t)0x0004U)  /*!< PHY Speed mask                                  */
#define PHY_DUPLEX_STATUS               ((uint16_t)0x0010U)  /*!< PHY Duplex mask                                 */

#define PHY_MICR_INT_EN                 ((uint16_t)0x0002U)  /*!< PHY Enable interrupts                           */
#define PHY_MICR_INT_OE                 ((uint16_t)0x0001U)  /*!< PHY Enable output interrupt events              */

#define PHY_MISR_LINK_INT_EN            ((uint16_t)0x0020U)  /*!< Enable Interrupt on change of link status       */
#define PHY_LINK_INTERRUPT              ((uint16_t)0x2000U)  /*!< PHY link status interrupt mask    */

//PIO specific definition for cubemx hal driver library
//from stm32f4 cubemx generator
#define A20_Pin GPIO_PIN_4
#define A20_GPIO_Port GPIOE
#define A19_Pin GPIO_PIN_3
#define A19_GPIO_Port GPIOE
#define MII_TXD3_Pin GPIO_PIN_2
#define MII_TXD3_GPIO_Port GPIOE
#define MII_TXD1_Pin GPIO_PIN_14
#define MII_TXD1_GPIO_Port GPIOG
#define FMC_NBL1_Pin GPIO_PIN_1
#define FMC_NBL1_GPIO_Port GPIOE
#define FMC_NBL0_CY7C1071DV33__Pin GPIO_PIN_0
#define FMC_NBL0_CY7C1071DV33__GPIO_Port GPIOE
#define I2C1_SCL_Pin GPIO_PIN_8
#define I2C1_SCL_GPIO_Port GPIOB
#define ULPI_D7_Pin GPIO_PIN_5
#define ULPI_D7_GPIO_Port GPIOB
#define TRST_Pin GPIO_PIN_4
#define TRST_GPIO_Port GPIOB
#define TDO_SWO_Pin GPIO_PIN_3
#define TDO_SWO_GPIO_Port GPIOB
#define FMC_NE1_Pin GPIO_PIN_7
#define FMC_NE1_GPIO_Port GPIOD
#define uSD_CLK_Pin GPIO_PIN_12
#define uSD_CLK_GPIO_Port GPIOC
#define TDI_Pin GPIO_PIN_15
#define TDI_GPIO_Port GPIOA
#define TCK_SWCLK_Pin GPIO_PIN_14
#define TCK_SWCLK_GPIO_Port GPIOA
#define TMS_SWDIO_Pin GPIO_PIN_13
#define TMS_SWDIO_GPIO_Port GPIOA
#define A21_Pin GPIO_PIN_5
#define A21_GPIO_Port GPIOE
#define A22_Pin GPIO_PIN_6
#define A22_GPIO_Port GPIOE
#define MII_TXD0_Pin GPIO_PIN_13
#define MII_TXD0_GPIO_Port GPIOG
#define I2C1_SDA_Pin GPIO_PIN_9
#define I2C1_SDA_GPIO_Port GPIOB
#define PAR_VSYNC_Pin GPIO_PIN_7
#define PAR_VSYNC_GPIO_Port GPIOB
#define QSPI_BK1_NCS_Pin GPIO_PIN_6
#define QSPI_BK1_NCS_GPIO_Port GPIOB
#define SDNCAS_Pin GPIO_PIN_15
#define SDNCAS_GPIO_Port GPIOG
#define MII_TX_EN_Pin GPIO_PIN_11
#define MII_TX_EN_GPIO_Port GPIOG
#define FMC_NWAIT_Pin GPIO_PIN_6
#define FMC_NWAIT_GPIO_Port GPIOD
#define D2_Pin GPIO_PIN_0
#define D2_GPIO_Port GPIOD
#define uSD_D3_Pin GPIO_PIN_11
#define uSD_D3_GPIO_Port GPIOC
#define uSD_D2_Pin GPIO_PIN_10
#define uSD_D2_GPIO_Port GPIOC
#define USB_FS1_DP_Pin GPIO_PIN_12
#define USB_FS1_DP_GPIO_Port GPIOA
#define MFX_IRQOUT_Pin GPIO_PIN_8
#define MFX_IRQOUT_GPIO_Port GPIOI
#define FMC_NBL2_Pin GPIO_PIN_4
#define FMC_NBL2_GPIO_Port GPIOI
#define DSI_RESET_Pin GPIO_PIN_7
#define DSI_RESET_GPIO_Port GPIOK
#define LED4_Pin GPIO_PIN_6
#define LED4_GPIO_Port GPIOK
#define LED3_Pin GPIO_PIN_5
#define LED3_GPIO_Port GPIOK
#define FMC_NOE_Pin GPIO_PIN_5
#define FMC_NOE_GPIO_Port GPIOD
#define D3_Pin GPIO_PIN_1
#define D3_GPIO_Port GPIOD
#define D27_Pin GPIO_PIN_3
#define D27_GPIO_Port GPIOI
#define D26_Pin GPIO_PIN_2
#define D26_GPIO_Port GPIOI
#define USB_FS1_DM_Pin GPIO_PIN_11
#define USB_FS1_DM_GPIO_Port GPIOA
#define TAMPER_KEY_Pin GPIO_PIN_13
#define TAMPER_KEY_GPIO_Port GPIOC
#define A0_Pin GPIO_PIN_0
#define A0_GPIO_Port GPIOF
#define FMC_NBL3_Pin GPIO_PIN_5
#define FMC_NBL3_GPIO_Port GPIOI
#define D29_Pin GPIO_PIN_7
#define D29_GPIO_Port GPIOI
#define D31_Pin GPIO_PIN_10
#define D31_GPIO_Port GPIOI
#define D28_Pin GPIO_PIN_6
#define D28_GPIO_Port GPIOI
#define LED2_Pin GPIO_PIN_4
#define LED2_GPIO_Port GPIOK
#define LED1_Pin GPIO_PIN_3
#define LED1_GPIO_Port GPIOK
#define FMC_NE2_Pin GPIO_PIN_9
#define FMC_NE2_GPIO_Port GPIOG
#define FMC_NOED4_Pin GPIO_PIN_4
#define FMC_NOED4_GPIO_Port GPIOD
#define uSD_CMD_Pin GPIO_PIN_2
#define uSD_CMD_GPIO_Port GPIOD
#define D23_Pin GPIO_PIN_15
#define D23_GPIO_Port GPIOH
#define D25_Pin GPIO_PIN_1
#define D25_GPIO_Port GPIOI
#define RS232_IrDA_RX_Pin GPIO_PIN_10
#define RS232_IrDA_RX_GPIO_Port GPIOA
#define A1_Pin GPIO_PIN_1
#define A1_GPIO_Port GPIOF
#define D30_Pin GPIO_PIN_9
#define D30_GPIO_Port GPIOI
#define D21_Pin GPIO_PIN_13
#define D21_GPIO_Port GPIOH
#define D22_Pin GPIO_PIN_14
#define D22_GPIO_Port GPIOH
#define D24_Pin GPIO_PIN_0
#define D24_GPIO_Port GPIOI
#define RS232_IrDA_TX_Pin GPIO_PIN_9
#define RS232_IrDA_TX_GPIO_Port GPIOA
#define ULPI_DIR_Pin GPIO_PIN_11
#define ULPI_DIR_GPIO_Port GPIOI
#define uSD_D1_Pin GPIO_PIN_9
#define uSD_D1_GPIO_Port GPIOC
#define LCD_BL_CTRL_Pin GPIO_PIN_8
#define LCD_BL_CTRL_GPIO_Port GPIOA
#define OSC_IN_Pin GPIO_PIN_0
#define OSC_IN_GPIO_Port GPIOH
#define A2_Pin GPIO_PIN_2
#define A2_GPIO_Port GPIOF
#define uSD_D0_Pin GPIO_PIN_8
#define uSD_D0_GPIO_Port GPIOC
#define MIK_CK_Pin GPIO_PIN_7
#define MIK_CK_GPIO_Port GPIOC
#define OSC_OUT_Pin GPIO_PIN_1
#define OSC_OUT_GPIO_Port GPIOH
#define A3_Pin GPIO_PIN_3
#define A3_GPIO_Port GPIOF
#define ULPI_NXT_Pin GPIO_PIN_4
#define ULPI_NXT_GPIO_Port GPIOH
#define SDCLK_Pin GPIO_PIN_8
#define SDCLK_GPIO_Port GPIOG
#define I2S3_CK_Pin GPIO_PIN_6
#define I2S3_CK_GPIO_Port GPIOC
#define A4_Pin GPIO_PIN_4
#define A4_GPIO_Port GPIOF
#define SDNWE_Pin GPIO_PIN_5
#define SDNWE_GPIO_Port GPIOH
#define SDNE0_Pin GPIO_PIN_3
#define SDNE0_GPIO_Port GPIOH
#define SAI1_MCLKA_Pin GPIO_PIN_7
#define SAI1_MCLKA_GPIO_Port GPIOG
#define QSPI_BK1_IO2_Pin GPIO_PIN_7
#define QSPI_BK1_IO2_GPIO_Port GPIOF
#define QSPI_BK1_IO3_Pin GPIO_PIN_6
#define QSPI_BK1_IO3_GPIO_Port GPIOF
#define A5_Pin GPIO_PIN_5
#define A5_GPIO_Port GPIOF
#define SDCKE0_Pin GPIO_PIN_2
#define SDCKE0_GPIO_Port GPIOH
#define D1_Pin GPIO_PIN_15
#define D1_GPIO_Port GPIOD
#define ULPI_D6_Pin GPIO_PIN_13
#define ULPI_D6_GPIO_Port GPIOB
#define D15_Pin GPIO_PIN_10
#define D15_GPIO_Port GPIOD
#define QSPI_CLK_Pin GPIO_PIN_10
#define QSPI_CLK_GPIO_Port GPIOF
#define QSPI_BK1_IO1_Pin GPIO_PIN_9
#define QSPI_BK1_IO1_GPIO_Port GPIOF
#define QSPI_BK1_IO0_Pin GPIO_PIN_8
#define QSPI_BK1_IO0_GPIO_Port GPIOF
#define MII_TX_CLK_Pin GPIO_PIN_3
#define MII_TX_CLK_GPIO_Port GPIOC
#define D0_Pin GPIO_PIN_14
#define D0_GPIO_Port GPIOD
#define ULPI_D5_Pin GPIO_PIN_12
#define ULPI_D5_GPIO_Port GPIOB
#define D14_Pin GPIO_PIN_9
#define D14_GPIO_Port GPIOD
#define D13_Pin GPIO_PIN_8
#define D13_GPIO_Port GPIOD
#define ULPI_STP_Pin GPIO_PIN_0
#define ULPI_STP_GPIO_Port GPIOC
#define MII_MDC_Pin GPIO_PIN_1
#define MII_MDC_GPIO_Port GPIOC
#define MII_TXD2_Pin GPIO_PIN_2
#define MII_TXD2_GPIO_Port GPIOC
#define A6_Pin GPIO_PIN_12
#define A6_GPIO_Port GPIOF
#define A11_Pin GPIO_PIN_1
#define A11_GPIO_Port GPIOG
#define A9_Pin GPIO_PIN_15
#define A9_GPIO_Port GPIOF
#define A17_Pin GPIO_PIN_12
#define A17_GPIO_Port GPIOD
#define A18_Pin GPIO_PIN_13
#define A18_GPIO_Port GPIOD
#define A13_Pin GPIO_PIN_3
#define A13_GPIO_Port GPIOG
#define A12_Pin GPIO_PIN_2
#define A12_GPIO_Port GPIOG
#define D20_Pin GPIO_PIN_12
#define D20_GPIO_Port GPIOH
#define MII_RX_CLK_Pin GPIO_PIN_1
#define MII_RX_CLK_GPIO_Port GPIOA
#define Potentiometer_Pin GPIO_PIN_4
#define Potentiometer_GPIO_Port GPIOA
#define MII_RXD0_Pin GPIO_PIN_4
#define MII_RXD0_GPIO_Port GPIOC
#define A7_Pin GPIO_PIN_13
#define A7_GPIO_Port GPIOF
#define A10_Pin GPIO_PIN_0
#define A10_GPIO_Port GPIOG
#define D5_Pin GPIO_PIN_8
#define D5_GPIO_Port GPIOE
#define A16_Pin GPIO_PIN_11
#define A16_GPIO_Port GPIOD
#define MII_RXD3_Pin GPIO_PIN_7
#define MII_RXD3_GPIO_Port GPIOH
#define D17_Pin GPIO_PIN_9
#define D17_GPIO_Port GPIOH
#define D19_Pin GPIO_PIN_11
#define D19_GPIO_Port GPIOH
#define MII_MDIO_Pin GPIO_PIN_2
#define MII_MDIO_GPIO_Port GPIOA
#define ULPI_CK_Pin GPIO_PIN_5
#define ULPI_CK_GPIO_Port GPIOA
#define MII_RXD1_Pin GPIO_PIN_5
#define MII_RXD1_GPIO_Port GPIOC
#define A8_Pin GPIO_PIN_14
#define A8_GPIO_Port GPIOF
#define DSI_TE_Pin GPIO_PIN_2
#define DSI_TE_GPIO_Port GPIOJ
#define SDNRAS_Pin GPIO_PIN_11
#define SDNRAS_GPIO_Port GPIOF
#define D6_Pin GPIO_PIN_9
#define D6_GPIO_Port GPIOE
#define D8_Pin GPIO_PIN_11
#define D8_GPIO_Port GPIOE
#define D11_Pin GPIO_PIN_14
#define D11_GPIO_Port GPIOE
#define ULPI_D3_Pin GPIO_PIN_10
#define ULPI_D3_GPIO_Port GPIOB
#define MII_RXD2_Pin GPIO_PIN_6
#define MII_RXD2_GPIO_Port GPIOH
#define D16_Pin GPIO_PIN_8
#define D16_GPIO_Port GPIOH
#define D18_Pin GPIO_PIN_10
#define D18_GPIO_Port GPIOH
#define ULPI_D0_Pin GPIO_PIN_3
#define ULPI_D0_GPIO_Port GPIOA
#define MII_RX_DV_Pin GPIO_PIN_7
#define MII_RX_DV_GPIO_Port GPIOA
#define ULPI_D2_Pin GPIO_PIN_1
#define ULPI_D2_GPIO_Port GPIOB
#define ULPI_D1_Pin GPIO_PIN_0
#define ULPI_D1_GPIO_Port GPIOB
#define D4_Pin GPIO_PIN_7
#define D4_GPIO_Port GPIOE
#define D7_Pin GPIO_PIN_10
#define D7_GPIO_Port GPIOE
#define D9_Pin GPIO_PIN_12
#define D9_GPIO_Port GPIOE
#define D12_Pin GPIO_PIN_15
#define D12_GPIO_Port GPIOE
#define D10_Pin GPIO_PIN_13
#define D10_GPIO_Port GPIOE
#define ULPI_D4_Pin GPIO_PIN_11
#define ULPI_D4_GPIO_Port GPIOB
#define USB_FS2_DM_Pin GPIO_PIN_14
#define USB_FS2_DM_GPIO_Port GPIOB
#define USB_FS2_DP_Pin GPIO_PIN_15
#define USB_FS2_DP_GPIO_Port GPIOB


/*===========================================
Includes
=============================================*/


/*===========================================
Declaration
=============================================*/

#endif
