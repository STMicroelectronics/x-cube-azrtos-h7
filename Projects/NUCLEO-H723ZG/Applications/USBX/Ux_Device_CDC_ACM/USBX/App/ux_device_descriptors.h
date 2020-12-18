/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    ux_device_descriptors.h
  * @author  MCD Application Team
  * @brief   USBX Device descriptor header file
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __UX_DEVICE_DESCRIPTORS_H__
#define __UX_DEVICE_DESCRIPTORS_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "ux_api.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stm32h7xx_hal.h"
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/

/* USER CODE BEGIN EFP */
uint8_t *USBD_Get_Device_Framework_Speed(uint8_t Speed, ULONG *Length);
uint8_t *USBD_Get_String_Framework(ULONG *Length);
uint8_t *USBD_Get_Language_Id_Framework(ULONG *Length);
/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
/* USER CODE BEGIN Private defines */
#define USBD_VID                                        0x0483
#define USBD_PID                                        0x5740
#define USBD_LANGID_STRING                              0x409
#define USBD_MANUFACTURER_STRING                        "STMicroelectronics"
#define USBD_PRODUCT_STRING                             "STM32 Virtual ComPort"
#define USBD_SERIAL_NUMBER                              "0001"

#define STRING_FRAMWORK_MAX_SIZE                        256
#define USBD_MAX_NUM_CONFIGURATION                      1U

#define USB_HS_MAX_PACKET_SIZE                          512U
#define USB_FS_MAX_PACKET_SIZE                          64U
#define USB_MAX_EP0_SIZE                                64U

#define USBD_IDX_LANGID_STR                             0x00U
#define USBD_IDX_MFC_STR                                0x01U
#define USBD_IDX_PRODUCT_STR                            0x02U
#define USBD_IDX_SERIAL_STR                             0x03U
#define USBD_IDX_CONFIG_STR                             0x04U
#define USBD_IDX_INTERFACE_STR                          0x05U


#define  FULL_SPEED                                     0x00
#define  HIGH_SPEED                                     0x01

#define USB_CDC_CONFIG_DESC_SIZ                         75U

#define CDC_IN_EP                                       0x83U  /* EP3 for data IN */
#define CDC_OUT_EP                                      0x01U  /* EP1 for data OUT */
#define CDC_CMD_EP                                      0x82U  /* EP2 for CDC commands */

#define  USB_DESC_TYPE_DEVICE                           0x01U
#define  USB_DESC_TYPE_CONFIGURATION                    0x02U
#define  USB_DESC_TYPE_STRING                           0x03U
#define  USB_DESC_TYPE_INTERFACE                        0x04U
#define  USB_DESC_TYPE_ENDPOINT                         0x05U
#define  USB_DESC_TYPE_DEVICE_QUALIFIER                 0x06U
#define  USB_DESC_TYPE_OTHER_SPEED_CONFIGURATION        0x07U
#define  USB_DESC_TYPE_BOS                              0x0FU
#define  USB_LEN_DEV_QUALIFIER_DESC                     0x0AU

/* CDC Endpoints parameters: you can fine tune these values depending on the needed baudrates and performance. */
#define CDC_DATA_HS_MAX_PACKET_SIZE                     512U  /* Data Endpoint Packet size */
#define CDC_DATA_FS_MAX_PACKET_SIZE                     64U   /* Data Endpoint Packet size */
#define CDC_CMD_PACKET_SIZE                             8U    /* Control Endpoint Packet size */

#ifndef CDC_HS_BINTERVAL
#define CDC_HS_BINTERVAL                                0x10U
#endif /* CDC_HS_BINTERVAL */

#ifndef CDC_FS_BINTERVAL
#define CDC_FS_BINTERVAL                                0x10U
#endif /* CDC_FS_BINTERVAL */

#ifndef LOBYTE
#define LOBYTE(x)  ((uint8_t)((x) & 0x00FFU))
#endif

#ifndef HIBYTE
#define HIBYTE(x)  ((uint8_t)(((x) & 0xFF00U) >> 8U))
#endif

#ifndef MIN
#define MIN(a, b)  (((a) < (b)) ? (a) : (b))
#endif

#ifndef MAX
#define MAX(a, b)  (((a) > (b)) ? (a) : (b))
#endif

/* USER CODE END Private defines */

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

#ifdef __cplusplus
}
#endif
#endif  /* __UX_DEVICE_DESCRIPTORS_H__ */
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
