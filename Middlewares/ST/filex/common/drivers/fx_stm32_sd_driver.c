/**************************************************************************/
/*                                                                        */
/*       Partial Copyright (c) Microsoft Corporation. All rights reserved.*/
/*                                                                        */
/*       This software is licensed under the Microsoft Software License   */
/*       Terms for Microsoft Azure RTOS. Full text of the license can be  */
/*       found in the LICENSE file at https://aka.ms/AzureRTOS_EULA       */
/*       and in the root directory of this software.                      */
/*      Partial Copyright (c) STMicroelctronics 2020. All rights reserved */
/**************************************************************************/


/* Include necessary system files.  */
#include "fx_stm32_sd_driver.h"


ALIGN_32BYTES (UCHAR scratch[DEFAULT_SECTOR_SIZE]);

UINT  _fx_partition_offset_calculate(void  *partition_sector, UINT partition, ULONG *partition_start, ULONG *partition_size);

static UINT sd_read_data(FX_MEDIA *media_ptr, ULONG sector, UINT num_sectors, UINT use_scratch_buffer);
static UINT sd_write_data(FX_MEDIA *media_ptr, ULONG sector, UINT num_sectors, UINT use_scratch_buffer);

static TX_SEMAPHORE transfer_semaphore;

static uint8_t is_initialized = 0;


static int32_t check_sd_status(uint32_t instance)
{
    uint32_t start = tx_time_get();

    while (tx_time_get() - start < DEFAULT_TIMEOUT)
    {
      if (BSP_SD_GetCardState(instance) == SD_TRANSFER_OK)
      {
        return BSP_ERROR_NONE;
      }
    }

    return BSP_ERROR_BUSY;
}

/**
  * @brief This function is the entry point to the STM32 SDIO disk driver.     */
 /*        It relies on the STM32 peripheral library from ST.
  * @param media_ptr: FileX's Media Config Block
  * @retval None
  */
VOID  fx_stm32_sd_driver(FX_MEDIA *media_ptr)
{
    UINT status;
    UINT unaligned_buffer = 0;
    ULONG partition_start;
    ULONG partition_size;

#if (FX_DRIVER_CALLS_SD_INIT == 0)
    is_initialized = 1; /* the SD  was initialized by the application*/
#endif
   /* before performing any operation, check the status of the SDMMC */
    if (is_initialized == 1)
    {
        if (check_sd_status(SD_INSTANCE) != BSP_ERROR_NONE)
        {
            media_ptr->fx_media_driver_status =  FX_IO_ERROR;
            return;
        }
    }

    /* Process the driver request specified in the media control block.  */
    switch(media_ptr->fx_media_driver_request)
    {
        case FX_DRIVER_INIT:
        {
#if (FX_DRIVER_CALLS_SD_INIT == 1)
            /* Initialize the SD instance */
            if (is_initialized == 0)
            {
                status = BSP_SD_Init(SD_INSTANCE);

                if (status == BSP_ERROR_NONE)
                {
                    is_initialized = 1;
#endif
                    /* Create a counting semaphore to check the DMA transfer status */
                    if (tx_semaphore_create(&transfer_semaphore, "sdmmc dma transfer semaphore", 1) != TX_SUCCESS)
                    {
                        media_ptr->fx_media_driver_status =  FX_IO_ERROR;
                    }
                    else
                    {
                        media_ptr->fx_media_driver_status =  FX_SUCCESS;
                    }

#if (FX_DRIVER_CALLS_SD_INIT == 1)
                }
                else
                {
                    media_ptr->fx_media_driver_status =  FX_IO_ERROR;
                }
            }
#endif
            break;
        }

        case FX_DRIVER_UNINIT:
        {
            tx_semaphore_delete(&transfer_semaphore);

#if (FX_DRIVER_CALLS_SD_INIT == 1)
            BSP_SD_DeInit(SD_INSTANCE);
            is_initialized = 0;
#endif
            /* Successful driver request.  */
            media_ptr->fx_media_driver_status = FX_SUCCESS;
            break;
        }

        case FX_DRIVER_READ:
        {
            media_ptr->fx_media_driver_status = FX_IO_ERROR;
            unaligned_buffer = (UINT)(media_ptr->fx_media_driver_buffer) & 0x3;

            if (sd_read_data(media_ptr, media_ptr->fx_media_driver_logical_sector + media_ptr->fx_media_hidden_sectors,
                             media_ptr->fx_media_driver_sectors, unaligned_buffer) == FX_SUCCESS)
            {
                media_ptr->fx_media_driver_status = FX_SUCCESS;
            }

            break;
        }

        case FX_DRIVER_WRITE:
        {
            media_ptr->fx_media_driver_status = FX_IO_ERROR;
            unaligned_buffer = (UINT)(media_ptr->fx_media_driver_buffer) & 0x3;

            if (sd_write_data(media_ptr, media_ptr->fx_media_driver_logical_sector + media_ptr->fx_media_hidden_sectors,
                              media_ptr->fx_media_driver_sectors, unaligned_buffer) == FX_SUCCESS)
            {
                media_ptr->fx_media_driver_status = FX_SUCCESS;
            }

            break;
        }

        case FX_DRIVER_FLUSH:
        {
            /* Return driver success.  */
            media_ptr->fx_media_driver_status =  FX_SUCCESS;
            break;
        }

        case FX_DRIVER_ABORT:
        {
            /* Return driver success.  */
            media_ptr->fx_media_driver_status =  FX_SUCCESS;
            break;
        }

        case FX_DRIVER_BOOT_READ:
        {
            /* Check supplied buffer alignment*/
            unaligned_buffer = (UINT)(media_ptr->fx_media_driver_buffer) & 0x3;

            /* the boot sector is the sector zero */
            status = sd_read_data(media_ptr, 0, media_ptr->fx_media_driver_sectors, unaligned_buffer);

            if (status != FX_SUCCESS)
            {
                media_ptr->fx_media_driver_status = status;
                break;
            }

            /* Check if the sector 0 is the actual boot sector, otherwise calculate the offset into it.
            Please note that this should belong to higher level of MW to do this check and it is here
            as a temporary work solution */

            partition_start =  0;

            status =  _fx_partition_offset_calculate(media_ptr -> fx_media_driver_buffer, 0,
                                                                &partition_start, &partition_size);

            /* Check partition read error.  */
            if (status)
            {
                /* Unsuccessful driver request.  */
                media_ptr -> fx_media_driver_status =  FX_IO_ERROR;
                break;
            }

            /* Now determine if there is a partition...   */
            if (partition_start)
            {

                if (check_sd_status(SD_INSTANCE) != BSP_ERROR_NONE)
                {
                    media_ptr->fx_media_driver_status =  FX_IO_ERROR;
                    break;
                }

                /* Yes, now lets read the actual boot record.  */
                status = sd_read_data(media_ptr, partition_start, media_ptr->fx_media_driver_sectors, unaligned_buffer);

                if (status != FX_SUCCESS)
          	    {
            	    media_ptr->fx_media_driver_status = status;
                    break;
                }
            }

            /* Successful driver request.  */
            media_ptr -> fx_media_driver_status =  FX_SUCCESS;
            break;
        }

        case FX_DRIVER_BOOT_WRITE:
        {
            unaligned_buffer = (UINT)(media_ptr->fx_media_driver_buffer) & 0x3;

            status = sd_write_data(media_ptr, 0, media_ptr->fx_media_driver_sectors, unaligned_buffer);

            media_ptr->fx_media_driver_status = status;

            break;
        }

        default:
        {
            media_ptr->fx_media_driver_status =  FX_IO_ERROR;
            break;
        }
    }
}

/**
  * @brief BSP Tx Transfer completed callbacks
  * @param Instance the SD instance
  * @retval None
  */
void BSP_SD_WriteCpltCallback(uint32_t Instance)
{
    tx_semaphore_put(&transfer_semaphore);
}

/**
  * @brief BSP Rx Transfer completed callbacks
  * @param Instance the sd instance
  * @retval None
  */
void BSP_SD_ReadCpltCallback(uint32_t Instance)
{
    tx_semaphore_put(&transfer_semaphore);
}

/**
  * @brief Read buffer using BSP SD API taking into account the scratch buffer
  * @param FX_MEDIA *media_ptr a pointer the main FileX structure
  * @param ULONG start_sector first sector to start reading from
  * @param UINT num_sectors number of sectors to be read
  * @param UINT use_scratch_buffer to enable scratch buffer usage or not.
  * @retval FX_SUCCESS on success FX_BUFFER_ERROR / FX_ACCESS_ERROR / FX_IO_ERROR otherwise
  */

static UINT sd_read_data(FX_MEDIA *media_ptr, ULONG start_sector, UINT num_sectors, UINT use_scratch_buffer)
{
    INT i = 0;
    UINT status;
    UCHAR *read_addr;

    /* Lock access to HW resources for this operation */
    if(tx_semaphore_get(&transfer_semaphore, DEFAULT_TIMEOUT) != TX_SUCCESS)
    {
        return FX_ACCESS_ERROR;
    }

    if (use_scratch_buffer)
    {
    	read_addr = media_ptr->fx_media_driver_buffer;

        for (i = 0; i < num_sectors; i++)
        {
            /* Start DMA read into the scratch buffer */
            status = BSP_SD_ReadBlocks_DMA(SD_INSTANCE, (uint32_t*)scratch, start_sector++, 1);

            if (status != BSP_ERROR_NONE)
            {
                /* DMA transfer failed, release semaphore and return immediately */
                tx_semaphore_put(&transfer_semaphore);
                return FX_IO_ERROR;
            }

            /* Block while trying to get the semaphore until DMA transfer is complete */
            if(tx_semaphore_get(&transfer_semaphore, DEFAULT_TIMEOUT) != TX_SUCCESS)
            {
                return FX_ACCESS_ERROR;
            }

#if (ENABLE_CACHE_MAINTENANCE == 1)
            SCB_InvalidateDCache_by_Addr((uint32_t*)scratch, DEFAULT_SECTOR_SIZE);
#endif

            _fx_utility_memory_copy(scratch, read_addr, DEFAULT_SECTOR_SIZE);
            read_addr += DEFAULT_SECTOR_SIZE;
        }

        /* Check if all sectors were read */
        if (i == num_sectors)
        {
            status = FX_SUCCESS;
        }
        else
        {
            status = FX_BUFFER_ERROR;
        }
    }
    else
    {

        status = BSP_SD_ReadBlocks_DMA(SD_INSTANCE, (uint32_t*)media_ptr->fx_media_driver_buffer, start_sector, num_sectors);

        if (status != BSP_ERROR_NONE)
        {
            /* DMA transfer failed, release semaphore and return immediately */
            tx_semaphore_put(&transfer_semaphore);
            return FX_IO_ERROR;
        }

        /* Block while trying to get the semaphore until DMA transfer is complete */
        if(tx_semaphore_get(&transfer_semaphore, DEFAULT_TIMEOUT) != TX_SUCCESS)
        {
            return FX_ACCESS_ERROR;
        }

#if (ENABLE_CACHE_MAINTENANCE == 1)
        SCB_InvalidateDCache_by_Addr((uint32_t*)media_ptr->fx_media_driver_buffer, num_sectors * DEFAULT_SECTOR_SIZE);
#endif

        status = FX_SUCCESS;
    }

    /* Operation finished, release semaphore */
    tx_semaphore_put(&transfer_semaphore);

    return status;
}

/**
  * @brief write buffer using BSP SD API taking into account the scratch buffer
  * @param FX_MEDIA *media_ptr a pointer the main FileX structure
  * @param ULONG start_sector first sector to start writing from
  * @param UINT num_sectors number of sectors to be written
  * @param UINT use_scratch_buffer to enable scratch buffer usage or not.
  * @retval FX_SUCCESS on success FX_BUFFER_ERROR / FX_ACCESS_ERROR / FX_IO_ERROR otherwise
  */

static UINT sd_write_data(FX_MEDIA *media_ptr, ULONG start_sector, UINT num_sectors, UINT use_scratch_buffer)
{
    INT i = 0;
    UINT status;
    UCHAR *write_addr;

    /* Lock access to HW resources for this operation */
    if(tx_semaphore_get(&transfer_semaphore, DEFAULT_TIMEOUT) != TX_SUCCESS)
    {
        return FX_ACCESS_ERROR;
    }

    if (use_scratch_buffer)
    {
    	write_addr = media_ptr->fx_media_driver_buffer;

        for (i = 0; i < num_sectors; i++)
        {
            _fx_utility_memory_copy(write_addr, scratch, DEFAULT_SECTOR_SIZE);
            write_addr += DEFAULT_SECTOR_SIZE;

#if (ENABLE_CACHE_MAINTENANCE == 1)
            /* Clean the DCache to make the SD DMA see the actual content of the scratch buffer */
            SCB_CleanDCache_by_Addr((uint32_t*)scratch, DEFAULT_SECTOR_SIZE);
#endif

            status = BSP_SD_WriteBlocks_DMA(SD_INSTANCE, (uint32_t*)scratch, (uint32_t)start_sector++, 1);

            if (status != BSP_ERROR_NONE)
            {
                /* DMA transfer failed, release semaphore and return immediately */
                tx_semaphore_put(&transfer_semaphore);
                return FX_IO_ERROR;
            }

            /* Block while trying to get the semaphore until DMA transfer is complete */
            if(tx_semaphore_get(&transfer_semaphore, DEFAULT_TIMEOUT) != TX_SUCCESS)
            {
                return FX_ACCESS_ERROR;
            }
        }

        if (i == num_sectors)
        {
            status = FX_SUCCESS;
        }
        else
        {
            status = FX_BUFFER_ERROR;
        }
    }
    else
    {
#if (ENABLE_CACHE_MAINTENANCE == 1)
        SCB_CleanDCache_by_Addr((uint32_t*)media_ptr->fx_media_driver_buffer, num_sectors * DEFAULT_SECTOR_SIZE);
#endif
        status = BSP_SD_WriteBlocks_DMA(SD_INSTANCE, (uint32_t*)media_ptr->fx_media_driver_buffer, start_sector, num_sectors);

        if (status != BSP_ERROR_NONE)
        {
            /* DMA transfer failed, release semaphore and return immediately */
            tx_semaphore_put(&transfer_semaphore);
            return FX_IO_ERROR;
        }

        /* Block while trying to get the semaphore until DMA transfer is complete */
        if(tx_semaphore_get(&transfer_semaphore, DEFAULT_TIMEOUT) != TX_SUCCESS)
        {
            return FX_ACCESS_ERROR;
        }

        status = FX_SUCCESS;
    }

    /* Operation finished, release semaphore */
    tx_semaphore_put(&transfer_semaphore);

    return status;
}
