
## <b>Ux_Device_DualHID application description</b>

This application provides an example of Azure RTOS USBX stack usage on STM32H747I-DISCO board, it shows how to develop a composite USB Human Interface
Device class "HID Mouse" and "HID Keyboard" based application.

The application is designed to emulate a USB HID composite mouse & keyboard device, the code provides all required device descriptors framework
and associated class descriptor report to build a compliant USB HID composite device.

At the beginning ThreadX calls the entry function tx_application_define(), at this stage, all USBx resources are initialized, the HID class driver is
registered and the application creates 3 threads with the same priorities :

  - ux_device_app_thread (Prio : 10; PreemptionPrio : 10) used to initialize USB OTG HAL PCD driver and start the device.
  - ux_hid_mouse_thread (Prio : 20; PreemptionPrio : 20)  used to send HID reports to move automatically the PC host machine cursor.
  - ux_hid_Keyboard_thread (Prio : 20; PreemptionPrio : 20)  used to send HID reports key.

To customize the HID application by sending the mouse position & key characters every 10ms:
  - For each 10ms, the application calls the GetPointerData() API to update the mouse position (x, y)
    and send the report buffer through the ux_device_class_hid_event_set() API.
  - For each 10ms, the application calls the GetKeyData() API to update the key character and
    send the report buffer through the ux_device_class_hid_event_set() API.

#### <b>Expected success behavior</b>

When plugged to PC host, the STM32H747I-DISCO must be properly enumerated a composite device as an HID, USB serial device and an STlink COM port.
During the enumeration phase, the device must provide host with the requested descriptors (Device descriptor, configuration descriptor, string descriptors).
Those descriptors are used by host driver to identify the device capabilities. Once STM32H747I-DISCO USB device successfully completed the enumeration phase:

- Connect USB cable to Host.
- Press Joystick to send cursor position.
- Change keyboard settings to qwerty open file test and press user button to send character.

#### <b>Error behaviors</b>

Host PC shows that USB device does not operate as designed (enumeration failed, for example PC cursor doesn't move or COM port enumeration failed).

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

#### <b>ThreadX usage hints</b>

 - ThreadX uses the Systick as time base, thus it is mandatory that the HAL uses a separate time base through the TIM IPs.
 - ThreadX is configured with 100 ticks/sec by default, this should be taken into account when using delays or timeouts at application. It is always possible to reconfigure it, by updating the "TX_TIMER_TICKS_PER_SECOND" define in the "tx_user.h" file. The update should be reflected in "tx_initialize_low_level.S" file too.
 - ThreadX is disabling all interrupts during kernel start-up to avoid any unexpected behavior, therefore all system related calls (HAL, BSP) should be done either at the beginning of the application or inside the thread entry functions.
 - ThreadX offers the "tx_application_define()" function, that is automatically called by the tx_kernel_enter() API.
   It is highly recommended to use it to create all applications ThreadX related resources (threads, semaphores, memory pools...)  but it should not in any way contain a system API call (HAL or BSP).
 - Using dynamic memory allocation requires to apply some changes to the linker file.
   ThreadX needs to pass a pointer to the first free memory location in RAM to the tx_application_define() function, using the "first_unused_memory" argument.
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

- The DTCM (0x20000000) memory region should not be used by application in case USB DMA is enabled.
- Should make sure to configure the USB pool memory region with attribute "Non-Cacheable" to ensure coherency between CPU and USB DMA.

### <b>Keywords</b>

RTOS, ThreadX, USBX, USBXDevice, USB_OTG, High Speed, HID, Keyboard, Mouse.

### <b>Hardware and Software environment</b>

  - This example runs on STM32H747xx devices.
  - This example has been tested with STMicroelectronics STM32H747I-DISCO boards revision MB1520-H747I-B02
    and can be easily tailored to any other supported device and development board.
  - STM32H747I-DISCO Set-up
  - Connect the STM32H747I-DISCO board CN1 to the PC through "MICRO-USB" to "Standard A" cable.

### <b>How to use it ?</b>

In order to make the program work, you must do the following :

 - Open your preferred toolchain
 - For each target configuration (Ux_Device_DualHID_CM4 first then Ux_Device_DualHID_CM7) :
     - Rebuild all files
     - Load images into target memory
 - After loading the two images, you have to reset the board in order to boot (Cortex-M7) and CPU2 (Cortex-M4) at once.
 - In the workspace toolbar select the project configuration:
 - Ux_Device_DualHID_CM7: to configure the project for STM32H747xx devices using USB OTG HS peripheral.
 - Run the application