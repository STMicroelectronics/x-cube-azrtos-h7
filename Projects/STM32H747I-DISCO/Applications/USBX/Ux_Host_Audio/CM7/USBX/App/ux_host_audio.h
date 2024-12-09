/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    ux_host_audio.h
  * @author  MCD Application Team
  * @brief   USBX Host Audio applicative header file
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
#ifndef __UX_HOST_AUDIO_H__
#define __UX_HOST_AUDIO_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "ux_api.h"
#include "ux_host_class_audio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

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
VOID audio_playback_thread_entry(ULONG thread_input);
VOID playback_transfer_request_completion(UX_HOST_CLASS_AUDIO_TRANSFER_REQUEST *transfer_request);
/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
/* USER CODE BEGIN PD */
typedef enum {
  AUDIO_DEMO_PLAYBACK_IDLE = 0,
  AUDIO_DEMO_PLAYBACK_WAIT,
  AUDIO_DEMO_PLAYBACK_EXPLORE,
  AUDIO_DEMO_PLAYBACK_CONFIG,
  AUDIO_DEMO_PLAYBACK_START,
  AUDIO_DEMO_PLAYBACK_PLAY,
  AUDIO_DEMO_PLAYBACK_NEXT,
}audio_demo_playback_stateTypeDef;
/* USER CODE END PD */

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

#ifdef __cplusplus
}
#endif
#endif  /* __UX_HOST_AUDIO_H__ */
