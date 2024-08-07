
/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    app_filex.c
  * @author  MCD Application Team
  * @brief   FileX applicative file
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
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
#include "app_filex.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/

/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
static UCHAR Read_buffer[100];
static UCHAR Write_buffer[] = "USBX_STM32_Host_Mass_Storage";
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/

/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/**
  * @brief  Application FileX Initialization.
  * @param memory_ptr: memory pointer
  * @retval int
  */
UINT MX_FileX_Init(VOID *memory_ptr)
{
  UINT ret = FX_SUCCESS;

  TX_BYTE_POOL *byte_pool = (TX_BYTE_POOL*)memory_ptr;

  /* USER CODE BEGIN MX_FileX_MEM_POOL */
  (void)byte_pool;

  /* USER CODE END MX_FileX_MEM_POOL */
  /* USER CODE BEGIN MX_FileX_Init */

  /* USER CODE END MX_FileX_Init */

  /* Initialize FileX.  */
  fx_system_initialize();

  /* USER CODE BEGIN MX_FileX_Init 1*/

  /* USER CODE END MX_FileX_Init 1*/

  return ret;
}

/* USER CODE BEGIN 1 */

/**
  * @brief  Application Write File process.
  * @param  fx_media: media pointer
  * @retval status
  */
UINT App_File_Write(FX_MEDIA *fx_media)
{
  FX_FILE my_file;
  UINT status;

  /* Open "TEST.TXT" file */
  status = fx_file_open(fx_media, &my_file, "TEST.TXT", FX_OPEN_FOR_WRITE);

  /* Check the file open status */
  if (status == FX_SUCCESS)
  {
    /* Seek the start of File */
    fx_file_seek(&my_file, 0);

    /* Write a string to "TEST.TXT" file */
    status = fx_file_write(&my_file, Write_buffer, sizeof(Write_buffer));

    /* Check the file write status */
    if (status == FX_SUCCESS)
    {
      /* Close File after Finish writing */
      fx_file_close(&my_file);

      /* Flush media.  */
      fx_media_flush(fx_media);
    }
    else
    {
      /* Close File after Finish writing */
      fx_file_close(&my_file);

      /* Delete File */
      fx_file_delete(fx_media, "TEST.TXT");
    }
  }

  return status ;
}

/**
  * @brief  Application Read File process.
  * @param  fx_media: media pointer
  * @retval status
  */
UINT App_File_Read(FX_MEDIA *fx_media)
{
  FX_FILE my_file;
  ULONG requested_length;
  UINT file_attribute;
  UINT status;

  /* Try to read the file attributes */
  status = fx_file_attributes_read(fx_media, "TEST.TXT", &file_attribute);

  if (status == FX_SUCCESS)
  {
    /* If this is a directory, pass */
    if ((file_attribute & (FX_DIRECTORY | FX_VOLUME)) == 0U)
    {
      /* Try to open the file */
      status = fx_file_open(fx_media, &my_file, "TEST.TXT", FX_OPEN_FOR_READ);

      /* Check the file open status */
      if (status == FX_SUCCESS)
      {
        /* Seek start of File */
        fx_file_seek(&my_file, 0);

        /* Read the file in blocks */
        status = fx_file_read(&my_file, Read_buffer, sizeof(Read_buffer),
                              &requested_length);

        /* Read the entire file */
        if (status == FX_SUCCESS)
        {
          /* Compare Written buffer and read buffer */
          status = ux_utility_memory_compare(Write_buffer, Read_buffer,
                                             sizeof(Write_buffer));
        }

        /* Closing File */
        fx_file_close(&my_file);
      }
    }
  }

  return status;
}

/**
  * @brief  Application Create File process.
  * @param  fx_media: media pointer
  * @retval status
  */
UINT App_File_Create(FX_MEDIA *fx_media)
{
  UINT status;

  /* Create a file called TEST.TXT in the root directory */
  status = fx_file_create(fx_media, "TEST.TXT");

  /* Check for an already created status */
  if (status == FX_ALREADY_CREATED)
  {
    /* File Creation success */
    status = FX_SUCCESS;
  }

  return status;
}

/* USER CODE END 1 */
