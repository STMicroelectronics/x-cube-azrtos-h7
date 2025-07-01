/**
  ******************************************************************************
  * @file    pdm2pcm_glo.h
  * @author  MCD Application Team
  * @brief   Global header for PDM2PCM conversion code
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2019(-2022) STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __PDM2PCM_GLO_H
#define __PDM2PCM_GLO_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "pdm2pcm.h"

/* Exported constants --------------------------------------------------------*/
#define PDM_FILTER_ENDIANNESS_LE        PDM2PCM_ENDIANNESS_LE
#define PDM_FILTER_ENDIANNESS_BE        PDM2PCM_ENDIANNESS_BE

#define PDM_FILTER_BIT_ORDER_LSB        PDM2PCM_BIT_ORDER_LSB
#define PDM_FILTER_BIT_ORDER_MSB        PDM2PCM_BIT_ORDER_MSB

#define PDM_FILTER_DEC_FACTOR_48        PDM2PCM_DEC_FACTOR_48
#define PDM_FILTER_DEC_FACTOR_64        PDM2PCM_DEC_FACTOR_64
#define PDM_FILTER_DEC_FACTOR_80        PDM2PCM_DEC_FACTOR_80
#define PDM_FILTER_DEC_FACTOR_128       PDM2PCM_DEC_FACTOR_128
#define PDM_FILTER_DEC_FACTOR_16        PDM2PCM_DEC_FACTOR_16
#define PDM_FILTER_DEC_FACTOR_24        PDM2PCM_DEC_FACTOR_24
#define PDM_FILTER_DEC_FACTOR_32        PDM2PCM_DEC_FACTOR_32

#define PDM_FILTER_INIT_ERROR           PDM2PCM_INIT_ERROR
#define PDM_FILTER_CONFIG_ERROR         PDM2PCM_CONFIG_ERROR
#define PDM_FILTER_ENDIANNESS_ERROR     PDM2PCM_ENDIANNESS_ERROR
#define PDM_FILTER_BIT_ORDER_ERROR      PDM2PCM_BIT_ORDER_ERROR
#define PDM_FILTER_CRC_LOCK_ERROR       PDM2PCM_CRC_LOCK_ERROR
#define PDM_FILTER_DECIMATION_ERROR     PDM2PCM_DECIMATION_ERROR
#define PDM_FILTER_GAIN_ERROR           PDM2PCM_GAIN_ERROR
#define PDM_FILTER_SAMPLES_NUMBER_ERROR PDM2PCM_SAMPLES_NUMBER_ERROR

#define PDM2PCM_INTERNAL_MEMORY_SIZE    PDM2PCM_MEMORY_SIZE

/* Exported types ------------------------------------------------------------*/

typedef PDM2PCM_Handler_t PDM_Filter_Handler_t;
typedef PDM2PCM_Config_t  PDM_Filter_Config_t;

/* Exported macros -----------------------------------------------------------*/

/* Exported functions ------------------------------------------------------- */
// legacy API with inverted bit_order config logic
uint32_t PDM_Filter_Init(PDM_Filter_Handler_t *pHandler);
uint32_t PDM_Filter_setConfig(PDM_Filter_Handler_t *pHandler, PDM_Filter_Config_t *pConfig);
uint32_t PDM_Filter_getConfig(PDM_Filter_Handler_t *pHandler, PDM_Filter_Config_t *pConfig);
uint32_t PDM_Filter(void *pDataIn, void *pDataOut, PDM_Filter_Handler_t *pHandler);

#ifdef __cplusplus
}
#endif

#endif /* __PDM2PCM_GLO_H */


