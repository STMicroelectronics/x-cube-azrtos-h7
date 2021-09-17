
##  <b>Fx_Dual_Instance application description</b>

This application provide user a working example of two storage media managed by two independent instances of FileX/LevelX running on STM32H747I-DISCO board (Dual core). 
This is to demonstrate the coexistence capability of two FileX/Levelx stacks running independently on each core.

Two independent media storage: µSD and QSPI NOR Flash will be used on STM32H747I-DISCO board(Dual core).

Each core will create his own file system using one instance:

  - Cortex-M7: will create file system on µSD using FileX stack.
  - Cortex-M4: will create file system on QSPI NOR Flash using FileX/LevelX stacks.
  
CM7 core:
System Init, System clock config, voltage scaling and L1-Cache configuration are done by Cortex-M7.
In the meantime Domain D2 is put in STOP mode (Cortex-M4 in deep sleep mode) to save power consumption.

When system initialization is finished, Cortex-M7 will release Cortex-M4 when needed by means of HSEM notification but release could be also ensured 
by any Domain D2 wakeup source (SEV,EXTI..). This will guarantee that Cortex-M4 code execution starts after system initialization by Cortex-M7.

After Domain D2 wakeup, if Cortex-M7 attempts to use any resource from such a domain, 
the access will not be safe until “clock ready flag” of D2 domain is set (by hardware). 
The check could be done using this macro : __HAL_RCC_GET_FLAG(RCC_FLAG_D2CKRDY).

The main entry function, tx_application_define(), is called by ThreadX during kernel start. At this stage, all FileX resources are initialized, the SD card detection event is registered and drivers are initialized.

A single thread is created:

  - fx_thread (Prio : 10; PreemptionPrio : 10) used to initialize the SD driver and starting FileX's file operations.

A message queue is used to signal the SD card detection event to the fx_thread thread:

  - tx_msg_queue (Msg size : 1 (UINT); total messages : 16) used to notify the fx_thread about the SD card insertion status.

User should format the µSD before using it in the application otherwise an error will occur.

The fx_thread starts by checking whether the SD card is initially inserted or not. In the true case, it sends a message to the queue to ensure that the first iteration starts properly.
The wait on the message queue blocks till it receives a new message about whether the SD card is inserted or removed. Interrupt callback for SD detection is registered and it is used
to send the event information to the message queue.

The fx_thread uses FileX services to open the SD media for file operations and attempt to create file STM32.TXT. If the file exists already, it will be overwritten.
Dummy content is then written into the file and it is closed. The file is opened once again in read mode and content is checked if matches what was previously written.

CM4 core:

The application starts by calling the ThreadX's initialization routine which executes the main thread that handles file operations.
 At this stage, all FileX resources are created, the MT25TL01G driver is initialized and a single thread is created:
 
  - fx_thread (Prio : 10; PreemptionPrio : 10) used for file operations.

The fx_thread will start by formatting the NOR Flash using FileX services. The resulting file system is a FAT32 compatible, with 512 bytes per sector and 8 sectors per cluster.
Optionally, the NOR flash can be erased prior to format, this allows LevelX and FileX to create a clean FAT FileSystem. Chip erase operation takes considerable time to finish whole flash reset, therefore it is disabled by default. 
To enable it, please set the following flags in "lx_stm32_qspi_driver.h":

  - LX_DRIVER_CALLS_QSPI_INIT
  - LX_DRIVER_ERASES_QPSI_AFTER_INIT

Upon successful opening of the flash media, FileX creates a file called "STM32.TXT" into the root directory, then writes into it some dummy data. Then file is re-opened in read only mode and its content is checked.

Through all the steps, FileX/LevelX services are called to print (using USRAT1) the flash size available before and after the example file is written into the flash. The number of occupied sectors is also shown.

#### <b>Expected success behavior</b>

- CM7 core:

      - A file named STM32.TXT should be visible in the root directory of the SD card.
      - A blinking blue LED light marks the success of the file operations.
	  
- CM4 core:

      - Successful operation is marked by a toggeling green LED.
      - Information regarding the total and available size of the flash media is printed to the serial port(USART1).

#### <b> Error behaviors</b>

- CM7 core:

      - On failure, the red LED should start blinking and blue LED is off.
      - Error handler is called at the spot where the error occurred.
	  
- CM4 core:

      - On failure, the green LED is off.
	  - an "Error" message will be printed to the serial port(USART1).

#### <b>Assumptions if any</b>
None

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
      - Please refer to the **AN4839** "Level 1 cache on STM32F7 Series"
 4. The Application was only tested in release mode for the STM32CubeIDE and may encounter runtime issues in debug mode.

#### <b>ThreadX usage hints</b>

 - ThreadX uses the Systick as time base, thus it is mandatory that the HAL uses a separate time base through the TIM IPs.
 - ThreadX is configured with 100 ticks/sec by default, this should be taken into account when using delays or timeouts at application. It is always possible to reconfigure it in the "tx_user.h", the "TX_TIMER_TICKS_PER_SECOND" define,but this should be reflected in "tx_initialize_low_level.s" file too.
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
	  
    + The "tx_initialize_low_level.s" should be also modified to enable the "USE_DYNAMIC_MEMORY_ALLOCATION" flag.
               
               
#### <b>FileX/LevelX usage hints</b>

- FileX sd driver is using SDMMC1 and the DMA, thus only D1 AXI-SRAM memoru (@ 0x24000000) is accessible by the sd driver.
- When calling the fx_media_format() API, it is highly recommended to understand all the parameters used by the API to correctly generate a valid filesystem.
- When calling the fx_media_format() API, NOR sector size is always 512 bytes.
- FileX is using data buffers, passed as arguments to fx_media_open(), fx_media_read() and fx_media_write() API it is recommended that these buffers are multiple of sector size and "32 bytes" aligned to avoid cache maintenance issues.


### <b>Keywords</b>

RTOS, ThreadX, FileX, LevelX, File system, NOR, SDMMC, QSPI, FAT32

### <b>Hardware and Software environment</b>

  - This application runs on STM32H747xx/STM32H757xx devices.
  - This application has been tested with STMicroelectronics STM32H747I-DISCO boards
    and can be easily tailored to any other supported device and development board.

  - This application uses USART1 to display Cortex-M4 logs, the hyperterminal configuration is as follows:
      - BaudRate = 115200 baud
      - Word Length = 8 Bits
      - Stop Bit = 1
      - Parity = None
      - Flow control = None

###  <b>How to use it ?</b>

In order to make the program work, you must do the following :

 - Open your preferred toolchain
 - For each target configuration (STM32H747_DISCO_CM7 and STM32H747_DISCO_CM4):
      - Rebuild all files 
	  - Load your image into target memory.
 - After loading the two images, you have to reset the board in order to boot (Cortex-M7) and CPU2 (Cortex-M4) at once.	 
 - Run the application
 
