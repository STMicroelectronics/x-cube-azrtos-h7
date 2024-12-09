
## <b>Nx_HTTP_SSE_wifi application description</b>

This application provides an example of Azure RTOS NetX/NetXDuo stack usage.

It demonstrates a HTTP server application using the WiFi connectivity on Cypress module.
The IP address of the target board to connect to is displayed on the UART.
The end-user can connect to it through an HTTP client browser.

#### <b>Expected success behavior</b>

This HTTP server response one html page:
 - the page is updated each time **events are sent by the server**, meaning information about the Live thread list.


#### <b>Error behaviors</b>

An error message is displayed on the UART in case there are too many clients trying to connect.
Removing a web page for a client enables a new client to be requested.

#### <b>Assumptions if any</b>

None

#### <b>ThreadX usage hints</b>

- ThreadX uses the Systick as time base, thus it is mandatory that the HAL uses a separate time base through the TIM IPs.
- ThreadX is configured with 1000 ticks/sec, this should be taken into account when using delays or timeouts at application. It is always possible to reconfigure it in the `tx_user.h`, the `TX_TIMER_TICKS_PER_SECOND` define, but this should be reflected in `tx_initialize_low_level.S` file too.
- ThreadX is disabling all interrupts during kernel start-up to avoid any unexpected behavior, therefore all system related calls (HAL, BSP) should be done either at the beginning of the application or inside the thread entry functions.
- ThreadX offers the `tx_application_define()` function, that is automatically called by the tx_kernel_enter() API.
  It is highly recommended to use it to create all applications ThreadX related resources (threads, semaphores, memory pools...) but it should not in any way contain a system API call (HAL or BSP).
- Using dynamic memory allocation requires to apply some changes to the linker file.
  ThreadX needs to pass a pointer to the first free memory location in RAM to the `tx_application_define()` function, using the `first_unused_memory` argument.
  This requires changes in the linker files to expose this memory location.
    - For EWARM add the following section into the .icf file:
     ```
        place in RAM_region    { last section FREE_MEM };
     ```
    - For MDK-ARM:
    either define the RW_IRAM1 region in the ".sct" file
    or modify the line below in `tx_initialize_low_level.S` to match the memory region being used
    ```
        LDR r1, =|Image$$RW_IRAM1$$ZI$$Limit|
    ```
    - For STM32CubeIDE add the following section into the .ld file:
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
    The `._threadx_heap` must be located between the `.bss` and the `._user_heap_stack sections` in the linker script.
    Caution: Make sure that ThreadX does not need more than the provided heap memory (64KBytes in this example).
    Read more in STM32CubeIDE User Guide, chapter: "Linker script".

    - The `tx_initialize_low_level.S` should be also modified to enable the `USE_DYNAMIC_MEMORY_ALLOCATION` compilation flag.

#### <b>NetX Duo usage hints</b>

For more details about the MPU configuration please refer to the [AN4838](https://www.st.com/resource/en/application_note/dm00272912-managing-memory-protection-unit-in-stm32-mcus-stmicroelectronics.pdf)

### <b>Keywords</b>

RTOS, ThreadX, NetXDuo, WIFI, Access Point, server-sent events, microSD


### <b>Hardware and Software environment</b>

- This application runs on STM32H747xx devices.
- This application has been tested with STMicroelectronics STM32H747I-DISCO boards Revision: MB1248-H747I-D02
  and can be easily tailored to any other supported device and development board.

- A daughter board with the WiFi module is to be plugged into the microSD card connector CN12 of the STM32H747I-DISCO board.
- The daughter board that was used is made up of:
  - A Murata uSD-M.2 Adapter Kit rev A (*J1 position 2-3, VBAT from microSD connector*)
  - The 1DX M.2 Module for the Cypress WiFi (CYW4343W)

  > Connect *Pin 20 of STMod+ connector P2 of STM32H747I-DISCO* to the uSD M.2 Adapter (*J9 pin3 WL_REG_ON_HOST*) that supports the Cypress device
    as described with the below pictures:
  
<center>

![picture1](_htmresc/IMG_3601.png "picture1")
<br/><br/>
![picture2](_htmresc/IMG_3602.png "picture2")

</center>


- This application uses USART1 to provide a console for commands, the hyperterminal configuration is as follows:
  - BaudRate = 115200 baud
  - Word Length = 8 Bits
  - Stop Bit = 1
  - Parity = None
  - Flow control = None
  - Line endings set to LF (receive).

 - Remote PC Set-up
   - PC must share the same WiFi network configuration from the server on board, i.e., **"MyHotSpot"** non secure Access Point server with the **192.168.1.1** address.


### <b>How to use it?</b>

In order to make the program work, you must do the following:

- Open your preferred toolchain
- For each target configuration (Nx_HTTP_SSE_wifi_CM4 first then Nx_HTTP_SSE_wifi_CM7) : 
     - Rebuild all files 
     - Load images into target memory
- After loading the two images, you have to reset the board in order to boot (Cortex-M7) and CPU2 (Cortex-M4) at once.
- Run the application with Access Point mode set by default.
