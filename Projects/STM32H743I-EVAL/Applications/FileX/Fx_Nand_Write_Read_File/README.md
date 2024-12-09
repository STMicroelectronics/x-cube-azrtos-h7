
## <b>Fx_NAND_Write_Read_File application description</b>

This application provides an example of Azure RTOS FileX and LevelX stacks usage on custom
board(doesn't run on STM32H743I-EVAL board unless a custom board is added). It demonstrates how to create a Fat File system
on the NAND flash using FileX alongside LevelX. The application is designed to execute file operations on the Micron MT29F2G16ABAEAWP
NAND flash device, the code provides all required software code for properly managing it.

 - This application has been tested using an internal custom test board embedding a Micron MT29F2G16ABAEAWP NAND flash device configured as below:

      - MemoryDataWidth = 16 bit
      - PageSize = 1024 words
      - SpareAreaSize = 32 words
      - BlockSize = 64 pages
      - BlockNbr = 2048
      - PlaneSize = 2048 (NAND memory plane size is measured in number of blocks )
      - PlaneNbr = 1

  - User should have attention when configuring timings for the NAND memory. for more details about the timings please refer to AN4761.

  - The timings are configured the same for the ComSpaceTiming and AttSpaceTiming as below:
      - SetupTime = 1
      - WaitSetupTime = 7
      - HoldSetupTime = 2
      - HiZSetupTime = 8

  - User can design his own hardware according to the following FMC GPIO configuration:

      - PG9   ------> FMC_NCE
      - PD0   ------> FMC_D2
      - PD1   ------> FMC_D3
      - PD4   ------> FMC_NOE
      - PD5   ------> FMC_NWE
      - PD6   ------> FMC_NWAIT
      - PD8   ------> FMC_D13
      - PD9   ------> FMC_D14
      - PD10  ------> FMC_D15
      - PD11  ------> FMC_CLE (FMC_A16)
      - PD12  ------> FMC_ALE (FMC_A17)
      - PD13  ------> FMC_A18
      - PD14  ------> FMC_D0
      - PD15  ------> FMC_D1
      - PE7   ------> FMC_D4
      - PE8   ------> FMC_D5
      - PE9   ------> FMC_D6
      - PE10  ------> FMC_D7
      - PE11  ------> FMC_D8
      - PE12  ------> FMC_D9
      - PE13  ------> FMC_D10
      - PE14  ------> FMC_D11
      - PE15  ------> FMC_D12

The application starts by calling the ThreadX's initialization routine which executes the main thread that handles file operations.
At this stage, all FileX resources are created, the NAND flash is initialized and a single thread is created:

  - fx_app_thread (Prio : 1; PreemptionPrio : 1) used for file operations.

The fx_app_thread will start by formatting the NAND Flash using FileX services. The resulting file system is a FAT32 compatible, with 2048 bytes per sector and 1 sector per cluster.
In case FAT FileSystem is not created, please check the following flag in "fx_stm32_levelx_nand_driver.h" is enabled:

  - FX_NAND_FORMAT_FLASH_BEFORE_OPEN

Optionally, the NAND flash can be erased prior to format, this allows LevelX and FileX to create a clean FAT FileSystem. 
To enable flash mass erase, please set the following flag in "lx_stm32_nand_custom_driver.h":

  - LX_DRIVER_ERASES_FLASH_AFTER_INIT

Note: When formatting the NAND Flash , sector size parameter should always equal the page size of the underlying NAND hardware.

Upon successful opening of the flash media, FileX continue with creating a file called "STM32.TXT" into the root directory, then write into it some dummy data. Then file is re-opened in read only mode and content is checked.

Through all the steps, FileX/LevelX services are called to print the flash size available before and after the example file is written into the flash. The number of occupied sectors is also shown.

#### <b>Expected success behavior</b>

Successful operation is marked by a toggling green LED light.
Also, information regarding the total and available size of the flash media is printed to the serial port.

#### <b> Error behaviors</b>

On failure, the red LED starts toggling while the green LED is switched OFF.

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
      - Please refer to the **AN4839** "Level 1 cache on STM32F7 and STM32H7 Series"

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


#### <b>FileX/LevelX usage hints</b>

- FileX sd driver is using the DMA, thus the DTCM (0x20000000) memory should not be used by the application, as it is not accessible by the SD DMA.
- When calling the fx_media_format() API, it is highly recommended to understand all the parameters used by the API to correctly generate a valid filesystem.
- When calling the fx_media_format() API, total sectors number should be one full block size of sectors less than the NAND flash provides. This will help ensure best performance during the wear level processing.
- FileX is using data buffers, passed as arguments to fx_media_open(), fx_media_read() and fx_media_write() API it is recommended that these buffers are multiple of sector size and "32 bytes" aligned to avoid cache maintenance issues.


### <b>Keywords</b>

RTOS, ThreadX, FileX, LevelX, File System, NAND, FMC, FAT32

### <b>Hardware and Software environment</b>

  - This application runs on STM32H743xx devices.
  - This application has been tested using an internal custom test board embedding a Micron MT29F2G16ABAEAWP NAND flash device and plugged on
    STM32H743I-EVAL boards Revision: MB1246-B03 and can be easily tailored to any other supported device and development board.

  - This application uses USART1 to display logs, the hyperterminal configuration is as follows:
      - BaudRate = 115200 baud
      - Word Length = 8 Bits
      - Stop Bit = 1
      - Parity = None
      - Flow control = None


### <b>How to use it ?</b>

In order to make the program work, you must do the following :

 - Open your preferred toolchain
 - Rebuild all files and load your image into target memory
 - Run the application
