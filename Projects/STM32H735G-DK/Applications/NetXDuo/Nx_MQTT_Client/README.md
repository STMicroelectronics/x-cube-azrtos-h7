
## <b>Nx_MQTT_Client application description</b>

This application provides an example of Azure RTOS NetX/NetXDuo stack usage . 
It shows how to exchange data between client and server using MQTT protocol in an encrypted mode supporting TLS v1.2.

The main entry function tx_application_define() is called by ThreadX during kernel start, at this stage, all NetX resources are created.

 + A NX_PACKET_POOL is allocated
 
 + A NX_IP instance using that pool is initialized
 
 + The ARP, ICMP and protocols (TCP and UDP) are enabled for the NX_IP instance
 
 + A DHCP client is created.
 
The application then creates 2 threads with different priorities:

 + **AppMainThread** (priority 10, PreemtionThreashold 10) : created with the TX_AUTO_START flag to start automatically.

 + **AppMQTTClientThread** (priority 5, PreemtionThreashold 5) : created with the TX_DONT_START flag to be started later.
 
The **AppMainThread** starts and perform the following actions:

  + starts the DHCP client
  
  + waits for the IP address resolution
  
  + resumes the **AppMQTTClientThread**

The **AppMQTTClientThread**, once started:

  + creates a dns_client with USER_DNS_ADDRESS used as DNS server then get MQTT broker address from MQTT_BROKER_NAME predefined on app_netxduo.h.

  + creates an mqtt_client

  + connects mqtt_client to the online MQTT broker; connection with server will be secure and a **tls_setup_callback** will set TLS parametres. By default MQTT_PORT for encrypted mode is 8883.
  
          refer to note 4 below, to know how to setup an x509 certificate.
  
  + mqtt_client subscribes on a predefined topic TOPIC_NAME  "Temperature" with a Quality Of Service QOS0
  
  + depending on the number of message "NB_MESSAGE" defined by user, mqtt_client will publish a random number generated by RNG. If NB_MESSAGE = 0, it means that number of messages is infinitely.
  
  + mqtt_client will get messages from the MQTT broker and print them. 
  
#### <b> Expected success behavior</b>

 + The board IP address is printed on the HyperTerminal
 + Connection's information are printed on the HyperTerminal (broker's name, mqtt port, topic and messages received)
 
 ```
MQTT client connected to broker < test.mosquitto.org > at PORT 8883 :
Message 1 received: TOPIC = Temperature, MESSAGE = 34
Message 2 received: TOPIC = Temperature, MESSAGE = 35
Message 3 received: TOPIC = Temperature, MESSAGE = 35
Message 4 received: TOPIC = Temperature, MESSAGE = 34
Message 5 received: TOPIC = Temperature, MESSAGE = 33
client disconnected from server
 ```
 + Green led is toggling after successfully receiving all messages.
 
#### <b> Error behaviors</b>

+ The red LED is toggling to indicate any error that has occurred.

#### <b> Assumptions if any</b>

- The Application is using the DHCP to acquire IP address, thus a DHCP server should be reachable by the board in the LAN used to test the application.
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
  heth.Init.MACAddr[1] =   0x11;
  heth.Init.MACAddr[2] =   0x83;
  heth.Init.MACAddr[3] =   0x45;
  heth.Init.MACAddr[4] =   0x26;
  heth.Init.MACAddr[5] =   0x20;
```
#### <b>Known limitations</b>

* Default NX_DNS_MAX_RETRIES value in "nx_user.h" may not work with some MQTT servers, they should be tuned for example:

    #define NX_DNS_MAX_RETRIES  10

* Since NetXDuo does not support proxy, mqtt_client should be connected directly to the server.

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
      
 4. To make an encrypted connection with MQTT server, user should follow these steps to add an x509 certificate to the _mqtt\_client_ and use it to ensure server's authentication :
      - download certificate authority CA (in this application "mosquitto.org.der" downloaded from [test.mosquitto](https://test.mosquitto.org)
      - convert certificate downloaded by executing the following cmd from the file downloaded path : 
 
   		                xxd.exe -i mosquitto.org.der > mosquitto.cert.h
				
      - add the converted file under the application : NetXDuo/Nx_MQTT_Client/NetXDuo/App
      - configure MOSQUITTO_CERT_FILE with your certificate name.
 
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

RTOS, Network, ThreadX, NetXDuo, MQTT, TLS, UART

### <b>Hardware and Software environment</b>

 - This application runs on STM32H735xx devices.
 - This application has been tested with STMicroelectronics STM32H735G-DK boards Revision: MB1520-H735I-B02
    and can be easily tailored to any other supported device and development board.

 - This application uses USART3 to display logs, the hyperterminal configuration is as follows:
      - BaudRate = 115200 baud
      - Word Length = 8 Bits
      - Stop Bit = 1
      - Parity = none
      - Flow control = None

###  <b>How to use it ?</b>

In order to make the program work, you must do the following :

 - Open your preferred toolchain
 - Edit the file app_netxduo.h : define the USER_DNS_ADDRESS, the MQTT_BROKER_NAME and NB_MESSAGE.
 - Rebuild all files and load your image into target memory
 - Run the application  