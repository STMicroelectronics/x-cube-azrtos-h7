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
#ifndef FX_STM32_SD_DRIVER_H
#define FX_STM32_SD_DRIVER_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "fx_api.h"

#include "stm32NNxx_hal.h"

#ifdef HAL_DCACHE_MODULE_ENABLED
extern DCACHE_HandleTypeDef hdcache1;
#endif

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
#define FX_STM32_SD_DEFAULT_TIMEOUT                         (10000)

#define FX_STM32_SD_INSTANCE                                0
#define FX_STM32_SD_DEFAULT_SECTOR_SIZE                     512

#define FX_STM32_SD_CACHE_MAINTENANCE                       0

#define FX_STM32_SD_INIT                                    1
#define FX_STM32_SD_DMA_API                                 1

#if (FX_STM32_SD_CACHE_MAINTENANCE == 1)

#ifdef HAL_DCACHE_MODULE_ENABLED

#define InvalidateDCache_by_Addr(__ptr__, __size__)      HAL_DCACHE_InvalidateByAddr(&hdcache1, (UINT *)__ptr__, (UINT)__size__)
#define CleanDCache_by_Addr(__ptr__, __size__)           HAL_DCACHE_CleanByAddr(&hdcache1, (UINT *)__ptr__, (UINT)__size__)

#else

#define InvalidateDCache_by_Addr(__ptr__, __size__)
#define CleanDCache_by_Addr(__ptr__, __size__)

#endif

#endif
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

#define FX_STM32_SD_CURRENT_TIME                       HAL_GetTick

#define  FX_STM32_SD_PRE_INIT(_media_ptr)
#define FX_STM32_SD_POST_INIT(_media_ptr)


#define FX_STM32_SD_POST_DEINIT(_media_ptr)

#define FX_STM32_SD_PRE_READ_TRANSFER(_media_ptr)   do { \
                                                          extern UINT read_transfer_completed;    \
                                                          read_transfer_completed = 0; \
                                                    } while(0)

#define FX_STM32_SD_POST_READ_TRANSFER(_media_ptr)
#define FX_STM32_SD_READ_TRANSFER_ERROR(_status_)


#define FX_STM32_SD_READ_CPLT_NOTIFY                   do { \
                                                            extern UINT read_transfer_completed;    \
                                                            UINT start = HAL_GetTick(); \
                                                            while( HAL_GetTick() - start < FX_STM32_SD_DEFAULT_TIMEOUT) \
                                                            { \
                                                              if (read_transfer_completed == 1) \
                                                                break; \
                                                            } \
                                                              if (read_transfer_completed == 0) \
                                                                return FX_IO_ERROR;\
                                                        } while(0)

#define FX_STM32_SD_WRITE_CPLT_NOTIFY                    do { \
                                                            extern UINT write_transfer_completed;    \
                                                            UINT start = HAL_GetTick(); \
                                                            while( HAL_GetTick() - start < FX_STM32_SD_DEFAULT_TIMEOUT) \
                                                            { \
                                                              if (write_transfer_completed == 1) \
                                                                break; \
                                                            } \
                                                              if (write_transfer_completed == 0) \
                                                                return FX_IO_ERROR;\
                                                        } while(0)


#define FX_STM32_SD_PRE_WRITE_TRANSFER(__media_ptr)   do { \
                                                          extern UINT write_transfer_completed;    \
                                                          write_transfer_completed = 0; \
                                                    } while(0)

#define FX_STM32_SD_POST_WRITE_TRANSFER       FX_STM32_SD_POST_READ_TRANSFER
#define FX_STM32_SD_WRITE_TRANSFER_ERROR      FX_STM32_SD_READ_TRANSFER_ERROR
#define FX_STM32_SD_POST_ABORT                FX_STM32_SD_POST_DEINIT


/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
INT fx_stm32_sd_init(UINT Instance);
INT fx_stm32_sd_deinit(UINT Instance);

INT fx_stm32_sd_get_status(UINT Instance);

INT fx_stm32_sd_read_blocks(UINT Instance, UINT *Buffer, UINT StartSector, UINT NbrOfBlocks);
INT fx_stm32_sd_write_blocks(UINT Instance, UINT *Buffer, UINT StartSector, UINT NbrOfBlocks);

VOID  fx_stm32_sd_driver(FX_MEDIA *media_ptr);

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

#endif /* FX_STM32_SD_DRIVER_H */
