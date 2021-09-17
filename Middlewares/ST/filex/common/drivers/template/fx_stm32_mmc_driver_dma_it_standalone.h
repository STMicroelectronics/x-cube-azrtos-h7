/**************************************************************************/
/*                                                                        */
/*       Copyright (c) Microsoft Corporation. All rights reserved.        */
/*                                                                        */
/*       This software is licensed under the Microsoft Software License   */
/*       Terms for Microsoft Azure RTOS. Full text of the license can be  */
/*       found in the LICENSE file at https://aka.ms/AzureRTOS_EULA       */
/*       and in the root directory of this software.                      */
/*                                                                        */
/**************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef FX_STM32_MMC_DRIVER_H
#define FX_STM32_MMC_DRIVER_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "fx_api.h"

#include "stm32NNxx_ha.h"

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/

/* Default timeout used to wait for fx operations */
#define FX_STM32_MMC_DEFAULT_TIMEOUT                           (10 * 1000)

/* SD instance default to O */
#define FX_STM32_MMC_INSTANCE                                  0

/* Default SD sector size typically 512 for uSD */
#define FX_STM32_MMC_DEFAULT_SECTOR_SIZE                       512

/* let the filex low-level driver initialize the SD driver */
#define FX_STM32_MMC_INIT                                      1

/* Use the SD DMA API, when enabled cache maintenance
 * may be required
 */
#define FX_STM32_MMC_DMA_API                                   0

/* Enable the cache maintenance, needed when using SD DMA
 * and accessing buffers in cacheable area
 * this is valid only for CM7 based products or those
 * with dedicated cache IP.
 */
#define FX_STM32_MMC_CACHE_MAINTENANCE                         0


/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */


#if (FX_STM32_MMC_CACHE_MAINTENANCE == 1)

/* USER CODE BEGIN CACHE_MAINTENANCE_MACROS  */
#define invalidate_cache_by_addr(__ptr__, __size__)           SCB_InvalidateDCache_by_Addr(__ptr__, __size__)
#define clean_cache_by_addr(__ptr__, __size__)                SCB_CleanDCache_by_Addr(__ptr__, __size__)

/* USER CODE BEGIN CACHE_MAINTENANCE_MACROS  */

#endif

/* Get the current time in ticks */

/* USER CODE BEGIN FX_STM32_MMC_CURRENT_TIME */

#define FX_STM32_MMC_CURRENT_TIME                       HAL_GetTick

/* USER CODE END FX_STM32_MMC_CURRENT_TIME */

/* USER CODE BEGIN FX_STM32_MMC_PRE_INIT */

#define  FX_STM32_MMC_PRE_INIT(_media_ptr)

/* USER CODE END FX_STM32_MMC_PRE_INIT */


/* USER CODE BEGIN FX_STM32_MMC_POST_INIT */

#define FX_STM32_MMC_POST_INIT(_media_ptr)

/* USER CODE END FX_STM32_MMC_POST_INIT */

/* USER CODE BEGIN FX_STM32_MMC_POST_DEINIT */

#define FX_STM32_MMC_POST_DEINIT(_media_ptr)

/* USER CODE END FX_STM32_MMC_POST_DEINIT */

/* USER CODE BEGIN FX_STM32_MMC_POST_ABORT */

#define FX_STM32_MMC_POST_ABORT(_media_ptr)

/* USER CODE END FX_STM32_MMC_POST_ABORT */

/* USER CODE BEGIN FX_STM32_MMC_PRE_READ_TRANSFER */

#define FX_STM32_MMC_PRE_READ_TRANSFER(_media_ptr)   do { \
                                                          extern UINT read_transfer_completed;    \
                                                          read_transfer_completed = 0; \
                                                    } while(0)

/* USER CODE END FX_STM32_MMC_PRE_READ_TRANSFER */

/* USER CODE BEGIN FX_STM32_MMC_POST_READ_TRANSFER */

#define FX_STM32_MMC_POST_READ_TRANSFER(_media_ptr)

/* USER CODE END FX_STM32_MMC_POST_READ_TRANSFER */

/* USER CODE BEGIN FX_STM32_MMC_READ_TRANSFER_ERROR */

#define FX_STM32_MMC_READ_TRANSFER_ERROR(_status_)

/* USER CODE END FX_STM32_MMC_READ_TRANSFER_ERROR */

/* USER CODE BEGIN FX_STM32_MMC_READ_CPLT_NOTIFY */

#define FX_STM32_MMC_READ_CPLT_NOTIFY()                   do { \
                                                            extern UINT read_transfer_completed;    \
                                                            UINT start = HAL_GetTick(); \
                                                            while( HAL_GetTick() - start < FX_STM32_MMC_DEFAULT_TIMEOUT) \
                                                            { \
                                                              if (read_transfer_completed == 1) \
                                                                break; \
                                                            } \
                                                              if (read_transfer_completed == 0) \
                                                                return FX_IO_ERROR;\
                                                        } while(0)

/* USER CODE END FX_STM32_MMC_READ_CPLT_NOTIFY */

/* USER CODE BEGIN FX_STM32_MMC_WRITE_CPLT_NOTIFY */

#define FX_STM32_MMC_WRITE_CPLT_NOTIFY()                    do { \
                                                            extern UINT write_transfer_completed;    \
                                                            UINT start = HAL_GetTick(); \
                                                            while( HAL_GetTick() - start < FX_STM32_MMC_DEFAULT_TIMEOUT) \
                                                            { \
                                                              if (write_transfer_completed == 1) \
                                                                break; \
                                                            } \
                                                              if (write_transfer_completed == 0) \
                                                                return FX_IO_ERROR;\
                                                        } while(0)

/* USER CODE END FX_STM32_MMC_WRITE_CPLT_NOTIFY */

/* USER CODE BEGIN FX_STM32_MMC_PRE_WRITE_TRANSFER */

#define FX_STM32_MMC_PRE_WRITE_TRANSFER(__media_ptr)   do { \
                                                          extern UINT write_transfer_completed;    \
                                                          write_transfer_completed = 0; \
                                                    } while(0)

/* USER CODE END FX_STM32_MMC_PRE_WRITE_TRANSFER */

/* USER CODE BEGIN FX_STM32_MMC_POST_WRITE_TRANSFER */

#define FX_STM32_MMC_POST_WRITE_TRANSFER        FX_STM32_MMC_POST_READ_TRANSFER

/* USER CODE END FX_STM32_MMC_POST_WRITE_TRANSFER */

/* USER CODE BEGIN FX_STM32_MMC_WRITE_TRANSFER_ERROR */

#define FX_STM32_MMC_WRITE_TRANSFER_ERROR        FX_STM32_MMC_READ_TRANSFER_ERROR
/* USER CODE END FX_STM32_MMC_WRITE_TRANSFER_ERROR */

/* Exported functions prototypes ---------------------------------------------*/

INT fx_stm32_mmc_init(UINT Instance);
INT fx_stm32_mmc_deinit(UINT Instance);

INT fx_stm32_mmc_get_status(UINT Instance);

INT fx_stm32_mmc_read_blocks(UINT Instance, UINT *Buffer, UINT StartSector, UINT NbrOfBlocks);
INT fx_stm32_mmc_write_blocks(UINT Instance, UINT *Buffer, UINT StartSector, UINT NbrOfBlocks);


VOID  fx_stm32_mmc_driver(FX_MEDIA *media_ptr);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

#ifdef __cplusplus
}
#endif

#endif /* FX_STM32_MMC_DRIVER_H */
