
##  Nx_WebServer application description

This application provides an example of Azure RTOS NetX Duo stack usage on STM32H735G-DK board, it shows how to develop Web HTTP server based application.

The application is designed to load files and static web pages stored in SD card using a Web HTTP server, the code provides all required features to build a compliant Web HTTP Server.

The main entry function tx_application_define() is called by ThreadX during kernel start, at this stage, both of the FileX and the NetXDuo system are initialized, the NX_IP instance and the Web HTTP server are created and configured, then the application creates the main thread with(Prio :4; PreemptionPrio :4) used to assign a dynamic IP address, open the SD card driver as a FileX Media and start the Web HTTP server.

Fetching a dynamic IP address to the stm32H735 board is a step blocking until an IP address is obtained.

Once the server is started, the user's browser can load a static web page, download a zip folder or watch a video that are stored in SD card.

In order to open simultaneous sessions for the server, you should define the number of simultaneous sessions NX_WEB_HTTP_SERVER_SESSION_MAX in "nx_user.h".

####  Expected success behavior

When an SD card is inserted into the STM32H735G-DK SD card reader and the board is powered up and connected to DHCP enabled Ethernet network, the green LED switches ON when the Web HTTP server is successfully started.
Then the static files can be loaded on the web browser after entring the url http://@IP/file_name.
An example web page is provided for testing the application that can be found under "NetXDuo/Nx_WebServer/Web_Content/index.html".

#### Error behaviors

If the WEB HTTP server is not successfully started, the green LED stays OFF.

In case of other errors, the Web HTTP server does not operate as designed (Files stored in the SD card are not loaded in the web browser).

#### Assumptions if any

The uSD card must be plugged before starting the application.

The board must be in a DHCP Ethernet network.

#### Known limitations
Hotplug is not implemented for this example, that is, the SD card is expected to be inserted before application running.


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
 - ThreadX offers the "tx_application_define()" function, that is automatically called by the tx_kernel_enter() API. It is highly recommended to use it to create all applications ThreadX related resources (threads, semaphores, memory pools...) but it should not in any way contain a system API call (HAL or BSP).
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
   
              
#### FileX/LevelX usage hints

- FileX sd driver is using the DMA, thus the DTCM (0x20000000) memory should not be used by the application, as it is not accessible by the SD DMA.
- When calling the fx_media_format() API, it is highly recommended to understand all the parameters used by the API to correctly generate a valid filesystem.
- FileX is using data buffers, passed as arguments to fx_media_open(), fx_media_read() and fx_media_write() API it is recommended that these buffers are multiple of sector size and "32 bytes" aligned to avoid cache maintenance issues.

#### NetX Duo usage hints

- The ETH TX And RX descriptors are accessed by the CPU and the ETH DMA IP, thus they should not be allocated into the DTCM RAM "0x20000000".
- Make sure to allocate them into a "Non-Cacheable" memory region to always ensure data coherency between the CPU and ETH DMA.
- Depending on the application scenario, the total TX and RX descriptors may need to be increased by updating respectively  the "ETH_TX_DESC_CNT" and "ETH_RX_DESC_CNT" in the "stm32h7xx_hal_conf.h", to guarantee the application correct behaviour, but this will cost extra memory to allocate.
- The NetXDuo driver has an internal data structure that needs to be located into a non cacheable area called '**NxDriverSection**'. Therefore the application should.
 + Declare the aforementioned section in the linker file as below
   + For EWARM ".icf" file
   ```
   define symbol __ICFEDIT_region_NXDATA_start__  = 0x24018200;
   define symbol __ICFEDIT_region_NXDATA_end__   = 0x24027FFF;
   define region ITCMRAM_region  = mem:[from __ICFEDIT_region_ITCMRAM_start__ to __ICFEDIT_region_ITCMRAM_end__];
   place in NXDriver_region { section NXDriverSection};
   ```
   + For STM32CubeIDE ".ld" file
   ``` 
   .nx_section 0x24018200 (NOLOAD): {
     *(.NXDriverSection)
     } >RAM_D1
   ```
  + Configure the MPU for the following memory regions:
   + **64KB** as non-cacheable starting from the address __ICFEDIT_region_NXDATA_start__ - 0x200 
   + **256Bytes** as Device memory at the beginning of these **64KB**
   
   the region configured as Device memory should  contain the ETH DMA RX and TX descriptors.


### Keywords

RTOS, ThreadX, Network, NetxDuo, FileX, File ,SDMMC, UART



### Hardware and Software environment
  - This application runs on STM32H735xx devices.

  - This application has been tested with STMicroelectronics STM32H735G-DK boards Revision: MB1520-H735I-B02
    and can be easily tailored to any other supported device and development board.

  - This application uses USART3 to display logs, the hyperterminal configuration is as follows: 
      - BaudRate = 9600 baud
      - Word Length = 8 Bits
      - Stop Bit = 1
      - Parity = odd
      - Flow control = None



###  How to use it ?

In order to make the program work, you must do the following :
 - Open your preferred toolchain
 - Rebuild all files and load your image into target memory
 - Run the application

