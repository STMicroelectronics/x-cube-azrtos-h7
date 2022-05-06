/**
  ******************************************************************************
  * @file    readme.txt
  * @author  MCD Application Team
  * @brief   This file lists the main changes done by STMicroelectronics on
  *          USBX host controller driver

  ******************************************************************************
  */

### V1.0.3 / 24-December-2021 ###
=================================
Main changes
-------------
- Free HCD memory resource during HCD DeInit

Dependencies:
-------------
- Azure RTOS USBX V6.1.7 or higher

### V1.0.2 / 22-November-2021 ###
=================================
Main changes
-------------
- Avoid halting channels during URB notification
- Avoid compilation issue with USB_DRD IP (DMA not supported)
- Fix endpoint type for isochronous transfer

Dependencies:
-------------
- Azure RTOS USBX V6.1.7 or higher

### V1.0.1 / 21-June-2021 ###
=============================
Main changes
-------------
- Remove trailing spaces.

Dependencies:
-------------
- Azure RTOS USBX V6.1.7

### V1.0.0 / 25-February-2021 ###
=================================
Main changes
-------------
- First official release of Azure RTOS USBX Host controller driver for STM32 MCU series

Dependencies:
-------------
- Azure RTOS LevelX V6.1.3
- STM32Cube HCD HAL drivers
