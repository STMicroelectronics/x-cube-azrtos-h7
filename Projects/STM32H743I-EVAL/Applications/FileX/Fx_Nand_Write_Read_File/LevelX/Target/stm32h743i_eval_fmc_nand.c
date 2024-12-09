/**
  ******************************************************************************
* @file    stm32h743i_eval_fmc_nand.c
* @author  MCD Application Team
* @brief   This file includes a standard driver for the MT29F2G16AABWP FMC
*          memory mounted on STM32H743I-EVAL board.
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

/* Includes ------------------------------------------------------------------*/
#include "stm32h743i_eval_fmc_nand.h"

/** @addtogroup BSP
* @{
*/

/** @addtogroup STM32H743I_EVAL
* @{
*/

/** @defgroup STM32H743I_EVAL_FMC STM32H743I_EVAL_FMC
* @{
*/
/* Exported variables --------------------------------------------------------*/
/** @addtogroup STM32H743I_EVAL_FMC_NAND_Exported_Variables FMC_NAND Exported Variables
* @{
*/
NAND_HandleTypeDef hnand[NAND_INSTANCES_NBR];

/** @defgroup STM32H743I_EVAL_NOR_Private_Functions_Prototypes NOR Private Functions Prototypes
* @{
*/
static void NAND_MspInit(NAND_HandleTypeDef *hnand);
static void NAND_MspDeInit(NAND_HandleTypeDef *hnand);

/**
* @brief  Initializes the NAND device.
* @param  Instance  NAND Instance
* @retval BSP status
*/
int32_t BSP_FMC_NAND_Init(uint32_t Instance)
{
  int32_t ret = BSP_ERROR_NONE;

  if (Instance >= NAND_INSTANCES_NBR)
  {
    ret =  BSP_ERROR_WRONG_PARAM;
  }
  else
  {
#if (USE_HAL_NAND_REGISTER_CALLBACKS == 0)
    /* Msp NAND initialization */
    NAND_MspInit(&hnand[Instance]);
#endif /* USE_HAL_NAND_REGISTER_CALLBACKS */

    hnand[Instance].Instance  = FMC_NAND_DEVICE;

    /* __weak function can be rewritten by the application */
    if (MX_NAND_Init(&hnand[Instance]) != HAL_OK)
    {
      ret = BSP_ERROR_NO_INIT;
    }
  }

  return ret;
}

/**
* @brief  DeInitializes the NAND memory.
* @retval BSP status
*/
int32_t BSP_FMC_NAND_DeInit(uint32_t Instance)
{
  int32_t ret = BSP_ERROR_NONE;;

  if (Instance >= NAND_INSTANCES_NBR)
  {
    ret =  BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    /* NAND memory de-initialization */
    hnand[Instance].Instance = FMC_NAND_DEVICE;

    if (HAL_NAND_DeInit(&hnand[Instance]) != HAL_OK)
    {
      ret = BSP_ERROR_PERIPH_FAILURE;
    }
    else
    {
#if (USE_HAL_NAND_REGISTER_CALLBACKS == 0)
      /* NAND controller de-initialization */
      NAND_MspDeInit(&hnand[Instance]);
#endif /* (USE_HAL_NAND_REGISTER_CALLBACKS == 0) */

    }
  }

  return ret;
}

/**
* @brief  Reads NAND flash IDs.
* @param  Instance  NAND Instance
* @param  pNAND_ID   Pointer to NAND ID structure
* @retval BSP status
*/
int32_t BSP_FMC_NAND_ReadID(uint32_t Instance, NAND_IDTypeDef *pNAND_ID)
{
  int32_t ret = BSP_ERROR_NONE;

  if (Instance >= NAND_INSTANCES_NBR)
  {
    ret =  BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    if (HAL_NAND_Read_ID(&hnand[Instance], pNAND_ID) != HAL_OK)
    {
      ret = BSP_ERROR_PERIPH_FAILURE;
    }
  }

  return ret;
}

/**
* @brief  Reads current status of the NAND memory.
* @param  Instance  NAND Instance
* @retval NAND memory status: whether busy or not
*/
int32_t BSP_FMC_NAND_GetStatus(uint32_t Instance)
{
  int32_t ret = BSP_ERROR_NONE;
  uint32_t status;

  if (Instance >= NAND_INSTANCES_NBR)
  {
    ret =  BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    status = HAL_NAND_Read_Status(&hnand[Instance]);

    switch (status)
    {
      case NAND_BUSY:
        ret = BSP_ERROR_BUSY;
        break;

      case NAND_ERROR:
        ret = BSP_ERROR_PERIPH_FAILURE;
        break;

      case NAND_READY:
        ret = BSP_ERROR_NONE;
        break;
    }
  }

  return ret;
}

/**
* @brief  Erases the entire NAND chip.
* @param  Instance  NAND Instance
* @retval BSP status
*/
int32_t BSP_FMC_NAND_EraseChip(uint32_t Instance)
{
  int32_t ret = BSP_ERROR_NONE;
  uint32_t i;
  NAND_AddressTypeDef Address = {0};

  if (Instance >= NAND_INSTANCES_NBR)
  {
    ret =  BSP_ERROR_WRONG_PARAM;
  }
  else /*Erase all blocks*/
  {
    for (i = 0; i < hnand[Instance].Config.BlockNbr; i++)
    {
      Address.Block = i;

      if (HAL_NAND_Erase_Block(&hnand[Instance], &Address) != HAL_OK)
      {
        ret = BSP_ERROR_PERIPH_FAILURE;
      }
    }
  }

  return ret;
}

/**
* @brief  Erases the specified block of the NAND device.
* @param  Instance  NAND Instance
* @param  pAddress  pointer to BSP_NAND address structure
* @retval BSP status
*/

int32_t BSP_FMC_NAND_Erase_Block(uint32_t Instance, BSP_NAND_AddressTypeDef_t *pAddress)
{
  int32_t ret = BSP_ERROR_NONE;

  NAND_AddressTypeDef Address = {0};

  if (Instance >= NAND_INSTANCES_NBR)
  {
    ret =  BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    Address.Block = pAddress->Block;
    Address.Page = pAddress->Page;

    if (HAL_NAND_Erase_Block(&hnand[Instance], &Address) != HAL_OK)
    {
      ret = BSP_ERROR_PERIPH_FAILURE;
    }
  }

  return ret;
}

/**
* @brief  Reads an amount of data from the NAND device.
* @param  Instance       NAND Instance
* @param  pBuffer        pointer to destination read buffer
* @param  pAddress       pointer to BSP_NAND address structure
* @param  NumPageToRead  number of pages to read from block
* @retval BSP status
*/
int32_t BSP_FMC_NAND_Read(uint32_t Instance, BSP_NAND_AddressTypeDef_t *pAddress, uint16_t *pBuffer, uint32_t NumPageToRead)
{
  int32_t ret = BSP_ERROR_NONE;
  NAND_AddressTypeDef Address = {0};

  if (Instance >= NAND_INSTANCES_NBR)
  {
    ret =  BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    Address.Block = pAddress->Block;
    Address.Page = pAddress->Page;

    if (HAL_NAND_Read_Page_16b(&hnand[Instance], &Address, pBuffer, NumPageToRead) != HAL_OK)
    {
      ret = BSP_ERROR_PERIPH_FAILURE;
    }
  }

  return ret;
}

/**
* @brief  Write Page(s) to NAND memory block
* @param  Instance         NAND Instance
* @param  pAddress         pointer to BSP_NAND address structure
* @param  pBuffer          pointer to source buffer to write.
* @param  NumPageToWrite   number of pages to write to block
* @retval BSP status
*/
int32_t BSP_FMC_NAND_Write(uint32_t Instance, BSP_NAND_AddressTypeDef_t *pAddress, uint16_t *pBuffer, uint32_t NumPageToWrite)
{
  int32_t ret = BSP_ERROR_NONE;
  NAND_AddressTypeDef Address = {0};

  if (Instance >= NAND_INSTANCES_NBR)
  {
    ret =  BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    Address.Block = pAddress->Block;
    Address.Page = pAddress->Page;

    if (HAL_NAND_Write_Page_16b(&hnand[Instance], &Address, pBuffer, NumPageToWrite) != HAL_OK)
    {
      ret = BSP_ERROR_PERIPH_FAILURE;
    }
  }

  return ret;
}

/**
* @brief  Read Spare area(s) from NAND device
* @param  Instance             NAND Instance
* @param  pAddress             pointer to BSP_NAND address structure
* @param  pBuffer              pointer to source buffer to write. pBuffer should be 16bits aligned.
* @param  NumSpareAreaToRead   Number of spare area to read
* @retval HAL status
*/
int32_t  BSP_FMC_NAND_Read_SpareArea(uint32_t Instance, BSP_NAND_AddressTypeDef_t *pAddress, uint16_t *pBuffer, uint32_t NumSpareAreaToRead)
{
  int32_t ret = BSP_ERROR_NONE;
  NAND_AddressTypeDef Address = {0};

  if (Instance >= NAND_INSTANCES_NBR)
  {
    ret =  BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    Address.Block = pAddress->Block;
    Address.Page = pAddress->Page;

    if (HAL_NAND_Read_SpareArea_16b(&hnand[Instance], &Address, pBuffer, NumSpareAreaToRead) != HAL_OK)
    {
      ret = BSP_ERROR_PERIPH_FAILURE;
    }
  }

  return ret;
}

/**
* @brief  Write Spare area(s) to NAND device
* @param  Instance              NAND Instance
* @param  pAddress              pointer to BSP_NAND address structure
* @param  pBuffer               pointer to source buffer to write
* @param  NumSpareAreaTowrite   number of spare areas to write to block
* @retval HAL status
*/
int32_t  BSP_FMC_NAND_Write_SpareArea(uint32_t Instance, BSP_NAND_AddressTypeDef_t *pAddress, uint16_t *pBuffer, uint32_t NumSpareAreaTowrite)
{
  int32_t ret = BSP_ERROR_NONE;
  NAND_AddressTypeDef Address = {0};

  if (Instance >= NAND_INSTANCES_NBR)
  {
    ret =  BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    Address.Block = pAddress->Block;
    Address.Page = pAddress->Page;

    if (HAL_NAND_Write_SpareArea_16b(&hnand[Instance], &Address, pBuffer, NumSpareAreaTowrite) != HAL_OK)
    {
      ret = BSP_ERROR_PERIPH_FAILURE;
    }
  }

  return ret;
}

/**
* @brief  Initializes the NAND periperal.
* @param  hNand NAND handle
* @retval HAL status
*/
__weak HAL_StatusTypeDef MX_NAND_Init(NAND_HandleTypeDef *hNand)
{
  FMC_NAND_PCC_TimingTypeDef ComSpaceTiming = {0};
  FMC_NAND_PCC_TimingTypeDef AttSpaceTiming = {0};

  /* hNand Init */
  hNand->Init.NandBank        = FMC_NAND_BANK3;
  hNand->Init.Waitfeature     = FMC_NAND_WAIT_FEATURE_ENABLE;
  hNand->Init.MemoryDataWidth = FMC_NAND_MEM_BUS_WIDTH_16;
  hNand->Init.EccComputation  = FMC_NAND_ECC_DISABLE;
  hNand->Init.ECCPageSize     = FMC_NAND_ECC_PAGE_SIZE_2048BYTE;
  hNand->Init.TCLRSetupTime   = 2;
  hNand->Init.TARSetupTime    = 2;

  /* hNand Config */
  hNand->Config.PageSize = 1024; /*1024 words*/
  hNand->Config.SpareAreaSize = 32; /*32 words */
  hNand->Config.BlockSize = 64; /*64 pages*/
  hNand->Config.BlockNbr = 2048;
  hNand->Config.PlaneSize = 2048; /*NAND memory plane size measured in number of blocks */
  hNand->Config.PlaneNbr = 1;
  hNand->Config.ExtraCommandEnable = DISABLE;

  /* ComSpaceTiming */
  ComSpaceTiming.SetupTime = 1; /*according to AN4761 page 33*/
  ComSpaceTiming.WaitSetupTime = 7;
  ComSpaceTiming.HoldSetupTime = 2;
  ComSpaceTiming.HiZSetupTime = 8;

  /* AttSpaceTiming */
  AttSpaceTiming.SetupTime = 1; /*according to AN4761 page 33*/
  AttSpaceTiming.WaitSetupTime = 7;
  AttSpaceTiming.HoldSetupTime = 2;
  AttSpaceTiming.HiZSetupTime = 8;

  if (HAL_NAND_Init(hNand, &ComSpaceTiming, &AttSpaceTiming) != HAL_OK)
  {
    return  HAL_ERROR;
  }

  if (HAL_NAND_Reset(hNand) != HAL_OK)
  {
    return  HAL_ERROR;
  }

  return HAL_OK;
}
/** @defgroup STM32H743I_EVAL_NAND_Private_Functions NAND Private Functions
* @{
*/
/**
* @brief  Initializes the NAND MSP.
* @param  hnand NAND handle
* @retval None
*/
static void NAND_MspInit(NAND_HandleTypeDef *hnand)
{
  GPIO_InitTypeDef GPIO_Init_Structure;
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hnand);

  /* Enable FMC clock */
  __HAL_RCC_FMC_CLK_ENABLE();

  /* Enable GPIOs clock */
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();

  /** FMC GPIO Configuration
  PG9   ------> FMC_NCE
  PD0   ------> FMC_D2
  PD1   ------> FMC_D3
  PD4   ------> FMC_NOE
  PD5   ------> FMC_NWE
  PD6   ------> FMC_NWAIT
  PD8   ------> FMC_D13
  PD9   ------> FMC_D14
  PD10   ------> FMC_D15
  PD11   ------> FMC_CLE (FMC_A16)
  PD12   ------> FMC_ALE (FMC_A17)
  PD13   ------> FMC_A18
  PD14   ------> FMC_D0
  PD15   ------> FMC_D1

  PE7   ------> FMC_D4
  PE8   ------> FMC_D5
  PE9   ------> FMC_D6
  PE10   ------> FMC_D7
  PE11   ------> FMC_D8
  PE12   ------> FMC_D9
  PE13   ------> FMC_D10
  PE14   ------> FMC_D11
  PE15   ------> FMC_D12
  */

  /* Common GPIO configuration */
  GPIO_Init_Structure.Mode      = GPIO_MODE_AF_PP;
  GPIO_Init_Structure.Pull      = GPIO_PULLUP;
  GPIO_Init_Structure.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_Init_Structure.Alternate = GPIO_AF12_FMC;

  /* GPIOD configuration */
  GPIO_Init_Structure.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_4 | GPIO_PIN_5 | \
                            GPIO_PIN_6 | GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | \
                            GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_14 | GPIO_PIN_15;
  HAL_GPIO_Init(GPIOD, &GPIO_Init_Structure);

  /* GPIOE configuration */
  GPIO_Init_Structure.Pin = GPIO_PIN_7  | GPIO_PIN_8  | GPIO_PIN_9  | GPIO_PIN_10 | GPIO_PIN_11 | \
                            GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
  HAL_GPIO_Init(GPIOE, &GPIO_Init_Structure);

  /* GPIOG configuration */
  GPIO_Init_Structure.Pin = GPIO_PIN_9 ;
  HAL_GPIO_Init(GPIOG, &GPIO_Init_Structure);
}

/**
* @brief  DeInitializes NAND MSP.
* @param  hnand NAND handle
* @retval None
*/
static void NAND_MspDeInit(NAND_HandleTypeDef *hnand)
{
  GPIO_InitTypeDef GPIO_Init_Structure;
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hnand);

  /* GPIOD configuration */
  GPIO_Init_Structure.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_4 | GPIO_PIN_5 | \
                            GPIO_PIN_6 | GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | \
                            GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_14 | GPIO_PIN_15;
  HAL_GPIO_DeInit(GPIOD, GPIO_Init_Structure.Pin);

  /* GPIOE configuration */
  GPIO_Init_Structure.Pin = GPIO_PIN_7  | GPIO_PIN_8  | GPIO_PIN_9  | GPIO_PIN_10 | GPIO_PIN_11 | \
                            GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
  HAL_GPIO_DeInit(GPIOE, GPIO_Init_Structure.Pin);

  /* GPIOG configuration */
  GPIO_Init_Structure.Pin = GPIO_PIN_9 ;
  HAL_GPIO_DeInit(GPIOG, GPIO_Init_Structure.Pin);

  /* Disable FMC clock */
  __HAL_RCC_FMC_CLK_DISABLE();
}
