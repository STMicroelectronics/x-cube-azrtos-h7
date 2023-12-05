
## <b>Ux_Device_CCID application description</b>

This application provides an example of Azure RTOS USBX stack usage on NUCLEO-H723ZG board,
it shows how to develop USB Device for Integrated Circuit Card Interface Device based application.

The application is designed to emulate a smart card reader device, the code provides all required device descriptors framework
and associated class descriptor to build a USB CCID device.

At the beginning ThreadX calls the entry function tx_application_define(), at this stage, all USBx resources
are initialized, the CCID class driver is registered and the application creates 2 threads with the same priorities :

  - usbx_app_thread_entry (Prio : 10; PreemptionPrio : 10) used to initialize USB_OTG HAL PCD driver and start the device.
  - usbx_ccid_thread_entry (Prio : 20; PreemptionPrio : 20) used to simulate card insert remove detection.

CCID reader example is hosted on PC and can be tested with any PC/SC application using contactless interface smart card.
The PC/SC interface is a specification for contact and contactless smartcard into the computer eco-system.
When device is plugged with the USB link, the CCID device is controlled using this interface through several commands APDUs.

The smart card receives an ISO7816-4 compliant APDU within a CCID device.
With respect to PC/SC specification the reader interprets this APDU.
Each command APDU sent has an associated ending response.

Command APDU: CLA INS P1 P2 Lc Data Le
Response APDU: Data SW1 SW2

#### <b>Expected success behavior</b>

When plugged to PC host, the NUCLEO-H723ZG must be properly enumerated as an USB CCID device.
During the enumeration phase, device provides host with the requested descriptors (device, configuration, string).
Those descriptors are used by host driver to identify the device capabilities.
Once the NUCLEO-H723ZG USB device successfully completed the enumeration phase,
send and receive the CCID class commands through the bulk out, bulk in and interrupt endpoints:
- CCID Events like card detection or removal are sent on the Interrupt Endpoint.
- CCID Commands are sent on BULK-OUT Endpoint.
- CCID Responses are sent on BULK-IN Endpoint.

To start using pssc tool such as Cryptware Smart Card Console, Smart card ToolSet Pro or SpringCard PC/SC Diag,
follow next steps:
- Connect the board to the PC with the USB cable.
- Open the PC/SC application tool, the card gets detected and the ATR appears.
- Enter the following commands one by one in the command APDU area and check the response.

APDU Supported:
- ENABLE CHV1:  A0 28 00 01 08 CHV1 value       - success Message 90 00 is displayed.
- SELECT:       A0 A4 00 00 02 File ID          - success Message 90 00 is displayed.
- VERIFY:       A0 20 00 xx 08 CHV Value        - success Message 90 00 is displayed.
- CHANGE:       A0 24 00 xx 10 Old CHV, New CHV - success Message 90 00 is displayed.
- GET RESPONSE: A0 C0 00 00                     - success Message 90 00 is displayed and the written data is displayed
- READ BINARY:  A0 B0 xx xx                     - success Message 90 00 is displayed and the read data is displayed.

For warning processing, execution error or the unsupported command: message 69 99 - 63 C0 - 88 88 is displayed.

#### <b>Error behaviors</b>

Host PC shows that USB device does not operate as designed (CCID Device enumeration failed, no card reader detected in PS/SC tool).

#### <b>Assumptions if any</b>

User is familiar with USB 2.0 "Universal Serial BUS" specification and CCID class specification.

#### <b>Known limitations</b>

None

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
- Should make sure to configure the USB pool memory region with attribute "Non-Cacheable" to ensure coherency between CPU and USB DMA.

### <b>Keywords</b>

RTOS, ThreadX, USBXDevice, USB_OTG, Full Speed, CCID, ICC

### <b>Hardware and Software environment</b>

  - This example runs on STM32H723xx devices.
  - This example has been tested with STMicroelectronics NUCLEO-H723ZG boards revision MB1364-H723ZG-E01 and can be easily tailored to any other supported device and development board.

### <b>How to use it ?</b>

In order to make the program work, you must do the following :

 - Open your preferred toolchain
 - Rebuild all files and load your image into target memory
 - Run the application