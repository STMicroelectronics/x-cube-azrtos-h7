/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    app_netxduo.c
  * @author  MCD Application Team
  * @brief   NetXDuo applicative file
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
 */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "app_netxduo.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include   "main.h"
#include   "nxd_dhcp_client.h"
#include   "nx_web_http_server.h"
#include   "app_filex.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* Define the ThreadX , NetX and FileX object control blocks. */

/* Define Threadx global data structures. */
TX_THREAD AppMainThread;
TX_THREAD AppWebServerThread;
TX_SEMAPHORE Semaphore;

/* Define NetX global data structures. */

NX_PACKET_POOL EthPool;
NX_PACKET_POOL WebServerPool;

NX_IP  EthIP;
NX_DHCP DHCPClient;

ULONG IpAddress;
ULONG NetMask;

/* App memory pointer. */
UCHAR   *pointer;

NX_WEB_HTTP_SERVER HTTPServer;

/* Define FileX global data structures. */

/* the web server reads the web content from the uSD, a FX_MEDIA instance is required */
FX_MEDIA                SDMedia;

/* Buffer for FileX FX_MEDIA sector cache. this should be 32-Bytes aligned to avoid
   cache maintenance issues */
ALIGN_32BYTES (uint32_t DataBuffer[512]);

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */

/* Set nx_server_pool start address to 0x24030100 */
#if defined ( __ICCARM__ ) /* IAR Compiler */
#pragma location = 0x24030100
#elif defined ( __CC_ARM ) /* MDK ARM Compiler */
__attribute__((section(".NxServerPoolSection")))
#elif defined ( __GNUC__ ) /* GNU Compiler */
__attribute__((section(".NxServerPoolSection")))
#endif
static uint8_t nx_server_pool[SERVER_POOL_SIZE];

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* WEB HTTP server thread entry */
static void  App_Main_Thread_Entry(ULONG thread_input);
static void  nx_server_thread_entry(ULONG thread_input);

/* DHCP state change notify callback */
static VOID ip_address_change_notify_callback(NX_IP *ip_instance, VOID *ptr);

/* Web Server callback when a new request from a web client is triggered */
static UINT webserver_request_notify_callback(NX_WEB_HTTP_SERVER *server_ptr, UINT request_type, CHAR *resource, NX_PACKET *packet_ptr);

/* USER CODE END PFP */
/**
  * @brief  Application NetXDuo Initialization.
  * @param memory_ptr: memory pointer
  * @retval int
  */
UINT App_NetXDuo_Init(VOID *memory_ptr)
{
  UINT ret = NX_SUCCESS;
  TX_BYTE_POOL *byte_pool = (TX_BYTE_POOL*)memory_ptr;

  /* USER CODE BEGIN App_NetXDuo_Init */
  PRINT_APP_START(Nx_WebServer);

  /* Initialize the NetXDuo system. */
  nx_system_initialize();

  /* Allocate the Ethernet packet pool. */
  ret = tx_byte_allocate(byte_pool, (VOID **) &pointer, NX_PACKET_POOL_SIZE, TX_NO_WAIT);

  /* Check ethernet packet pool memory allocation. */
  if (ret != NX_SUCCESS)
  {
    printf("Packed pool memory allocation failed : 0x%02x\n", ret);
    Error_Handler();
  }

  /* Create a packet pool. */
  ret = nx_packet_pool_create(&EthPool, "NetX Main Packet Pool", PACKET_PAYLOAD_SIZE, pointer, NX_PACKET_POOL_SIZE);

  /* Check for packet pool creation status. */
  if (ret != NX_SUCCESS)
  {
    printf("Packed creation failed : 0x%02x\n", ret);
    Error_Handler();
  }

  /* Allocate the server packet pool. */
  ret = tx_byte_allocate(byte_pool, (VOID **) &pointer, SERVER_POOL_SIZE, TX_NO_WAIT);

  /* Check server packet pool memory allocation. */
  if (ret != NX_SUCCESS)
  {
    printf("Packed pool memory allocation failed : 0x%02x\n", ret);
    Error_Handler();
  }

  /* Create the server packet pool. */
  ret = nx_packet_pool_create(&WebServerPool, "HTTP Server Packet Pool", SERVER_PACKET_SIZE, nx_server_pool, SERVER_POOL_SIZE);

  /* Check for server pool creation status. */
  if (ret != NX_SUCCESS)
  {
    printf("Server pool creation failed : 0x%02x\n", ret);
    Error_Handler();
  }

  /* Allocate the NetX IP Instance memory. */
  ret = tx_byte_allocate(byte_pool, (VOID **) &pointer, 2 * DEFAULT_MEMORY_SIZE, TX_NO_WAIT);

  /* Check NetX IP Instance memory allocation. */
  if (ret != NX_SUCCESS)
  {
    printf("NetX IP Instance memory allocation failed : 0x%02x\n", ret);
    Error_Handler();
  }

  /* Create an IP instance by linking the nx_driver_St32h7xx driver */
  ret = nx_ip_create(&EthIP, "NetX IP Instance 0", NULL_IP_ADDRESS, NULL_IP_ADDRESS, &EthPool, nx_stm32_eth_driver,
                      pointer, 2 * DEFAULT_MEMORY_SIZE, DEFAULT_PRIORITY);

  if (ret != NX_SUCCESS)
  {
    printf("IP Instance creation failed : 0x%02x\n",ret);
    Error_Handler();
  }

  /* Allocate the ARP cache. */
  ret = tx_byte_allocate(byte_pool, (VOID **) &pointer, ARP_CACHE_SIZE, TX_NO_WAIT);

  /* Check ARP cache memory allocation. */
  if (ret != NX_SUCCESS)
  {
    printf("ARP cache memory allocation failed : 0x%02x\n", ret);
    Error_Handler();
  }

  /* Enable ARP and supply ARP cache memory for IP Instance 0. */
  ret =  nx_arp_enable(&EthIP, (void *) pointer, ARP_CACHE_SIZE);

  if (ret != NX_SUCCESS)
  {
    printf("ARP Enable for IP error : 0x%02x\n", ret);
    Error_Handler();
  }

   /* Enable the ICMP support to be able to ping the board */
  ret = nx_icmp_enable(&EthIP);

  if (ret != NX_SUCCESS)
  {
    printf("ICMP enable for IP error : 0x%02x\n", ret);
    Error_Handler();
  }

  /* Enable UDP support needed  by DHCP client */
  ret =  nx_udp_enable(&EthIP);

  if (ret != NX_SUCCESS)
  {
    printf("UDP enable for IP error : 0x%02x\n", ret);
    Error_Handler();
  }

  /* Enable the TCP protocol */
  ret =  nx_tcp_enable(&EthIP);

  if (ret!=NX_SUCCESS)
  {
    printf("TCP enable for IP error : 0x%02x\n", ret);
    Error_Handler();
  }

  /* Allocate the server stack. */
  ret = tx_byte_allocate(byte_pool, (VOID **) &pointer, SERVER_STACK, TX_NO_WAIT);

  /* Check server stack memory allocation. */
  if (ret != NX_SUCCESS)
  {
    printf("Server stack memory allocation failed : 0x%02x\n", ret);
    Error_Handler();
  }

  /* Create the HTTP Server. */
  ret = nx_web_http_server_create(&HTTPServer, "WEB HTTP Server", &EthIP, CONNECTION_PORT,&SDMedia, pointer,
                                   SERVER_STACK, &WebServerPool, NX_NULL, webserver_request_notify_callback);

  if (ret != NX_SUCCESS)
  {
     printf("HTTP WEB Server creation failed: 0x%02x\n", ret);
     Error_Handler();
  }

  /* Allocate the main thread. */
  ret = tx_byte_allocate(byte_pool, (VOID **) &pointer, 2 * DEFAULT_MEMORY_SIZE, TX_NO_WAIT);

  /* Check main thread memory allocation. */
  if (ret != NX_SUCCESS)
  {
    printf("Main thread memory allocation failed : 0x%02x\n", ret);
    Error_Handler();
  }

  /* Create the main thread */
  ret = tx_thread_create(&AppMainThread, "App Main thread", App_Main_Thread_Entry, 0, pointer, 2 * DEFAULT_MEMORY_SIZE,
                         DEFAULT_MAIN_PRIORITY, DEFAULT_MAIN_PRIORITY, TX_NO_TIME_SLICE, TX_AUTO_START);

  if (ret != TX_SUCCESS)
  {
    return NX_NOT_ENABLED;
  }

  /* Allocate the TCP server thread stack. */
  ret = tx_byte_allocate(byte_pool, (VOID **) &pointer, 2 * DEFAULT_MEMORY_SIZE, TX_NO_WAIT);

  /* Check server thread memory allocation. */
  if (ret != NX_SUCCESS)
  {
    printf("Server thread memory allocation failed : 0x%02x\n", ret);
    Error_Handler();
  }

  /* create the TCP server thread */
  ret = tx_thread_create(&AppWebServerThread, "App Web Server Thread", nx_server_thread_entry, 0, pointer, 2 * DEFAULT_MEMORY_SIZE,
                         DEFAULT_PRIORITY, DEFAULT_PRIORITY, TX_NO_TIME_SLICE, TX_DONT_START);

  if (ret != TX_SUCCESS)
  {
    return NX_NOT_ENABLED;
  }

  /* Create the DHCP instance. */
  ret = nx_dhcp_create(&DHCPClient, &EthIP, "dhcp_client");
  if (ret != NX_SUCCESS)
  {
    printf("DHCP Instance creation failed : 0x%02x\n", ret);
  }

  ret = nx_ip_address_change_notify(&EthIP, ip_address_change_notify_callback, NULL);
  if (ret != NX_SUCCESS)
  {
    Error_Handler();
  }

  ret = tx_semaphore_create(&Semaphore, "App Semaphore", 0);
  if (ret != TX_SUCCESS)
  {
    printf("Semaphore creation failed : 0x%02x\n", ret);
    Error_Handler();
  }

  /* USER CODE END App_NetXDuo_Init */

  return ret;
}

/* USER CODE BEGIN 1 */

/**
* @brief  ip address change callback
* @param  ip_instance : NX_IP instance registered for this callback
* @param   ptr : VOID * optional data pointer
* @retval None
*/
static VOID ip_address_change_notify_callback(NX_IP *ip_instance, VOID *ptr)
{
  /* as soon as the IP address is ready, the semaphore is released to let the web server start */
  tx_semaphore_put(&Semaphore);
}


static VOID App_Main_Thread_Entry(ULONG thread_input)
{
  UINT ret;

  ret = nx_ip_address_change_notify(&EthIP, ip_address_change_notify_callback, NULL);
  if (ret != NX_SUCCESS)
  {
    Error_Handler();
  }

  ret = nx_dhcp_start(&DHCPClient);
  if (ret != NX_SUCCESS)
  {
    Error_Handler();
  }

  /* wait until an IP address is ready */
  if(tx_semaphore_get(&Semaphore, TX_WAIT_FOREVER) != TX_SUCCESS)
  {
    Error_Handler();
  }
  /* get IP address */
  ret = nx_ip_address_get(&EthIP, &IpAddress, &NetMask);

  PRINT_IP_ADDRESS(IpAddress);

  if (ret != TX_SUCCESS)
  {
    Error_Handler();
  }
  /* the network is correctly initialized, start the TCP server thread */
  tx_thread_resume(&AppWebServerThread);

  /* this thread is not needed any more, we relinquish it */
  tx_thread_relinquish();

  return;
}

UINT webserver_request_notify_callback(NX_WEB_HTTP_SERVER *server_ptr, UINT request_type, CHAR *resource, NX_PACKET *packet_ptr)
{
  /*
   * At each new request we toggle the green led, but in a real use case this callback can serve
   * to trigger more advanced tasks, like starting background threads or gather system info
   * and append them into the web page.
   */
  BSP_LED_Toggle(LED_GREEN);
  return NX_SUCCESS;
}

/**
* @brief  Application thread for HTTP web server
* @param  thread_input : thread input
* @retval None
*/

void nx_server_thread_entry(ULONG thread_input)
{
  /* HTTP WEB SERVER THREAD Entry */
  UINT    status;
  NX_PARAMETER_NOT_USED(thread_input);

  /* Open the SD disk driver. */
  status = fx_media_open(&SDMedia, "STM32_SDIO_DISK", fx_stm32_sd_driver, SD_DRIVER_INFO_POINTER, DataBuffer, sizeof(DataBuffer));

  /* Check the media opening status. */
  if (status != FX_SUCCESS)
  {
    /*Print Media Opening error. */
    printf("FX media opening failed : 0x%02x\n", status);
    /* Error, call error handler.*/
    Error_Handler();
  }
  else
  {
    /* Print Media Opening Success. */
    printf("Fx media successfully opened.\n");
  }

  /* Start the WEB HTTP Server. */
  status = nx_web_http_server_start(&HTTPServer);

  /* Check the WEB HTTP Server starting status. */
  if (status != NX_SUCCESS)
  {
    /* Print HTTP WEB Server starting error. */
    printf("HTTP WEB Server Starting Failed, error: 0x%02x\n", status);
    /* Error, call error handler.*/
    Error_Handler();
  }
  else
  {
    /* Print HTTP WEB Server Starting success. */
    printf("HTTP WEB Server successfully started.\n");
    /* LED_GREEN On. */
  }
}
/* USER CODE END 1 */
