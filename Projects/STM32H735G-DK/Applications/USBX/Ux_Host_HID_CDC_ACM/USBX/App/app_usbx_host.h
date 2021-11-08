/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    app_usbx_host.h
  * @author  MCD Application Team
  * @brief   USBX Host applicative header file
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2020-2021 STMicroelectronics.
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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "ux_system.h"
#include "ux_utility.h"
#include "ux_hcd_stm32.h"
#include "stm32h735g_discovery.h"
#include "ux_host_class_hid.h"
#include "ux_host_class_hid_mouse.h"
#include "ux_host_class_hid_keyboard.h"
#include "ux_host_class_cdc_acm.h"
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
UINT MX_USBX_Host_Init(VOID *memory_ptr);

/* USER CODE BEGIN EFP */
void  hid_mouse_thread_entry(ULONG arg);
void  hid_keyboard_thread_entry(ULONG arg);
void cdc_acm_send_app_thread_entry(ULONG arg);
void cdc_acm_recieve_app_thread_entry(ULONG arg);
/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define USBX_APP_STACK_SIZE                          1024
#define USBX_MEMORY_SIZE                             (64 * 1024)
#define APP_QUEUE_SIZE                               1
#define NEW_RECEIVED_DATA                            0x01
#define NEW_DATA_TO_SEND                             0x02
#define BUTTON_KEY                                   BUTTON_USER
#define BUTTON_KEY_PIN                               USER_BUTTON_PIN
/* USER CODE END PD */

/* USER CODE BEGIN 1 */

typedef enum
{
  USB_VBUS_FALSE = 0,
  USB_VBUS_TRUE,
} USB_VBUS_State;

typedef enum
{
  Device_disconnected = 1,
  Device_connected,
  No_Device,
} Device_state;

typedef enum
{
  Unknown_Device = 0,
  CDC_ACM_Device = 1,
  Mouse_Device = 2,
  Keyboard_Device,
  Composite_HID_CDC_ACM,
  Unsupported_Device,
} USB_Device_Type;

typedef struct
{
  USB_Device_Type CDC_Device_Type;
  USB_Device_Type HID_Device_Type;
  USB_Device_Type COMPO_Device_Type;
  Device_state    Dev_state;
} ux_app_devInfotypeDef;

typedef enum
{
  App_Ready = 1,
  App_Start,
  App_Idle,
} ux_app_stateTypeDef;

/* USER CODE END 1 */

#ifdef __cplusplus
}
#endif
#endif /* __APP_USBX_HOST_H__ */
