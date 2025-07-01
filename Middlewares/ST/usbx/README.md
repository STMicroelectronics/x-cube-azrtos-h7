# Middleware USBX Component

![latest tag](https://img.shields.io/github/v/tag/STMicroelectronics/stm32-mw-usbx.svg?color=green)

## Overview
The Middleware USBX component is a STM32 tailored fork of the [AzureRTOS USBX](https://github.com/eclipse-threadx/usbx).

USBX is a complete USB Host/Device stack designed for embedded systems.

It provides a complete set of USB device and host classes like CDC-ACM, Mass storage, HID.

The goal of this component is to integrate STM32 USB Hardware IP through the STM32 HAL/USB drivers.

## Low level drivers

The folder `common` contains the following usbx controllers:

* **usbx_stm32_device_controllers**

    A STM32 reference implementation for USBX device based on **STM32 HAL/DCD driver.**

* **usbx_stm32_host_controllers**

    A STM32 reference implementation for USBX host based on **STM32 HAL/HCD driver.**

## Documentation

A detailed documentation can be found under the [STM32 wiki page](https://wiki.st.com/stm32mcu/index.php?title=Introduction_to_USBX&sfr=stm32mcu)

## Compatibility information

Please refer to the release note in the repository of the STM32Cube **firmware** you are using to know which version of this middleware library to use with other components' versions (**e.g.**, other middleware libraries, drivers). It is **crucial** that you use a consistent set of versions.

## Troubleshooting
Please refer to the [CONTRIBUTING.md](CONTRIBUTING.md) guide.


