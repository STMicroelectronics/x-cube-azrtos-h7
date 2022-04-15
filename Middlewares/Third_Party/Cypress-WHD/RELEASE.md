
# Wi-Fi Host Driver (WHD)  v1.70.0
Please refer to the [README File](./README.md) and the [WHD API Reference Manual](https://github.com/cypresssemiconductorco/wifi-host-driver.git) for a complete description of the Wi-Fi Host Driver.


# Wi-Fi Host Driver (WHD)  v1.70.0
Please refer to the [README File](./README.md) and the [WHD API Reference Manual](https://cypresssemiconductorco.github.io/wifi-host-driver/API/index.html) for a complete description of the Wi-Fi Host Driver.

## Features
* Supports Wi-Fi Station (STA) and AP mode of operation
* Supports concurrent operation of STA and AP interface
* Supports multiple security methods such as WPA2, WPA3, and open
* Provides functions for Advanced Power Management
* Supports low-power offloads, including ARP, packet filters, TCP Keepalive offload, DHCP lease time renewal offload, and Beacon trim
* Includes WFA pre-certification support for 802.11n and WPA3

## Changes since v1.40.0
### New Features
* Mfgtest support
* Power saving enhancements

### Defect Fixes
* APSTA fix
* Roaming failure fix

### Known Issues
None

### Firmware Changes
#### CYW4343W
* --- 7.45.98.92 ---

#### CYW43012
* --- 13.10.271.218 ---
* Fixed softap PHYTX error due to probe response length mismatch.
* Fixed reinit path for PM2 mode.
* Fixed tempsense as a work around.
* Fixed PHY deaf for ATE.
* Fixed reassociation problem when beacon loss and deauth roam is triggered in sequence.
* Fixed security trap with softap.
* Fixed PHYTX for BTCOEX stability.
* Zero stalls and very low throughput fixed due to minimum schedule configuration.
* Fixed PHY CRS stuck as a work around.
* Enable additional IOVAR for PHY i.e. PHY_EXT_IOV.
* Fixed coverity, build issues.
* Enhanced WPA3 PWE speed optimization.
* --- 13.10.271.203 ---

Note: [r] is regulatory-related

## Supported Software and Tools
This version of the WHD was validated for compatibility with the following software and tools:

| Software and Tools                                      | Version      |
| :---                                                    | :----        |
| GCC Compiler                                            | 7.2.1        |
| IAR Compiler                                            | 8.32         |
| Arm Compiler 6                                          | 6.11         |
| Mbed OS                                                 | 5.13.1       |
| ThreadX/NetX-Duo                                        | 5.8          |
| FreeRTOS/LWIP                                           | 2.0.3        |


## More Information
* [Wi-Fi Host Driver README File](./README.md)
* [Wi-Fi Host Driver API Reference Manual and Porting Guide](https://cypresssemiconductorco.github.io/wifi-host-driver/API/index.html)
* [Cypress Semiconductor](http://www.cypress.com)

---
© Cypress Semiconductor Corporation, 2019.
