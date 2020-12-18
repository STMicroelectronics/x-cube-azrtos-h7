
##  Tx_Thread_Sync application description

This application provides an example of Azure RTOS ThreadX stack usage, it shows how to develop an application using the ThreadX synchronization APIs.
The main entry function tx_application_define() is called by ThreadX during kernel start, at this stage, the application creates 2 threads with the same priorities :
  - 'ThreadOne' (Priority : 10; Preemption Threshold : 10)
  - 'ThreadTwo' (Priority : 10; Preemption Threshold : 10)

The function "Led_Toggle()" is the entry function for both threads to toggle the leds.Therefore it is considered as a "critical section" that needs protection with a 'SyncObject' flag in the file "app_threadx.h"
Each thread is running in an infinite loop as following:
- 'ThreadOne':
  + try to acquire the 'SyncObject' immediately.
  + On Success toggle the 'LED_GREEN' each 500ms for 5 seconds.
  + Release the 'SyncObject'
  + Sleep for 10ms.
  + repeat the steps above

- 'ThreadTwo':
  + try to acquire the 'SyncObject' immediately.
  + On Success toggle the 'LED_RED' each 500ms for 5 seconds.
  + Release the 'SyncObject'
  + Sleep for 10ms.
  + Repeat the steps above.

By default the 'SyncObject' is defined as  "TX_MUTEX" .It is possible to use a binany "TX_SEMAPHORE" by tuning
the compile flags in the file "app_threadx.h".

####  Expected success behavior
  - 'LED_GREEN' toggles every 500ms for 5 seconds
  - 'LED_RED' toggles every 500ms for 5 seconds
  - Messages on HyperTerminal : 
     + "** ThreadXXX : waiting for SyncObject !! **" : When thread is waiting for the SyncObject.
	 + "** ThreadXXX : waiting SyncObject released **" : When thread put the SyncObject.
	 + "** ThreadXXX : waiting SyncObject acquired **" : When thread get the SyncObject.
	 
#### Error behaviors
'LED_RED' toggles every 1 second if any error occurs.

#### Assumptions if any
None

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
               

### Keywords

RTOS, ThreadX, Thread, Semaphore, Mutex


### Hardware and Software environment

  - This example runs on STM32H735xx devices
  - This example has been tested with STMicroelectronics STM32H735G-DK boards Revision: MB1520-H735I-B02.
    and can be easily tailored to any other supported device and development board.
  - A virtual COM port appears in the HyperTerminal:
      - Hyperterminal configuration:
        + Data Length = 8 Bits
        + One Stop Bit
        + No parity
        + BaudRate = 115200 baud
        + Flow control: None

###  How to use it ?

In order to make the program work, you must do the following :
 - Open your preferred toolchain
 - Rebuild all files and load your image into target memory
 - Run the application

