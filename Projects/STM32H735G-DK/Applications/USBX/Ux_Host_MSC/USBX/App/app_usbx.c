/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    app_usbx.c
  * @author  MCD Application Team
  * @brief   USBX applicative file
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "app_usbx.h"

/**
  * @brief  Application USBX Initialization.
  * @param  memory_ptr: memory pointer
  * @retval status
  */
UINT MX_USBX_Init(VOID *memory_ptr)
{
  UINT ret = UX_SUCCESS;

  UCHAR *pointer;
  TX_BYTE_POOL *byte_pool = (TX_BYTE_POOL*)memory_ptr;

  /* USER CODE BEGIN MX_USBX_Init0 */

  /* USER CODE END MX_USBX_Init0 */

  /* Allocate the stack for USBX Memory */
  if (tx_byte_allocate(byte_pool, (VOID **) &pointer,
                       USBX_MEMORY_STACK_SIZE, TX_NO_WAIT) != TX_SUCCESS)
  {
    /* USER CODE BEGIN USBX_ALLOCATE_STACK_ERROR */
    return TX_POOL_ERROR;
    /* USER CODE END USBX_ALLOCATE_STACK_ERROR */
  }

  /* Initialize USBX Memory */
  if (ux_system_initialize(pointer, USBX_MEMORY_STACK_SIZE, UX_NULL, 0) != UX_SUCCESS)
  {
    /* USER CODE BEGIN USBX_SYSTEM_INITIALIZE_ERROR */
    return UX_ERROR;
    /* USER CODE END USBX_SYSTEM_INITIALIZE_ERROR */
  }

  if(MX_USBX_Host_Init(byte_pool) != UX_SUCCESS)
  {
    /* USER CODE BEGIN MX_USBX_Host_Init_Error */
    while(1)
    {
    }
    /* USER CODE END MX_USBX_Host_Init_Error */
  }

  /* USER CODE BEGIN MX_USBX_Init1 */

  /* USER CODE END MX_USBX_Init1 */

  return ret;
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
