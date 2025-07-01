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

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/* Includes ------------------------------------------------------------------*/
#include "app_netxduo.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define NULL_IP_ADDRESS      IP_ADDRESS(0,0,0,0)

#define LED_ON     "led=LED_ON"
#define LED_OFF    "led=LED_OFF"
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
TX_THREAD          WebServer_thread;
TX_SEMAPHORE       WebServer_Semaphore;
NX_PACKET_POOL     net_packet_pool;
NX_PACKET_POOL     WebServerPool;
NX_IP              cdc_ecm_ip;
NX_DHCP            dhcp_client;
NX_WEB_HTTP_SERVER HTTPServer;

FX_MEDIA SDMedia;
ALIGN_32BYTES(uint32_t DataBuffer[512]);

/* Set nx_server_pool start address */
#if defined ( __ICCARM__ ) /* IAR Compiler */
#pragma location = ".UsbxAppSection"
#elif defined ( __CC_ARM ) || defined(__ARMCC_VERSION) /* ARM Compiler 5/6 */
__attribute__((section(".UsbxAppSection")))
#elif defined ( __GNUC__ ) /* GNU Compiler */
__attribute__((section(".UsbxAppSection")))
#endif
static uint8_t nx_server_pool[SERVER_POOL_SIZE];

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

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

  /* USER CODE BEGIN 0 */
  UCHAR *pointer;
  /* USER CODE END 0 */

  /* USER CODE BEGIN MX_NetXDuo_Init */

  /* Initialize the NetXDuo system */
  nx_system_initialize();

  /* Allocate stack for Web Server thread */
  if (tx_byte_allocate(byte_pool, (VOID **) &pointer, 1024, TX_NO_WAIT) != TX_SUCCESS)
  {
    Error_Handler();
  }

  /* Create the Web Server thread */
  if (tx_thread_create(&WebServer_thread, "Web Server App thread", nx_server_thread_entry, 0,
                       pointer, 1024, 10, 10, TX_NO_TIME_SLICE, TX_AUTO_START) != TX_SUCCESS)
  {
    Error_Handler();
  }

  /* Allocate stack for the packet pool */
  if (tx_byte_allocate(byte_pool, (VOID **) &pointer, NX_PACKET_POOL_SIZE, TX_NO_WAIT) != TX_SUCCESS)
  {
    Error_Handler();
  }

  /* Create a packet pool */
  if (nx_packet_pool_create(&net_packet_pool, "NetX Main Packet Pool",
                            PACKET_PAYLOAD_SIZE, pointer,
                            NX_PACKET_POOL_SIZE) != NX_SUCCESS)
  {
    Error_Handler();
  }

  /* Create the DHCP Server IP instance */
  if (nx_packet_pool_create(&WebServerPool, "HTTP Server Packet Pool",
                            SERVER_PACKET_SIZE, nx_server_pool,
                            SERVER_POOL_SIZE) != NX_SUCCESS)
  {
    Error_Handler();
  }

  /* Allocate stack for the IP instance */
  if (tx_byte_allocate(byte_pool, (VOID **) &pointer, 2048, TX_NO_WAIT) != TX_SUCCESS)
  {
    Error_Handler();
  }

  /* Creates an CDC ECM Internet Protocol instance */
  if (nx_ip_create(&cdc_ecm_ip, "NetX IP Instance 0", NULL_IP_ADDRESS,
                   NULL_IP_ADDRESS, &net_packet_pool, _ux_network_driver_entry,
                   pointer, 2048, 10) != NX_SUCCESS)
  {
    Error_Handler();
  }

  /* Allocate stack for ARP */
  if (tx_byte_allocate(byte_pool, (VOID **) &pointer, 2048, TX_NO_WAIT) != TX_SUCCESS)
  {
    Error_Handler();
  }

  /* Enable ARP and supply ARP cache memory for IP Instance CDC ECM */
  if (nx_arp_enable(&cdc_ecm_ip, (void *) pointer, 1024) != NX_SUCCESS)
  {
    Error_Handler();
  }

  /* Enable TCP traffic */
  if (nx_tcp_enable(&cdc_ecm_ip) != NX_SUCCESS)
  {
    Error_Handler();
  }

  /* Enable UDP traffic */
  if (nx_udp_enable(&cdc_ecm_ip) != NX_SUCCESS)
  {
    Error_Handler();
  }

  /* Enable ICMP to enable the ping utility */
  if (nx_icmp_enable(&cdc_ecm_ip) != NX_SUCCESS)
  {
    Error_Handler();
  }

  /* Allocate stack for HTTP Server memory . */
  if (tx_byte_allocate(byte_pool, (VOID **) &pointer, 4096, TX_NO_WAIT) != TX_SUCCESS)
  {
    Error_Handler();
  }

  /* Create the HTTP Server */
  if (nx_web_http_server_create(&HTTPServer, "WEB HTTP Server thread", &cdc_ecm_ip,
                                CONNECTION_PORT, &SDMedia, pointer,
                                4096, &WebServerPool, NX_NULL,
                                webserver_request_notify_callback) != NX_SUCCESS)
  {
    Error_Handler();
  }

  /* Create the DHCP Client instance */
  if (nx_dhcp_create(&dhcp_client, &cdc_ecm_ip, "dhcp_client") != NX_SUCCESS)
  {
    Error_Handler();
  }

  /* Register a callback function for ip notify */
  if (nx_ip_address_change_notify(&cdc_ecm_ip, ip_address_change_notify_callback,
                                  NULL) != NX_SUCCESS)
  {
    Error_Handler();
  }

  /* Create WebServer semaphore */
  if (tx_semaphore_create(&WebServer_Semaphore, "WEB HTTP Semaphore", 0) != TX_SUCCESS)
  {
    Error_Handler();
  }

  /* USER CODE END MX_NetXDuo_Init */

  return ret;
}

/* USER CODE BEGIN 2 */

/**
  * @brief  ip_address_change_notify_callback
            IP change notification function
  * @param  ip_instance : NX_IP instance registered for this callback
  * @param  ptr : optional data pointer
  * @retval none
  */
static VOID ip_address_change_notify_callback(NX_IP *ip_instance, VOID *ptr)
{
  tx_semaphore_put(&WebServer_Semaphore);
}

/**
  * @brief  WebServer_RequestNotifyCallback
  *         This function is invoked to response the received Client request.
  * @param  server_ptr: Pointer to HTTP Server
  * @param  request_type: HTTP Web Server Request type.
  * @param  resource: Pointer to URL string for requested resource.
  * @param  packet_ptr: Pointer to the packet to send.
  * @retval status
  */
UINT webserver_request_notify_callback(NX_WEB_HTTP_SERVER *server_ptr,
                                       UINT request_type, CHAR *resource,
                                       NX_PACKET *packet_ptr)
{
  CHAR request_data[256] = {0};
  UINT request_size = 0;

  /* Get the requested data from packet */
  if (nx_web_http_server_content_get(server_ptr, packet_ptr, 0,
                                     request_data, 100, &request_size) == NX_SUCCESS)
  {
    /* Check if requested data equal LED_ON */
    if (strncmp((char const *)request_data, LED_ON, sizeof(LED_ON)) == 0)
    {
      HAL_GPIO_WritePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin, GPIO_PIN_RESET);
    }
    /* Check if requested data equal LED_OFF */
    else if (strncmp((char const *)request_data, LED_OFF, sizeof(LED_OFF)) == 0)
    {
      HAL_GPIO_WritePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin, GPIO_PIN_SET);
    }
  }

  return NX_SUCCESS;
}

/**
  * @brief  nx_server_thread_entry
  *         Application thread for HTTP web server
  * @param  thread_input : thread input
  * @retval None
  */

void nx_server_thread_entry(ULONG thread_input)
{
  ULONG IPAddress;
  ULONG NetMask;

  NX_PARAMETER_NOT_USED(thread_input);

  printf("Starting DHCP client...\n");

  /* Start the DHCP Client */
  if (nx_dhcp_start(&dhcp_client) != NX_SUCCESS)
  {
    Error_Handler();
  }

  /* Check if the semaphore is released */
  if (tx_semaphore_get(&WebServer_Semaphore, TX_WAIT_FOREVER) != TX_SUCCESS)
  {
    Error_Handler();
  }

  /* Checks for errors in the IP address */
  nx_ip_address_get(&cdc_ecm_ip, &IPAddress, &NetMask);

  /* Log IP address */
  PRINT_ADDRESS(IPAddress);

  /* Log NetMask */
  PRINT_ADDRESS(NetMask);

  /* Open the SD disk driver */
  if (fx_media_open(&SDMedia, "STM32_SDIO_DISK", fx_stm32_sd_driver,
                    0, DataBuffer, sizeof(DataBuffer)) != FX_SUCCESS)
  {
    Error_Handler();
  }

  /* Start the WEB HTTP Server */
  if (nx_web_http_server_start(&HTTPServer) != NX_SUCCESS)
  {
    Error_Handler();
  }
}

/* USER CODE END 2 */
