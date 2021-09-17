/**
  ******************************************************************************
  * @file    readme.txt
  * @author  MCD Application Team
  * @brief   This file lists the main changes done by STMicroelectronics on
  *          LevelX low level drivers for STM32 devices.
  ******************************************************************************
  */

### V2.1.1 / 13-September-2021 ###
=================================
Main changes
-------------
- Inherit correct define from the 'lx_stm32_ospi_driver.h' file
- Align the 'LX_STM32_OSPI_POST_INIT' macro call to the new prototype
- Align the OctoSPI templates against the new architecture

### V2.1.0 / 27-August-2021 ###
=================================
Main changes
-------------
- Move the LevelX 'sector buffer' from the driver to application level
- Add checks on the memory status before any read/write operation
- Align the QuadSPI templates against the new architecture

Dependencies:
-------------
- Azure RTOS LevelX V6.1.7
- STM32Cube OCTOSPI and QuadSPI HAL drivers


### V2.0.0 / 21-June-2021 ###
=================================
Main changes
-------------
- Decouple QSPI and OSPI drivers from BSP API
- Add "lx_stm32xx_driver_glue.c" and "lx_stm32xx_driver.h" generic templates

Dependencies:
-------------
- Azure RTOS LevelX V6.1.7

### V1.0.0 / 25-February-2021 ###
=================================
Main changes
-------------
- First official release of Azure RTOS LevelX low level drivers for STM32 MCU series

Dependencies:
-------------
- Azure RTOS LevelX V6.1.3
- STM32Cube OCTOSPI and QUADSPI BSP drivers
