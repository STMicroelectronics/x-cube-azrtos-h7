# Middleware LevelX Component

![latest tag](https://img.shields.io/github/v/tag/STMicroelectronics/stm32-mw-levelx.svg?color=green)

## Overview
The Middleware LevelX component is a STM32 tailored fork of the [AzureRTOS LevelX](https://github.com/eclipse-threadx/levelx).

LevelX is a Flash translation layer, FTL, that offers a set of APIs to access NOR and NAND flash memories. it comes also with the **wear leveling** mechanism to avoid block deterioration due to erase operations.

LevelX is designed for fault tolerance, thus it is able to recover a consistent flash state in case of errors, power loss for instance, during write operation.

The goal of this component is to support different NOR and NAND flash memories through the STM32 HAL and BSP component drivers.

## Low level drivers

The folder `common/drivers` contains a set of generic ready to use low level drivers and template files. The ready to use drivers are:

* **lx_stm32_nor_simulator_driver.c/lx_stm32_nand_simulator_driver.c**

    Two generic drivers to simulate respectively a NOR and a NAND flash memory using an internal RAM region.

* **lx_stm32_qspi_driver.c/lx_stm32_ospi_driver.c**

    Two generic drivers to aceess NOR flash memories through STM32 **HAL/QuadSPI and HAL/OctoSPI** APIs respectively.

    Those drivers should be combined with FileX to be able to create a file system on NOR and NAND flash memories.

## Documentation

A detailed documentation can be found under the [STM32 wiki page](https://wiki.st.com/stm32mcu/index.php?title=Introduction_to_LEVELX&sfr=stm32mcu)

## Compatibility information

Please refer to the release note in the repository of the STM32Cube **firmware** you are using to know which version of this middleware library to use with other components' versions (**e.g.**, other middleware libraries, drivers). It is **crucial** that you use a consistent set of versions.

## Troubleshooting
Please refer to the [CONTRIBUTING.md](CONTRIBUTING.md) guide.


