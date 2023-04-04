# <b>Ux_Host_HID_Standalone application description</b>

This application provides an example of Azure RTOS USBX stack usage .
It shows how to develop bare metal USB Host Human Interface "HID" able to enumerate and communicates with a mouse or a keyboard.

The application's main calls the MX_USBX_Host_Init() function in order to Initialize USBX and USBX_Host_Process in the while loop.

As stated earlier, the present application runs in standalone mode without ThreadX, for this reason, the standalone variant of USBX is enabled by adding the following flag in ux_user.h:

 - #define UX_STANDALONE


The application is designed to behave as an USB HID Host, the code provides required requests to properly enumerate
HID devices , HID Class APIs to decode HID reports received from a mouse or a keyboard and display data on uart HyperTerminal.

#### <b>Expected success behavior</b>

When a HID device is plugged to NUCLEO-H723ZG board, a Message will be displayed on the uart HyperTerminal showing
the Vendor ID and Product ID of the attached device.
After enumeration phase, a message will indicate that the device is ready for use.
The host must be able to properly decode HID reports sent by the corresponding device and display those information on the HyperTerminal.

The received HID reports are used by host to identify:
in case of a mouse
   - (x,y) mouse position
   - Wheel position
   - Pressed mouse buttons

in case of a keyboard
 - Pressed key

#### <b>Error behaviors</b>

Errors are detected such as (Unsupported device, Enumeration Fail) and the corresponding message is displayed on the HyperTerminal.

#### <b>Assumptions if any</b>

User is familiar with USB 2.0 "Universal Serial BUS" Specification and HID class Specification.

#### <b>Known limitations</b>

None.

### <b>Notes</b>

 1. Some code parts can be executed in the ITCM-RAM (64 KB up to 256kB) which decreases critical task execution time, compared to code execution from Flash memory. This feature can be activated using '#pragma location = ".itcmram"' to be placed above function declaration, or using the toolchain GUI (file options) to execute a whole source file in the ITCM-RAM.
 2.  If the application is using the DTCM/ITCM memories (@0x20000000/ 0x0000000: not cacheable and only accessible by the Cortex M7 and the MDMA), no need for cache maintenance when the Cortex M7 and the MDMA access these RAMs. If the application needs to use DMA (or other masters) based access or requires more RAM, then the user has to:
      - Use a non TCM SRAM. (example : D1 AXI-SRAM @ 0x24000000).
      - Add a cache maintenance mechanism to ensure the cache coherence between CPU and other masters (DMAs,DMA2D,LTDC,MDMA).
      - The addresses and the size of cacheable buffers (shared between CPU and other masters) must be properly defined to be aligned to L1-CACHE line size (32 bytes).
 3.  It is recommended to enable the cache and maintain its coherence:
      - Depending on the use case it is also possible to configure the cache attributes using the MPU.
      - Please refer to the **AN4838** "Managing memory protection unit (MPU) in STM32 MCUs".
      - Please refer to the **AN4839** "Level 1 cache on STM32F7 Series and STM32H7 Series"

#### <b>USBX usage hints</b>

- The DTCM (0x20000000) memory region should not be used by application in case USB DMA is enabled
- Should make sure to configure the USB pool memory region with attribute "Non-Cacheable" to ensure coherency between CPU and USB DMA

### <b>Keywords</b>

Standalone, USBXHost, USB_OTG, Full Speed, HID, Mouse, Keyboard,

### <b>Hardware and Software environment</b>

  - This application runs on STM32H723xx devices.
  - This application has been tested with STMicroelectronics NUCLEO-H723ZG boards Revision MB1364-H723ZG-E01 and can be easily tailored to any other supported device and development board.

  - NUCLEO-H723ZG Set-up
    - Plug the USB HID device into the NUCLEO-H723ZG board through 'USB micro A-Male  to A-Female' cable to the connector:
      - CN13 : to use USB High Speed OTG IP.
    - Connect ST-Link cable to the PC USB port to display data on the HyperTerminal.

    A virtual COM port will then appear in the HyperTerminal:
     - Hyperterminal configuration
       - Data Length = 8 Bits
       - One Stop Bit
       - No parity
       - BaudRate = 115200 baud
       - Flow control: None

### <b>How to use it ?</b>

In order to make the program work, you must do the following :

 - Open your preferred toolchain
 - Rebuild all files and load your image into target memory
 - Run the application

<b>Notes</b>

The user has to check the list of the COM ports in Device Manager to find out the number of the COM ports that have been assigned (by OS) to the Stlink VCP.

The application uses the bypass HSE clock (STlink MCO Output) which is HSI/2 (Default configuration).

It is recommended for USB Host applications on NUCLEO-H723ZG board to change the USB Clock source to StlinkV3 MCO output : HSE/3(8.33MHZ) in order to have more accurate HSE input clock.
To change MCO Clock Output please follow the steps below :

  - Connect to your board using Cubeprogrammer
  - Select "Firmware upgrade".
  - Select "Open in update mode".
  - Select MCO Output : HSE/3 (8.33MHZ).
  - Select "Upgrade".
  - Update the Clock configuration settings in your application code in order to obtain  48MHZ as USB clock source.
