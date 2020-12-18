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
#define USBD_VID                                       0x0483
#define USBD_PID                                       0x5720
#define USBD_LANGID_STRING                             0x409
#define USBD_MANUFACTURER_STRING                       "STMicroelectronics"
#define USBD_PRODUCT_STRING                            "STM32 MSC SD Storage"
#define USBD_SERIAL_NUMBER                             "000000000001"

#ifndef USBD_STORAGE_FS_BINTERVAL
#define USBD_STORAGE_FS_BINTERVAL                      0x00U
#endif /* USBD_STORAGE_FS_BINTERVAL */

#ifndef USBD_STORAGE_HS_BINTERVAL
#define USBD_STORAGE_HS_BINTERVAL                      0x00U
#endif /* USBD_STORAGE_HS_BINTERVAL */

#define USBD_STORAGE_EPIN_ADDR                         0x81U
#define USBD_STORAGE_EPIN_SIZE                         0x40U

#define USBD_STORAGE_EPOUT_ADDR                        0x02U
#define USBD_STORAGE_FS_EPOUT_SIZE                     0x40U
#define USBD_STORAGE_HS_EPOUT_SIZE                     0x200U

#define USBD_STORAGE_CONFIG_DESC_SIZE                  32U

#define USBD_MAX_NUM_CONFIGURATION                     0x01U
#define USBD_MAX_EP0_SIZE                              64U

#define USBD_IDX_MFC_STR                               0x01U
#define USBD_IDX_PRODUCT_STR                           0x02U
#define USBD_IDX_SERIAL_STR                            0x03U

#define STRING_FRAMEWORK_MAX_LENGTH                    256U

#define FULL_SPEED                                     0x00U
#define HIGH_SPEED                                     0x01U

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
