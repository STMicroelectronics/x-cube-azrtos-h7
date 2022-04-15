
## <b>Nx_IAP_Client_wifi application description</b>

This application provides an example of Azure RTOS NetX/NetXDuo stack usage.

It shows how to use the WiFi module to perform an IAP ("In Application Programming") using STM32 Cube HAL.

This application may be used with a Smartphone or PC server (more detailed setup instructions are described in section "Hardware and Software environment").



#### <b>Expected success behavior</b>

- If the "USER" Button is pressed when the application starts, then the WiFi IAP module is started. Otherwise, the user code downloaded into Flash offset is executed.
  If no code is downloaded yet into Flash memory offset, then the WiFi IAP application will start.

- At startup, the WiFi module is initialized, the UART terminal shows an `O.K!` message in case of success.

- Once initialized, the board attempts to join WiFi access point using the predefined SSID and
  password setup by user into configuration file at compilation time.
  In case of success the acquired IP address is displayed.

- After joining the access point, the board attempts to establish a connection to the
  remote HTTP server using the predefined address and port setup by user in configuration file.

- As soon as the connection to the remote server is established, the "hello.bin" file is downloaded.
  On success the following messages is displayed on the UART:

  `Downloaded firmware ' hello_h747_CM7.bin ', size = 30156 Bytes`

- When downloading is finished, the message `State: Programming...` is displayed on the UART.
  The "hello_xxx.bin" fw is being written into the flash.
  On Success the message `Programming Done!` is displayed on the UART and green LED is blinking.

- Reset the board (using "RESET" button) and keep "USER" button unpressed in order to execute
  the binary file downloaded into Flash memory.

#### <b>Error behaviors</b>

- Failure message is shown and the red LED is toggling to indicate the download failed.

#### <b>Assumptions if any</b>

- The Application is using the DHCP to acquire IP address, thus a DHCP server should be reachable by the board in the LAN used to test the application.


It is possible to configure a PC to run as a WiFi hotspot alongside a webserver.
For Windows PC you can use the [Wamp server](http://www.wampserver.com/en)

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

RTOS, Network, ThreadX, NetXDuo, WiFi, Hotspot, IAP Client, HTTP server, microSD

### <b>Hardware and Software environment</b>

- This application runs on STM32H747xx devices.
- This application has been tested with STMicroelectronics STM32H747I-DISCO boards Revision: MB1248-H747I-D02
  and can be easily tailored to any other supported device and development board.

- A daughter board with the WiFi module is to be plugged into the microSD card connector CN12 of the STM32H747I-DISCO board.
- The daugtherboard that was used is made up of:
  - A Murata uSD-M.2 Adapter Kit rev A (*J1 position 2-3, VBAT from microSD connector*)
  - The 1DX M.2 Module for the Cypress WiFi (CYW4343W)

> Connect *Pin 20 of STMod+ connector P2 of STM32H747I-DISCO* to the uSD M.2 Adapter (*J9 pin3 WL_REG_ON_HOST*) that supports the Cypress device
  as described with the below pictures:
  
<center>

![picture1](_htmresc/IMG_3601.png "picture1")
<br/><br/>
![picture2](_htmresc/IMG_3602.png "picture2")

</center>

- This application uses USART1 to display logs, the hyperterminal configuration is as follows:
  - BaudRate = 115200 baud
  - Word Length = 8 Bits
  - Stop Bit = 1
  - Parity = None
  - Flow control = None
  - Line endings set to LF (receive).

- A remote host running a web server (either Linux or Windows PC).
> The server listening port should be set according to the value of the `HOST_PORT` used to build the application.

> The access right on the server must be defined to allow the server to provide the binary requested by the application running on the device.

- When using Linux or Windows PC:
  - Start the Apache or Wamp server respectively on the Linux or Windows machine.
  - Copy the "hello_h747_CM7.bin" file under an accessible path on the webserver
  - The PC should be connected to the local network and accessible via the WiFi access point.

### <b>How to use it ?</b>

In order to make the program work, you must do the following:

- Open your preferred toolchain
- Edit the file `CM7/Core/Inc/cy_wifi_conf.h` to enter the name of your WiFi access point (`WIFI_SSID`) to connect to and its password (`WIFI_PASSWORD`).
- Edit the file `CM7/Core/Inc/main.h` and adjust the defines:

  `HOST_ADDRESS`    The IP Address of the PC or phone running the web server.

  `HOST_PORT`       The HTTP port used by the web server. When using Palpa WebServer on a mobile phone,
                    it is possible to edit the 'webserver root dir' under (WebServer Settings->Components->lighttpd->Port)

  `FW_FILE`         The relative path of the "hello_h747_CM7.bin" w.r.t the web server root dir. When using Palpa WebServer on a mobile phone,
                    it is possible to edit the 'webserver root dir' under (WebServer Settings->Components->lighttpd->Document Root)

  `DATA_MAX_SIZE`   Data size required to hold the "hello_h747_CM7.bin". (should take into account HTTP overhead size)

- For each target configuration (Nx_IAP_Client_wifi_CM4 first then Nx_IAP_Client_wifi_CM7) : 
     - Rebuild all files 
     - Load images into target memory
- After loading the two images, you have to reset the board in order to boot (Cortex-M7) and CPU2 (Cortex-M4) at once.
- Run the application


#### <b>Notes</b>

To be correctly written into the Flash the fw binary maximum size can't exceed 256 kb.
