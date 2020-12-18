
##  Ux_Host_HID application description 


This application provides an example of Azure RTOS USBX stack usage . 
It shows how to develop USB Host Human Interface "HID" able to enumerate and communicates with a mouse or a keyboard.

The application is designed to behave as an USB HID Host, the code provides required requests to properly enumerate 
HID devices , HID Class APIs to decode HID reports received from a mouse or a keyboard and display data on uart HyperTerminal.

The main entry function tx_application_define() is then called by ThreadX during kernel start, at this stage, all USBx resources 
are initialized, the HID Class driver and HID clients are registered. 
The application creates 3 threads with different priorities : 
  - usbx_app_thread_entry     (Priority : 20; Preemption threshold : 20) used to initialize USB OTG HAL HCD driver and start the Host.
  - hid_mouse_thread_entry    (Priority : 30; Preemption threshold : 30) used to decode HID reports received  from a mouse.
  - hid_keyboard_thread_entry (Priority : 30; Preemption threshold : 30) used to decode HID reports received  from a keyboard.

####  Expected success behavior

When a hid device plugged to NUCLEO-H723ZG board, a Message will be displayed on the uart HyperTerminal showing 
the Vendor ID and Product ID of the attached device.
After enumeration phase, a message will indicates that the device is ready for use.
The host must be able to properly decode HID reports sent by the corresponding device and display those information on the HyperTerminal.

The received HID reports are used by host to identify: 
in case of a mouse
   - (x,y) mouse position 
   - Wheel position 
   - Pressed mouse buttons 

in case of a keyboard
 - Pressed key

#### Error behaviors
Errors are detected such as (Unsupported device, Enumeration Fail) and the corresponding message is displayed on the HyperTerminal.

#### Assumptions if any
User is familiar with USB 2.0 "Universal Serial BUS" Specification and HID class Specification.

#### Known limitations
None

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
 - ThreadX offers the "tx_application_define()" function, that is automatically called by the tx_kernel_enter() API.     It is highly recommended to use it to create all applications ThreadX related resources (threads, semaphores, memory pools...)  but it should not in any way contain a system API call (HAL or BSP).
 - ThreadX needs to know the first free memory block in the RAM to be used for resource allocations.
     + For EWARM ".icf" file add a reference to the "***FREE_MEM***" global symbol as below:       
``` place in RAM_region    { last section FREE_MEM}; ```

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
-  Should make sure to configure the USB pool memory region with attribute "Non-Cacheable" to ensure coherency between CPU and USB DMA

### Keywords

Connectivity, USBX Host, ThreadX, USB, HID, Mouse, Keyboard, UART, USART,


### Hardware and Software environment

  - This application runs on STM32H723xx devices 

  - This application has been tested with STMicroelectronics NUCLEO-H723ZG MB1364-H723ZG-E01.
    and can be easily tailored to any other supported device and development board.
	
- NUCLEO-H723ZG Set-up
    - Plug the USB HID device into the NUCLEO-H723ZG board through 'USB micro A-Male  to A-Female' cable to the connector:
      - CN13 : to use USB High Speed OTG IP in full speed (HS_IN_FS)
    - Connect ST-Link cable to the PC USB port to display data on the HyperTerminal.

    A virtual COM port will then appear in the HyperTerminal:
     - Hyperterminal configuration
       - Data Length = 8 Bits
       - One Stop Bit
       - No parity
       - BaudRate = 115200 baud
       - Flow control: None


###  How to use it ?

In order to make the program work, you must do the following :
 - Open your preferred toolchain
 - Rebuild all files and load your image into target memory
 - Run the application

**Note**
   The user has to check the list of the COM ports in Device Manager to find out the number of the COM ports that have been assigned (by OS) to the Stlink VCP .



