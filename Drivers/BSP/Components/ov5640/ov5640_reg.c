/**
  ******************************************************************************
  * @file    ov5640_reg.c
  * @author  MCD Application Team
  * @brief   This file provides unitary register function to control the OV5640
  *          Camera driver.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2019-2020 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "ov5640_reg.h"

/** @addtogroup BSP
  * @{
  */

/** @addtogroup Components
  * @{
  */

/** @addtogroup OV5640
  * @brief     This file provides a set of functions needed to drive the
  *            OV5640 Camera sensor.
  * @{
  */

/**
  * @brief  Read OV5640 component registers
  * @param  ctx component context
  * @param  reg Register to read from
  * @param  pdata Pointer to data buffer
  * @param  length Number of data to read
  * @retval Component status
  */
int32_t ov5640_read_reg(ov5640_ctx_t *ctx, uint16_t reg, uint8_t *pdata, uint16_t length)
{
  return ctx->ReadReg(ctx->handle, reg, pdata, length);
}

/**
  * @brief  Write OV5640 component registers
  * @param  ctx component context
  * @param  reg Register to write to
  * @param  pdata Pointer to data buffer
  * @param  length Number of data to write
  * @retval Component status
  */
int32_t ov5640_write_reg(ov5640_ctx_t *ctx, uint16_t reg, uint8_t *data, uint16_t length)
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
