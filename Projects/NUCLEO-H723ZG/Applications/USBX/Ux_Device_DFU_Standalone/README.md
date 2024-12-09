
## <b>Ux_Device_DFU_Standalone application description</b>

This application provides an example of Azure RTOS USBX stack usage on NUCLEO-H723ZG board,
it shows how to develop USB Device Firmware Upgrade "DFU" based bare metal application.

The application is designed to emulate a USB DFU device, the code provides all required device descriptors framework
and associated class descriptor report to build a compliant USB DFU device.

The application's main calls the MX_USBX_Device_Init() function in order to Initialize USBX and USBX_Device_Process in the while loop.
As stated earlier, the present application runs in standalone mode without RTOS, for this reason, the standalone variant of USBX is enabled by adding the following flag in ux_user.h:

  - #define UX_STANDALONE

The DFU transactions are based on Endpoint 0 (control endpoint) transfer. All requests and status
control are sent/received through this endpoint.

The internal flash memory is split as follows:
 - DFU area located in [0x08000000 : USBD_DFU_APP_DEFAULT_ADD-1]: Only read access
 - Application area located in [USBD_DFU_APP_DEFAULT_ADD : Device's end address]: Read, Write, and Erase
   access

#### <b>Note 1</b>

In this application, two operating modes are available:

 1. DFU operating mode:
    This mode is entered after an MCU reset in case:
     - The DFU mode is forced by the user: the user presses the User button.
     - No valid code found in the application area: a code is considered valid if the MSB of the initial
       Main Stack Pointer (MSP) value located in the first address of the application area is equal to
       0x20000.

 2. Run-time application mode:
    This is the normal run-time activities. A binary which toggles LED1 and LED2 on the NUCLEO-H723ZG board "NUCLEO-H723ZG_LED_Toggle_@0x08020000" is provided in Binary directory.

#### <b>Note 2</b>

 1. Download the appropriate "STM32CubeProgrammer" from ST web site and install it.
 2. verify from Windows Device Manager that DFU driver is selected with the "STM32CubeProgrammer" driver.
 3.After each device reset (unplug the NUCLEO-H723ZG board from PC), plug the NUCLEO-H723ZG board with Key User button pressed to enter the DFU mode.

#### <b>Expected success behavior</b>

When plugged to PC host, the STM32H723xx must be properly enumerated as an USB DFU device.
During the enumeration phase, device provides host with the requested descriptors (device, configuration, string).
Those descriptors are used by host driver to identify the device capabilities.
Once the STM32H723xx USB device successfully completed the enumeration phase, open "STM32CubeProgrammer" you can now download and upload to/from media .

#### <b>Error behaviors</b>

Host PC shows that USB device does not appear as DFU device.

#### <b>Assumptions if any</b>

User is familiar with USB 2.0 "Universal Serial BUS" specification and DFU class specification.

#### <b>Known limitations</b>

None

### <b>Notes</b>

 1. Some code parts can be executed in the ITCM-RAM (64 KB up to 256kB) which decreases critical task execution time, compared to code execution from Flash memory. This feature can be activated using '#pragma location = ".itcmram"' to be placed above function declaration, or using the toolchain GUI (file options) to execute a whole source file in the ITCM-RAM.
 2.  If the application is using the DTCM/ITCM memories (@0x20000000/ 0x0000000: not cacheable and only accessible by the Cortex M7 and the MDMA), no need for cache maintenance when the Cortex M7 and the MDMA access these RAMs. If the application needs to use DMA (or other masters) based access or requires more RAM, then the user has to:
      - Use a non TCM SRAM. (example : D1 AXI-SRAM @ 0x20000000).
      - Add a cache maintenance mechanism to ensure the cache coherence between CPU and other masters (DMAs,DMA2D,LTDC,MDMA).
      - The addresses and the size of cacheable buffers (shared between CPU and other masters) must be properly defined to be aligned to L1-CACHE line size (32 bytes).
 3.  It is recommended to enable the cache and maintain its coherence:
      - Depending on the use case it is also possible to configure the cache attributes using the MPU.
      - Please refer to the **AN4838** "Managing memory protection unit (MPU) in STM32 MCUs".
      - Please refer to the **AN4839** "Level 1 cache on STM32F7 and STM32H7 Series"

#### <b>USBX usage hints</b>

- The DTCM (0x20000000) memory region should not be used by application in case USB DMA is enabled
- Should make sure to configure the USB pool memory region with attribute "Non-Cacheable" to ensure coherency between CPU and USB DMA

### <b>Keywords</b>

Standalone, USBXDevice, mode Full Speed, USB_OTG, DFU, Firmware upgrade

### <b>Hardware and Software environment</b>

  - This example runs on STM32H723xx devices.
  - This example has been tested with STMicroelectronics NUCLEO-H723ZG boards revision MB1364-H723ZG-E01
    and can be easily tailored to any other supported device and development board.
  - NUCLEO-H723ZG board Set-up
    - Download the appropriate "STM32CubeProgrammer" from ST web site and install it.
    - Connect the NUCLEO-H723ZG board to the PC.
    - Connect the NUCLEO-H723ZG board to the PC through micro A-Male to standard A Male cable connected to the connector.
    - CN13 : to use USB High Speed OTG IP in mode Full Speed.

### <b>How to use it ?</b>

In order to make the program work, you must do the following :

 - Open your preferred toolchain
 - Rebuild all files and load your image into target memory
 - To run the application, proceed as follows:

    - Download the appropriate "STM32CubeProgrammer" from ST web site and install it.
    - Once you've updated the STM32 DFU device driver you must verify from Windows Device Manager
      that DFU driver is selected with the "STM32CubeProgrammer" driver.
    - Open "STM32CubeProgrammer", select the usb interface and connect to DFU device.
    - Download the "NUCLEO-H723ZG_LED_Toggle_@0x08020000.bin" provided in Binary directory and set the start address.
    - To run the downloaded application, reset the board.