/**
  ******************************************************************************
  * @file    readme.txt
  * @author  MCD Application Team
  * @brief   This file lists the main changes done by STMicroelectronics on
  *          LevelX low level drivers for STM32 devices.
  ******************************************************************************
  */

## V3.2.3 / 03-10-2025 ###
=================================
Main changes
-------------
-  Remove unused flash buffer "nand_flash_buffer".
 + template/lx_stm32_nand_driver.c

## V3.2.2 / 22-08-2025 ###
=================================
Main changes
-------------
-  Make "NAND Simulator" and "NAND Custom" interfaces cohabitate.
 + template/lx_stm32_nand_driver.c
 + template/lx_stm32_nand_driver.h

## V3.2.1 / 02-06-2025 ###
=================================
Main changes
-------------
-  Set the size of the 'lx_nor_flash_sector_buffer' to LX_NOR_SECTOR_SIZE instead of NOR flash physical block size.
 + lx_stm32_xspi_driver.c
 + lx_stm32_ospi_driver.c
 + lx_stm32_qspi_driver.c

## V3.2.0 / 09-09-2024 ###
=================================
Main changes
-------------
-  Add xspi NOR flash driver and its respective template files
 + lx_stm32_xspi_driver.c
 + template/lx_stm32_xspi_driver.h
 + template/lx_stm32_xspi_driver_glue.c

## V3.1.0 / 17-May-2024 ###
=================================
Main changes
-------------
- Make the NOR flash base address start from a custom offset
 + lx_stm32_ospi_driver.c
 + lx_stm32_qspi_driver.c
 + template/lx_stm32_ospi_driver.h
 + template/lx_stm32_qspi_driver.h

- Fix issues when calling fx_media_format() followed by fx_media_open()
  + lx_stm32_ospi_driver.c
  + lx_stm32_qspi_driver.c

- Add missing PHYSICAL_PAGES_PER_BLOCK define in NAND driver header
  + template/lx_stm32_nand_driver.h

Dependencies:
-------------
- Azure RTOS LevelX V6.2.1 or higher

### V3.0.0 / 17-November-2023 ###
=================================
Main changes
-------------
- Add new low level APIs for LevelX NAND template driver required by LevelX 6.2.1
   + template/lx_stm32_nand_driver.c

- Add LX_DRIVER_ERASES_FLASH_AFTER_INIT config flag to enabling NAND flash erasing by the driver
  + template/lx_stm32_nand_driver.h

- Add new config flags for LevelX NAND template driver to define NAND flash characteristics
  + template/lx_stm32_nand_driver.h

Dependencies:
-------------
- Azure RTOS LevelX V6.2.1

### V2.1.4 / 06-October-2023 ###
=================================
Main changes
-------------
- Align LevelX NAND simulator driver against new Azure RTOS LevelX V6.2.1 version.

Dependencies:
-------------
- Azure RTOS FileX V6.2.1
- Azure RTOS LevelX V6.2.1
- STM32Cube OCTOSPI and QuadSPI HAL drivers

### V2.1.3 / 28-January-2022 ###
=================================
- Remove checks on the LX_STM32_QSPI_INIT and LX_STM32_OSPI_INIT to make the low_level_init always called.
  + lx_stm32_ospi_driver.c
  + lx_stm32_qspi_driver.c

### V2.1.2 / 05-November-2021 ###
=================================
-  Fix check_status() function to consider the timeout when checking the IP status
- Add lowlevel deinit function for QuadSPI and OctoSPI drivers to let the application
  deinitialize the IP.

Dependencies:
-------------
- Azure RTOS LevelX V6.1.7 or higher
- STM32Cube OCTOSPI and QuadSPI HAL drivers

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
