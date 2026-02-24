/**
  ******************************************************************************
  * @file    stmpe811_reg.c
  * @author  MCD Application Team
  * @brief   This file provides a set of functions needed to manage the STMPE811
  *          IO Expander component registers drivers.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stmpe811_reg.h"

/** @addtogroup BSP
  * @{
  */

/** @addtogroup Components
  * @{
  */

/** @addtogroup STMPE811
  * @{
  */

/** @addtogroup STMPE811_Exported_Functions
  * @{
  */
/*******************************************************************************
* Function Name : stmpe811_read_reg
* Description   : Generic Reading function. It must be fulfilled with either
*                 I2C or SPI reading functions.
* Input         : Register Address, length of buffer
* Output        : Data Read
*******************************************************************************/
int32_t stmpe811_read_reg(stmpe811_ctx_t *ctx, uint16_t reg, uint8_t* data, uint16_t length)
{
  return ctx->ReadReg(ctx->handle, reg, data, length);
}

/*******************************************************************************
* Function Name : stmpe811_write_reg
* Description   : Generic Writing function. It must be fulfilled with either
*                 I2C or SPI writing function.
* Input         : Register Address, Data to be written, length of buffer
* Output        : None
*******************************************************************************/
int32_t stmpe811_write_reg(stmpe811_ctx_t *ctx, uint16_t reg, uint8_t* data, uint16_t length)
{
  return ctx->WriteReg(ctx->handle, reg, data, length);
}


/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

