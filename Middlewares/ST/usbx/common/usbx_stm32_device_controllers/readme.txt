/**
  ******************************************************************************
  * @file    readme.txt
  * @author  MCD Application Team
  * @brief   This file lists the main changes done by STMicroelectronics on
  *          USBX device controller driver

  ******************************************************************************
  */

### V1.2.0 / 01-April-2022 ###
=================================
Main changes
-------------
- Align USBX STM32 Device controllers against Azure RTOS USBX v6.1.10:
    - Add transfer abort support
    - Add USBX standalone mode support
    - Add support of Iso IN/OUT incomplete for OTG IP
- Minor changes in USBX device bidirectional endpoint support
- Add support of transfer request timeout instead of waiting forever

Dependencies:
-------------
- Azure RTOS USBX V6.1.10
- STM32Cube PCD HAL drivers


### V1.1.0 / 05-November-2021 ###
=================================
Main changes
-------------
- Add USBX device bidirectional endpoint support

Dependencies:
-------------
- Azure RTOS USBX V6.1.8
- STM32Cube PCD HAL drivers

### V1.0.1 / 21-June-2021 ###
=================================
Main changes
-------------
- Stop device when disconnected from the host
- Fix compile warnings

Dependencies:
-------------
- Azure RTOS USBX V6.1.7
- STM32Cube PCD HAL drivers

### V1.0.0 / 25-February-2021 ###
=================================
Main changes
-------------
- First official release of Azure RTOS USBX device controller driver for STM32 MCU series

Dependencies:
-------------
- Azure RTOS USBX V6.1.3
- STM32Cube PCD HAL drivers
