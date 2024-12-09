
## <b>Ux_Device_HID_MSC_Standalone application description</b>

This application provides an example of Azure RTOS USBX stack usage on STM32H735G-DK board, it shows how to develop a composite USB device mass storage "MSC" and "HID" class based bare metal application.
The application is designed to emulate a USB HID mouse device and USB MSC device, the code provides all required device descriptors framework and associated to both classes
descriptor report to build a composite compliant USB HID_MSC device.

The application's main calls the MX_USBX_Device_Init() function in order to Initialize USBX and USBX_Device_Process in the while loop.
As stated earlier, the present application runs in standalone mode without RTOS, for this reason, the standalone variant of USBX is enabled by adding the following flag in ux_user.h:

  - #define UX_STANDALONE

In addition two callback functions are needed for the USBX mass storage class device:

  - USBD_STORAGE_Read used to read data through DMA from the mass storage device.
  - USBD_STORAGE_Write used to write data through DMA into the mass storage device.

#### <b>Expected success behavior</b>

When plugged to PC host, the STM32H735G-DK should enumerate as a composite device as an HID, USB MSC device. During the enumeration phase, device must provide host with the requested
descriptors (device descriptor, configuration descriptor, string descriptors).
Those descriptors are used by the host driver to identify the device capabilities.
Once the STM32H735G-DK USB device successfully completed the enumeration phase:
  - a new removable drive appears in the system window and write/read/format operations can be performed as with any other removable drive.
  - When USER_Button is pressed, the device sends an HID report. Each report sent should move the PC host machine mouse cursor by one step.

#### <b>Error behaviors</b>

Host PC shows that USB device does not operate as designed (enumeration failled, for example PC cursor doesn't move or the new removable drive appears but read, write or format operations fail).

#### <b>Assumptions if any</b>

  - User is familiar with USB 2.0 "Universal Serial BUS" specification and with "MSC" and "HID" class specification.
  - USB cable should not be unplugged during enumeration and driver installation.
  - SD card should be inserted before application is started.

#### <b>Known limitations</b>

The Eject operation is not supported yet by MSC class.

### <b>Notes</b>

 1. Some code parts can be executed in the ITCM-RAM (64 KB up to 256kB) which decreases critical task execution time, compared to code execution from Flash memory. This feature can be activated using '#pragma location = ".itcmram"' to be placed above function declaration, or using the toolchain GUI (file options) to execute a whole source file in the ITCM-RAM.
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
- Should make sure to configure the USB pool memory region with attribute "Non-Cacheable" to ensure coherency between CPU and USB DMA.

### <b>Keywords</b>

Standalone, USBXDevice, Device, USB_OTG, Full Speed, MSC, HID, Mass Storage, Mouse, SD Card, DMA, SDMMC

### <b>Hardware and Software environment</b>

  - This example runs on STM32H735xx devices.
  - This example has been tested with STMicroelectronics STM32H735G-DK boards revision: MB1520-H735I-B02
    and can be easily tailored to any other supported device and development board.

### <b>How to use it ?</b>

In order to make the program work, you must do the following :

 - Open your preferred toolchain
 - Rebuild all files and load your image into target memory
 - Run the application
