
## <b>IAP_main application description</b>

This application provides an example of Azure RTOS FileX stack usage on STM32H735G-DK board, it implements an In-Application programming (IAP) demonstrating FileX's SD file access capabilities. 
The application is designed to erase and write to on-chip flash memory, it provides all required software code for handling SD card and flash memory I/O operations.

This is a typical application on how to use the STM32H735 SD card peripheral for firmware upgrade application or IAP, allowing user to erase and write to on-chip flash memory.

The application starts by checking the state of the user button (BUTTON_USER), and depending on its state the application will enter one of the two startup sequences:


#### <b>Programming new software sequence</b>

If the user button is pressed at application start-up, the program try to flash a new software, loaded from the SD card, to the flash memory using **fx_thread** (Priority : 10; Preemption Priority : 10) as below:

  - initialize the SD card driver
  - open the SD card media and look for the executable to be loaded which should be named as defined by **FW_NAME_STRING** located in "FileX/FX_IAP/IAP_main/FileX/App/app_filex.h" 
  - go over the file and program it into flash, 32 bytes a time
  - upon completion, the Application will enter an infinite loop toggling the green LED, marking the success of the flashing operation.

<b> Notes:</b>

  - The flash address for programming is defined by the flag **APP_ADDRESS** (by default 0x08020000).
  - A power cycle or depressing the reset button is required in order to exit the programming mode.


#### <b>Loading the new software sequence</b>

If the user button is found to be not pressed at application start-up, the program will try to load the previously programmed application from the address defined by **APP_ADDRESS**.

For that the steps below will be followed:

  - signature of the to-be loaded application is checked prior of its starting to check if the it was already programmed starting at the previously mentioned address
  - call to the function **CallUserApp** to de-initialize all the previously initialized resources in the IAP application
  - disables all the active interrupts and clear any pending ones. This is in order to pass control over to the loaded application in a state similar to a Power On Reset (POR)
  - vector table and Stack pointer are set to point to the ones defined by the loaded application
  - the Program Counter is also updated
  - the call to the loaded application is made
  - control is then handed over to the new application which should start.

<b>Note</b>:

  - A FAT32 compatible SD card is expected to be used in this example.
  - Firmware of the loaded application must be located at the root of A FAT32 compatible SD card, the format must be raw binary.
  - The program will start file operations without formatting the media, so all user related files are kept.


### <b>Expected success behavior</b>

In programming new software sequence, success is marked by a blinking green LED.
In the loading new software sequence, the loaded application should start and run as expected.


### <b>Error behaviors</b>

On failure, the red LED starts toggeling while the green LED is switched OFF.


### <b>Assumptions if any</b>

The SD card is expected to be inserted before application start (only in programming new software sequence).
The loaded application must be placed in the root directory of the SD card as a raw binary.


### <b>Known limitations</b>

The value defined for **USE_SD_TRANSCEIVER** need to be set to 0U in the HAL system configuration section in the header file "FileX/FX_IAP/IAP_main/Core/Inc/stm32h7xx_hal_conf.h".

#### <b>Loaded-App requirements</b>

The loaded-App should be configured to start from an offset into the flash that does not overlap with the IAP application memory sections.
Particularly, linker options should be changed to set the **Vector Table** and the **ROM START** both pointing to **APP_ADDRESS**.

Upon startup, the loaded-App will set the VTOR register with its Interrupt Vector Table starting address, so offset should be taken into account. 
This can be achieved by setting the offset to the defined name **VECT_TABLE_OFFSET** located in file **system_stm32h7xx.c**.

The Loaded-App must be generated as raw binary, this can be achieved by setting the output format of the IDE to **Raw binary**.
The name for the binary should also be specified there as defined by **FW_NAME_STRING** located in "FileX/FX_IAP/IAP_main/FileX/App/app_filex.h".

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

- FileX SD driver is using the DMA, thus the DTCM (0x20000000) memory should not be used by the application, as it is not accessible by the SD DMA.
- FileX is using data buffers, passed as arguments to fx_media_open(), fx_media_read() and fx_media_write() API it is recommended that these buffers are multiple of sector size and "32 bytes" aligned to avoid cache maintenance issues.

#### <b>Debugging</b>

While in loading new software sequence, the loaded-App can be debugged using IAR by going into **Project** menu and click **Attach to Running Target**.

### <b>Keywords</b>

RTOS, ThreadX, FileX, File system, SDMMC, SDIO, FAT32


### <b>Hardware and Software environment</b>

  - This example runs on STM32H735xx devices.
  - This example has been tested with STMicroelectronics STM32H735G-DK boards Revision: MB1520-H735I-B02 and can be easily tailored to any other supported device and development board.
  - This application uses USART3 to display logs, the hyperterminal configuration is as follows:
      - BaudRate = 115200 baud
      - Word Length = 8 Bits
      - Stop Bit = 1
      - Parity = none
      - Flow control = None

###  <b>How to use it ?</b>

In order to make the program work, you must do the following:

  - Open your preferred toolchain
  - Rebuild all files and load your image into target memory
  - Run the application
