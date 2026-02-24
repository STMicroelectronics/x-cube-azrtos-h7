/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    app_usbx_host.h
  * @author  MCD Application Team
  * @brief   USBX Host applicative header file
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __APP_USBX_HOST_H__
#define __APP_USBX_HOST_H__

#ifdef __cplusplus
extern "C" {
#endif

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/* Includes ------------------------------------------------------------------*/
#include "ux_api.h"
#include "main.h"
#include "ux_host_mouse.h"
#include "ux_host_keyboard.h"
#include "ux_hcd_stm32.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "usb_otg.h"
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/

/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */
#define USBH_UsrLog(...)   printf(__VA_ARGS__);\
                           printf("\n");

#define USBH_ErrLog(...)   printf("ERROR: ") ;\
                           printf(__VA_ARGS__);\
                           printf("\n");
/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
UINT MX_USBX_Host_Init(VOID);
UINT MX_USBX_Host_Stack_Init(void);
UINT MX_USBX_Host_Stack_DeInit(void);

/* USER CODE BEGIN EFP */
VOID USBH_DriverVBUS(uint8_t state);
VOID USBX_Host_Process(VOID *arg);

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* USER CODE BEGIN 2 */

typedef enum
{
  USB_VBUS_FALSE = 0,
  USB_VBUS_TRUE,
} USB_VBUS_State;

/* USER CODE END 2 */

#ifdef __cplusplus
}
#endif
#endif /* __APP_USBX_HOST_H__ */
