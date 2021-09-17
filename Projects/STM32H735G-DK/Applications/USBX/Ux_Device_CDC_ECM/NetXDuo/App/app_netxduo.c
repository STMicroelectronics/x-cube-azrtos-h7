/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    app_netxduo.c
  * @author  MCD Application Team
  * @brief   NetXDuo applicative file
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2020-2021 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "app_netxduo.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "main.h"
#include "nxd_dhcp_client.h"
#include "nx_web_http_server.h"
#include "app_filex.h"
#include "ux_api.h"
#include "ux_network_driver.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* Define the ThreadX , NetX and FileX object control blocks. */

/* Define Threadx global data structures. */
TX_THREAD Main_thread;
TX_SEMAPHORE Semaphore;

/* Define NetX global data structures. */
NX_PACKET_POOL EthPool;
NX_PACKET_POOL WebServerPool;
NX_IP  EthIP;
NX_DHCP DHCPClient;
NX_WEB_HTTP_SERVER HTTPServer;
ULONG IPAddress;
ULONG NetMask;

/* Define FileX global data structures. */

/* the web server reads the web content from the uSD, a FX_MEDIA instance is required */
FX_MEDIA SDMedia;

/* Buffer for FileX FX_MEDIA sector cache. this should be 32-Bytes aligned to avoid
   cache maintenance issues */
ALIGN_32BYTES(uint32_t DataBuffer[512]);

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define TOGGLE_GREEN_LED                             "led=ToggleGreen"
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */

/* Set nx_server_pool start address to 0x24046000 */
#if defined ( __ICCARM__ ) /* IAR Compiler */
#pragma location = 0x24046000
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
static void  nx_server_thread_entry(ULONG thread_input);

/* DHCP state change notify callback */
static VOID ip_address_change_notify_callback(NX_IP *ip_instance, VOID *ptr);

/* Web Server callback when a new request from a web client is triggered */
static UINT webserver_request_notify_callback(NX_WEB_HTTP_SERVER *server_ptr,
                                              UINT request_type, CHAR *resource,
                                              NX_PACKET *packet_ptr);

/* USER CODE END PFP */
/**
  * @brief  Application NetXDuo Initialization.
  * @param memory_ptr: memory pointer
  * @retval int
  */
UINT MX_NetXDuo_Init(VOID *memory_ptr)
{
  UINT ret = NX_SUCCESS;
  TX_BYTE_POOL *byte_pool = (TX_BYTE_POOL*)memory_ptr;

  /* USER CODE BEGIN MX_NetXDuo_MEM_POOL */

  /* USER CODE END MX_NetXDuo_MEM_POOL */

  /* USER CODE BEGIN MX_NetXDuo_Init */

  /* Web App memory pointer. */
  UCHAR   *web_app_pointer;

  /* Initialize the NetXDuo system. */
  nx_system_initialize();

  /* Allocate the web stack instance. */
  ret = tx_byte_allocate(byte_pool, (VOID **) &web_app_pointer,
                         WEB_STACK_SIZE, TX_NO_WAIT);

  /* Check web stack allocation */
  if (ret != TX_SUCCESS)
  {
    printf("Web stack allocation failed: 0x%02x\n", ret);
    Error_Handler();
  }

  /* Create the main thread. */
  ret = tx_thread_create(&Main_thread, "Main_thread", nx_server_thread_entry,
                         ENTRY_INPUT, web_app_pointer, WEB_STACK_SIZE,
                         DEFAULT_PRIORITY, THREAD_PREEMPT_THRESHOLD,
                         TX_NO_TIME_SLICE, TX_AUTO_START);

  /* Check Main Thread creation */
  if (ret != TX_SUCCESS)
  {
    printf("Main Thread creation failed: 0x%02x\n", ret);
    Error_Handler();
  }

  /* Allocate the packet pool. */
  ret = tx_byte_allocate(byte_pool, (VOID **) &web_app_pointer,
                         NX_PACKET_POOL_SIZE, TX_NO_WAIT);

  /* Check packet pool allocation */
  if (ret != TX_SUCCESS)
  {
    printf("Packet pool allocation failed: 0x%02x\n", ret);
    Error_Handler();
  }

  /* Create a packet pool. */
  ret = nx_packet_pool_create(&EthPool, "NetX Main Packet Pool",
                              PACKET_PAYLOAD_SIZE, web_app_pointer,
                              NX_PACKET_POOL_SIZE);

  /* Check for packet pool creation status. */
  if (ret != NX_SUCCESS)
  {
    printf("Packed creation failed : 0x%02x\n", ret);
    Error_Handler();
  }

  /* Create the server packet pool. */
  ret = nx_packet_pool_create(&WebServerPool, "HTTP Server Packet Pool",
                              SERVER_PACKET_SIZE, nx_server_pool,
                              SERVER_POOL_SIZE);

  /* Check for server pool creation status. */
  if (ret != NX_SUCCESS)
  {
    printf("Server pool creation failed : 0x%02x\n", ret);
    Error_Handler();
  }

  /* Allocate the IP instance. */
  ret = tx_byte_allocate(byte_pool, (VOID **) &web_app_pointer,
                         IP_MEMORY_SIZE, TX_NO_WAIT);

  /* Check for IP instance pool Allocation status. */
  if (ret != NX_SUCCESS)
  {
    printf("IP instance Allocation failed : 0x%02x\n", ret);
    Error_Handler();
  }

  /* Create an IP instance by linking the ux network driver */
  ret = nx_ip_create(&EthIP, "NetX IP Instance 0", NULL_IP_ADDRESS,
                     NULL_IP_ADDRESS, &EthPool, _ux_network_driver_entry,
                     web_app_pointer, IP_MEMORY_SIZE, DEFAULT_PRIORITY);

  /* Check the IP instance creation */
  if (ret != NX_SUCCESS)
  {

    printf("IP Instance creation failed : 0x%02x\n", ret);
    Error_Handler();
  }

  /* Allocate the arp cache memory. */
  ret = tx_byte_allocate(byte_pool, (VOID **) &web_app_pointer,
                         ARP_CACHE_SIZE, TX_NO_WAIT);

  /* Check the arp cache memory Allocation status. */
  if (ret != NX_SUCCESS)
  {
    printf("IP instance Allocation failed : 0x%02x\n", ret);
    Error_Handler();
  }

  /* Enable ARP and supply ARP cache memory for IP Instance 0. */
  ret =  nx_arp_enable(&EthIP, (void *) web_app_pointer, ARP_CACHE_SIZE);

  /* Check the ARP instance activation */
  if (ret != NX_SUCCESS)
  {
    printf("ARP Enable for IP error : 0x%02x\n", ret);
    Error_Handler();
  }

  /* Enable the ICMP support to be able to ping the board */
  ret = nx_icmp_enable(&EthIP);

  /* Check the ICMP activation */
  if (ret != NX_SUCCESS)
  {
    printf("ICMP enable for IP error : 0x%02x\n", ret);
    Error_Handler();
  }

  /* Enable UDP support needed  by DHCP client */
  ret =  nx_udp_enable(&EthIP);

  /* Check UDP support activation */
  if (ret != NX_SUCCESS)
  {
    printf("UDP enable for IP error : 0x%02x\n", ret);
    Error_Handler();
  }

  /* Enable the TCP protocol */
  ret =  nx_tcp_enable(&EthIP);

  /* Check the TCP activation */
  if (ret != NX_SUCCESS)
  {
    printf("TCP enable for IP error : 0x%02x\n", ret);
    Error_Handler();
  }

  /* Allocate the SERVER_STACK memory . */
  ret = tx_byte_allocate(byte_pool, (VOID **) &web_app_pointer,
                         SERVER_STACK, TX_NO_WAIT);

  /* Check the SERVER_STACK memory Allocation status. */
  if (ret != NX_SUCCESS)
  {
    printf("Server stack Allocation failed : 0x%02x\n", ret);
    Error_Handler();
  }

  /* Create the HTTP Server. */
  ret = nx_web_http_server_create(&HTTPServer, "WEB HTTP Server", &EthIP,
                                  CONNECTION_PORT, &SDMedia, web_app_pointer,
                                  SERVER_STACK, &WebServerPool, NX_NULL,
                                  webserver_request_notify_callback);

  /* Check the web server creation */
  if (ret != NX_SUCCESS)
  {
    printf("HTTP WEB Server creation failed: 0x%02x\n", ret);
    Error_Handler();
  }

  /* Create the DHCP instance. */
  ret = nx_dhcp_create(&DHCPClient, &EthIP, "dhcp_client");

  /* Check the DHCP instance creation */
  if (ret != NX_SUCCESS)
  {
    printf("DHCP Instance creation failed : 0x%02x\n", ret);
  }

  /* Register a callback function for ip notify */
  ret = nx_ip_address_change_notify(&EthIP, ip_address_change_notify_callback,
                                    NULL);

  /* Check the callback function creation*/
  if (ret != NX_SUCCESS)
  {
    Error_Handler();
  }

  /* Create semaphore */
  ret = tx_semaphore_create(&Semaphore, "App Semaphore", 0);

  /* Check semaphore creation */
  if (ret != TX_SUCCESS)
  {
    printf("Semaphore creation failed : 0x%02x\n", ret);
    Error_Handler();
  }

  /* USER CODE END MX_NetXDuo_Init */

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

UINT webserver_request_notify_callback(NX_WEB_HTTP_SERVER *server_ptr,
                                       UINT request_type, CHAR *resource,
                                       NX_PACKET *packet_ptr)
{
  CHAR request_data[256] = {0};
  UINT request_size = 0;
  UINT status;

  /*
   * At each new request we toggle the green led, but in a real use case this callback can serve
   * to trigger more advanced tasks, like starting background threads or gather system info
   * and append them into the web page.
   */

  /* Get the requested data from packet */
  status = nx_web_http_server_content_get(server_ptr, packet_ptr, 0,
                                          request_data, 100, &request_size);
  /* Check get data */
  if (status == NX_SUCCESS)
  {
    /* Check if requested data equal TOGGLE_GREEN_LED */
    if (strncmp((char const *)request_data, TOGGLE_GREEN_LED, sizeof(TOGGLE_GREEN_LED)) == 0)
    {
      BSP_LED_Toggle(LED_GREEN);
    }
  }

  return NX_SUCCESS;
}

/**
* @brief  Application thread for HTTP web server
* @param  thread_input : thread input
* @retval None
*/

void nx_server_thread_entry(ULONG thread_input)
{
  /* Sleep for 1s */
  tx_thread_sleep(MS_TO_TICK(1000)); 
  
  /* HTTP WEB SERVER THREAD Entry */
  UINT    status;
  NX_PARAMETER_NOT_USED(thread_input);

  printf("Starting DHCP client...\n");

  /* Start the DHCP Client. */
  status = nx_dhcp_start(&DHCPClient);

  /* Check DHCP Client Starting status. */
  if (status != NX_SUCCESS)
  {
    /* Print DHCP Instance creation error. */
    printf("DHCP Instance Starting error : 0x%02x\n", status);
  }

  /* Check if the semaphore is released */
  if (tx_semaphore_get(&Semaphore, TX_WAIT_FOREVER) != TX_SUCCESS)
  {
    Error_Handler();
  }

  /* Checks for errors in the IP address */
  nx_ip_address_get(&EthIP, &IPAddress, &NetMask);

  /* Log IP address */
  PRINT_ADDRESS(IPAddress);
  /* Log NetMask */
  PRINT_ADDRESS(NetMask);

  /* Open the SD disk driver. */
  status = fx_media_open(&SDMedia, "STM32_SDIO_DISK", fx_stm32_sd_driver,
                         SD_DRIVER_INFO_POINTER, DataBuffer, sizeof(DataBuffer));

  /* Check the media opening status. */
  if (status != FX_SUCCESS)
  {
    /* Print Media Opening error. */
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
  }
}
/* USER CODE END 1 */
