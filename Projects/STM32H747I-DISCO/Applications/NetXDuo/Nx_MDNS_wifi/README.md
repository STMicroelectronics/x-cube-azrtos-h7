
## <b>Nx_MDNS_wifi application description</b>

This application demonstrates mDNS on Cypress module for the STM32H747I board.

It performs mDNS services announce and deannounce functions using the NetxDuo stack.


#### <b>Expected success behavior</b>

- Application announces and deannounces various mDNS services and displays the same on the UART.

#### <b>Error behaviors</b>

None

#### <b>Assumptions if any</b>

- The application is using the DHCP to acquire IP address, thus a DHCP server should be reachable by the board in the LAN used to test the application.


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

RTOS, Network, ThreadX, NetXDuo, mDNS, WiFi, Station mode, microSD


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

 - This application requires a WiFi access point to connect to:
    - With a transparent Internet connectivity: no proxy, no firewall blocking the outgoing traffic.
    - Running a DHCP server delivering the IP and DNS configuration to the board.

 - Development PC with the Bonjour SDK installed and connected to the same WiFi access point as the board.
    - This is used to verify the services being announced/deannounced.


### <b>How to use it ?</b>

In order to make the program work, you must do the following:

- Open your preferred toolchain

- Edit the file `CM7/Core/Inc/cy_wifi_conf.h` to enter the name of your WiFi access point (`WIFI_SSID`) to connect to and its password (`WIFI_PASSWORD`).

- For each target configuration (Nx_MDNS_wifi_CM4 first then Nx_MDNS_wifi_CM7) : 
     - Rebuild all files 
     - Load images into target memory
- After loading the two images, you have to reset the board in order to boot (Cortex-M7) and CPU2 (Cortex-M4) at once.

- Enter the following commands in a Command Prompt window (you may open multiple windows for each command) to verify
  the services being announced/deannounced:

  - `C:\Windows\System32> dns-sd.exe -L test_announce _test_st local`

  - `C:\Windows\System32> dns-sd.exe -L test_announce1 _test_st local`

  - `C:\Windows\System32> dns-sd.exe -L test_announce2 _test_st local`

  - `C:\Windows\System32> dns-sd.exe -L test_announce3 _test_st local`

  - `C:\Windows\System32> dns-sd.exe -B _test_st local`

- Run the application

- One can also ping the device using the hostname to verify the automatic IPv4 address resolved by mDNS:

  - `C:\Windows\System32> ping.exe mDNS-HOST.local`

- Application announces and deannounces various mDNS services and displays the same on the UART.
