## <b>Ux_Device_RNDIS application description</b>

This application provides an example of Azure RTOS RNDIS stack usage on STM32H735G-DK board, it shows how to run Web HTTP server based application stack
over USB interface. The application is designed to load files and web pages stored in SD card using a Web HTTP server through USB RNDIS interface
class, the code provides all required features to build a compliant Web HTTP Server. The main entry function tx_application_define() is called by ThreadX during
kernel start, at this stage, the USBX initialize the network layer through USBx Class (RNDIS) also the FileX and the NetXDuo system are initialized,
the NX_IP instance and the Web HTTP server are created and configured, then the application creates two main threads
From the host point of view, the STM32 is seen as an HTTP server accessed through address 192.168.0.10.

  - usbx_app_thread_entry (Prio : 10; PreemptionPrio : 10) used to initialize USB OTG HAL PCD driver and start the device.
  - WebServer_thread (Prio :10; PreemptionPrio :10) used to assign a dynamic IP address, open the SD card driver as a FileX Media and start the Web HTTP server.

In order to configure correctly the USB Host, following setup must be followed:

  - Deactivate IPv6 and activate IPv4 mode
  - Setup static addresses as follows:
      - Address = 192.168.0.10
      - Network Mask = 255.255.255.0
      - Gateway = 192.168.0.10
  - Automatic DNS and Routing deactivated.

Once the server is started, the user's browser can load web pages as index.html and STM32H7xxLED.html.

#### <b>Expected success behavior</b>

When an SD card is inserted into the STM32H735G-DK SD card reader and the board is powered up and connected to DHCP enabled Ethernet network.
Then the web page can be loaded on the web browser after entring the url http://192.168.0.10/index.html also the user can switch to STM32H7xxLED.html page to toggle the green led.
An example web pages is provided for testing the application that can be found under "USBX/Ux_Device_RNDIS/Web_Content/"

#### <b>Error behaviors</b>

If the WEB HTTP server is not successfully started, the LED_RED should start blinking.
In case of other errors, the Web HTTP server does not operate as designed (Files stored in the SD card are not loaded in the web browser).

#### <b>Assumptions if any</b>

The uSD card must be plugged before starting the application.

#### <b>Known limitations</b>

Hotplug is not implemented for this example, that is, the SD card is expected to be inserted before application running.

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
 - ThreadX is configured with 100 ticks/sec by default, this should be taken into account when using delays or timeouts at application. It is always possible to reconfigure it in the "tx_user.h", the "TX_TIMER_TICKS_PER_SECOND" define,but this should be reflected in "tx_initialize_low_level.S" file too.
 - ThreadX is disabling all interrupts during kernel start-up to avoid any unexpected behavior, therefore all system related calls (HAL, BSP) should be done either at the beginning of the application or inside the thread entry functions.
 - ThreadX offers the "tx_application_define()" function, that is automatically called by the tx_kernel_enter() API.
   It is highly recommended to use it to create all applications ThreadX related resources (threads, semaphores, memory pools...)  but it should not in any way contain a system API call (HAL or BSP).
 - Using dynamic memory allocation requires to apply some changes to the linker file.
   ThreadX needs to pass a pointer to the first free memory location in RAM to the tx_application_define() function,
   using the "first_unused_memory" argument.
   This require changes in the linker files to expose this memory location.
    + For EWARM add the following section into the .icf file:
     ```
     place in RAM_region    { last section FREE_MEM };
     ```
    + For MDK-ARM:
     ```
    either define the RW_IRAM1 region in the ".sct" file
    or modify the line below in "tx_low_level_initilize.s to match the memory region being used
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


#### <b>FileX/LevelX usage hints</b>
- FileX sd driver is using the DMA, thus the DTCM (0x20000000) memory should not be used by the application, as it is not accessible by the SD DMA.
- When calling the fx_media_format() API, it is highly recommended to understand all the parameters used by the API to correctly generate a valid filesystem.
- FileX is using data buffers, passed as arguments to fx_media_open(), fx_media_read() and fx_media_write() API it is recommended that these buffers are multiple of sector size and "32 bytes" aligned to avoid cache maintenance issues.
- Should make sure to configure the USB pool memory region with attribute "Non-Cacheable" to ensure coherency between CPU and USB DMA

### <b>Keywords</b>

RTOS, ThreadX, USBXDevice, RNDIS, Network, NetxDuo, FileX, File ,SDMMC, UART

### <b>Hardware and Software environment</b>

  - This application runs on STM32H735xx devices.
  - This application has been tested with STMicroelectronics STM32H735G-DK boards Revision: MB1520-H735I-B02
    and can be easily tailored to any other supported device and development board.

  - STM32H735G-DK Set-up
    - Connect the DK board to remote PC (through a USB cable)
     - Http clients: Microsoft Internet Explorer (v8 and later) or Google Chrome (v107)

  - Remote PC Set-up
    - In most cases, it is sufficient to disable IPv6 from the setting tray and skip steps below.
    - Disable IPv6: Open /etc/default/grub with a text editor, and add "ipv6.disable=1" to GRUB_CMDLINE_LINUX variable
      and  apply the modified GRUB/GRUB2 settings by running sudo update-grub

    - set Linux IP address and mask to 192.168.0.10, 255.255.255.0, 192.168.0.10
      and Set IPv4 to manual mode with DHCP and routing disabled:
      use nmcli con show command to obtain your connection name then run the command:
      nmcli con mod "your connection name" ipv4.addresses "192.168.0.10/24,255.255.255.0" ipv4.gateway "192.168.0.10" ipv4.method "manual"

    - Set Windows 7/10 IP address and mask to 192.168.0.10, 255.255.255.0, 192.168.0.10 manually:
      open Control Panel \ Network and Internet \ Network Connections panel, and with right button click and select properties:
      double click on Internet Protocol version 4(TCP/IPv4) and select "use the following IP address":
      "IP address: 192.168.0.10" "Subnet mask: 255.255.255.0" "Default gateway: 192.168.0.10"

### <b>How to use it ?</b>

In order to make the program work, you must do the following :

 - Open your preferred toolchain
 - Rebuild all files and load your image into target memory
 - Run the application
