
## <b>Ux_Host_HID application description</b>

This application provides an example of Azure RTOS USBX stack usage.
It shows how to develop a USB Host Human Interface "HID" able to enumerate and communicate with a mouse or a keyboard.

The application is designed to behave as a USB HID Host, the code provides required requests to properly enumerate
HID devices, HID class APIs to decode HID reports received from a mouse or a keyboard and display data on UART HyperTerminal.

The main entry function tx_application_define() is then called by ThreadX during kernel start, at this stage, all USBx resources
are initialized, the HID class driver and HID clients are registered.
The application creates 3 threads with different priorities :

  - usbx_app_thread_entry     (Priority : 25; Preemption threshold : 25) used to initialize USB OTG HAL HCD driver and start the Host.
  - hid_mouse_thread_entry    (Priority : 30; Preemption threshold : 30) used to decode HID reports received  from a mouse.
  - hid_keyboard_thread_entry (Priority : 30; Preemption threshold : 30) used to decode HID reports received  from a keyboard.

#### <b>Expected success behavior</b>

When a HID device is plugged to NUCLEO-H723ZG board, a message will be displayed on the UART HyperTerminal showing
the Vendor ID and Product ID of the attached device.
After enumeration phase, a message will indicate that the device is ready for use.
The host must be able to properly decode HID reports sent by the corresponding device and display those information on the HyperTerminal.

The received HID reports are used by host to identify:
- in case of a mouse
   - (x,y) mouse position
   - Wheel position
   - Pressed mouse buttons

- in case of a keyboard
   - Pressed key

#### <b>Error behaviors</b>

Errors are detected (such as unsupported device, enumeration fail) and the corresponding message is displayed on the HyperTerminal.

#### <b>Assumptions if any</b>

User is familiar with USB 2.0 "Universal Serial BUS" specification and HID class specification.

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
- Should make sure to configure the USB pool memory region with attribute "Non-Cacheable" to ensure coherency between CPU and USB DMA

### <b>Keywords</b>

Connectivity, USBX Host, ThreadX, USB, HID, Mouse, Keyboard, UART, USART,

### <b>Hardware and Software environment</b>

  - This application runs on STM32H723xx devices
  - This application has been tested with STMicroelectronics NUCLEO-H723ZG MB1364-H723ZG-E01
    and can be easily tailored to any other supported device and development board.

- NUCLEO-H723ZG Set-up
    - Plug the USB HID device into the NUCLEO-H723ZG board through 'USB micro A-Male  to A-Female' cable to the connector:
      - CN13 : to use USB High Speed OTG IP
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

<b>Note</b>

The user has to check the list of the COM ports in Device Manager to find out the number of the COM ports that have been assigned (by OS) to the Stlink VCP.
The application uses the bypass HSE clock (STlink MCO Output) which is HSI/2 (Default configuration).

It is recommended for USB Host applications on NUCLEO-H723ZG board to change the USB Clock source to StlinkV3 MCO output : HSE/3(8.33MHZ) in order to have more accurate HSE input clock.
To change MCO Clock Output please follow the steps below :

  - Connect to your board using Cubeprogrammer
  - Select "Firmware upgrade".
  - Select "Open in update mode".
  - Select MCO Output : HSE/3 (8.33MHZ).
  - Select "Upgrade".
  - Update the Clock configuration settings in your application code in order to obtain 48MHZ as USB clock source.
