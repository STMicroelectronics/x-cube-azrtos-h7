/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    ux_device_printer.c
  * @author  MCD Application Team
  * @brief   USBX Device Printer applicative source file
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
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
#include "ux_device_printer.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "app_usbx_device.h"
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
/* Define Printer device ID */
UCHAR USBD_PRINTER_DeviceID [] = {
  /* USER CODE BEGIN USBD_PRINTER_DeviceID */

  "  "                             /* Length */
  "MFG:Generic;"                   /* Manufacturer */
  "MDL:Generic_/_Text_Only;"       /* Model */
  "CMD:1284.4;"                    /* Command Set */
  "CLS:PRINTER;"                   /* Class */
  "DES:Generic text only printer;" /* Description */

  /* USER CODE END USBD_PRINTER_DeviceID */
};

/* USER CODE BEGIN PV */
UX_DEVICE_CLASS_PRINTER *printer;
UCHAR device_printer_port_status_value;
static UCHAR device_printer_buffer[512];
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  USBD_PRINTER_GetDeviceID
  *         Get the printer device ID.
  * @param  none
  * @retval USBD_PRINTER_DeviceID : Pointer to printer device ID.
  */
UCHAR *USBD_PRINTER_GetDeviceID(VOID)
{
  /* USER CODE BEGIN USBD_PRINTER_GetDeviceID */

  ux_utility_short_put_big_endian(USBD_PRINTER_DeviceID, sizeof(USBD_PRINTER_DeviceID));

  /* USER CODE END USBD_PRINTER_GetDeviceID */

  return USBD_PRINTER_DeviceID;
}

/**
  * @brief  USBD_PRINTER_Activate
  *         This function is called while inserting a printer device.
  * @param  printer_instance: Pointer to the printer class instance.
  * @retval none
  */
VOID USBD_PRINTER_Activate(VOID *printer_instance)
{
  /* USER CODE BEGIN USBD_PRINTER_Activate */
  if (printer == UX_NULL)
  {
    /* Save the Printer instance */
    printer = (UX_DEVICE_CLASS_PRINTER *)printer_instance;

    /* Set the printer port status */
    ux_device_class_printer_ioctl(printer,
                                  UX_DEVICE_CLASS_PRINTER_IOCTL_PORT_STATUS_SET,
                                  &device_printer_port_status_value);
  }
  /* USER CODE END USBD_PRINTER_Activate */

  return;
}

/**
  * @brief  USBD_PRINTER_Deactivate
  *         This function is called while extracting a printer device.
  * @param  printer_instance: Pointer to the printer class instance.
  * @retval none
  */
VOID USBD_PRINTER_Deactivate(VOID *printer_instance)
{
  /* USER CODE BEGIN USBD_PRINTER_Deactivate */

  /* Reset the Printer instance */
  if ((VOID *)printer == printer_instance)
  {
    printer = UX_NULL;
  }
  /* USER CODE END USBD_PRINTER_Deactivate */

  return;
}

/**
  * @brief  USBD_PRINTER_SoftReset
  *         This function is called while resetting a printer device.
  * @param  printer_instance: Pointer to the printer class instance.
  * @retval none
  */
VOID USBD_PRINTER_SoftReset(VOID *printer_instance)
{
  /* USER CODE BEGIN USBD_PRINTER_SoftReset */
  UX_PARAMETER_NOT_USED(printer_instance);
  /* USER CODE END USBD_PRINTER_SoftReset */

  return;
}

/* USER CODE BEGIN 1 */
/**
  * @brief  Function implementing USBX_Printer_Read_TASK.
  * @param  none
  * @retval none
  */
VOID USBX_Printer_Read_TASK(VOID)
{
  UX_SLAVE_DEVICE *device;
  ULONG actual_length;
  ULONG i;

  /* Get the pointer to the device */
  device = &_ux_system_slave->ux_system_slave_device;

  /* Check if the device state already configured */
  if ((device->ux_slave_device_state == UX_DEVICE_CONFIGURED) && (printer != UX_NULL))
  {
    /* Reads from the printer device */
    if (ux_device_class_printer_read_run(printer,
                                         device_printer_buffer,
                                         512, &actual_length)== UX_STATE_NEXT)
    {

    /* Displaying received data */
    printf("Printing %ld bytes:", actual_length);

    for (i = 0; i < actual_length; i ++)
    {
      if ((i & 0xF) == 0)
      {
        printf("\n[%03lx]", i);
      }
      printf(" %02X", device_printer_buffer[i]);
    }
    printf("\n");
    }
  }
}

/**
  * @brief  Function implementing USBX_Printer_Write_TASK.
  * @param  none
  * @retval none
  */
VOID USBX_Printer_Write_TASK(VOID)
{
  UCHAR port_status = device_printer_port_status_value;
  UX_SLAVE_DEVICE *device;

  /* Get the pointer to the device */
  device = &_ux_system_slave->ux_system_slave_device;

  /* Check if the device state already configured and status change check */
  if ((device->ux_slave_device_state == UX_DEVICE_CONFIGURED) && (printer != UX_NULL) &&
      (port_status != device_printer_port_status_value))
  {
    /* Set the printer port status */
    ux_device_class_printer_ioctl(printer,
                                  UX_DEVICE_CLASS_PRINTER_IOCTL_PORT_STATUS_SET,
                                  &device_printer_port_status_value);

    /* Update port status value */
    port_status = device_printer_port_status_value;
  }
}
/* USER CODE END 1 */
