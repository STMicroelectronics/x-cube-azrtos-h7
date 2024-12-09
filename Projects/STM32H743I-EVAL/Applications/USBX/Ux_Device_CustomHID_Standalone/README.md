
## <b>Ux_Device_CustomHID_Standalone application description</b>

This application provides an example of Azure RTOS USBX stack usage on STM32H743I-EVAL board,
it shows how to develop USB Device Human Interface "HID" custom based bare metal application.

The application is designed to emulate an USB HID custom device, the code provides all required device descriptors framework
and associated class descriptor report to build a compliant USB HID custom device.

The application's main calls the MX_USBX_Device_Init() function in order to Initialize USBX and USBX_Device_Process in the while loop.
As stated earlier, the present application runs in standalone mode without RTOS, for this reason, the standalone variant of USBX is enabled by adding the following flag in ux_user.h:

  - #define UX_STANDALONE

#### <b>Expected success behavior</b>

When plugged to PC host, the STM32H743I-EVAL must be properly enumerated as an USB HID custom device.
During the enumeration phase, device provides host with the requested descriptors (device, configuration, string).
Those descriptors are used by host driver to identify the device capabilities.

Once the STM32H743I-EVAL USB device successfully completed the enumeration phase:
 - Start the "USB HID Demonstrator" PC applet, available for download from www.st.com,
   and connect STM32 USB to PC
 - The device should be detected and shown in the USB HID target box
 - Press Graphic View button
 - Select "use SET_FEATURE" or "use SET_REPORT" in order to use SET_REPORT request to send HID report
   for LED control
 - Use the potentiometer of the STM32H743I-EVAL board to transfer the result of the converted voltage
  (via the ADC) to the PC host (these values are sent to the PC using the endpoint1 IN)
 - Make sure that the following report IDs are configured: LED1 ID (0x1), LED2 ID(0x2), LED3 ID(0x3),
   LED4 ID(0x4), BUTTON1_ID(0x6) and Potentiometer_ID(0x7).
 - Select LEDs to switch on/off on the STM32H743I-EVAL board: a SET_REPORT request will be sent.

#### <b>Error behaviors</b>

Host PC shows that USB device does not operate as designed (HID device enumeration failed).

#### <b>Assumptions if any</b>

User is familiar with USB 2.0 "Universal Serial BUS" specification and HID class specification.

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

Standalone, USBXDevice, USB_OTG, High Speed, HID, Custom, Potentiometer, ADC

### <b>Hardware and Software environment</b>

  - This example runs on STM32H743xx devices.
  - This example has been tested with STMicroelectronics STM32H743I-EVAL boards revision MB1246-B03 and can be easily
    tailored to any other supported device and development board.
  - STM32H743I-EVAL set-up:
    - Connect the STM32H743I-EVAL board to the PC through 'USB micro A-Male
      to A-Male' cable to the connector:
      - CN14: to use USB High Speed (HS)

### <b>How to use it ?</b>

In order to make the program work, you must do the following :

 - Open your preferred toolchain
 - Rebuild all files and load your image into target memory
 - Run the application