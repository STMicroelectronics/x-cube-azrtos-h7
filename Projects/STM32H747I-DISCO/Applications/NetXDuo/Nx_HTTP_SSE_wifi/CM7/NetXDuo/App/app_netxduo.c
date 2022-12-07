/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    app_netxduo.c
  * @author  MCD Application Team
  * @brief   NetXDuo applicative file
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
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
#include "app_azure_rtos.h"
#include "main.h"
#include "httpserver_sentevents_socket.h"
#include "nx_ip.h"
#include "msg.h"
#include <stdbool.h>
#include "nx_stm32_cypress_whd_driver.h"

#include "nxd_dhcp_server.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* Define the interface index.  */
#define NX_DHCP_INTERFACE_INDEX     0

#define NX_DHCP_SERVER_IP_ADDRESS   IP_ADDRESS(192, 168,   1, 1)
#define NX_DHCP_SERVER_IP_MASK      IP_ADDRESS(255, 255, 254, 0)

#define START_IP_ADDRESS_LIST_0     IP_ADDRESS(192, 168,   1, 10)
#define END_IP_ADDRESS_LIST_0       IP_ADDRESS(192, 168,   1, 19)
#define NX_DHCP_SUBNET_MASK_0       IP_ADDRESS(255, 255, 254,  0)

#define NX_DHCP_DEFAULT_GATEWAY_0   IP_ADDRESS(192, 168,   1,  1)
#define NX_DHCP_DNS_SERVER_0        IP_ADDRESS(192, 168,   1,  1)
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
static TX_THREAD AppMainThread;

static NX_PACKET_POOL AppPacketPool;

NX_IP IpInstance;


NX_DHCP_SERVER DhcpServer;


/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */
static UINT DHCP_Server_start(TX_BYTE_POOL *byte_pool);

static VOID App_Main_Thread_Entry(ULONG thread_input);

static VOID ip_address_change_notify_callback(NX_IP *ip_instance, VOID *ptr);
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
  (void)byte_pool;
  /* USER CODE END MX_NetXDuo_MEM_POOL */

  /* USER CODE BEGIN 0 */

  /* USER CODE END 0 */

  /* USER CODE BEGIN MX_NetXDuo_Init */

  /* Change default WiFi mode to Access Point for this application. */
  WifiMode = WIFI_MODE_AP;

  MSG_INFO("Nx_HTTP_SERVER application started..\n");
  MSG_INFO("# build: %s-%s, %s, %s %s\n",
           "SDIO",
           "RTOS",
           "Network on host",
           __TIME__, __DATE__);

  MSG_INFO("\nTX_TIMER_TICKS_PER_SECOND: %"PRIu32"\n", (uint32_t)TX_TIMER_TICKS_PER_SECOND);
  MSG_INFO("NX_IP_PERIODIC_RATE      : %"PRIu32"\n\n", (uint32_t)NX_IP_PERIODIC_RATE);

  /* Initialize the NetX system. */
  nx_system_initialize();

  /* Allocate the memory for packet_pool.  */
  {
    const ULONG pool_size = NX_PACKET_POOL_SIZE;
    VOID *pool_start;

    if (tx_byte_allocate(byte_pool, &pool_start, pool_size, TX_NO_WAIT) != TX_SUCCESS)
    {
      return TX_POOL_ERROR;
    }

    /* Create the Packet pool to be used for packet allocation */
    ret = nx_packet_pool_create(&AppPacketPool, "Main Packet Pool", PAYLOAD_SIZE, pool_start, pool_size);

    if (ret != NX_SUCCESS)
    {
      return NX_NOT_ENABLED;
    }
    MSG_DEBUG("nx_packet_pool_create() done\n");
  }

  /* Allocate the memory for Ip_Instance */
  {
    const ULONG stack_size = NETX_IP_THREAD_STACK_SIZE;
    VOID *stack_ptr;
    if (tx_byte_allocate(byte_pool, &stack_ptr, stack_size, TX_NO_WAIT) != TX_SUCCESS)
    {
      return TX_POOL_ERROR;
    }

    /* Create the main NX_IP instance. */
    ULONG ip_address = NX_DHCP_SERVER_IP_ADDRESS;
    ULONG network_mask = NX_DHCP_SERVER_IP_MASK;

    ret = nx_ip_create(&IpInstance, "NetX IP Instance 0",
                       ip_address,
                       network_mask,
                       &AppPacketPool, nx_driver_cypress_whd_entry,
                       stack_ptr, stack_size, NETX_IP_THREAD_PRIORITY);

    if (ret != NX_SUCCESS)
    {
      return NX_NOT_ENABLED;
    }
    MSG_DEBUG("nx_ip_create() done\n");
  }

  {
    ret = nx_ip_address_change_notify(&IpInstance, ip_address_change_notify_callback, NULL);
    if (ret != NX_SUCCESS)
    {
      Error_Handler();
    }
    MSG_DEBUG("nx_ip_address_change_notify() done\n");
  }


  /* Allocate the memory for ARP */
  {
    const ULONG arp_cache_size = ARP_MEMORY_SIZE;
    VOID *arp_cache_memory;

    if (tx_byte_allocate(byte_pool, &arp_cache_memory, arp_cache_size, TX_NO_WAIT) != TX_SUCCESS)
    {
      return TX_POOL_ERROR;
    }

    /* Enable the ARP protocol and provide the ARP cache size for the IP instance */
    ret = nx_arp_enable(&IpInstance, arp_cache_memory, arp_cache_size);

    if (ret != NX_SUCCESS)
    {
      return NX_NOT_ENABLED;
    }
    MSG_DEBUG("nx_arp_enable() done\n");
  }

  /* Enable the ICMP */
  {
    ret = nx_icmp_enable(&IpInstance);

    if (ret != NX_SUCCESS)
    {
      return NX_NOT_ENABLED;
    }
    MSG_DEBUG("nx_icmp_enable() done\n");
  }

  /* Enable the UDP protocol required for DNS/DHCP communication. */
  {
    ret = nx_udp_enable(&IpInstance);

    if (ret != NX_SUCCESS)
    {
      return NX_NOT_ENABLED;
    }
    MSG_DEBUG("nx_udp_enable() done\n");
  }

  /* Enable the TCP protocol. */
  {
    ret = nx_tcp_enable(&IpInstance);

    if (ret != NX_SUCCESS)
    {
      return NX_NOT_ENABLED;
    }
    MSG_DEBUG("nx_tcp_enable() done\n");
  }

  /* Allocate the memory for main thread */
  {
    const ULONG stack_size = MAIN_THREAD_STACK_SIZE;
    VOID *stack_ptr;

    if (tx_byte_allocate(byte_pool, &stack_ptr, stack_size, TX_NO_WAIT) != TX_SUCCESS)
    {
      return TX_POOL_ERROR;
    }

    /* Create the main thread */
    ret = tx_thread_create(&AppMainThread, "App Main thread", App_Main_Thread_Entry,
                           (ULONG)byte_pool, stack_ptr, stack_size,
                           MAIN_THREAD_PRIORITY, MAIN_THREAD_PRIORITY, TX_NO_TIME_SLICE, TX_AUTO_START);

    if (ret != TX_SUCCESS)
    {
      return NX_NOT_ENABLED;
    }
    MSG_DEBUG("tx_thread_create() done\n");
  }

  /* USER CODE END MX_NetXDuo_Init */

  return ret;
}

/* USER CODE BEGIN 1 */
static UINT DHCP_Server_start(TX_BYTE_POOL *byte_pool)
{
  UINT status;
  UINT addresses_added;
  const ULONG stack_size = NETX_DHCP_SERVER_THREAD_STACK_SIZE;
  VOID *stack_ptr;


  /* Allocate the memory for DHCP Server thread. */
  if (tx_byte_allocate(byte_pool, &stack_ptr, stack_size, TX_NO_WAIT) != TX_SUCCESS)
  {
    return TX_POOL_ERROR;
  }

  /* Create the DHCP Server. */
  status = nx_dhcp_server_create(&DhcpServer, &IpInstance, stack_ptr, stack_size,
                                 "DHCP Server", &AppPacketPool);

  /* Check for errors creating the DHCP Server. */
  if (status)
  {
    MSG_ERROR("Creating DHCP server failed!\n");
    Error_Handler();
  }

  /* Load the assignable DHCP IP addresses for the first interface.  */
  status = nx_dhcp_create_server_ip_address_list(&DhcpServer, NX_DHCP_INTERFACE_INDEX,
                                                 START_IP_ADDRESS_LIST_0,
                                                 END_IP_ADDRESS_LIST_0, &addresses_added);

  /* Check for errors creating the list. */
  if (status)
  {
    MSG_ERROR("Creating DHCP IP address list failed!\n");
    Error_Handler();
  }

  /* Verify all the addresses were added to the list. */
  if (addresses_added != 10)
  {
    MSG_ERROR("Adding IP address failed!\n");
    Error_Handler();
  }

  /* Set the interface network parameters.  */
  status = nx_dhcp_set_interface_network_parameters(&DhcpServer, NX_DHCP_INTERFACE_INDEX,
                                                    NX_DHCP_SUBNET_MASK_0,
                                                    NX_DHCP_DEFAULT_GATEWAY_0,
                                                    NX_DHCP_DNS_SERVER_0);

  /* Check for errors setting network parameters. */
  if (status)
  {
    MSG_ERROR("Setting network parameters failed!\n");
    Error_Handler();
  }

  /* Start DHCP Server task.  */
  status = nx_dhcp_server_start(&DhcpServer);

  /* Check for errors starting up the DHCP server.  */
  if (status)
  {
    MSG_ERROR("Starting up the DHCP server failed!\n");
    Error_Handler();
  }

  MSG_DEBUG("DHCP Server started successfully !!\n");

  return NX_SUCCESS;
}


/**
  * @brief  ip address change callback.
  * @param ip_instance: NX_IP instance
  * @param ptr: user data
  * @retval none
  */
static VOID ip_address_change_notify_callback(NX_IP *ip_instance, VOID *ptr)
{
  NX_PARAMETER_NOT_USED(ip_instance);
  NX_PARAMETER_NOT_USED(ptr);

  MSG_DEBUG(">\"%s\"\n", ip_instance->nx_ip_name);
}


/**
  * @brief  Main thread entry.
  * @param thread_input: ULONG user argument used by the thread entry
  * @retval none
  */
static VOID App_Main_Thread_Entry(ULONG thread_input)
{
  UINT ret;
  TX_BYTE_POOL *const byte_pool = (TX_BYTE_POOL *) thread_input;

  MSG_DEBUG(">\n");

  /* Create the DHCP Server. */
  DHCP_Server_start(byte_pool);


#ifdef ENABLE_IOT_INFO
  {
    const UINT IpIndex = 0;
    CHAR *InterfaceName;
    ULONG IpAddress;
    ULONG NetMask;
    ULONG MtuSize;
    ULONG PhysicalAddresMsw;
    ULONG PhysicalAddressLsw;

    _nx_ip_interface_info_get(&IpInstance, IpIndex, &InterfaceName,
                              &IpAddress, &NetMask,
                              &MtuSize,
                              &PhysicalAddresMsw, &PhysicalAddressLsw);

    MSG_INFO("\nIP: \"%s\", MTU: %"PRIu32"\n", InterfaceName, (uint32_t)MtuSize);
  }
#endif /* ENABLE_IOT_INFO */

  /* Read back IP address and gateway address. */
  {
    ULONG IpAddress;
    ULONG NetMask;

    ret = nx_ip_address_get(&IpInstance, &IpAddress, &NetMask);

    if (ret != TX_SUCCESS)
    {
      Error_Handler();
    }

    MSG_INFO("\n- Network Interface connected: ");
    PRINT_IP_ADDRESS(IpAddress);
    MSG_INFO("\n");

    MSG_INFO("Mask: ");
    PRINT_IP_ADDRESS(NetMask);
  }

  MSG_INFO("\n\nAP mode set, please join \"%s\"\n\n", WIFI_SSID);


  /* The network is correctly initialized, start the TCP server thread */
  ret = http_server_socket_init(byte_pool);

  if (ret != TX_SUCCESS)
  {
    Error_Handler();
  }

  while (1)
  {
    /* Everything else is handled from TCP callbacks dispatched from
     * IP instance thread
     */
    tx_thread_suspend(tx_thread_identify());
  }

}
/* USER CODE END 1 */
