
## <b>Tx_Thread_Creation application description</b>

This application provides an example of Azure RTOS ThreadX stack usage, it shows how to develop an application using the ThreadX thread management APIs.
It demonstrates how to create and destroy multiple threads using Azure RTOS ThreadX APIs. In addition, it shows how to use preemption threshold between threads and change priorities on-fly.
The main entry function tx_application_define() is then called by ThreadX during kernel start, at this stage, the application creates 3 threads with different priorities:

  - MainThread (Prio : 5; Preemption Threshold : 5)
  - ThreadOne  (Prio : 10; Preemption Threshold : 9)
  - ThreadTwo  (Prio : 10; Preemption Threshold : 10)

Once started, the <b>MainThread</b> is suspended waiting for the event flag.
The *ThreadOne* starts to toggle the *LED_GREEN* each 500ms and <b>ThreadTwo</b> cannot as its priority is less than the *ThreadOne* threshold.
After 5 seconds it sends an event *THREAD_ONE_EVT* to the <b>MainThread</b>.

After receiving the *THREAD_ONE_EVT*, the <b>MainThread</b> changes the <b>ThreadTwo</b> priority to 8 and its preemption threshold to 8 to be more than the *ThreadOne* threshold then waits for an event.

Now, the <b>ThreadTwo</b> can preempt the <b>ThreadOne</b> and start toggling the *LED_GREEN* each 200ms for 5 seconds. Once done it sends the <b>THREAD_TWO_EVT</b> to <b>MainThread</b>.
Once *ThreadTwo_Evt* is received, the <b>MainThread</b> resets the <b>ThreadTwo</b> priority and preemption threshold to their original values (10, 10), <b>ThreadOne</b> is rescheduled and the above scenario is redone.
After repeating the sequence above 3 times, the <b>MainThread</b> should destroy <b>ThreadOne</b> and <b>ThreadTwo</b> and toggles the *LED_GREEN* each 1 second for ever.

####  <b>Expected success behavior</b>

  - LED_GREEN toggles every 500ms for 5 seconds
  - LED_GREEN toggles every 200ms for 5 seconds
  - Success status (After 3 times) : 'LED_GREEN' toggles every 1 second forever.

#### <b>Error behaviors</b>

LED_RED toggles every 1 second if an error occurs.
An error message is printed to the serial port.

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

### <b>Keywords</b>

RTOS, ThreadX, Threading, Event flags, Preemption threshold

### <b>Hardware and Software environment</b>

  - This example runs on STM32H723xx devices.
  - This example has been tested with STMicroelectronics NUCLEO-H723ZG boards revision MB1364-H723ZG-E01
    and can be easily tailored to any other supported device and development board.
  - This application uses USART3 to display logs, the hyperterminal configuration is as follows:
      - BaudRate = 115200 baud
      - Word Length = 8 Bits
      - Stop Bit = 1
      - Parity = None
      - Flow control = None

###  <b>How to use it ?</b>

In order to make the program work, you must do the following :

 - Open your preferred toolchain
 - Rebuild all files and load your image into target memory
 - Run the application
