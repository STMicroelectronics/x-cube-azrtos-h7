/**
  ******************************************************************************
  * @file    readme.txt
  * @author  MCD Application Team
  * @brief   This file lists the main changes done by STMicroelectronics on
  *          NetXDuo low level drivers for STM32 devices.
  ******************************************************************************
  */

### V3.0.0 / 01-April-2022 ###
=================================
Main changes
-------------
- Add Cypress WiFi module driver.
- Align the NetXDuo STM32 Ethernet driver against new HAL Ethernet driver APIs.
- Fix NetXDuo STM32 Ethernet driver to correctly check the phy link down/up status.

Dependencies:
-------------
- Azure RTOS NetXDuo V6.1.10 or higher
- STM32CubeH7 Ethernet HAL driver V1.11.0
- STM32CubeF4 Ethernet HAL driver V1.8.0
- STM32CubeF7 Ethernet HAL driver V1.3.0
- Cypress_WHD middlware 1.70.0
- MX_WIFI component driver 2.1.11

### V2.1.2 / 28-January-2022 ###
=================================
Main changes
-------------
- Use correct license header for template files
  + mx_wifi_azure_rtos_conf.h

### V2.1.1 / 05-November-2021 ###
=================================
Main changes
-------------
- Invalidate the cache when filling the ETH/DMA descriptors in the hardware initialize function

Dependencies:
-------------
- Azure RTOS NetXDuo V6.1.7 or higher
- STM32CubeH7 Ethernet HAL driver V1.10.0
- STM32CubeF4 Ethernet HAL driver delivered within X-CUBE-AZRTOS-F4
- STM32CubeF7 Ethernet HAL driver delivered within X-CUBE-AZRTOS-F7
- MX_WIFI component driver  2.1.11


### V2.1.0 / 27-August-2021 ###
=================================
Main changes
-------------
- Update ethernet driver to support both STM32F4 and STM32H7 families

Dependencies:
-------------
- Azure RTOS NetXDuo V6.1.7
- STM32CubeH7 Ethernet HAL driver V1.10.0
- STM32CubeF4 Ethernet HAL driver delivered within X-CUBE-AZRTOS-F4
- MX_WIFI component driver  2.1.11


### V2.0.0 / 21-June-2021 ###
=================================
Main changes
-------------
- restructure  the folder tree to split ethernet and wifi drivers
- Add MXCHIP wifi module driver

Dependencies:
-------------
- Azure RTOS NetXDuo V6.1.7
- STM32CubeH7 Ethernet HAL driver V1.10.0
- MX_WIFI component driver  2.1.11

### V1.0.0 / 25-February-2021 ###
=================================
Main changes
-------------
- First official release of Azure RTOS NetXDuo low level drivers for STM32 MCU series

Dependencies:
-------------
- Azure RTOS NetXDuo V6.1.3
- STM32CubeH7 Ethernet HAL driver V1.10.0
