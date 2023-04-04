/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    app_netxduo.c
  * @author  MCD Application Team
  * @brief   NetXDuo applicative file
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2021 STMicroelectronics.
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
#include "nxd_dns.h"
#include "nx_ip.h"
#include "msg.h"
#include "nxd_sntp_client.h"
#include <stdbool.h>

#include <time.h>
#include "timing_system.h"
#include "nx_user.h"
#include "nx_secure_user.h"

#if defined(BROKER_CONF1)
#include MOSQUITTO_CERT_FILE
#define BROKER_CA_CERT      mosquitto_org_der
#define BROKER_CA_CERT_SIZE mosquitto_org_der_len

#elif defined(BROKER_CONF2)
#include BROKER_EMQX_IO_CERT_FILE
#define BROKER_CA_CERT      broker_emqx_io_ca_der
#define BROKER_CA_CERT_SIZE broker_emqx_io_ca_der_len

#else
#endif /* BROKER_CONF1 */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* EPOCH_TIME_DIFF is equivalent to 70 years in sec
   calculated with www.epochconverter.com/date-difference
   This constant is used to delete difference between :
   Epoch converter (referenced to 1970) and SNTP (referenced to 1900) */
#define EPOCH_TIME_DIFF             2208988800

#define SAMPLE_SNTP_SYNC_MAX            (uint32_t)30
#define SAMPLE_SNTP_UPDATE_MAX          (uint32_t)10
#define SAMPLE_SNTP_UPDATE_INTERVAL     (NX_IP_PERIODIC_RATE / 2)

#define NTP_SERVER_0        "0.pool.ntp.org"
#define NTP_SERVER_1        "1.pool.ntp.org"
#define NTP_SERVER_2        "2.pool.ntp.org"
#define NTP_SERVER_3        "3.pool.ntp.org"

#define RETRY_TIMES (uint8_t)3
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
static TX_THREAD AppMainThread;
static TX_THREAD AppMQTTClientThread;
static TX_EVENT_FLAGS_GROUP AppMQTTClientFlag;

TX_BYTE_POOL *AppBytePool;

static NX_PACKET_POOL AppPacketPool;

static NX_IP IpInstance;

static NX_DHCP DhcpClient;
static TX_SEMAPHORE DhcpSemaphore;

static NX_DNS DnsClient;

#ifdef NX_DNS_CACHE_ENABLE
static UCHAR DnsLocalCache[2048];
#endif /* NX_DNS_CACHE_ENABLE */

static NX_SNTP_CLIENT SntpClient;

static NXD_MQTT_CLIENT MqttClient;
static ULONG MqttClientStack[MQTT_CLIENT_STACK_SIZE];

/* Declare buffers to hold message and topic. */

#ifdef NX_SECURE_ENABLE
/* TLS buffers and certificate containers. */
extern const NX_SECURE_TLS_CRYPTO nx_crypto_tls_ciphers;
/* calculated with nx_secure_tls_metadata_size_calculate */
static CHAR crypto_metadata_client[11600 * 2];
/* Define the TLS packet reassembly buffer. */
static UCHAR tls_packet_buffer[4000 * 2];
#endif /* NX_SECURE_ENABLE */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */
static VOID App_Main_Thread_Entry(ULONG thread_input);

static VOID App_MQTT_Client_Thread_Entry(ULONG thread_input);

static VOID ip_address_change_notify_callback(NX_IP *ip_instance, VOID *ptr);

static void NetXDuo_DeInit(void);

static UINT dns_create(NX_DNS *dns_ptr);
static UINT message_generate(void);

#ifdef NX_SECURE_ENABLE
static ULONG tls_session_time_function(VOID);
static UINT mqtt_client_tls_setup_callback(NXD_MQTT_CLIENT *client_ptr,
                                           NX_SECURE_TLS_SESSION *TLS_session_ptr,
                                           NX_SECURE_X509_CERT *certificate_ptr,
                                           NX_SECURE_X509_CERT *trusted_certificate_ptr);

#endif /* NX_SECURE_ENABLE */

static VOID mqtt_client_disconnect_notify(NXD_MQTT_CLIENT *client_ptr);
static VOID mqtt_client_receive_notify(NXD_MQTT_CLIENT *client_ptr, UINT number_of_messages);

static UINT sntp_time_sync_internal(ULONG sntp_server_address);
static UINT sntp_time_sync(VOID);

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
  AppBytePool = byte_pool;
  /* USER CODE END MX_NetXDuo_MEM_POOL */

  /* USER CODE BEGIN 0 */

  /* USER CODE END 0 */

  /* USER CODE BEGIN MX_NetXDuo_Init */

  MSG_INFO("Nx_MQTT_Client application started..\n");
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

  /* Allocate the memory for MQTT client thread. */
  {
    const ULONG stack_size = MQTT_CLIENT_THREAD_STACK_SIZE;
    VOID *stack_ptr;

    if (tx_byte_allocate(byte_pool, &stack_ptr, stack_size, TX_NO_WAIT) != TX_SUCCESS)
    {
      return TX_POOL_ERROR;
    }

    /* Create the MQTT client thread. */
    ret = tx_thread_create(&AppMQTTClientThread, "App MQTT Thread", App_MQTT_Client_Thread_Entry,
                           (ULONG)&AppPacketPool, stack_ptr, stack_size,
                           MQTT_CLIENT_THREAD_PRIORITY, MQTT_CLIENT_THREAD_PRIORITY, TX_NO_TIME_SLICE, TX_DONT_START);

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
  UINT ret = NX_SUCCESS;

  NX_PARAMETER_NOT_USED(thread_input);

  MSG_DEBUG(">\n");

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
    ULONG GatewayAddress;
    UCHAR dhcp_ip_string[4];
    UINT size;

    ret = nx_ip_address_get(&IpInstance, &IpAddress, &NetMask);

    if (ret != TX_SUCCESS)
    {
      Error_Handler();
    }

    MSG_INFO("\n- Network Interface connected: ");
    PRINT_IP_ADDRESS(IpAddress);
    MSG_INFO("\n");

    /* Check for Default Gateway IP from DHCP. */
    size = sizeof(dhcp_ip_string);
    ret = nx_dhcp_user_option_retrieve(&DhcpClient, NX_DHCP_OPTION_GATEWAYS,
                                       dhcp_ip_string, &size);
    if (ret)
    {
      Error_Handler();
    }

    /* Get the gateway address of IP instance. */
    ret = nx_ip_gateway_address_get(&IpInstance, &GatewayAddress);

    if (ret != TX_SUCCESS)
    {
      Error_Handler();
    }
    /*Set the gateway address for IP instance. */
    ret = nx_ip_gateway_address_set(&IpInstance, GatewayAddress);
    if (ret != TX_SUCCESS)
    {
      Error_Handler();
    }

    MSG_INFO("Mask: ");
    PRINT_IP_ADDRESS(NetMask);

    MSG_INFO("Gateway: ");
    PRINT_IP_ADDRESS(GatewayAddress);

    /* Create a DNS client */
    ret = dns_create(&DnsClient);
    if (ret != NX_SUCCESS)
    {
      MSG_DEBUG("dns_create failed: 0x%x\n", ret);
      Error_Handler();
    }
  }

  /* Sync up time by SNTP at start up. */
  ret = sntp_time_sync();

  /* Check status.  */
  if (ret != NX_SUCCESS)
  {
    MSG_INFO("SNTP Time Sync failed.\n");
  }
  else
  {
    MSG_INFO("SNTP Time Sync successfully.\n");
    /* Waste 2 seconds, to see if seconds evolve normally in the system. */
    tx_thread_sleep(2 * TX_TIMER_TICKS_PER_SECOND);

    {
      time_t the_time = time(0);

      MSG_DEBUG("<%"PRIu32"\n", (uint32_t)the_time);
      MSG_INFO("Updated time: %s\n", ctime(&the_time));
    }
  }

  /* The network is correctly initialized, start the MQTT client thread. */
  tx_thread_resume(&AppMQTTClientThread);

  /* this thread is not needed any more, we relinquish it */
  tx_thread_relinquish();

  return;
}


/* Declare the disconnect notify function. */
static VOID mqtt_client_disconnect_notify(NXD_MQTT_CLIENT *client_ptr)
{
  NX_PARAMETER_NOT_USED(client_ptr);
  MSG_INFO("Client disconnected from broker < %s >.\n", MQTT_BROKER_NAME);
}


/* Declare the notify function. */
static VOID mqtt_client_receive_notify(NXD_MQTT_CLIENT *client_ptr, UINT number_of_messages)
{
  NX_PARAMETER_NOT_USED(client_ptr);
  NX_PARAMETER_NOT_USED(number_of_messages);

  MSG_DEBUG(">\n");

  tx_event_flags_set(&AppMQTTClientFlag, DEMO_MESSAGE_EVENT, TX_OR);
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
  * @brief  message generation Function.
  * @param  RandomNbr
  * @retval none
  */
static UINT message_generate(void)
{
  UINT RandomNbr = (UINT)NX_RAND();

  return RandomNbr %= 9999;
}


#ifdef NX_SECURE_ENABLE

static ULONG tls_session_time_function(VOID)
{
  time_t the_time = time(0);

  MSG_DEBUG("<%"PRIu32"\n", (uint32_t)the_time);
  MSG_DEBUG("time: %s\n", ctime(&the_time));
  return the_time;
}


/* Callback to setup TLS parameters for secure MQTT connection. */
static UINT mqtt_client_tls_setup_callback(NXD_MQTT_CLIENT *client_ptr,
                                           NX_SECURE_TLS_SESSION *TLS_session_ptr,
                                           NX_SECURE_X509_CERT *certificate_ptr,
                                           NX_SECURE_X509_CERT *trusted_certificate_ptr)
{
  UINT ret = NX_SUCCESS;
  NX_PARAMETER_NOT_USED(client_ptr);

  /* Initialize TLS module */
  nx_secure_tls_initialize();

  /* Create a TLS session */
  ret = nx_secure_tls_session_create(TLS_session_ptr, &nx_crypto_tls_ciphers,
                                     crypto_metadata_client, sizeof(crypto_metadata_client));
  if (ret != NX_SUCCESS)
  {
    Error_Handler();
  }

  ret = nx_secure_tls_session_time_function_set(TLS_session_ptr, tls_session_time_function);
  if (ret != NX_SUCCESS)
  {
    Error_Handler();
  }

  /* Need to allocate space for the certificate coming in from the broker. */
  memset((void *)(certificate_ptr), 0, sizeof(NX_SECURE_X509_CERT));

  /* Allocate space for packet reassembly. */
  ret = nx_secure_tls_session_packet_buffer_set(TLS_session_ptr, tls_packet_buffer,
                                                sizeof(tls_packet_buffer));
  if (ret != NX_SUCCESS)
  {
    Error_Handler();
  }

  /* allocate space for the certificate coming in from the remote host */
  ret = nx_secure_tls_remote_certificate_allocate(TLS_session_ptr, certificate_ptr,
                                                  tls_packet_buffer, sizeof(tls_packet_buffer));
  if (ret != NX_SUCCESS)
  {
    Error_Handler();
  }

  /* initialize Certificate to verify incoming server certificates. */
  ret = nx_secure_x509_certificate_initialize(trusted_certificate_ptr, (UCHAR *)BROKER_CA_CERT,
                                              (USHORT)BROKER_CA_CERT_SIZE, NX_NULL, 0, NULL, 0,
                                              NX_SECURE_X509_KEY_TYPE_NONE);
  if (ret != NX_SUCCESS)
  {
    MSG_ERROR("Certificate issue.. (0x%"PRIx32")\n", (uint32_t)ret);
    MSG_ERROR("Please make sure that your X509_certificate is valid.\n");
    Error_Handler();
  }

  /* Add a CA Certificate to our trusted store */
  ret = nx_secure_tls_trusted_certificate_add(TLS_session_ptr, trusted_certificate_ptr);
  if (ret != NX_SUCCESS)
  {
    Error_Handler();
  }

  MSG_DEBUG("<\n");
  return ret;
}
#endif /* NX_SECURE_ENABLE */


/**
  * @brief  MQTT Client thread entry.
  * @param thread_input: ULONG user argument used by the thread entry
  * @retval none
  */
static VOID App_MQTT_Client_Thread_Entry(ULONG thread_input)
{
  NX_PACKET_POOL *const pool_ptr = (NX_PACKET_POOL *) thread_input;
  UINT ret = NX_SUCCESS;
  NXD_ADDRESS mqtt_server_ip = {0};
  ULONG events;
  UINT topic_length;
  UINT remaining_msg = NB_MESSAGE;
  UINT message_count = 0;
  UINT unlimited_publish = NX_FALSE;

  MSG_DEBUG(">\n\n");

#if defined(NX_SECURE_INCLUDE_USER_DEFINE_FILE)
  MSG_INFO("NX_SECURE_INCLUDE_USER_DEFINE_FILE               defined\n");
#endif /* NX_SECURE_INCLUDE_USER_DEFINE_FILE */

#if defined(NX_SECURE_AEAD_CIPHER_CHECK)
  MSG_INFO("NX_SECURE_AEAD_CIPHER_CHECK                      defined\n");
#endif /* NX_SECURE_AEAD_CIPHER_CHECK */

#if defined(NX_SECURE_ENABLE_AEAD_CIPHER)
  MSG_INFO("NX_SECURE_ENABLE_AEAD_CIPHER                     defined\n");
#endif /* NX_SECURE_AEAD_CIPHER_CHECK */

#if defined(NX_SECURE_ENABLE_ECC_CIPHERSUITE)
  MSG_INFO("NX_SECURE_ENABLE_ECC_CIPHERSUITE                 defined\n");
#endif /* NX_SECURE_ENABLE_ECC_CIPHERSUITE */

#if defined(NX_SECURE_ENABLE_PSK_CIPHERSUITES)
  MSG_INFO("NX_SECURE_ENABLE_PSK_CIPHERSUITES                defined\n");
#endif /* NX_SECURE_ENABLE_PSK_CIPHERSUITES */

#if defined(NX_SECURE_TLS_DISABLE_PROTOCOL_VERSION_DOWNGRADE)
  MSG_INFO("NX_SECURE_TLS_DISABLE_PROTOCOL_VERSION_DOWNGRADE defined\n");
#endif /* NX_SECURE_TLS_DISABLE_PROTOCOL_VERSION_DOWNGRADE */

#if defined(NX_SECURE_TLS_DISABLE_TLS_1_1)
  MSG_INFO("NX_SECURE_TLS_DISABLE_TLS_1_1                    defined\n");
#endif /* NX_SECURE_TLS_DISABLE_TLS_1_1 */

#if defined(NX_SECURE_TLS_DISABLE_TLS_1_2)
  MSG_INFO("NX_SECURE_TLS_DISABLE_TLS_1_2                    defined\n");
#endif /* NX_SECURE_TLS_DISABLE_TLS_1_2 */

#if defined(NX_SECURE_TLS_ENABLE_TLS_1_3)
  MSG_INFO("NX_SECURE_TLS_ENABLE_TLS_1_3                     defined\n");
#endif /* NX_SECURE_TLS_ENABLE_TLS_1_3 */

#if defined(NX_SECURE_TLS_SERVER_DISABLED)
  MSG_INFO("NX_SECURE_TLS_SERVER_DISABLED                    defined\n");
#endif /* NX_SECURE_TLS_SERVER_DISABLED */

#if (defined(NX_SECURE_TLS_TLS_1_3_ENABLED) && (NX_SECURE_TLS_TLS_1_3_ENABLED == 1))
  MSG_INFO("NX_SECURE_TLS_TLS_1_3_ENABLED                    set to 1\n");
#endif /* NX_SECURE_TLS_TLS_1_3_ENABLED */

  mqtt_server_ip.nxd_ip_version = NX_IP_VERSION_V4;

  /* Look up MQTT Server address. */
  ret = nx_dns_host_by_name_get(&DnsClient, (UCHAR *)MQTT_BROKER_NAME,
                                &mqtt_server_ip.nxd_ip_address.v4, DEFAULT_TIMEOUT);

  /* Check status.  */
  if (ret != NX_SUCCESS)
  {
    Error_Handler();
  }

  const UINT server_port =
#ifdef NX_SECURE_ENABLE
    NXD_MQTT_TLS_PORT;
#else
    NXD_MQTT_PORT;
#endif /*NX_SECURE_ENABLE*/

  MSG_INFO("Broker Ip address: ");
  PRINT_IP_ADDRESS(mqtt_server_ip.nxd_ip_address.v4);
  MSG_INFO("\n");
  MSG_INFO("Broker port: %"PRIu32"\n", (uint32_t)server_port);

  /* Create MQTT client instance. */
  ret = nxd_mqtt_client_create(&MqttClient, "my_client", CLIENT_ID_STRING, STRLEN(CLIENT_ID_STRING),
                               &IpInstance, pool_ptr, (VOID *)MqttClientStack, MQTT_CLIENT_STACK_SIZE,
                               MQTT_THREAD_PRIORTY, NX_NULL, 0);

  if (ret != NX_SUCCESS)
  {
    Error_Handler();
  }

  /* Register the disconnect notification function. */
  nxd_mqtt_client_disconnect_notify_set(&MqttClient, mqtt_client_disconnect_notify);

  /* Set the receive notify function. */
  nxd_mqtt_client_receive_notify_set(&MqttClient, mqtt_client_receive_notify);

  /* Create an MQTT flag */
  ret = tx_event_flags_create(&AppMQTTClientFlag, "my app event");
  if (ret != TX_SUCCESS)
  {
    Error_Handler();
  }

  /* Start a connection to the server. */
#ifdef NX_SECURE_ENABLE
  ret = nxd_mqtt_client_secure_connect(&MqttClient, &mqtt_server_ip, server_port, mqtt_client_tls_setup_callback,
                                       MQTT_KEEP_ALIVE_TIMER, CLEAN_SESSION, NX_WAIT_FOREVER);
#else
  ret = nxd_mqtt_client_connect(&MqttClient, &mqtt_server_ip, server_port,
                                MQTT_KEEP_ALIVE_TIMER, CLEAN_SESSION, NX_WAIT_FOREVER);
#endif /*NX_SECURE_ENABLE*/

  if (ret != NX_SUCCESS)
  {
    MSG_ERROR("\nMQTT client failed to connect to broker < %s > with error: 0x%"PRIx32".\n",
              MQTT_BROKER_NAME, (uint32_t)ret);
    Error_Handler();
  }
  else
  {
    MSG_INFO("\nMQTT client connected to broker < %s > at PORT:  %"PRIu32"\n",
             MQTT_BROKER_NAME, (uint32_t)server_port);
  }

  /* Subscribe to the topic with QoS level 1. */
  ret = nxd_mqtt_client_subscribe(&MqttClient, TOPIC_NAME, STRLEN(TOPIC_NAME), QOS1);

  if (ret != NX_SUCCESS)
  {
    Error_Handler();
  }

  if (NB_MESSAGE == 0)
  {
    unlimited_publish = NX_TRUE;
  }

  while (unlimited_publish || remaining_msg)
  {
    static char message[NXD_MQTT_MAX_MESSAGE_LENGTH];
    const size_t message_size = sizeof(message);

    memset(message, 0, message_size);
    snprintf(message, message_size, "%04"PRIu32, (uint32_t)message_generate());

    /* Publish a message with QoS Level 1. */
    ret = nxd_mqtt_client_publish(&MqttClient, TOPIC_NAME, STRLEN(TOPIC_NAME),
                                  (CHAR *)message, strlen(message), NX_TRUE, QOS1, NX_WAIT_FOREVER);
    if (ret != NX_SUCCESS)
    {
      Error_Handler();
    }
    else
    {
      MSG_DEBUG("[%06"PRIu32"] Message \"%s\" published\n", HAL_GetTick(), message);
    }

    /* Wait for the broker to publish the message. */
    tx_event_flags_get(&AppMQTTClientFlag, DEMO_ALL_EVENTS, TX_OR_CLEAR, &events, TX_WAIT_FOREVER);

    /* check event received */
    if (events & DEMO_MESSAGE_EVENT)
    {
      static UCHAR topic_buffer[NXD_MQTT_MAX_TOPIC_NAME_LENGTH];
      const UINT topic_buffer_size = sizeof(topic_buffer);

      static UCHAR message_buffer[NXD_MQTT_MAX_MESSAGE_LENGTH];
      const UINT message_buffer_size = sizeof(message_buffer);

      UINT message_length;

      memset(message_buffer, 0, message_buffer_size);
      memset(topic_buffer, 0, topic_buffer_size);

      /* Get the message from the broker. */
      ret = nxd_mqtt_client_message_get(&MqttClient, topic_buffer, topic_buffer_size, &topic_length,
                                        message_buffer, message_buffer_size, &message_length);
      if (ret == NXD_MQTT_SUCCESS)
      {
        MSG_INFO("[%06"PRIu32"] Message %02"PRIu32" received: TOPIC = \"%s\", MESSAGE = \"%s\"\n", HAL_GetTick(),
                 (uint32_t)(message_count + 1), topic_buffer, message_buffer);
      }
      else
      {
        Error_Handler();
      }
    }

    /* Decrement message number. */
    remaining_msg-- ;
    message_count++ ;

    /* Delay 1s between each publication. */
    tx_thread_sleep(TX_TIMER_TICKS_PER_SECOND);
  }

  /* Publish an empty message with QoS Level 1. */
  /* Avoid the message being retained by the broker to get the same number of sent/received messages. */
  ret = nxd_mqtt_client_publish(&MqttClient, TOPIC_NAME, STRLEN(TOPIC_NAME),
                                NULL, 0, NX_TRUE, QOS1, NX_WAIT_FOREVER);
  if (ret != NX_SUCCESS)
  {
    Error_Handler();
  }
  else
  {
    MSG_INFO("[%06"PRIu32"] Empty message published at end\n", HAL_GetTick());
  }

  /* Now un-subscribe the topic. */
  ret = nxd_mqtt_client_unsubscribe(&MqttClient, TOPIC_NAME, STRLEN(TOPIC_NAME));

  if (ret != NX_SUCCESS)
  {
    Error_Handler();
  }
  MSG_DEBUG("nxd_mqtt_client_unsubscribe() done\n");

  /* Disconnect from the broker. */
  ret = nxd_mqtt_client_disconnect(&MqttClient);

  if (ret != NX_SUCCESS)
  {
    Error_Handler();
  }
  MSG_DEBUG("nxd_mqtt_client_disconnect() done\n");

  /* Delete the client instance, release all the resources. */
  ret = nxd_mqtt_client_delete(&MqttClient);

  if (ret != NX_SUCCESS)
  {
    Error_Handler();
  }
  MSG_DEBUG("nxd_mqtt_client_delete() done\n");

  NetXDuo_DeInit();

  /* test OK -> success Handler */
  Success_Handler();
}



/* Sync up the local time.  */
static UINT sntp_time_sync_internal(ULONG sntp_server_address)
{
  UINT ret;

  MSG_DEBUG(">\n");

  /* Create the SNTP Client to run in broadcast mode.. */
  ret = nx_sntp_client_create(&SntpClient, &IpInstance, 0, &AppPacketPool,
                              NX_NULL,
                              NX_NULL,
                              NX_NULL /* no random_number_generator callback */);

  /* Check status.  */
  if (ret != NX_SUCCESS)
  {
    return ret;
  }
  MSG_DEBUG("nx_sntp_client_create() done\n");

  /* Use the IPv4 service to initialize the Client and set the IPv4 SNTP server. */
  ret = nx_sntp_client_initialize_unicast(&SntpClient, sntp_server_address);

  /* Check status.  */
  if (ret != NX_SUCCESS)
  {
    nx_sntp_client_delete(&SntpClient);
    return ret;
  }
  MSG_DEBUG("nx_sntp_client_initialize_unicast() done\n");

  /* Set local time to 0 */
  ret = nx_sntp_client_set_local_time(&SntpClient, 0, 0);

  /* Check status.  */
  if (ret != NX_SUCCESS)
  {
    nx_sntp_client_delete(&SntpClient);
    return ret;
  }

  /* Run Unicast client */
  ret = nx_sntp_client_run_unicast(&SntpClient);

  /* Check status.  */
  if (ret != NX_SUCCESS)
  {
    nx_sntp_client_stop(&SntpClient);
    nx_sntp_client_delete(&SntpClient);
    return ret;
  }

  /* Wait till updates are received */
  for (uint32_t i = 0; i < SAMPLE_SNTP_UPDATE_MAX; i++)
  {
    UINT server_status;

    /* First verify we have a valid SNTP service running. */
    ret = nx_sntp_client_receiving_updates(&SntpClient, &server_status);

    /* Check status.  */
    if ((ret == NX_SUCCESS) && (server_status == NX_TRUE))
    {
      /* Server status is good. Now get the Client local time. */
      ULONG sntp_seconds;
      ULONG sntp_fraction;

      /* Get the local time. */
      ret = nx_sntp_client_get_local_time_extended(&SntpClient,
                                                   &sntp_seconds, &sntp_fraction,
                                                   NULL, 0);

      /* Check status. */
      if (ret != NX_SUCCESS)
      {
        continue;
      }

      const time_t epoch_time_now = sntp_seconds - EPOCH_TIME_DIFF;
      TimingSystemSetSystemTime(epoch_time_now);

      /* Stop and delete SNTP. */
      nx_sntp_client_stop(&SntpClient);
      nx_sntp_client_delete(&SntpClient);

      MSG_DEBUG("<\n");
      return NX_SUCCESS;
    }

    /* Sleep.  */
    tx_thread_sleep(SAMPLE_SNTP_UPDATE_INTERVAL);
  }

  /* Time sync failed.  */

  /* Stop and delete SNTP.  */
  nx_sntp_client_stop(&SntpClient);
  nx_sntp_client_delete(&SntpClient);

  /* Return success.  */
  return NX_NOT_SUCCESSFUL;
}


static UINT sntp_time_sync(VOID)
{
  UINT status;
  static const char *sntp_servers[] =
  {
    NTP_SERVER_0,
    NTP_SERVER_1,
    NTP_SERVER_2,
    NTP_SERVER_3
  };
  static UINT sntp_server_index;
  ULONG sntp_server_address[3];

  MSG_DEBUG(">\n");

  /* Sync time by NTP servers. */
  for (uint32_t i = 0; i < SAMPLE_SNTP_SYNC_MAX; i++)
  {
    printf("SNTP Time Sync...%s\r\n", sntp_servers[sntp_server_index]);

    /* Look up SNTP Server address. */
    status = nx_dns_host_by_name_get(&DnsClient, (UCHAR *)sntp_servers[sntp_server_index], &sntp_server_address[0],
                                     5 * NX_IP_PERIODIC_RATE);

    /* Check status.  */
    if (status == NX_SUCCESS)
    {
      /* Start SNTP to sync the local time. */
      status = sntp_time_sync_internal(sntp_server_address[0]);

      /* Check status.  */
      if (status == NX_SUCCESS)
      {
        return NX_SUCCESS;
      }
    }

    /* Switch SNTP server every time.  */
    sntp_server_index = (sntp_server_index + 1) % (sizeof(sntp_servers) / sizeof(sntp_servers[0]));
  }

  return NX_NOT_SUCCESSFUL;
}


static void NetXDuo_DeInit(void)
{
  MSG_DEBUG(">\n");

  /* delete dhcp client */
  {
    const UINT status = nx_dhcp_delete(&DhcpClient);
    MSG_INFO("nx_dhcp_delete() done with 0x%"PRIx32"\n", (uint32_t)status);
  }

  /* delete dns client */
  {
    const UINT status = nx_dns_delete(&DnsClient);
    MSG_INFO("nx_dns_delete() done with 0x%"PRIx32"\n", (uint32_t)status);
  }

  /* delete Main Ip Instance */
  {
    const UINT status = nx_ip_delete(&IpInstance);
    MSG_INFO("nx_ip_delete() done with 0x%"PRIx32"\n", (uint32_t)status);
  }

  {
    CHAR *name = TX_NULL;
    ULONG available_bytes = 0;
    ULONG fragments = 0;
    TX_THREAD *first_suspended;
    ULONG suspended_count;
    TX_BYTE_POOL *next_pool;

    tx_byte_pool_info_get(AppBytePool, &name,
                          &available_bytes, &fragments,
                          &first_suspended, &suspended_count, &next_pool);
    MSG_INFO("Pool \"%s\" (%"PRIu32")\n", (name != TX_NULL) ? name : "", (uint32_t)AppBytePool->tx_byte_pool_size);
    MSG_INFO(" - available bytes: %"PRIu32"\n", (uint32_t)available_bytes);
    MSG_INFO(" - fragments      : %"PRIu32"\n", (uint32_t)fragments);
    MSG_INFO(" - suspended count: %"PRIu32"\n", (uint32_t)suspended_count);
  }

  {
    ULONG total_packets = 0;
    ULONG free_packets = 0;
    ULONG empty_pool_requests = 0;
    ULONG empty_pool_suspensions = 0;
    ULONG invalid_packet_releases = 0;

    nx_packet_pool_info_get(&AppPacketPool,
                            &total_packets, &free_packets,
                            &empty_pool_requests, &empty_pool_suspensions,
                            &invalid_packet_releases);
    MSG_INFO("Packet Pool \"%s\" (%"PRIu32")\n",
             AppPacketPool.nx_packet_pool_name,
             (uint32_t)AppPacketPool.nx_packet_pool_payload_size);
    MSG_INFO(" - total packets          : %"PRIu32"\n", (uint32_t)total_packets);
    MSG_INFO(" - free packets           : %"PRIu32"\n", (uint32_t)free_packets);
    MSG_INFO(" - invalid packet releases: %"PRIu32"\n", (uint32_t)invalid_packet_releases);
  }

  MSG_DEBUG("<\n");
}
/* USER CODE END 1 */
