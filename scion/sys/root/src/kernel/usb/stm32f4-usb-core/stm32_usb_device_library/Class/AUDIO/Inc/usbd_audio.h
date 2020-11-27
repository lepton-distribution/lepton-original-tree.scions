/**
  ******************************************************************************
  * @file    usbd_audio.h
  * @author  MCD Application Team
  * @version V2.4.1
  * @date    19-June-2015
  * @brief   header file for the usbd_audio.c file.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2015 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */
 
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USB_AUDIO_H
#define __USB_AUDIO_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include  "usbd_ioreq.h"

/** @addtogroup STM32_USB_DEVICE_LIBRARY
  * @{
  */
  
/** @defgroup USBD_AUDIO
  * @brief This file is the Header file for usbd_audio.c
  * @{
  */ 


/** @defgroup USBD_AUDIO_Exported_Defines
  * @{
  */ 
//#define AUDIO_OUT_EP                                  0x01
//#define AUDIO_IN_EP                                   0x82

#define NUODIO_USB
#define USE_AUDIO_2_INTERFACES
//#define USE_AUDIO_4_INTERFACES

#ifdef USE_AUDIO_2_INTERFACES
   #define AUDIO_OUT_CHANNEL_1_EP      (0X01)
   #define AUDIO_IN_CHANNEL_1_EP       (0X81)
#endif

#ifdef USE_AUDIO_4_INTERFACES
   #define AUDIO_OUT_CHANNEL_1_EP      (0X01)
   #define AUDIO_OUT_CHANNEL_2_EP      (0X02)
   #define AUDIO_IN_CHANNEL_1_EP       (0X81)
   #define AUDIO_IN_CHANNEL_2_EP       (0X82)
#endif
   

#ifdef NUODIO_USB
    #if defined(USE_AUDIO_2_INTERFACES)
      #define USB_AUDIO_CONFIG_DESC_SIZ                     (9+9+10+2*(12+9+9)+2*(9+9+7+11+9+7)) //(109+82+1)
   #elif defined(USE_AUDIO_4_INTERFACES)
      #define USB_AUDIO_CONFIG_DESC_SIZ                     (9+9+12+4*(12+9+9)+4*(9+9+7+11+9+7)) //(109+82+1)
   #endif
#else
   #define USB_AUDIO_CONFIG_DESC_SIZ                     (109)
#endif
#define AUDIO_INTERFACE_DESC_SIZE                     9
#define USB_AUDIO_DESC_SIZ                            0x09
#define AUDIO_STANDARD_ENDPOINT_DESC_SIZE             0x09
#define AUDIO_STREAMING_ENDPOINT_DESC_SIZE            0x07

#define AUDIO_DESCRIPTOR_TYPE                         0x21
#define USB_DEVICE_CLASS_AUDIO                        0x01
#define AUDIO_SUBCLASS_AUDIOCONTROL                   0x01
#define AUDIO_SUBCLASS_AUDIOSTREAMING                 0x02
#define AUDIO_PROTOCOL_UNDEFINED                      0x00
#define AUDIO_STREAMING_GENERAL                       0x01
#define AUDIO_STREAMING_FORMAT_TYPE                   0x02

/* Audio Descriptor Types */
#define AUDIO_INTERFACE_DESCRIPTOR_TYPE               0x24
#define AUDIO_ENDPOINT_DESCRIPTOR_TYPE                0x25

/* Audio Control Interface Descriptor Subtypes */
#define AUDIO_CONTROL_HEADER                          0x01
#define AUDIO_CONTROL_INPUT_TERMINAL                  0x02
#define AUDIO_CONTROL_OUTPUT_TERMINAL                 0x03
#define AUDIO_CONTROL_FEATURE_UNIT                    0x06

#define AUDIO_INPUT_TERMINAL_DESC_SIZE                0x0C
#define AUDIO_OUTPUT_TERMINAL_DESC_SIZE               0x09
#define AUDIO_STREAMING_INTERFACE_DESC_SIZE           0x07


#define AUDIO_CONTROL_MUTE                            0x0001
#define AUDIO_CONTROL_VOL                             0x0002

#define AUDIO_FORMAT_TYPE_I                           0x01
#define AUDIO_FORMAT_TYPE_III                         0x03

#define AUDIO_ENDPOINT_GENERAL                        0x01

#define AUDIO_REQ_SET_CUR                             0x01
#define AUDIO_REQ_GET_CUR                             0x81
   
#define AUDIO_REQ_SET_MIN                             0x02
#define AUDIO_REQ_GET_MIN                             0x82
   
#define AUDIO_REQ_SET_MAX                             0x03
#define AUDIO_REQ_GET_MAX                             0x83
   
#define AUDIO_REQ_SET_RES                             0x04
#define AUDIO_REQ_GET_RES                             0x84

#define AUDIO_OUT_STREAMING_CTRL                      0x02


//#define AUDIO_OUT_PACKET                              (uint32_t)(((USBD_AUDIO_FREQ * 2 * 2) /1000)) 
#define AUDIO_DEFAULT_VOLUME                          70
    
/* Number of sub-packets in the audio transfer buffer. You can modify this value but always make sure
  that it is an even number and higher than 3 */
//#define AUDIO_OUT_PACKET_NUM                          80

#define NB_CHANNEL 2
    
/*
 * ATT: with 3/4 bytes and 24/32bit it does not work on Windows: correct shown but error when tried to play
 */
//#define I2S_24BIT
#ifdef I2S_24BIT
   //works: 48 KHz, 24bit - but disable all Enhancements on Windows Sound - mode 1
   #define  HALF_WORD_BYTES						3			//3 half word (one channel)
   #define  SAMPLE_BITS								24			//24 bit per channel
#else
   //works: 48 KHz, 16bit - mode 0
   #define  HALF_WORD_BYTES						2			//2 half word (one channel)
   #define  SAMPLE_BITS								16			//16 bit per channel
   //but 96 KHz, 24bit does not work on Windows, 96 KHz, 16bit is OK
#endif

//works: 48 KHz, 24bit - but disable all Enhancements on Windows Sound - mode 1
#define	USBD_AUDIO_FREQ2							48000		//bits per seconds
#define  HALF_WORD_BYTES2							  3			//3 half word (one channel)
#define  SAMPLE_BITS2								 24			//24 bit per channel

/* AudioFreq * DataSize (HALF_WORD_BYTES bytes) * NumChannels (Stereo: 2) */
/* ATT: make sure that the AUDIO_OUT_PACKET uses the largest value ! */
#define AUDIO_OUT_PACKET                              (uint32_t)(((USBD_AUDIO_FREQ * HALF_WORD_BYTES*NB_CHANNEL)  /1000))
#define AUDIO_OUT_PACKET_SZ                           AUDIO_OUT_PACKET

#define AUDIO_IN_PACKET_SZ                            (uint32_t)(((USBD_AUDIO_FREQ * HALF_WORD_BYTES*NB_CHANNEL)  /1000))
#define AUDIO_IN_PACKET_SZ_MAX                        (uint32_t)(((USBD_AUDIO_FREQ * HALF_WORD_BYTES*NB_CHANNEL)  /1000)) + HALF_WORD_BYTES*NB_CHANNEL

/* Number of sub-packets in the audio transfer buffer. You can modify this value but always make sure
  that it is an even number and higher than 3 */
#define OUT_PACKET_NUM                                 4		//4
/* Total size of the audio transfer buffer */
#define TOTAL_OUT_BUF_SIZE                           ((uint32_t)(AUDIO_OUT_PACKET * OUT_PACKET_NUM))


#define AUDIO_PACKET_SZE(frq)          (uint8_t)(((frq * 2 * HALF_WORD_BYTES)/1000) & 0xFF), \
                                       (uint8_t)((((frq * 2 * HALF_WORD_BYTES)/1000) >> 8) & 0xFF)
                                         
#define SET_AUDIO_IN_PACKET_SZ_MAX(__PKT_SZ__)  (uint8_t)( (__PKT_SZ__) & 0xFF), \
                                                (uint8_t)( ((__PKT_SZ__) >> 8) & 0xFF)

#define AUDIO_SAMPLE_FREQ(frq)         (uint8_t)(frq), (uint8_t)((frq >> 8)), (uint8_t)((frq >> 16))
   
/* Total size of the audio transfer buffer */
#define AUDIO_TOTAL_BUF_SIZE                         TOTAL_OUT_BUF_SIZE
    
    /* Audio Commands enumeration */
typedef enum
{
  AUDIO_CMD_START = 1,
  AUDIO_CMD_PLAY,
  AUDIO_CMD_STOP,
}AUDIO_CMD_TypeDef;


typedef enum
{
  AUDIO_OFFSET_NONE = 0,
  AUDIO_OFFSET_HALF,
  AUDIO_OFFSET_FULL,  
  AUDIO_OFFSET_UNKNOWN,    
}
AUDIO_OffsetTypeDef;
/**
  * @}
  */ 


/** @defgroup USBD_CORE_Exported_TypesDefinitions
  * @{
  */
 typedef struct
{
   uint8_t cmd;   
   uint8_t data[USB_MAX_EP0_SIZE];  
   uint8_t len;  
   uint8_t unit;    
}
USBD_AUDIO_ControlTypeDef; 



typedef struct
{
  __IO uint32_t             alt_setting; 
  uint8_t                   buffer[AUDIO_TOTAL_BUF_SIZE];
  AUDIO_OffsetTypeDef       offset;
  uint8_t                    rd_enable;  
  uint16_t                   rd_ptr;  
  uint16_t                   wr_ptr;  
  USBD_AUDIO_ControlTypeDef control;   
}
USBD_AUDIO_HandleTypeDef; 


typedef struct
{
    int8_t  (*Init)         (uint32_t  AudioFreq, uint32_t Volume, uint32_t options);
    int8_t  (*DeInit)       (uint32_t options);
    int8_t  (*AudioCmd)     (uint8_t* pbuf, uint32_t size, uint8_t cmd);
    int8_t  (*VolumeCtl)    (uint8_t vol);
    int8_t  (*MuteCtl)      (uint8_t cmd);
    int8_t  (*PeriodicTC)   (uint8_t cmd);
    int8_t  (*GetState)     (void);
}USBD_AUDIO_ItfTypeDef;
/**
  * @}
  */ 



/** @defgroup USBD_CORE_Exported_Macros
  * @{
  */ 

/**
  * @}
  */ 

/** @defgroup USBD_CORE_Exported_Variables
  * @{
  */ 

extern USBD_ClassTypeDef  USBD_AUDIO;
#define USBD_AUDIO_CLASS    &USBD_AUDIO
/**
  * @}
  */ 

/** @defgroup USB_CORE_Exported_Functions
  * @{
  */ 
uint8_t  USBD_AUDIO_RegisterInterface  (USBD_HandleTypeDef   *pdev, 
                                        USBD_AUDIO_ItfTypeDef *fops);

void  USBD_AUDIO_Sync (USBD_HandleTypeDef *pdev, AUDIO_OffsetTypeDef offset);
/**
  * @}
  */ 

#ifdef __cplusplus
}
#endif

#endif  /* __USB_AUDIO_H */
/**
  * @}
  */ 

/**
  * @}
  */ 
  
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
