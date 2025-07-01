
## <b>Ux_Device_PIMA_MTP Application Description</b>

This application provides an example of Azure RTOS USBX stack usage on STM32H735G-DK board, it shows how to develop USB Device Media Transfer Protocol class based
application. The application is designed to emulate a USB PIMA MTP device, the code provides all required device descriptors framework and the associated class
descriptor report to build a compliant USB MTP device.

At the beginning ThreadX calls the entry function tx_application_define(), at this stage, all USBx resources are initialized, the mass storage class driver is
registered and the application creates one thread:

  - app_ux_device_thread_entry (Prio : 10; PreemptionPrio : 10) used to initialize USB_OTG HAL PCD driver and start the device.

#### <b>Expected success behavior</b>

When plugged to PC host, the STM32H735G-DK should enumerate as a USB MTP device. During the enumeration phase, device must provide host with the requested
descriptors (device descriptor, configuration descriptor, string descriptors).

Those descriptors are used by the host driver to identify the device capabilities.

Once the STM32H735G-DK USB device successfully completed the enumeration phase, a new removable drive appears in the system window.
Operations can be performed as with any other removable drive.
It is then possible to do all standard files operations:
  - Files: read, write, delete, copy, paste.

The provided application does not support modifying an existing file in the media storage.

The advantage of MTP over classic MSC interface (Mass Storage Controller) is that MTP doesn't need or use
any file system. Instead, MTP relies on simple files indexing and properties descriptions.

At startup, all files present in the SD card are parsed and indexed (unique index assigned to
each item in the form of a chained list). The files properties are extracted only when requested
by the PC Host.

PC Host is able to read the existing files by going through the available indexes and requesting
then extracting items properties or data one by one.

When a new file is created by the host it is added to the chained list of indexes with the right properties.

As MTP doesn't use a file system, a limited set of files/folders types can be supported. The USB MTP device
expose the list of supported types to the PC Host during first MTP enumeration phase.
This list of supported types (as well as supported properties) can be modified by user through the defines
available in ux_device_pima_mtp.h.

Note1: Folder and Sub Folder are not supported for this release.
Note2: After formatting media a refersh is required.

#### <b>Error behaviors</b>

Host PC shows that USB device does not operate as designed (MTP enumeration fails, the new removable driver appears but read, write or format operations fail).

#### <b>Assumptions if any</b>

  - SD card should be inserted before application is started.

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

#### <b>ThreadX usage hints</b>

 - ThreadX uses the Systick as time base, thus it is mandatory that the HAL uses a separate time base through the TIM IPs.
 - ThreadX is configured with 100 ticks/sec by default, this should be taken into account when using delays or timeouts at application. It is always possible to reconfigure it, by updating the "TX_TIMER_TICKS_PER_SECOND" define in the "tx_user.h" file. The update should be reflected in "tx_initialize_low_level.S" file too.
 - ThreadX is disabling all interrupts during kernel start-up to avoid any unexpected behavior, therefore all system related calls (HAL, BSP) should be done either at the beginning of the application or inside the thread entry functions.
 - ThreadX offers the "tx_application_define()" function, that is automatically called by the tx_kernel_enter() API.
   It is highly recommended to use it to create all applications ThreadX related resources (threads, semaphores, memory pools...)  but it should not in any way contain a system API call (HAL or BSP).
 - Using dynamic memory allocation requires to apply some changes to the linker file.
   ThreadX needs to pass a pointer to the first free memory location in RAM to the tx_application_define() function,
   using the "first_unused_memory" argument.
   This requires changes in the linker files to expose this memory location.
    + For EWARM add the following section into the .icf file:
     ```
    place in RAM_region    { last section FREE_MEM };
    ```
    + For MDK-ARM:
    ```
    either define the RW_IRAM1 region in the ".sct" file
    or modify the line below in "tx_initialize_low_level.S to match the memory region being used
        LDR r1, =|Image$$RW_IRAM1$$ZI$$Limit|
    ```
    + For STM32CubeIDE add the following section into the .ld file:
    ```
    ._threadx_heap :
      {
         . = ALIGN(8);
         __RAM_segment_used_end__ = .;
         . = . + 64K;
         . = ALIGN(8);
       } >RAM_D1 AT> RAM_D1
    ```

       The simplest way to provide memory for ThreadX is to define a new section, see ._threadx_heap above.
       In the example above the ThreadX heap size is set to 64KBytes.
       The ._threadx_heap must be located between the .bss and the ._user_heap_stack sections in the linker script.
       Caution: Make sure that ThreadX does not need more than the provided heap memory (64KBytes in this example).
       Read more in STM32CubeIDE User Guide, chapter: "Linker script".

    + The "tx_initialize_low_level.S" should be also modified to enable the "USE_DYNAMIC_MEMORY_ALLOCATION" flag.

#### <b>USBX usage hints</b>

- The DTCM (0x20000000) memory region should not be used by application in case USB DMA is enabled
- Should make sure to configure the USB pool memory region with attribute "Non-Cacheable" to ensure coherency between CPU and USB DMA

### <b>Keywords</b>

RTOS, ThreadX, USBXDevice, Device, USB_OTG, Full Speed, MTP, SD Card, SDMMC

### <b>Hardware and Software environment</b>

  - This application runs on STM32H735xx devices.
  - This application has been tested with STMicroelectronics STM32H735G-DK boards revision: MB1520-H735I-B02
    and can be easily tailored to any other supported device and development board.

### <b>How to use it ?</b>

In order to make the program work, you must do the following :

 - Open your preferred toolchain
 - Rebuild all files and load your image into target memory
 - Run the application