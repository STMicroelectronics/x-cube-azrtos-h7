# Middleware NetXDuo Component

![latest tag](https://img.shields.io/github/v/tag/STMicroelectronics/stm32-mw-netxduo.svg?color=green)

## Overview
The Middleware NetXDuo component is a STM32 tailored fork of the [AzureRTOS NetXDuo](https://github.com/eclipse-threadx/netxduo).

NetXDuo is a complete TCP/IP stack supporting both IPV4 and IPv6 protocols.

It comes with supports for different Applicative protocols such as HTTP, FTP, MQTT.
It also supports TLS 1.2 for secure communication.

The goal of this component is to support different STM32 networking low level drivers mainly **Ethernet** and **WiFi** via STM32 HAL and BSP component drivers.

## Low level drivers

The folder `common/drivers` contains a set of generic ready to use low level drivers and template files. The ready to use drivers are:

* **nx_stm32_eth_driver.c**

    An Ethernet driver based on the STM32 HAL/ETH API.

* **nx_stm32_phy_driver.c**

    This a driver needed by the **nx_stm32_eth_driver.c** to communicate with RJ45 Phy interface.

    Two main versions of this driver are provided to support **LAN8742** and **RTL8211** phys.

* **es_wifi/nx_driver_ism43362.c**

    A WiFi driver for the es_wifi module.

* **mxchip/nx_driver_emw3080.c**

    A WiFi driver for the MXCHIP emw3080 wifi module.
## Documentation

A detailed documentation can be found under the [STM32 wiki page](https://wiki.st.com/stm32mcu/index.php?title=Introduction_to_NETXDUO&sfr=stm32mcu)

## Compatibility information

Please refer to the release note in the repository of the STM32Cube **firmware** you are using to know which version of this middleware library to use with other components' versions (**e.g.**, other middleware libraries, drivers). It is **crucial** that you use a consistent set of versions.

## Troubleshooting
Please refer to the [CONTRIBUTING.md](CONTRIBUTING.md) guide.


