/**
  ******************************************************************************
  * @file    flash_if.c
  * @author  MCD Application Team
  * @brief   This file provides high level routines to manage internal Flash
  *          programming (erase and write).
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


/* Includes ------------------------------------------------------------------*/
#include "flash_if.h"
#include "main.h"
#include "stm32h7xx_hal_flash.h"
#include <string.h>
#include <inttypes.h>
#include "msg.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Base address of the Flash sectors Bank 2. */
#define ADDR_FLASH_SECTOR_0_BANK2     ((uint32_t)0x08100000) /* Base @ of Sector 0, 128 Kbytes */
#define ADDR_FLASH_SECTOR_1_BANK2     ((uint32_t)0x08120000) /* Base @ of Sector 1, 128 Kbytes */
#define ADDR_FLASH_SECTOR_2_BANK2     ((uint32_t)0x08140000) /* Base @ of Sector 2, 128 Kbytes */
#define ADDR_FLASH_SECTOR_3_BANK2     ((uint32_t)0x08160000) /* Base @ of Sector 3, 128 Kbytes */
#define ADDR_FLASH_SECTOR_4_BANK2     ((uint32_t)0x08180000) /* Base @ of Sector 4, 128 Kbytes */
#define ADDR_FLASH_SECTOR_5_BANK2     ((uint32_t)0x081A0000) /* Base @ of Sector 5, 128 Kbytes */
#define ADDR_FLASH_SECTOR_6_BANK2     ((uint32_t)0x081C0000) /* Base @ of Sector 6, 128 Kbytes */
#define ADDR_FLASH_SECTOR_7_BANK2     ((uint32_t)0x081E0000) /* Base @ of Sector 7, 128 Kbytes */

/* Flash Memory Location parameters
   Please make sure not to overwrite the WiFi example code area. */
#define USER_FLASH_END_ADDRESS           FLASH_END        /* 0x081FFFFF */

const uint32_t USER_FLASH_START_ADDRESS = ADDR_FLASH_SECTOR_6_BANK2;


/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
static uint32_t GetSector(uint32_t address);

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Unlocks Flash for write access
  * @param  None
  * @retval None
  */
void FLASH_If_Init(void)
{
  /* Unlock the internal flash. */
  if (HAL_OK != HAL_FLASH_Unlock())
  {
    /* Call the error handler. */
    Error_Handler();
  }
}

/**
  * @brief  Erase 256Ko of flash starting from the given address.
  * @param  address: The address from which to start the erasure.
  * @retval 0 if operation is successful, 1 else.
  */
int32_t FLASH_If_Erase(uint32_t address)
{
  /* Device voltage range supposed to be [2.7V to 3.6V],
     the operation will be done by word. */
  if ((USER_FLASH_START_ADDRESS <= address) && (address <= USER_FLASH_END_ADDRESS))
  {
    FLASH_EraseInitTypeDef erase_conf = {0};
    uint32_t flash_error = 0;
    erase_conf.TypeErase = FLASH_TYPEERASE_SECTORS;
    erase_conf.Banks = FLASH_BANK_2;
    /* Get the number of sector to start from. */
    erase_conf.Sector = GetSector(address);
    /* 2 * 128 Ko = 256 Ko */
    erase_conf.NbSectors = 2;
    erase_conf.VoltageRange = FLASH_VOLTAGE_RANGE_3;

    if (HAL_FLASHEx_Erase(&erase_conf, &flash_error) != HAL_OK)
    {
      MSG_ERROR("FLASH_If_Erase(0x%p) -> %"PRIu32"\n", (void *)address, flash_error);
      return 1;
    }
  }
  else
  {
    return 1;
  }

  MSG_DEBUG("FLASH_If_Erase(%p)\n", (void *)address);
  return 0;
}

/**
  * @brief  This function writes a data buffer in flash (data are 32-bit aligned).
  * @note   After writing data buffer, the flash content is checked.
  * @param  flash_address: start address for writing data buffer
  * @param  data: Data buffer
  * @param  data_length: length of data buffer
  * @retval 0: Data successfully written to Flash memory
  *         1: Error occurred while writing data in Flash memory
  *         2: Written Data in flash memory is different from expected one
  */
uint32_t FLASH_If_Write(uint32_t flash_address, const uint8_t *data, uint32_t data_length)
{
  const uint32_t write_type = FLASH_TYPEPROGRAM_FLASHWORD;
  /* The number of 256-bit (32 bytes) write operation. */
  const uint32_t i_max = (data_length + 32) / 32;
  const uint8_t *p_data = data;

  for (uint32_t i = 0; (i < i_max) && (flash_address <= (USER_FLASH_END_ADDRESS - 4)); i++)
  {
    /* 256 bits (32 bytes) correspond to 8 x 32-bit word. */
    uint32_t data_xwords[8];
    const size_t data_xwords_size = sizeof(data_xwords);
    memcpy(data_xwords, p_data, data_xwords_size);

    /* Device voltage range supposed to be [2.7V to 3.6V], the operation will be done by word. */
    if (HAL_FLASH_Program(write_type, flash_address, (uint32_t)data_xwords) == HAL_OK)
    {
      /* Increment FLASH destination address. */
      flash_address += data_xwords_size;
      /* Increment the position in the data, note that the last operation may contain out of interest data. */
      p_data += data_xwords_size;
    }
    else
    {
      /* Error occurred while writing data in Flash memory. */
      return 1;
    }
  }

  MSG_DEBUG("FLASH_If_Write() ended with %p\n", (void *)flash_address);

  return 0;
}

/**
  * @brief  Gets the sector of a given address
  * @param  address Address in the FLASH Memory
  * @retval The sector corresponding to the given address
  */
static uint32_t GetSector(uint32_t address)
{
  uint32_t sector = 0;

  if ((address < ADDR_FLASH_SECTOR_1_BANK2) &&
      (address >= ADDR_FLASH_SECTOR_0_BANK2))
  {
    sector = FLASH_SECTOR_0;
  }
  else if ((address < ADDR_FLASH_SECTOR_2_BANK2) &&
           (address >= ADDR_FLASH_SECTOR_1_BANK2))
  {
    sector = FLASH_SECTOR_1;
  }
  else if ((address < ADDR_FLASH_SECTOR_3_BANK2) &&
           (address >= ADDR_FLASH_SECTOR_2_BANK2))
  {
    sector = FLASH_SECTOR_2;
  }
  else if ((address < ADDR_FLASH_SECTOR_4_BANK2) &&
           (address >= ADDR_FLASH_SECTOR_3_BANK2))
  {
    sector = FLASH_SECTOR_3;
  }
  else if ((address < ADDR_FLASH_SECTOR_5_BANK2) &&
           (address >= ADDR_FLASH_SECTOR_4_BANK2))
  {
    sector = FLASH_SECTOR_4;
  }
  else if ((address < ADDR_FLASH_SECTOR_6_BANK2) &&
           (address >= ADDR_FLASH_SECTOR_5_BANK2))
  {
    sector = FLASH_SECTOR_5;
  }
  else if ((address < ADDR_FLASH_SECTOR_7_BANK2) &&
           (address >= ADDR_FLASH_SECTOR_6_BANK2))
  {
    sector = FLASH_SECTOR_6;
  }
  else if ((address < FLASH_END) &&
           (address >= ADDR_FLASH_SECTOR_7_BANK2))
  {
    sector = FLASH_SECTOR_7;
  }
  else
  {
    sector = FLASH_SECTOR_7;
  }

  return sector;
}
