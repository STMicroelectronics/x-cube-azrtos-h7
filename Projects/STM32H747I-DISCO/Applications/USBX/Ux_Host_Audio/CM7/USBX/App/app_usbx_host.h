/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    app_usbx_host.h
  * @author  MCD Application Team
  * @brief   USBX Host applicative header file
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
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

/* Includes ------------------------------------------------------------------*/
#include "ux_api.h"
#include "main.h"
#include "ux_host_audio.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "usb_otg.h"
#include "ux_hcd_stm32.h"
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
#define USBX_HOST_MEMORY_STACK_SIZE     11 * 1024

#define UX_HOST_APP_THREAD_STACK_SIZE   1024
#define UX_HOST_APP_THREAD_PRIO         10

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
UINT MX_USBX_Host_Init(VOID *memory_ptr);

/* USER CODE BEGIN EFP */
VOID USBX_APP_Host_Init(VOID);
VOID USBH_DriverVBUS(uint8_t state);
/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

#ifndef UX_HOST_APP_THREAD_NAME
#define UX_HOST_APP_THREAD_NAME  "USBX App Host Main Thread"
#endif

#ifndef UX_HOST_APP_THREAD_PREEMPTION_THRESHOLD
#define UX_HOST_APP_THREAD_PREEMPTION_THRESHOLD  UX_HOST_APP_THREAD_PRIO
#endif

#ifndef UX_HOST_APP_THREAD_TIME_SLICE
#define UX_HOST_APP_THREAD_TIME_SLICE  TX_NO_TIME_SLICE
#endif

#ifndef UX_HOST_APP_THREAD_START_OPTION
#define UX_HOST_APP_THREAD_START_OPTION  TX_AUTO_START
#endif

/* USER CODE BEGIN 1 */
typedef enum
{
  USB_VBUS_FALSE = 0,
  USB_VBUS_TRUE,
} USB_VBUS_State;
/* USER CODE END 1 */

#ifdef __cplusplus
}
#endif
#endif /* __APP_USBX_HOST_H__ */
