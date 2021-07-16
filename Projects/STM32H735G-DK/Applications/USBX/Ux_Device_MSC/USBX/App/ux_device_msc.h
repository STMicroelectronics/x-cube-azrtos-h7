/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    ux_device_msc.h
  * @author  MCD Application Team
  * @brief   USBX Device MSC header file
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
#ifndef __UX_DEVICE_MSC_H__
#define __UX_DEVICE_MSC_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "ux_api.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stm32h735g_discovery_sd.h"
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

UINT  STORAGE_Status(VOID *storage, ULONG lun, ULONG media_id, ULONG *media_status);

UINT  STORAGE_Read(VOID *storage, ULONG lun, UCHAR *data_pointer,
                   ULONG number_blocks, ULONG lba, ULONG *media_status);

UINT  STORAGE_Write(VOID *storage, ULONG lun, UCHAR *data_pointer,
                    ULONG number_blocks, ULONG lba, ULONG *media_status);

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

#ifdef __cplusplus
}
#endif
#endif  /* __UX_DEVICE_MSC_H__ */
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
