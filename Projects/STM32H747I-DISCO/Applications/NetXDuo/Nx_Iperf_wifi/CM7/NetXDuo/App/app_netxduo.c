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
#include "msg.h"
#include <stdbool.h>
#include "whd.h"
#include "whd_wifi_api.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define NX_IPERF_HTTP_STACK_SIZE         2048
#define NX_IPERF_STACK_SIZE              2048
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
static TX_THREAD AppMainThread;
static TX_THREAD AppIperfThread;

static NX_PACKET_POOL AppPacketPool;
static NX_PACKET_POOL IperfPacketPool;

static NX_IP IpInstance;

static NX_DHCP DhcpClient;
static TX_SEMAPHORE DhcpSemaphore;

static NX_DNS DnsClient;
static UCHAR DnsLocalCache[2048];
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */
static VOID App_Main_Thread_Entry(ULONG thread_input);

static VOID App_Iperf_Thread_Entry(ULONG thread_input);

static VOID ip_address_change_notify_callback(NX_IP *ip_instance, VOID *ptr);


void nx_iperf_entry(NX_PACKET_POOL *pool_ptr, NX_IP *ip_ptr,
                    UCHAR *http_stack, ULONG http_stack_size,
                    UCHAR *iperf_stack, ULONG iperf_stack_size);
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

  /* USER CODE BEGIN MX_NetXDuo_Init */

  MSG_INFO("Nx_Iperf application started..\n");
  MSG_INFO("\n\n\n\n\r#### Welcome to Iperf Application #####\n");
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

  /* Allocate the memory for Iperf packet_pool.  */
  {
    const ULONG pool_size = NX_PACKET_POOL_SIZE;
    VOID *pool_start;

    if (tx_byte_allocate(byte_pool, &pool_start, pool_size, TX_NO_WAIT) != TX_SUCCESS)
    {
      return TX_POOL_ERROR;
    }

    /* Create the Packet pool to be used for packet allocation */
    ret = nx_packet_pool_create(&IperfPacketPool, "Iperf Packet Pool", PAYLOAD_SIZE, pool_start, pool_size);

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
    ret = nx_ip_create(&IpInstance, "NetX IP Instance 0", 0, 0,
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

  /* Create the DHCP client. */
  ret = nx_dhcp_create(&DhcpClient, &IpInstance, "DHCP Client");
  if (ret != NX_SUCCESS)
  {
    return NX_NOT_ENABLED;
  }
  MSG_DEBUG("nx_dhcp_create() done\n");

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

  /* Allocate the memory for Iperf thread. */
  {
    const ULONG stack_size = APP_IPERF_THREAD_STACK_SIZE;
    VOID *stack_ptr;

    if (tx_byte_allocate(byte_pool, &stack_ptr, stack_size, TX_NO_WAIT) != TX_SUCCESS)
    {
      return TX_POOL_ERROR;
    }

    /* Create the Iperf thread. */
    ret = tx_thread_create(&AppIperfThread, "App Iperf Thread", App_Iperf_Thread_Entry,
                           (ULONG)byte_pool, stack_ptr, stack_size,
                           APP_IPERF_THREAD_PRIORITY, APP_IPERF_THREAD_PRIORITY, TX_NO_TIME_SLICE, TX_DONT_START);

    if (ret != TX_SUCCESS)
    {
      return NX_NOT_ENABLED;
    }
    MSG_DEBUG("tx_thread_create() done\n");
  }

  /* set DHCP notification callback  */
  tx_semaphore_create(&DhcpSemaphore, "DHCP Semaphore", 0);
  /* USER CODE END MX_NetXDuo_Init */

  return ret;
}

/* USER CODE BEGIN 1 */
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

  /* release the semaphore as soon as an IP address is available. */
  tx_semaphore_put(&DhcpSemaphore);
}


/**
  * @brief  Main thread entry.
  * @param thread_input: ULONG user argument used by the thread entry
  * @retval none
  */
static VOID App_Main_Thread_Entry(ULONG thread_input)
{
  UINT ret;
  ULONG dhcp_ip_address;
  UCHAR dhcp_ip_string[4];
  UINT size;
  NX_PARAMETER_NOT_USED(thread_input);

  MSG_DEBUG(">\n");

  ret = nx_ip_address_change_notify(&IpInstance, ip_address_change_notify_callback, NULL);
  if (ret != NX_SUCCESS)
  {
    Error_Handler();
  }
  MSG_DEBUG("nx_ip_address_change_notify() done\n");

  /* Start the DHCP client. */
  ret = nx_dhcp_start(&DhcpClient);
  if (ret != NX_SUCCESS)
  {
    Error_Handler();
  }
  MSG_DEBUG("nx_dhcp_start() done\n");

  /* Wait until an IP address is ready. */
  if (tx_semaphore_get(&DhcpSemaphore, DHCP_IP_TRANSITION_TIMEOUT) != TX_SUCCESS)
  {
    MSG_ERROR("ERROR: Cannot connect WiFi interface!\n");
    Error_Handler();
  }

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

  /* Check for Default Gateway IP from DHCP. */
  size = sizeof(dhcp_ip_string);
  ret = nx_dhcp_user_option_retrieve(&DhcpClient, NX_DHCP_OPTION_GATEWAYS,
                                     dhcp_ip_string, &size);
  if (ret)
  {
    Error_Handler();
  }

  /* Get the gateway address of IP instance. */
  ret = nx_ip_gateway_address_get(&IpInstance, &dhcp_ip_address);

  if (ret != TX_SUCCESS)
  {
    Error_Handler();
  }
  /*Set the gateway address for IP instance. */
  ret = nx_ip_gateway_address_set(&IpInstance, dhcp_ip_address);
  if (ret != TX_SUCCESS)
  {
    Error_Handler();
  }

  /* the network is correctly initialized, start the Iperf thread */
  tx_thread_resume(&AppIperfThread);

  /* this thread is not needed any more, we relinquish it */
  tx_thread_relinquish();

  return;
}


/**
  * @brief  DNS Create Function.
  * @param dns_ptr
  * @retval ret
  */
static UINT dns_create(NX_DNS *dns_ptr)
{
  UINT ret = NX_SUCCESS;
  MSG_DEBUG(">\n");

  /* Create a DNS instance for the Client */
  ret = nx_dns_create(dns_ptr, &IpInstance, (UCHAR *)"DNS Client");
  if (ret)
  {
    Error_Handler();
  }
  MSG_DEBUG("nx_dns_create() done\n");

#ifdef NX_DNS_CACHE_ENABLE
  ret = nx_dns_cache_initialize(dns_ptr, (VOID *)DnsLocalCache, (UINT)sizeof(DnsLocalCache));
#endif /* NX_DNS_CACHE_ENABLE*/

#ifndef NX_DNS_IP_GATEWAY_AND_DNS_SERVER
  UCHAR  dns_ip_string[4];
  ULONG dns_server_address;
  UINT  size;

  /* Obtain the IP address of the DNS server.  */
  size = sizeof(dns_ip_string);
  ret = nx_dhcp_user_option_retrieve(&DhcpClient, NX_DHCP_OPTION_DNS_SVR,
                                     dns_ip_string, &size);
  if (ret)
  {
    Error_Handler();
  }
  /*convert DNS IP from string to ulong format*/
  dns_server_address = nx_dhcp_user_option_convert(dns_ip_string);
  /*convert to network byte order*/
  dns_server_address = htonl(dns_server_address);
  /* Initialize DNS instance with the DNS server Address */
  ret = nx_dns_server_add(dns_ptr, dns_server_address);
  if (ret)
  {
    Error_Handler();
  }
#endif /* NX_DNS_IP_GATEWAY_AND_DNS_SERVER */

  return ret;
}


/**
  * @brief Iperf thread entry.
  * @param thread_input: ULONG user argument used by the thread entry
  * @retval none
  */
static VOID App_Iperf_Thread_Entry(ULONG thread_input)
{
  TX_BYTE_POOL *const byte_pool = (TX_BYTE_POOL *) thread_input;
  UINT ret = NX_SUCCESS;

  MSG_DEBUG(">\n");

  /* Create a DNS client */
  ret = dns_create(&DnsClient);

  if (ret != NX_SUCCESS)
  {
    Error_Handler();
  }

  {
    whd_mac_t mac;
    whd_wifi_get_mac_address(*Ifp, &mac);

    MSG_INFO(" - Device Name    : %s. \n", "Wlan WHD murata 1LD");
    MSG_INFO(" - Device ID      : %s. \n", "Unknown");
    MSG_INFO(" - Device Version : %s. \n", "Unknown");
    MSG_INFO(" - MAC address    : %02X.%02X.%02X.%02X.%02X.%02X\n",
             mac.octet[0], mac.octet[1], mac.octet[2],
             mac.octet[3], mac.octet[4], mac.octet[5]);
  }

  /* The network is correctly initialized, start the Iperf utility. */


  /* Allocate the memory for http and iperf stack */
  {
    const ULONG http_stack_size = NX_IPERF_HTTP_STACK_SIZE;
    UCHAR *http_stack;
    const ULONG iperf_stack_size = NX_IPERF_STACK_SIZE;
    UCHAR *iperf_stack;

    if (tx_byte_allocate(byte_pool, (VOID **)&http_stack, http_stack_size, TX_NO_WAIT) != TX_SUCCESS)
    {
      MSG_ERROR("Allocation failed!\n");
      Error_Handler();
    }

    if (tx_byte_allocate(byte_pool, (VOID **)&iperf_stack, iperf_stack_size, TX_NO_WAIT) != TX_SUCCESS)
    {
      MSG_ERROR("Allocation failed!\n");
      Error_Handler();
    }

    MSG_INFO("\n##### Please open a browser window with the Target board's IP address\n\n");

    /* Application body. */
    nx_iperf_entry(&IperfPacketPool, &IpInstance, http_stack, http_stack_size, iperf_stack, iperf_stack_size);
  }
}
/* USER CODE END 1 */
