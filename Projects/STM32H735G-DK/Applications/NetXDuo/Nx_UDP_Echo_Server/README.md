
##  Nx_UDP_Echo_Server application description

This application provides an example of Azure RTOS NetX/NetXDuo stack usage .
It shows how to develop a NetX UDP server to communicate with a a remote client using
the NetX UDP socket API.

The main entry function tx_application_define() is called by ThreadX during kernel start, at this stage, all NetX resources are created.

 + A _NX_PACKET_POOL_ is allocated

 + A _NX\_IP_ instance using that pool is initialized

 + The _ARP_, _ICMP_, _UDP_ and protocols are enabled for the _NX\_IP_ instance

 + A DHCP client is created.

The application then creates 2 threads with the same priorities:

 + **AppMainThread** (priority 10, PreemtionThreashold 10) : created with the _TX_AUTO_START_ flag to start automatically.

 + **AppUDPThread** (priority 10, PreemtionThreashold 10) : created with the _TX_DONT_START_ flag to be started later.

The **AppMainThread** starts and perform the following actions:
  + starts the DHCP client

  + waits for the IP address resolution

  + resumes the **AppUDPThread**

The **AppUDPThread**, once started:

  + creates an _UDP_ server socket

  + listen indefinitely on new client connections.

  + As soon as a new connection is established, the _UDP_ server socket starts receiving data packets from the client.

  + At each received message the server:
  
      + extracts the Client IP address and remote port
      + retrieve the data from the received packet
      + prints the info above on the HyperTerminal
      
  + Once the data exchange is completed the _UDP_ server enters in an idle state an toggles the green led.
      

####  Expected success behavior
 + The board IP address is printed on the HyperTerminal
 + The response messages sent by the server are printed on the HyerTerminal
 + if the [echotool](https://github.com/PavelBansky/EchoTool/releases/tag/v1.5.0.0) utility  is used messages similar to the shown below can be seen on the console:

```
Reply from 192.168.1.2:6000, time 47 ms OK
Reply from 192.168.1.2:6000, time 42 ms OK
Reply from 192.168.1.2:6000, time 44 ms OK
Reply from 192.168.1.2:6000, time 46 ms OK
Reply from 192.168.1.2:6000, time 47 ms OK

```

#### Error behaviors
+ the Yellow LED is toggling to indicate any error that have occurred.
+ In case the message exchange is not completed the Hyperterminal is not printing the received messages.

#### Assumptions if any
- The Application is using the DHCP to acquire IP address, thus a DHCP server should be reachable by the board in the LAN used to test the application.
- The application does not support Ethernet cable hot-plug, therefore the board should be connected to the

  LAN before running the application.

- The application is configuring the Ethernet IP with a static predefined _MAC Address_, make sure to change it in case multiple boards are connected on the same LAN to avoid any potential network traffic issues.

- The _MAC Address_ is defined in the `main.c`

```
void MX_ETH_Init(void)
{

  /* USER CODE BEGIN ETH_Init 0 */

  /* USER CODE END ETH_Init 0 */

  /* USER CODE BEGIN ETH_Init 1 */

  /* USER CODE END ETH_Init 1 */
  heth.Instance = ETH;
  heth.Init.MACAddr[0] =   0x00;
  heth.Init.MACAddr[1] =   0x11;
  heth.Init.MACAddr[2] =   0x83;
  heth.Init.MACAddr[3] =   0x45;
  heth.Init.MACAddr[4] =   0x26;
  heth.Init.MACAddr[5] =   0x20;
```
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

#### NetX Duo usage hints
- The ETH TX And RX descriptors are accessed by the CPU and the ETH DMA IP, thus they should not be allocated into the DTCM RAM "0x20000000".
- Make sure to allocate them into a "Non-Cacheable" memory region to always ensure data coherency between the CPU and ETH DMA.
- Depending on the application scenario, the total TX and RX descriptors may need to be increased by updating respectively  the "ETH_TX_DESC_CNT" and "ETH_RX_DESC_CNT" in the "stm32h7xx_hal_conf.h", to guarantee the application correct behavior, but this will cost extra memory to allocate.
- The NetXDuo driver has an internal data structure that needs to be located into a non cacheable area called '**NxDriverSection**'. Therefore the application should.
 + Declare the aforementioned section in the linker file as below
   + For EWARM ".icf" file
   ```
   define symbol __ICFEDIT_region_NXDATA_start__  = 0x24018200;
   define symbol __ICFEDIT_region_NXDATA_end__   = 0x24027FFF;
   define region ITCMRAM_region  = mem:[from __ICFEDIT_region_ITCMRAM_start__ to __ICFEDIT_region_ITCMRAM_end__];
   place in NXDriver_region { section NXDriverSection};
   ```
   + For STM32CubeIDE ".ld" file
   ```
   .nx_section 0x24018200 (NOLOAD): {
     *(.NXDriverSection)
     } >RAM_D1
   ```
  + Configure the MPU for the following memory regions:
   + **64KB** as non-cacheable starting from the address __ICFEDIT_region_NXDATA_start__ - 0x200
   + **256Bytes** as Device memory at the beginning of these **64KB**

   the region configured as Device memory should  contain the ETH DMA RX and TX descriptors.


### Keywords

RTOS, Network, ThreadX, NetXDuo, UDP, UART

### Hardware and Software environment

  - This application runs on STM32H735xx devices.
  - This application has been tested with STMicroelectronics STM32H735G-DK boards Revision: MB1520-H735I-B02
    and can be easily tailored to any other supported device and development board.

  - This application uses USART3 to display logs, the hyperterminal configuration is as follows:
      - BaudRate = 9600 baud
      - Word Length = 8 Bits
      - Stop Bit = 1
      - Parity = odd
      - Flow control = None

###  How to use it ?

In order to make the program work, you must do the following :
 - Open your preferred toolchain
 - Optionally edit the file <code> NetXDuo/App/app_netxduo.h</code> and update the _DEFAULT\_PORT_ to connect on.

 - run the [echotool](https://github.com/PavelBansky/EchoTool/releases/tag/v1.5.0.0) utility on a windows console as following:

   <code> c:\\> echotool.exe  <board IP address> /p udp  /r  _DEFAULT\_PORT_ /n 10 /d "Hello World" </code>

 - Rebuild all files and load your image into target memory
 - Run the application
