#ifndef __DEV_SAME70XPLAINED_H_INCLUDED__
#define __DEV_SAME70XPLAINED_H_INCLUDED__

//#include <conf_board.h>
//#include <compiler.h>

#ifdef __cplusplus
extern "C" {
#endif

//
#define PERPH_GSM    ((uint8_t)(1))
#define PERPH_GNSS   ((uint8_t)(2))
#define PERPH_NFC    ((uint8_t)(3))
#define PERPH_SDCARD ((uint8_t)(4))

//   
#define BOARD_PIO_INSERTION               ((uint32_t)(0x01))
#define BOARD_PIO_INSERTION_UNDEFINED     ((uint32_t)(0xFFFFFFFF))
#define BOARD_PIO_INSERTION_FALSE         ((uint32_t)(0x00000000))
#define BOARD_PIO_INSERTION_TRUE          ((uint32_t)(0x00000001))
   
#define BOARD_PIO_OPEN_HIGH               ((uint32_t)(0x02))
#define BOARD_PIO_OPEN_HIGH_UNDEFINED     ((uint32_t)(0xFFFFFFFF))
#define BOARD_PIO_OPEN_HIGH_FALSE         ((uint32_t)(0x00000000))
#define BOARD_PIO_OPEN_HIGH_TRUE          ((uint32_t)(0x00000001))
   
#define BOARD_PIO_OPEN_LOW               ((uint32_t)(0x03))
#define BOARD_PIO_OPEN_LOW_UNDEFINED     ((uint32_t)(0xFFFFFFFF))
#define BOARD_PIO_OPEN_LOW_FALSE         ((uint32_t)(0x00000000))
#define BOARD_PIO_OPEN_LOW_TRUE          ((uint32_t)(0x00000001))
   
#define BOARD_PIO_CHARGER_INT            ((uint32_t)(0x04))
#define BOARD_PIO_CHARGER_INT_UNDEFINED  ((uint32_t)(0xFFFFFFFF))
#define BOARD_PIO_CHARGER_INT_FALSE      ((uint32_t)(0x00000000))
#define BOARD_PIO_CHARGER_INT_TRUE       ((uint32_t)(0x00000001))
#define BOARD_PIO_CHARGER_INT_IN_CHARGE      BOARD_PIO_CHARGER_INT_FALSE
#define BOARD_PIO_CHARGER_INT_NOT_IN_CHARGE  BOARD_PIO_CHARGER_INT_TRUE
   
   
#define BOARD_PIO_USB_LVL                 ((uint32_t)(0x05))
#define BOARD_PIO_USB_LVL_UNDEFINED       ((uint32_t)(0xFFFFFFFF))
#define BOARD_PIO_USB_LVL_FALSE           ((uint32_t)(0x00000000))
#define BOARD_PIO_USB_LVL_TRUE            ((uint32_t)(0x00000001))
#define BOARD_PIO_USB_LVL_PRESENT         BOARD_PIO_USB_LVL_TRUE
#define BOARD_PIO_USB_LVL_NOTPRESENT      BOARD_PIO_USB_LVL_FALSE
    
#define BOARD_PIO_LED1  ((uint32_t)(0x101))
#define BOARD_PIO_LED2  ((uint32_t)(0x102))
#define BOARD_PIO_LED3  ((uint32_t)(0x103))  
   
#define BOARD_PIO_LED_RED     ((uint8_t)(0x04))
#define BOARD_PIO_LED_GREEN   ((uint8_t)(0x02))
#define BOARD_PIO_LED_BLUE    ((uint8_t)(0x01))
   
//specific ioctl command
#define BRDGSMUBLOXWARMUP    0x0001
   
#ifdef __cplusplus
}
#endif

#endif  /* __DEV_SAME70XPLAINED_H_INCLUDED__ */