
## <b>Nx_UDP_Echo_Client application description</b>

This application provides an example of Azure RTOS NetX/NetXDuo stack usage . 
It shows how to develop a NetX udp client to communicate with a remote sever using
the NetX UDP socket API.

The main entry function tx_application_define() is called by ThreadX during kernel start, at this stage, all NetX resources are created.

 + A NX_PACKET_POOL is allocated 
 + A NX_IP instance using that pool is initialized 
 + The ARP, ICMP and UDP protocols are enabled for the NX_IP instance 
 + A DHCP client is created.
 
The application then creates 2 threads with the same priorities:

 + **AppMainThread** (priority 10, PreemtionThreashold 10) : created with the TX_AUTO_START flag to start automatically. 
 + **AppUDPThread** (priority 10, PreemtionThreashold 10) : created with the TX_DONT_START flag to be started later.
 
The **AppMainThread** starts and perform the following actions:

  + starts the DHCP client  
  + waits for the IP address resolution  
  + resumes the **AppUDPThread**

The **AppUDPThread**, once started:

  + creates a UDP socket  
  + connects to the remote UDP server on the predefined port  
  + On connection success, the UDP client sends a MAX_PACKET_COUNT messages to the server.  
  + At each message sent, the UDP client reads the sever response and prints it on the Hyperterminal and the green led is toggled.
  
#### <b>Expected success behavior</b>

 + The board IP address is printed on the HyperTerminal
 + The response messages sent by the server are printed on the HyerTerminal
 + if the [echotool](https://github.com/PavelBansky/EchoTool/releases/tag/v1.5.0.0) utility is used the message sent by the client are displayed on the PC console.
 + a summary message similar to the following is printed on the HyperTerminal and the green LED is toggling.
 ```
  SUCCESS : 10 / 10 packets sent
```

#### <b>Error behaviors</b>

+ the Red LED is toggling to indicate any error that have occurred.
+ In case the message exchange is not completed a failure message is printed on the HyperTerminal showing the actual sent message compared to the expected ones.

#### <b>Assumptions if any</b>

- The Application is using the DHCP to acquire IP address, thus a DHCP server should be reachable by the board in the LAN used to test the application.
- The application is configuring the Ethernet IP with a static predefined MAC Address, make sure to change it in case multiple boards are connected on the 
same LAN to avoid any potential network traffic issues.
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
  heth.Init.MACAddr[1] =   0x11;
  heth.Init.MACAddr[2] =   0x83;
  heth.Init.MACAddr[3] =   0x45;
  heth.Init.MACAddr[4] =   0x26;
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
      - Please refer to the **AN4839** "Level 1 cache on STM32F7 Series"
  
#### <b>ThreadX usage hints</b>

 - ThreadX uses the Systick as time base, thus it is mandatory that the HAL uses a separate time base through the TIM IPs.
 - ThreadX is configured with 100 ticks/sec by default, this should be taken into account when using delays or timeouts at application. It is always possible to reconfigure it in the "tx_user.h", the "TX_TIMER_TICKS_PER_SECOND" define,but this should be reflected in "tx_initialize_low_level.s" file too.
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
    or modify the line below in "tx_low_level_initilize.s to match the memory region being used
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
	  
    + The "tx_initialize_low_level.s" should be also modified to enable the "USE_DYNAMIC_MEMORY_ALLOCATION" flag.
         
#### <b>NetX Duo usage hints</b>

- The ETH TX And RX descriptors are accessed by the CPU and the ETH DMA IP, thus they should not be allocated into the DTCM RAM "0x20000000".
- Make sure to allocate them into a "Non-Cacheable" memory region to always ensure data coherency between the CPU and ETH DMA.
- Depending on the application scenario, the total TX and RX descriptors may need to be increased by updating respectively  the "ETH_TX_DESC_CNT" and "ETH_RX_DESC_CNT" in the "stm32h7xx_hal_conf.h", to guarantee the application correct behaviour, but this will cost extra memory to allocate.
- The NetXDuo application needs to allocate the <b> <i> NX_PACKET </i> </b> pool in a dedicated section that is  configured as either "Cacheable Write-through" for <i>STM32H72XX</i> and <i>STM32H73XX </i>,  <i>STM32H7AXX</i> and <i>STM32H7BXX </i> or non-cacheable for other STM32H7 families. Below is an example of the section declaration for different IDEs.
   + For EWARM ".icf" file
   ```
   define symbol __ICFEDIT_region_NXDATA_start__  = 0x24018200;
   define symbol __ICFEDIT_region_NXDATA_end__   = 0x24027FFF;
   define region NXAppPool_region  = mem:[from __ICFEDIT_region_NXDATA_start__ to __ICFEDIT_region_NXDATA_end__];
   place in NXAppPool_region { section .NXAppPoolSection};
   ```
   + For MDK-ARM
   ```
    RW_NXDriverSection 0x24030200 0x20000  {
  *(.NetXPoolSection)
  }
   ```
   + For STM32CubeIDE ".ld" file
   ``` 
   .nx_section 0x24018200 (NOLOAD): {
     *(.NXAppPoolSection)
     } >RAM_D1
   ```

  this section is then used in the <code> app_azure_rtos.c</code> file to force the <code>nx_byte_pool_buffer</code> allocation.

```
/* USER CODE BEGIN NX_Pool_Buffer */

#if defined ( __ICCARM__ ) /* IAR Compiler */
#pragma location = ".NetXPoolSection"

#elif defined ( __CC_ARM ) /* MDK ARM Compiler */
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

RTOS, Network, ThreadX, NetXDuo, TCP, UART

### <b>Hardware and Software environment</b>

 - This application runs on STM32H735xx devices.
 - This application has been tested with STMicroelectronics STM32H735G-DK boards Revision: MB1520-H735I-B02
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
 - Edit the file <code> NetXDuo/App/app_netxduo.h</code> and correctly define the <UDP_SERVER_ADDRESS> and <UDP_SERVER_PORT> to connect on.
 - run the [echotool](https://github.com/PavelBansky/EchoTool/releases/tag/v1.5.0.0) utility    on a windows console as following:
   
       c:\> .\echotool.exe /p udp /s <UDP_SERVER_PORT> 
           
       example : c:\> .\echotool.exe /p udp /s 6001 
   
 - Rebuild all files and load your image into target memory
 - Run the application 
