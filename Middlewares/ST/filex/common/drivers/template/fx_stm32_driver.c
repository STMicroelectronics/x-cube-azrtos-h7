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

#include "fx_stm32_driver.h"

VOID  fx_stm32_driver(FX_MEDIA *media_ptr)
{
  switch (media_ptr -> fx_media_driver_request)
  {
    case FX_DRIVER_INIT:
    {
      /*
       * Implement the driver init
       */
      media_ptr -> fx_media_driver_status =  FX_SUCCESS;
      break;
    }

    case FX_DRIVER_UNINIT:
    {
      /*
       *Implment the driver deinit
       */
      media_ptr -> fx_media_driver_status =  FX_SUCCESS;
      break;
    }

    case FX_DRIVER_BOOT_READ:
    {
      source_buffer =  (UCHAR *)FX_SRAM_DISK_BASE_ADDRESS;
      break;
    }

    case FX_DRIVER_READ:
    {
      /*
       * implement how to read the data from the media
       */
      media_ptr -> fx_media_driver_status =  FX_SUCCESS;
      break;
    }

    case FX_DRIVER_WRITE:
    {

      /*
       * implement how to write data into the media
       */
      media_ptr -> fx_media_driver_status =  FX_SUCCESS;
      break;
    }

    case FX_DRIVER_FLUSH:
    {

      /*
       * implment the flush request
       */
      media_ptr -> fx_media_driver_status =  FX_SUCCESS;
      break;
    }

    case FX_DRIVER_ABORT:
    {

      /*
       * implement the abort request
       */
      media_ptr -> fx_media_driver_status =  FX_SUCCESS;
      break;
    }
    
    case FX_DRIVER_BOOT_WRITE:
    {
      /*
       * Write the media boot sector
       */
      media_ptr -> fx_media_driver_status =  FX_SUCCESS;
      break;
    }

    default:
    {
        /*
         * Invalid driver request.
         */
        media_ptr -> fx_media_driver_status =  FX_IO_ERROR;
        break;
    }
  }
}

