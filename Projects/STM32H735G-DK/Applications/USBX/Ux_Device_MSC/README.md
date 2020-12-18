
## Ux_Device_MSC application description

This application provides an example of Azure RTOS USBX stack usage on STM32H735G-DK board, it shows how to develop USB Device mass storage class based application.

The application is designed to emulate an USB MSC device, the code provides all required device descriptors framework and the associated Class descriptor report to build a compliant USB MSC device.

At the beginning ThreadX calls the entry function tx_application_define(), at this stage, all USBx resources are initialized, the mass storage Class driver is registered and the application creates one thread: 
  - usbx_app_thread_entry (Prio : 10; PreemptionPrio : 10) used to initialize USB OTG HAL PCD driver and start the device.
In addition two callback functions are needed for the USBX mass storage class device:
  - app_usb_device_thread_media_read used to read data through DMA from the mass storage device. 
  - app_usb_device_thread_media_write used to write data through DMA into the mass storage device.
 
#### Expected success behavior
When plugged to PC host, the STM32H735G-DK should enumerate as an USB MSC device. During the enumeration phase, device must provide host with the requested descriptors (Device descriptor, configuration descriptor, string descriptors).
Those descriptors are used by the host driver to identify the device capabilities.
Once the STM32H735G-DK USB device successfully completed the enumeration phase, a new removable drive appears in the system window and write/read/format operations can be performed as with any other removable drive.

#### Error behaviors
Host PC shows that USB device does not operate as designed (MSC enumeration fails, The new removable drive appears but read, write or format operations fail).

#### Assumptions if any

  - USB cable should not be unplugged during enumeration and driver install.
  - SD card should be inserted before application is started.

#### Known limitations

  - The Eject operation is not supported yet by MSC class.

### Notes
 1. Some code parts can be executed in the ITCM-RAM (64 KB up to 256kB) which decreases critical task execution time, compared to code execution from Flash memory. This feature can be activated using '#pragma location = ".itcmram"' to be placed above function declaration, or using the toolchain GUI (file options) to execute a whole source file in the ITCM-RAM.
 2.  If the application is using the DTCM/ITCM memories (@0x20000000/ 0x0000000: not cacheable and only accessible by the Cortex M7 and the MDMA), no need for cache maintenance when the Cortex M7 and the MDMA access these RAMs. If the application needs to use DMA (or other masters) based access or requires more RAM, then the user has to:
      - Use a non TCM SRAM. (example : D1 AXI-SRAM @ 0x24000000).
      - Add a cache maintenance mechanism to ensure the cache coherence between CPU and other masters (DMAs,DMA2D,LTDC,MDMA).
      - The addresses and the size of cacheable buffers (shared between CPU and other masters) must be properly defined to be aligned to L1-CACHE line size (32 bytes).
 3.  It is recommended to enable the cache and maintain its coherence:
      - Depending on the use case it is also possible to configure the cache attributes using the MPU.
      - Please refer to the **AN4838** "Managing memory protection unit (MPU) in STM32 MCUs".
      - Please refer to the **AN4839** "Level 1 cache on STM32F7 Series"
  
#### ThreadX usage hints
 - ThreadX uses the Systick as time base, thus it is mandatory that the HAL uses a separate time base through the TIM IPs.
 - ThreadX is configured with 100 ticks/sec by default, this should be taken into account when using delays or timeouts at application. It is always possible to reconfigure it in the "tx_user.h", the "TX_TIMER_TICKS_PER_SECOND" define,but this should be reflected in "tx_initialize_low_level.s" file too.
 - ThreadX is disabling all interrupts during kernel start-up to avoid any unexpected behavior, therefore all system related calls (HAL, BSP) should be done either at the beginning of the application or inside the thread entry functions.
 - ThreadX offers the "tx_application_define()" function, that is automatically called by the tx_kernel_enter() API.It is highly recommended to use it to create all applications ThreadX related resources (threads, semaphores, memory pools...) but it should not in any way contain a system API call (HAL or BSP).
 - ThreadX needs to know the first free memory block in the RAM to be used for resource allocations.
     + For EWARM ".icf" file add a reference to the "***FREE_MEM***" global symbol as below:
``` place in RAM_region { last section FREE_MEM}; ```

     + For CubeIDE ".ld" file add reference to "**\_\_***RAM_segment_used_end***\_\_**" as following:
```  ._user_heap_stack :
          { 
              . = ALIGN(8); `
              PROVIDE ( end = . );
              PROVIDE ( _end = . ); `
              . = . + _Min_Heap_Size;
              . = . + _Min_Stack_Size;
              . = ALIGN(8); `
              __RAM_segment_used_end__= .;
          } >RAM_D1
```
               
#### USBX usage hints
- The DTCM (0x20000000) memory region should not be used by application in case USB DMA is enabled
- Should make sure to configure the USB pool memory region with attribute "Non-Cacheable" to ensure coherency between CPU and USB DMA

### Keywords

RTOS, ThreadX, USB, USBX, DMA, SDMMC



### Hardware and Software environment

  - This example runs on STM32H735xx devices.
  - This example has been tested with STMicroelectronics STM32H735G-DK boards Revision: MB1520-H735I-B02.
    and can be easily tailored to any other supported device and development board.



### How to use it ?

In order to make the program work, you must do the following :
 - Open your preferred toolchain
 - Rebuild all files and load your image into target memory
 - Run the application
