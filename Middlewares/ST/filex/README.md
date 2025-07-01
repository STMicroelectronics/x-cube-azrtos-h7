# Middleware FileX Component

![latest tag](https://img.shields.io/github/v/tag/STMicroelectronics/stm32-mw-filex.svg?color=green)

## Overview
The Middleware FileX component is a STM32 tailored fork of the [AzureRTOS FileX](https://github.com/azure-rtos/filex).

FileX offers the common filesystem features, like formatting media, creating directories and files, accessing files in read and write modes.

FileX supports also `extFat` file system. available free of charge for the [licensed STM32 series](./LICENSED-HARDWARE.txt).

The main goal of this component is to integrate different media storage devices through the STM32 HAL and BSP component drivers.


## Low level drivers

The folder `common/drivers` contains a set of generic ready to use low level drivers and template files. The ready to use drivers are:

* **fx_stm32_sram_driver.c**

    A generic driver to simulate a FAT file system under internal RAM.

* **fx_stm32_sd_driver.c/fx_stm32_mmc_driver.c**

    Two generic drivers that communicate respectively with a µSD or an eMMC cards.

    They can be configured to use DMA and Polling HAL/SD and HAL/MMC APIs and to work in RTOS or baremetal modes.

* **fx_stm32_levelx_nor_driver.c**

    A generic driver needed to interface FileX with any LevelX low level NOR flash driver.

    It is configurable to easily integrate user custom drivers.

* **fx_stm32_levelx_nand_driver.c**

    A generic driver needed to interface FileX with any LevelX low level NAND flash driver.

    It is configurable to easily integrate any user custom drivers.

## Documentation

A detailed documentation can be found under the [STM32 wiki page](https://wiki.st.com/stm32mcu/index.php?title=Introduction_to_FILEX&sfr=stm32mcu)

## Compatibility information

Please refer to the release note in the repository of the STM32Cube **firmware** you are using to know which version of this middleware library to use with other components' versions (**e.g.**, other middleware libraries, drivers).

It is **crucial** that you use a consistent set of versions.

## Troubleshooting
Please refer to the [CONTRIBUTING.md](CONTRIBUTING.md) guide.


