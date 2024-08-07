
## <b>Ux_Host_HUB_HID_MSC Application Description</b>

This application provides an example of Azure RTOS USBX stack usage.
This application demonstrates the ability of USB Host Stack to interact with more than one USB Mass Storage and HID Devices through a Hub.
It shows how to develop a USB Host Hub application supporting Human Interface "HID" and Mass Storage "MSC" class drivers to be able to enumerate and communicate with:
  - An USB hub.
  - A mouse.
  - A keyboard.
  - An USB removable flash disk.

The application is designed to behave as a:

  - USB HUB Host, the code provides all required features to enumerate HUB device, the USBX initializes the host hub class and displays the device information through UART HyperTerminal.
  - USB HID Host, the code provides required requests to properly enumerate HID devices , HID Class APIs to decode HID reports received from a mouse or a keyboard and display data on UART HyperTerminal.
  - USB MSC Host able to operate with a USB flash disk using the Bulk Only Transfer (BOT) and Small Computer System Interface (SCSI) transparent commands combined with a file system AzureRTOS FILEX.

The main entry function tx_application_define() is then called by ThreadX during kernel start, at this stage, all USBx resources are initialized, the HUB, the MSC and the HID Class drivers are registered.
The application creates 4 threads with different priorities :

  - app_ux_host_thread_entry  (Priority : 10; Preemption threshold : 10) used to initialize USB OTG HAL HCD driver, start the Host and proceed to file operations or HID reports once the device is properly enumerated.
  - hid_mouse_thread_entry    (Priority : 30; Preemption threshold : 30) used to decode HID reports received from a mouse.
  - hid_keyboard_thread_entry (Priority : 30; Preemption threshold : 30) used to decode HID reports received from a keyboard.
  - msc_process_thread_entry  (Priority : 30; Preemption threshold : 30) used to proceed to file operations.


#### <b>Expected success behavior</b>

When a hub device plugged to STM32H747I-DISCO board, a Message will be displayed on the UART HyperTerminal showing
the Vendor ID and Product ID of the attached device.
After enumeration phase, a message will indicate that the device is ready for use and start connecting other devices through the hub.

When a HID device plugged to STM32H747I-DISCO board, a Message will be displayed on the UART HyperTerminal showing
the Vendor ID and Product ID of the attached device.
After enumeration phase, a message will indicate that the device is ready for use.
The host must be able to properly decode HID reports sent by the corresponding device and display those information on the HyperTerminal.

The received HID reports are used by host to identify:

in case of a mouse:

   - (x,y) mouse position
   - Wheel position
   - Pressed mouse buttons

in case of a keyboard:

 - Pressed key

When a USB flash disk is plugged to STM32H747I-DISCO board, a Message will be displayed on the uart HyperTerminal showing the Vendor ID and the Product ID of the attached device.
After enumeration phase, the host proceed to file operations :

  - Create a "Test.txt" file.
  - Write a small text in the created file.
  - Read the written text and check data integrity.
  - Close the File.

During the file operations process a message will be displayed on the HyperTerminal to indicate the outcome of each operation (Create/Write/Read/Close) .
If all operations were successful a message will be displayed on the HyperTerminal to indicate the end of operations.

<b>Note</b>

This application supports two instances HID: a mouse and a keyboard, and two instances for Mass Storage devices to enumerate and communicate with.

#### <b>Error behaviors</b>

Errors are detected such as (Unsupported device, Enumeration Fail) and the corresponding message is displayed on the HyperTerminal.

#### <b>Assumptions if any</b>

User is familiar with USB 2.0 "Universal Serial BUS" Specification, HUB, HID and Mass storage class Specification.

#### <b>Known limitations</b>

None

### <b>Notes</b>


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


### <b>Keywords</b>

Connectivity, USBXHost,HUB, FILEX, ThreadX, HID, Mouse, Keyboard, MSC, Mass Storage, BOT, SCSI, Removable drive, UART/USART


### <b>Hardware and Software environment</b>

  - This application runs on STM32H747xx devices
  - This application has been tested with STMicroelectronics STM32H747I-DISCO boards revision MB1520-H747I-B02
    and can be easily tailored to any other supported device and development board.

- STM32H747I-DISCO Set-up
    - Plug the USB Hub device into the STM32H747I-DISCO board through "MICRO-USB" to "Standard A" cable
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

 - The user has to check the list of the COM ports in Device Manager to find out the number of the COM ports that have been assigned (by OS) to the Stlink VCP .
 - The application uses the external HSE clock as USB source clock.
 - When Using a keyboard with multiple interfaces user must increase the size of USBX stack to avoid memory insufficient, increase the value of "USBX_HOST_MEMORY_STACK_SIZE", "USBX_HOST_APP_MEMORY_BUFFER_SIZE" and "UX_HOST_CLASS_HID_USAGES".
