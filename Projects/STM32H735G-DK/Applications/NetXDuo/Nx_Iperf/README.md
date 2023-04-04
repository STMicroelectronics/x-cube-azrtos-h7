
##  <b>Nx_Iperf application description</b>

This application provides an example of Azure RTOS NetXDuo stack usage .
It shows performance when using different modes : TCP_server, UDP_server, TCP_client and UDP_client.
The main entry function tx_application_define() is then called by ThreadX during kernel start, at this stage, all NetXDuo resources are created.

 + A NX_PACKET_POOL **NxAppPool** is allocated
 + A NX_IP instance **NetXDuoEthIpInstance** using that pool is initialized
 + A NX_PACKET_POOL **WebServerPool** is allocated
 + The ARP, ICMP and protocols (TCP and UDP) are enabled for the NX_IP instance
 + A DHCP client is created.

The application creates 1 thread :

 + **NxAppThread** (priority 4, PreemtionThreashold 4) : created with the <i>TX_AUTO_START</i> flag to start automatically.

The **NxAppThread** starts and perform the following actions:

  + Starts the DHCP client
  + Waits for the IP address resolution
  + Resumes the *nx_iperf_entry*

The **nx_iperf_entry**, once started:

  + Creates a NetXDuo Iperf demo web page.

The application then creates 4 threads with the same priorities :

   + **thread_tcp_rx_iperf** (priority 1, PreemtionThreashold 1) : created with the <i>TX_AUTO_START</i> flag to start automatically.
   + **thread_tcp_tx_iperf** (priority 1, PreemtionThreashold 1) : created with the <i>TX_AUTO_START</i> flag to start automatically.
   + **thread_udp_rx_iperf** (priority 1, PreemtionThreashold 1) : created with the <i>TX_AUTO_START</i> flag to start automatically.
   + **thread_udp_tx_iperf** (priority 1, PreemtionThreashold 1) : created with the <i>TX_AUTO_START</i> flag to start automatically.

####  <b>Expected success behavior</b>

 + The board IP address is printed on the HyperTerminal
 + When the Web HTTP server is successfully started.Then the user can test the performance on the web browser after entring the url http://@IP.
 + To execute each Iperf test you must do the following steps and have the expected result in this link https://docs.microsoft.com/en-us/azure/rtos/netx-duo/netx-duo-iperf/chapter3 .

#### <b>Error behaviors</b>

+ The Red LED is toggling to indicate any error that have occurred.

#### <b>Assumptions if any</b>

- The Application is using the DHCP to acquire IP address, thus a DHCP server should be reachable by the board in the LAN used to test the application.
- The application is configuring the Ethernet IP with a static predefined <i>MAC Address</i>, make sure to change it in case multiple boards are connected on the same LAN to avoid any potential network traffic issues.
- The _MAC Address_ is defined in the main.c`

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
  heth.Init.MACAddr[4] =   0x00;
  heth.Init.MACAddr[5] =   0x00;
```
#### <b>Known limitations</b>

  - The packet pool is not optimized. It can be less than that by reducing NX_PACKET_POOL_SIZE in file "app_netxduo.h" and NX_APP_MEM_POOL_SIZE in file "app_azure_rtos_config.h". This update can decrease NetXDuo performance.

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
       } >RAM AT> RAM
	```

       The simplest way to provide memory for ThreadX is to define a new section, see ._threadx_heap above.
       In the example above the ThreadX heap size is set to 64KBytes.
       The ._threadx_heap must be located between the .bss and the ._user_heap_stack sections in the linker script.
       Caution: Make sure that ThreadX does not need more than the provided heap memory (64KBytes in this example).
       Read more in STM32CubeIDE User Guide, chapter: "Linker script".

    + The "tx_initialize_low_level.S" should be also modified to enable the "USE_DYNAMIC_MEMORY_ALLOCATION" flag.

#### <b>NetX Duo usage hints</b>

 - The NetXDuo application needs to allocate the <b> <i> NX_PACKET </i> </b> pool in a dedicated section that is configured as below is an example
 of the section declaration for different IDEs.
 + For EWARM ".icf" file
   ```
   define symbol __ICFEDIT_region_NXDATA_start__  = 0x24030100;
   define symbol __ICFEDIT_region_NXDATA_end__   = 0x240340FF;
   define symbol __ICFEDIT_region_NXDATA_POOL_start__  = 0x24034100;
   define symbol __ICFEDIT_region_NXDATA_POOL_end__   = __ICFEDIT_region_RAM_end__;
   place in NXApp_region_pool { section .NetXPoolSection};
   ```
   + For MDK-ARM
   ```
   RW_NXDriverSection 0x24034100 0x1B800  {
  *(.NetXPoolSection)
  }
   ```
   + For STM32CubeIDE ".ld" file
   ```
   .nx_data 0x24030100 (NOLOAD):
   {
    *(.NetXPoolSection)
   } >RAM
   ```

  this section is then used in the <code> app_azure_rtos.c</code> file to force the <code>nx_byte_pool_buffer</code> allocation.

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
__ALIGN_BEGIN static UCHAR nx_byte_pool_buffer[NX_APP_MEM_POOL_SIZE] __ALIGN_END;
static TX_BYTE_POOL nx_app_byte_pool;
```
For more details about the MPU configuration please refer to the [AN4838](https://www.st.com/resource/en/application_note/dm00272912-managing-memory-protection-unit-in-stm32-mcus-stmicroelectronics.pdf)

### <b>Keywords</b>

RTOS, Network, ThreadX, NetXDuo, Iperf, UART

### <b>Hardware and Software environment</b>

  - This application runs on STM32H735xx devices
  - This application has been tested with STMicroelectronics STM32H735G-DK boards Revision: MB1520-H735I-B02
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
 - Run the [iperf tool] by following steps described in this link https://docs.microsoft.com/en-us/azure/rtos/netx-duo/netx-duo-iperf/chapter3 .
