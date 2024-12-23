
## <b>Tx_LowPower application description</b>

This application provides an example of Azure RTOS ThreadX stack usage, it shows how to develop an application using ThreadX LowPower feature.
It demonstrates how to configure the LowPower feature of Azure RTOS ThreadX stack. In addition, it shows how to use ThreadX MACRO related to the LowPower feature.
The main entry function tx_application_define() is then called by ThreadX during kernel start, at this stage, the application creates 1 thread and 1 semaphore:

  - MainThread (Prio : 10; Preemption Threshold : 10)
  - Semaphore (Initial count = 0 to prevent its acquisition by the MainThread, on the application start)

- 'MainThread':
  + Main thread tries to acquire the 'Semaphore'.
  + On success toggles the 'LED_GREEN' each 500 ms for 5 secs.
  + On failure it switches to supended mode waiting for the 'Semaphore' to be released.
  + The steps above are repeated indefinitely.

When the "MainThread" is in suspended state, waiting for the 'Semaphore', the system is in idle state and the scheduler decides to enter in low power mode
by calling the TX_LOW_POWER_USER_ENTER that makes the system enter the stop mode via the HAL_PWR_EnterSTOPMode() API.

As soon as the user button is pressed, the 'Semaphore' is released, and the 'MainThread' passes in Active state.
The scheduler exits the stop mode by calling the TX_LOW_POWER_EXIT.
In system STOP mode, all I/O pins keep the same state as in Run mode.
When exiting from STOP mode, the system clock, D3 domain bus matrix clocks and voltage scaling are reset, so they should be reconfigured.

####  <b>Expected success behavior</b>

  - LED_RED is on when system is in LowPower mode.
  - LED_GREEN toggles every 500ms for 5 seconds each time user presses the user button.

#### <b>Error behaviors</b>

  - LED_RED toggles every 1 second if any error occurs.

#### <b>Assumptions if any</b>

None

#### <b>Known limitations</b>

None

### <b>Notes</b>

1. <b>tx_low_power_enter()</b> service enters low power mode.
For keeping track of time while in low power mode, there are two possibilities:

    1.1. A ThreadX timer is active: Function **tx_timer_get_next** returns **TX_TRUE**.
         Note that in this situation, a low power clock must be used in order to wake up the CPU for the next ThreadX timer expiration.
         Therefore an alternative clock must be programmed. Program the hardware timer source such that the next timer interrupt is equal to: *tx_low_power_next_expiration* tick_frequency.
         The *tick_frequency* is application-specific and typically set up in **tx_low_level_initialize**.

    1.2. There are no ThreadX timers active: Function **tx_timer_get_next** returns **TX_FALSE**.
       1.2.1. The application may choose not to keep the ThreadX internal tick count updated (define **TX_LOW_POWER_TICKLESS**),
              therefore there is no need to set up a low power clock.
       1.2.2. The application still needs to keep the ThreadX tick up-to-date. In this case a low power clock needs to be set up.

2. Under [AzureRTOS Application/ThreadX App Init] in STM32CubeMX, thread entry function name, thread name, priority and stack size, semaphore name and semaphore init count are defined.

3. Entering low-power mode through WFI will be executed only if no interrupt is pending.

4. The 'debug in stop mode' feature should be disabled using the HAL_DBGMCU_DisableDBGStopMode() API, otherwise, it will generate interrupts that cause the wake-up of the system immediately after entering the STOP mode.

5. The user push-button interrupt priority must be set to the highest possible value, otherwise, after Exit STOP mode the processor will not execute the interrupt handler.

6. To measure the current consumption remove JP4 jumper and connect an amperemeter to JP4 to measure IDD current.

#### <b>ThreadX usage hints</b>

 - ThreadX uses the Systick as time base, thus it is mandatory that the HAL uses a separate time base through the TIM IPs.
 - ThreadX is configured with 100 ticks/sec by default, this should be taken into account when using delays or timeouts at application.
   It is always possible to reconfigure it, by updating the "TX_TIMER_TICKS_PER_SECOND" define in the "tx_user.h" file. The update should be reflected in "tx_initialize_low_level.S" file too.
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

RTOS, ThreadX, Threading, Semaphore, LowPower

### <b>Hardware and Software environment</b>

  - This example runs on STM32H723xx devices.
  - This example has been tested with STMicroelectronics NUCLEO-H723ZG boards revision: MB1364-H723ZG-E01
    and can be easily tailored to any other supported device and development board.

###  <b>How to use it ?</b>

In order to make the program work, you must do the following :

  - Open your preferred toolchain
  - Rebuild all files and load your image into target memory
  - Run the application
