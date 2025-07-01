/**
  ******************************************************************************
  * @file    pdm2pcm.h
  * @author  MCD Application Team
  * @brief   header for PDM2PCM conversion code
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
#ifndef __PDM2PCM_H_
#define __PDM2PCM_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>

/* Exported constants --------------------------------------------------------*/
/** @defgroup PDM2PCM_Exported_Constants PDM2PCM Exported Types
* @{
  */
#define PDM2PCM_ENDIANNESS_LE         0x0000U
#define PDM2PCM_ENDIANNESS_BE         0x0001U

#define PDM2PCM_BIT_ORDER_LSB         0x0000U
#define PDM2PCM_BIT_ORDER_MSB         0x0001U

#define PDM2PCM_DEC_FACTOR_48         0x0001U
#define PDM2PCM_DEC_FACTOR_64         0x0002U
#define PDM2PCM_DEC_FACTOR_80         0x0003U
#define PDM2PCM_DEC_FACTOR_128        0x0004U
#define PDM2PCM_DEC_FACTOR_16         0x0005U
#define PDM2PCM_DEC_FACTOR_24         0x0006U
#define PDM2PCM_DEC_FACTOR_32         0x0007U

#define PDM2PCM_INIT_ERROR            0x0010U
#define PDM2PCM_CONFIG_ERROR          0x0020U
#define PDM2PCM_ENDIANNESS_ERROR      0x0001U
#define PDM2PCM_BIT_ORDER_ERROR       0x0002U
#define PDM2PCM_CRC_LOCK_ERROR        0x0004U
#define PDM2PCM_DECIMATION_ERROR      0x0008U
#define PDM2PCM_GAIN_ERROR            0x0040U
#define PDM2PCM_SAMPLES_NUMBER_ERROR  0x0080U

#define PDM2PCM_MEMORY_SIZE           14
/**
  * @}
  */

/* Exported types ------------------------------------------------------------*/


/** @defgroup PDM2PCM_Exported_Types PDM2PCM Exported Types
* @{
  */
/**
* @brief  Library handler. It keeps track of the static parameters
*         and it handles the internal state of the algorithm.
*
*
* The endianness is for a specific use case in which PDM samples are transmitted
* through an I2S interface; knowing that I2S manages 16 bits words, if PDM bit_order is MSB first,
* first 8 bits PDM word is inside most significant byte of 16 bits I2S word
* and second 8 bits PDM word is inside least significant byte of 16 bits I2S word;
* thus to realign PDM bits order after I2S transmission, we must swap I2S most significant byte
* and least significant byte; that's why this endianness has been added in PDM config.
* In fact endianness may be viewed as a temporal swap of samples (2 by 2 samples).
*
* remarks:
* - if input is stereo, no need to swap I2S most significant byte and least significant byte
*   because channels bits order remains consistent (but left & right channels are swapped)
* - if there is more than 2 channels, channels are grouped by 2 inside I2S slots, considered as stereo
*   and managed in a PDM2PCM stereo config; the remaining channel (if odd number of channels)
*   is managed in a PDM2PCM mono config
* - if bit_order is LSB, nothing special to do (endianness must be set to little-endian)
*/

typedef struct
{
  uint16_t bit_order;                               /*!< bit order of the incoming samples; mind that legacy API PDM_Filter_* logic is reversed */
  uint16_t endianness;                              /*!< endianess is usefull in very specific case of capturing PDM through I2S 16-bit register as described */
  uint32_t high_pass_tap;                           /*!< high pass filtering with forgetting factor Q31; Closer to 1 means higher precision however convergence time is larger; 0 means no filtering at all */
  uint16_t in_ptr_channels;                         /*!< number of channels in input stream */
  uint16_t out_ptr_channels;                        /*!< number of channels in output stream */
  uint32_t *pInternalMemory[PDM2PCM_MEMORY_SIZE];   /*!< internal memory provision, don't change the size please*/
} PDM2PCM_Handler_t;

/**
* @brief  Library configuration. Dynamic parameters can be retrieved through PDM2PCM_getConfig
*
*/
typedef struct
{
  uint16_t decimation_factor;                       /*!< configuration of the decimation factor */
  uint16_t output_samples_number;                   /*!< number of 16-bit PCM output samples */
  int16_t  mic_gain;                                /*!< gain applied in dB from -12dB to +51dB */
} PDM2PCM_Config_t;

/**
  * @}
  */

/* Exported macros -----------------------------------------------------------*/

/* Exported functions ------------------------------------------------------- */

/** @defgroup PDM2PCM_Exported_Functions PDM2PCM Exported Functions
* @{
*/

/**
* @brief initialization function.
* @param  *pHandler pdm2pcm handler of the current filter instance
* @retval 0 if no error occurred, otherwise a bit is raised for each error type.
*/
uint32_t PDM2PCM_init(PDM2PCM_Handler_t *pHandler);

/**
* @brief  Dynamic parameters initialization function.
* @param  *pHandler pdm2pcm handler of the current filter instance
* @param  *pConfig PDM filter Config filled with the desired settings
* @retval 0 if no error occurred, otherwise a bit is raised for each error type.
*/
uint32_t PDM2PCM_setConfig(PDM2PCM_Handler_t *pHandler, PDM2PCM_Config_t *pConfig);
/**
* @brief  Dynamic parameters getter function.
* @param  *pHandler pdm2pcm handler of the filter instance
* @param  *pConfig PDM filter Config that will be filled with actual values
* @retval 0 if no error occurred, otherwise a bit is raised for each error type.
*/
uint32_t PDM2PCM_getConfig(PDM2PCM_Handler_t *pHandler, PDM2PCM_Config_t *pConfig);
/**
* @brief  Main function call. Calls the linked function after checking that the CONFIG_DONE status has been reached.
* @param  *pHandler pdm2pcm handler of the filter instance
* @param  pDataIn Pointer to input data
* @param  pDataOut Pointer to output data
* @retval 0 if no error occurred, otherwise a bit is raised for each error type.
*/
uint32_t PDM2PCM_process(PDM2PCM_Handler_t *pHandler, void *pDataIn, void *pDataOut);

/**
  * @}
  */


#ifdef __cplusplus
}
#endif

#endif /* __PDM2PCM_H_ */


