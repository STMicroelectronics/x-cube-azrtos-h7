
# <b>Ux_Device_HID_Standalone application description</b>

This application provides an example of Azure RTOS USBX stack usage on NUCLEO-H723ZG board,
it shows how to develop USB Device Human Interface "HID" mouse based bare metal application.

The application is designed to emulate a USB HID mouse device, the code provides all required device descriptors framework
and associated class descriptor report to build a compliant USB HID mouse device.

The application's main calls the MX_USBX_Device_Init() function in order to Initialize USBX and USBX_Device_Process in the while loop.
As stated earlier, the present application runs in standalone mode without ThreadX, for this reason, the standalone variant of USBX is enabled by adding the following flag in ux_user.h:

  - #define UX_STANDALONE

To customize the HID application by sending the mouse position step by step every 10ms:
  - For each 10ms, the application calls the GetPointerData() API to update the mouse position (x, y) and send the report buffer through the ux_device_class_hid_event_set() API.

#### <b>Expected success behavior</b>

When plugged to PC host, the NUCLEO-H723ZG must be properly enumerated as a USB HID mouse device.
During the enumeration phase, device provides host with the requested descriptors (device, configuration, string).
Those descriptors are used by host driver to identify the device capabilities.
Once the NUCLEO-H723ZG USB device successfully completed the enumeration phase, the device sends an HID report after a user button press.
Each report sent should move the mouse cursor by one step on host side.

#### <b>Error behaviors</b>

Host PC shows that USB device does not operate as designed (mouse enumeration failed, the mouse pointer doesn't move).

#### <b>Assumptions if any</b>

User is familiar with USB 2.0 "Universal Serial BUS" specification and HID class specification.

#### <b>Known limitations</b>

The remote wakeup feature is not yet implemented (used to bring the USB suspended bus back to the active condition).

### <b>Notes</b>

 1. Some code parts can be executed in the ITCM-RAM (64 KB up to 256kB) which decreases critical task execution time, compared to code execution from flash memory. This feature can be activated using '#pragma location = ".itcmram"' to be placed above function declaration, or using the toolchain GUI (file options) to execute a whole source file in the ITCM-RAM.
 2.  If the application is using the DTCM/ITCM memories (@0x20000000/ 0x0000000: not cacheable and only accessible by the Cortex M7 and the MDMA), no need for cache maintenance when the Cortex M7 and the MDMA access these RAMs. If the application needs to use DMA (or other masters) based access or requires more RAM, then the user has to:
      - Use a non TCM SRAM. (example : D1 AXI-SRAM @ 0x24000000).
      - Add a cache maintenance mechanism to ensure the cache coherence between CPU and other masters (DMAs,DMA2D,LTDC,MDMA).
      - The addresses and the size of cacheable buffers (shared between CPU and other masters) must be properly defined to be aligned to L1-CACHE line size (32 bytes).
 3.  It is recommended to enable the cache and maintain its coherence:
      - Depending on the use case it is also possible to configure the cache attributes using the MPU.
      - Please refer to the **AN4838** "Managing memory protection unit (MPU) in STM32 MCUs".
      - Please refer to the **AN4839** "Level 1 cache on STM32F7 Series and STM32H7 Series".

#### <b>USBX usage hints</b>

- The DTCM (0x20000000) memory region should not be used by application in case USB DMA is enabled
- Should make sure to configure the USB pool memory region with attribute "Non-Cacheable" to ensure coherency between CPU and USB DMA

### <b>Keywords</b>

Standalone, USBXDevice, USB_OTG, Full Speed, HID, Mouse.

### <b>Hardware and Software environment</b>

  - This example runs on STM32H723xx devices.
  - This example has been tested with STMicroelectronics NUCLEO-H723ZG boards revision MB1364-H723ZG-E01 and can be easily tailored to any other supported device and development board.

### <b>How to use it ?</b>

In order to make the program work, you must do the following :

 - Open your preferred toolchain
 - Rebuild all files and load your image into target memory
 - Run the application