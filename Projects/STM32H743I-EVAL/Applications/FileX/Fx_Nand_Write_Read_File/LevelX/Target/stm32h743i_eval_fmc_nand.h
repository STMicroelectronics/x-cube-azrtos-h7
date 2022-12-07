/**
  ******************************************************************************
  * @file    stm32h743i_eval_fmc_nand.h
  * @author  MCD Application Team
  * @brief   This file contains the common defines and functions prototypes for
  *          the stm32h743i_eval_fmc_nand.c driver.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef STM32H743I_EVAL_FMC_NAND_H
#define STM32H743I_EVAL_FMC_NAND_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32h743i_eval_errno.h"
#include "stm32h743i_eval_conf.h"

/** @addtogroup BSP
  * @{
  */

/** @addtogroup STM32H743I_EVAL
  * @{
  */

/** @addtogroup STM32H743I_EVAL_FMC
  * @{
  */

/* Exported types ------------------------------------------------------------*/
/** @defgroup STM32H743I_EVAL_NOR_Exported_Constants NOR Exported Constants
  * @{
  */
#define NAND_INSTANCES_NBR  1U

/** @defgroup STM32H743I_EVAL_FMC_NAND_Exported_Types Exported Types
  * @{
  */

typedef struct
{
  uint16_t Page;   /*!< NAND memory Page address  */

  uint16_t Block;  /*!< NAND memory Block address */

} BSP_NAND_AddressTypeDef_t;



/* Exported constants --------------------------------------------------------*/
/** @defgroup STM32H743I_EVAL_FMC_NAND_Exported_Constants FMC_NAND Exported Constants
  * @{
  */
#define BSP_NAND_MANUFACTURER_CODE  0x2C
#define BSP_NAND_THIRD_ID           0x90
#define BSP_NAND_FOURTH_ID          0xD5

/* Exported functions --------------------------------------------------------*/
/** @defgroup STM32H743I_EVAL_FMC_NAND_Exported_Functions NAND Exported Functions
  * @{
  */
int32_t BSP_FMC_NAND_Init(uint32_t Instance);
int32_t BSP_FMC_NAND_DeInit(uint32_t Instance);
int32_t BSP_FMC_NAND_ReadID(uint32_t Instance, NAND_IDTypeDef *pNAND_ID);
int32_t BSP_FMC_NAND_GetStatus(uint32_t Instance);
int32_t BSP_FMC_NAND_EraseChip(uint32_t Instance);
int32_t BSP_FMC_NAND_Erase_Block(uint32_t Instance, BSP_NAND_AddressTypeDef_t *pAddress);
int32_t BSP_FMC_NAND_Read(uint32_t Instance, BSP_NAND_AddressTypeDef_t *pAddress, uint16_t *pBuffer, uint32_t NumPageToRead);
int32_t BSP_FMC_NAND_Write(uint32_t Instance, BSP_NAND_AddressTypeDef_t *pAddress, uint16_t *pBuffer, uint32_t NumPageToWrite);
int32_t BSP_FMC_NAND_Read_SpareArea(uint32_t Instance, BSP_NAND_AddressTypeDef_t *pAddress, uint16_t *pBuffer, uint32_t NumSpareAreaToRead);
int32_t BSP_FMC_NAND_Write_SpareArea(uint32_t Instance, BSP_NAND_AddressTypeDef_t *pAddress, uint16_t *pBuffer, uint32_t NumSpareAreaTowrite);


/* These functions can be modified in case the current settings
   need to be changed for specific application needs */
HAL_StatusTypeDef MX_NAND_Init(NAND_HandleTypeDef *hNand);
/**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif /* STM32H743I_EVAL_FMC_NAND_H */
