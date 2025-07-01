/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    ux_device_dfu_media.h
  * @author  MCD Application Team
  * @brief   USBX Device DFU applicative header file
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
#ifndef __UX_DEVICE_DFU_MEDIA_H__
#define __UX_DEVICE_DFU_MEDIA_H__

#ifdef __cplusplus
extern "C" {
#endif

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/* Includes ------------------------------------------------------------------*/
#include "ux_api.h"
#include "ux_device_class_dfu.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "main.h"
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

#define LEAVE_DFU_ENABLED   1
#define LEAVE_DFU_DISABLED  0

/* Special Commands with Download Request */
#define DFU_CMD_GETCOMMANDS             0x00U
#define DFU_CMD_SETADDRESSPOINTER       0x21U
#define DFU_CMD_ERASE                   0x41U
#define DFU_CMD_READ_UNPROTECT          0x92U

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
VOID USBD_DFU_Activate(VOID *dfu_instance);
VOID USBD_DFU_Deactivate(VOID *dfu_instance);
UINT USBD_DFU_GetStatus(VOID *dfu_instance, ULONG *media_status);
UINT USBD_DFU_Read(VOID *dfu_instance, ULONG block_number, UCHAR *data_pointer,
                   ULONG length, ULONG *actual_length);
UINT USBD_DFU_Write(VOID *dfu_instance, ULONG block_number, UCHAR *data_pointer,
                    ULONG length, ULONG *media_status);
UINT USBD_DFU_Notify(VOID *dfu_instance, ULONG notification);
#ifdef UX_DEVICE_CLASS_DFU_CUSTOM_REQUEST_ENABLE
UINT USBD_DFU_CustomRequest(VOID *dfu_instance, UX_SLAVE_TRANSFER *transfer);
#endif /* UX_DEVICE_CLASS_DFU_CUSTOM_REQUEST_ENABLE */

/* USER CODE BEGIN EFP */
VOID usbx_dfu_download_thread_entry(ULONG thread_input);
/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
/* USER CODE BEGIN PD */

typedef struct
{
  ULONG wblock_num;
  ULONG wlength;
  UCHAR *data_ptr;
} ux_dfu_downloadInfotypeDef;

/* USER CODE END PD */

/* USER CODE BEGIN 2 */

/* USER CODE END 2 */

#ifdef __cplusplus
}
#endif
#endif  /* __UX_DEVICE_DFU_MEDIA_H__ */
