/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    ux_device_audio.h
  * @author  MCD Application Team
  * @brief   USBX Device Audio header file
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2020-2022 STMicroelectronics.
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
#ifndef __UX_DEVICE_AUDIO_H__
#define __UX_DEVICE_AUDIO_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "main.h"
#include "ux_api.h"
#include "ux_device_class_audio.h"
#include "ux_device_class_audio20.h"
#include "ux_device_descriptors.h"
#include "stm32h743i_eval_audio.h"
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */
#define AUDIO_TOTAL_BUF_SIZE   192*600

  
typedef enum
{
  AUDIOPLAYER_STOP = 0,
  AUDIOPLAYER_START,
  AUDIOPLAYER_PLAY,
  AUDIOPLAYER_PAUSE,
  AUDIOPLAYER_EOF,
  AUDIOPLAYER_ERROR,
}AUDIOPLAYER_StateTypedef;

typedef struct
{
  uint32_t        volume;
  uint32_t        mute;
  uint32_t        freq;
  AUDIOPLAYER_StateTypedef state;
}AUDIO_ProcessTypdef ;

typedef enum {
  PLAY_BUFFER_OFFSET_UNKNOWN = 0,
  PLAY_BUFFER_OFFSET_NONE,
  PLAY_BUFFER_OFFSET_HALF,
  PLAY_BUFFER_OFFSET_FULL,
}BUFFER_StateTypeDef;

/* Audio buffer control structure */
typedef struct {
  uint8_t buff[AUDIO_TOTAL_BUF_SIZE];
  BUFFER_StateTypeDef state;   /* empty (none), half, full*/
  uint8_t rd_enable;
  uint32_t rd_ptr;
  uint32_t wr_ptr;
  uint32_t fptr;
}AUDIO_OUT_BufferTypeDef;

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/

/* USER CODE BEGIN EFP */
VOID usbx_audio_play_app_thread(ULONG arg);
VOID Audio_Init(VOID  *audio_instance);
VOID Audio_DeInit(VOID *audio_instance);
UINT Audio_Control(UX_DEVICE_CLASS_AUDIO *audio, UX_SLAVE_TRANSFER *transfer);
VOID Audio_ReadDone(UX_DEVICE_CLASS_AUDIO_STREAM *stream, ULONG length);
VOID Audio_ReadChange(UX_DEVICE_CLASS_AUDIO_STREAM *stream, ULONG alternate_setting);
/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

#ifdef __cplusplus
}
#endif
#endif  /* __UX_DEVICE_AUDIO_H__ */
