
## <b>Nx_UDP_Echo_Server application description</b>

This application provides an example of Azure RTOS NetX/NetXDuo stack usage .
It shows how to develop a NetX UDP server to communicate with a remote client using the NetX UDP socket API.

The main entry function tx_application_define() is called by ThreadX during kernel start, at this stage, all NetX resources are created.

 + A NX_PACKET_POOL_is allocated
 + A NX_IP instance using that pool is initialized
 + The ARP, ICMP, UDP and protocols are enabled for the NX_IP instance
 + A DHCP client is created.

The application then creates 2 threads with the same priorities:

 + **NxAppThread** (priority 10, PreemtionThreashold 10) : created with the TX_AUTO_START flag to start automatically.
 + **AppUDPThread** (priority 10, PreemtionThreashold 10) : created with the TX_DONT_START flag to be started later.

The **NxAppThread** starts and performs the following actions:

  + Starts the DHCP client
  + Waits for the IP address resolution
  + Resumes the **AppUDPThread**

The **AppUDPThread**, once started:

  + Creates an UDP server socket.
  + Listens indefinitely on new client connections.
  + As soon as a new connection is established, the green LED starts toggling and the UDP server socket starts receiving data packets from the client.
  + At each received message the server:
      + Extracts the Client IP address and remote port
      + Retrieves the data from the received packet
      + Prints the info above on the HyperTerminal
  + Once the data exchange is completed the UDP server enters an idle state and the green LED keeps on toggling.

####  <b>Expected success behavior</b>

 + The board IP address is printed on the HyperTerminal
 + The response messages sent by the server are printed on the HyerTerminal
 + If the [echotool](https://github.com/PavelBansky/EchoTool/releases/tag/v1.5.0.0) utility is used, messages similar to the shown below can be seen on the console:

```
Reply from 192.168.1.2:6000, time 47 ms OK
Reply from 192.168.1.2:6000, time 42 ms OK
Reply from 192.168.1.2:6000, time 44 ms OK
Reply from 192.168.1.2:6000, time 46 ms OK
Reply from 192.168.1.2:6000, time 47 ms OK

```

#### <b>Error behaviors</b>

+ the red LED is toggling to indicate any error that have occurred.
+ In case the message exchange is not completed the Hyperterminal is not printing the received messages.

#### <b>Assumptions if any</b>

- The Application is using the DHCP to acquire IP address, thus a DHCP server should be reachable by the board in the LAN used to test the application.
- The application does not support Ethernet cable hot-plug, therefore the board should be connected to the LAN before running the application.
- The application is configuring the Ethernet IP with a static predefined MAC Address, make sure to change it in case multiple boards are connected on
  the same LAN to avoid any potential network traffic issues.
- The MAC Address is defined in the `main.c`

```
void MX_ETH_Init(void)
{

  /* USER CODE BEGIN ETH_Init 0 */

  /* USER CODE END ETH_Init 0 */

  /* USER CODE BEGIN ETH_Init 1 */

  /* USER CODE END ETH_Init 1 */
  heth.Instance = ETH;
  heth.Init.MACAddr[0] =   0x00;
  heth.Init.MACAddr[1] =   0x80;
  heth.Init.MACAddr[2] =   0xE1;
  heth.Init.MACAddr[3] =   0x00;
  heth.Init.MACAddr[4] =   0x30;
  heth.Init.MACAddr[5] =   0x20;
```

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

#### <b>NetX Duo usage hints</b>

- The ETH TX And RX descriptors are accessed by the CPU and the ETH DMA IP, thus they should not be allocated into the DTCM RAM "0x20000000".
- Make sure to allocate them into a "Non-Cacheable" memory region to always ensure data coherency between the CPU and ETH DMA.
- Depending on the application scenario, the total TX and RX descriptors may need to be increased by updating respectively the "ETH_TX_DESC_CNT" and "ETH_RX_DESC_CNT" in the "stm32h7xx_hal_conf.h", to guarantee the application's correct behavior, but this will cost extra memory to allocate.
- The NetXDuo application needs to allocate the <b> <i> NX_PACKET </i> </b> pool in a dedicated section that is  configured as either "Cacheable Write-through" for <i>STM32H72XX</i> and <i>STM32H73XX </i>,  <i>STM32H7AXX</i> and <i>STM32H7BXX </i> or non-cacheable for other STM32H7 families. Below is an example of the section declaration for different IDEs.
   + For EWARM ".icf" file
   ```
   define symbol __ICFEDIT_region_NXDATA_start__  = 0x24032100;
   define symbol __ICFEDIT_region_NXDATA_end__   = 0x2404FFFF;
   define region NXApp_region  = mem:[from __ICFEDIT_region_NXDATA_start__ to __ICFEDIT_region_NXDATA_end__];
   place in NXApp_region { section .NetXPoolSection};
   ```
   + For MDK-ARM
   ```
    RW_NXDriverSection 0x24032100 0x7E00  {
  *(.NetXPoolSection)
  }
   ```
   + For STM32CubeIDE ".ld" file
   ```
   .nx_section 0x24032100 (NOLOAD): {
     *(.NetXPoolSection)
     } >RAM_D1
   ```

  This section is then used in the <code> app_azure_rtos.c</code> file to force the <code>nx_byte_pool_buffer</code> allocation.

```
/* USER CODE BEGIN NX_Pool_Buffer */

#if defined ( __ICCARM__ ) /* IAR Compiler */
#pragma location = ".NetXPoolSection"

#elif defined ( __CC_ARM ) || defined(__ARMCC_VERSION) /* ARM Compiler 5/6 */
__attribute__((section(".NetXPoolSection")))

#elif defined ( __GNUC__ ) /* GNU Compiler */
__attribute__((section(".NetXPoolSection")))
#endif

/* USER CODE END NX_Pool_Buffer */
static UCHAR  nx_byte_pool_buffer[NX_APP_MEM_POOL_SIZE];
static TX_BYTE_POOL nx_app_byte_pool;
```
For more details about the MPU configuration please refer to the [AN4838](https://www.st.com/resource/en/application_note/dm00272912-managing-memory-protection-unit-in-stm32-mcus-stmicroelectronics.pdf)

### <b>Keywords</b>

RTOS, Network, ThreadX, NetXDuo, UDP, UART

### <b>Hardware and Software environment</b>

  - This application runs on STM32H735xx devices
  - This application has been tested with STMicroelectronics STM32H735G-DK boards revision MB1520-H735I-B02
    and can be easily tailored to any other supported device and development board.
  - This application uses USART3 to display logs, the hyperterminal configuration is as follows:
      - BaudRate = 115200 baud
      - Word Length = 8 Bits
      - Stop Bit = 1
      - Parity = None
      - Flow control = None

### <b>How to use it ?</b>

In order to make the program work, you must do the following :

 - Open your preferred toolchain
 - Optionally edit the file <code> NetXDuo/App/app_netxduo.h</code> and update the DEFAULT_PORT to connect on.
 - Run the [echotool](https://github.com/PavelBansky/EchoTool/releases/tag/v1.5.0.0) utility on a windows console as following:

       c:\> echotool.exe  <board IP address> /p udp  /r  <DEFAULT_PORT> /n 10 /d "Hello World"

       example :  c:\> echotool.exe 192.168.1.2 /p udp /r 6000 /n 10 /d "Hello World"

 - Rebuild all files and load your image into target memory
 - Run the application
