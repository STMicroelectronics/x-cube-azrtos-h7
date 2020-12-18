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
#include "stm32h735g_discovery.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

TX_THREAD AppMainThread;
TX_THREAD AppUDPThread;

TX_SEMAPHORE Semaphore;

NX_PACKET_POOL AppPool;

NX_IP IpInstance;
NX_DHCP DHCPClient;
NX_UDP_SOCKET UDPSocket;
ULONG IpAddress;
ULONG NetMask;

UCHAR *pointer;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */
static VOID App_Main_Thread_Entry(ULONG thread_input);
static VOID App_UDP_Thread_Entry(ULONG thread_input);

static VOID ip_address_change_notify_callback(NX_IP *ip_instance, VOID *ptr);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/* USER CODE END 0 */

/**
* @brief  Application NetXDuo initialization.
* @param memory_ptr: memory pointer
* @retval int
*/
UINT App_NetXDuo_Init(VOID *memory_ptr)
{
  UINT ret = NX_SUCCESS;
  /* USER CODE BEGIN  App_NetXDuo_Init */
  pointer = (UCHAR *) memory_ptr;

  /* Create the Packet pool to be used for packet allocation */
  ret = nx_packet_pool_create(&AppPool, "Main Packet Pool", PAYLOAD_SIZE, pointer, NX_PACKET_POOL_SIZE);

  if (ret != NX_SUCCESS)
  {
    return NX_NOT_ENABLED;
  }

  /* Increment the memory pointer by the size of the packet pool to avoid memory overlap */
  pointer = pointer + NX_PACKET_POOL_SIZE;

  /* Create the main NX_IP instance */
  ret = nx_ip_create(&IpInstance, "Main Ip instance", NULL_ADDRESS, NULL_ADDRESS, &AppPool,
                     nx_driver_stm32h7xx, pointer, 2 * DEFAULT_MEMORY_SIZE, DEFAULT_PRIORITY);

  if (ret != NX_SUCCESS)
  {
    return NX_NOT_ENABLED;
  }

  pointer = pointer + (2 * DEFAULT_MEMORY_SIZE);

  /* Enable the ARP protocol and provide the ARP cache size for the IP instance */
  ret = nx_arp_enable(&IpInstance, (VOID *)pointer, DEFAULT_MEMORY_SIZE);

  if (ret != NX_SUCCESS)
  {
    return NX_NOT_ENABLED;
  }

  pointer = pointer + DEFAULT_MEMORY_SIZE;

  /* Enable the ICMP */

  ret = nx_icmp_enable(&IpInstance);

  if (ret != NX_SUCCESS)
  {
    return NX_NOT_ENABLED;
  }

    /* Enable the UDP protocol required for  DHCP communication */
  ret = nx_udp_enable(&IpInstance);

  /* Create the main thread */
  ret = tx_thread_create(&AppMainThread, "App Main thread", App_Main_Thread_Entry, 0, pointer, 2 * DEFAULT_MEMORY_SIZE,
                   DEFAULT_PRIORITY, DEFAULT_PRIORITY, TX_NO_TIME_SLICE, TX_AUTO_START);

  if (ret != TX_SUCCESS)
  {
    return NX_NOT_ENABLED;
  }

  pointer =  pointer + (2 * DEFAULT_MEMORY_SIZE);

  /* create the TCP server thread */
  ret = tx_thread_create(&AppUDPThread, "App UDP Thread", App_UDP_Thread_Entry, 0, pointer, 2 * DEFAULT_MEMORY_SIZE,
                   DEFAULT_PRIORITY, DEFAULT_PRIORITY, TX_NO_TIME_SLICE, TX_DONT_START);

  if (ret != TX_SUCCESS)
  {
    return NX_NOT_ENABLED;
  }

  pointer =  pointer + (2 * DEFAULT_MEMORY_SIZE);


  /* create the DHCP client */
  ret = nx_dhcp_create(&DHCPClient, &IpInstance, "DHCP Client");

  if (ret != NX_SUCCESS)
  {
    return NX_NOT_ENABLED;
  }

  /* set DHCP notification callback  */

  tx_semaphore_create(&Semaphore, "DHCP Semaphore", 0);
  /* USER CODE END  App_NetXDuo_Init */

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
  /* release the semaphore as soon as an IP address is available */
  tx_semaphore_put(&Semaphore);
}

/* USER CODE BEGIN 1 */
/**
* @brief  Main thread entry.
* @param thread_input: ULONG user argument used by the thread entry
* @retval none
*/
static VOID App_Main_Thread_Entry(ULONG thread_input)
{
  UINT ret;
  ret = nx_ip_address_change_notify(&IpInstance, ip_address_change_notify_callback, NULL);
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

  ret = nx_ip_address_get(&IpInstance, &IpAddress, &NetMask);

  PRINT_IP_ADDRESS(IpAddress);

  if (ret != TX_SUCCESS)
  {
     Error_Handler();
  }
  /* the network is correctly initialized, start the TCP server thread */
  tx_thread_resume(&AppUDPThread);
  /* this thread is not needed any more, we relinquish it */
  tx_thread_relinquish();

  return;
}

static VOID App_UDP_Thread_Entry(ULONG thread_input)
{
  UINT ret;
  UINT count = 0;
  ULONG bytes_read;
  NX_PACKET *server_packet;
  UCHAR data_buffer[512];

  NX_PACKET *data_packet;

  /* create the UDP socket */
  ret = nx_udp_socket_create(&IpInstance, &UDPSocket, "UDP Server Socket", NX_IP_NORMAL, NX_FRAGMENT_OKAY, NX_IP_TIME_TO_LIVE, 512);

  if (ret != NX_SUCCESS)
   Error_Handler();


  ret = nx_udp_socket_bind(&UDPSocket, DEFAULT_PORT, TX_WAIT_FOREVER);

  if (ret != NX_SUCCESS)
    Error_Handler();

   while(count++ < MAX_PACKET_COUNT)
   {

      /* create the packet to send over the UDP socket */
      ret = nx_packet_allocate(&AppPool, &data_packet, NX_UDP_PACKET, TX_WAIT_FOREVER);

      if (ret != NX_SUCCESS)
      {
        Error_Handler();
      }

      ret = nx_packet_data_append(data_packet, (VOID *)DEFAULT_MESSAGE, sizeof(DEFAULT_MESSAGE), &AppPool, TX_WAIT_FOREVER);

      if (ret != NX_SUCCESS)
      {
        Error_Handler();
      }

      ret = nx_udp_socket_send(&UDPSocket, data_packet, UDP_SERVER_ADDRESS, UDP_SERVER_PORT);

      TX_MEMSET(data_buffer, '\0', sizeof(data_buffer));

      /* send the message */
      ret = nx_udp_socket_send(&UDPSocket, data_packet, UDP_SERVER_ADDRESS, UDP_SERVER_PORT);

     /* wait 10 sec to receive response from the server */
      ret = nx_udp_socket_receive(&UDPSocket, &server_packet, 10 * NX_IP_PERIODIC_RATE);

      if (ret == NX_SUCCESS)
      {
        ULONG source_ip_address;
        UINT source_port;

        /* get the server IP address and  port */
        nx_udp_source_extract(server_packet, &source_ip_address, &source_port);

        /* retrieve the data sent by the server */
        nx_packet_data_retrieve(server_packet, data_buffer, &bytes_read);

        /* print the received data */
        PRINT_DATA(source_ip_address, source_port, data_buffer);

        /* release the server packet */
        nx_packet_release(server_packet);

        /* release the packet initially allocated */
        nx_packet_release(data_packet);

        /* toggle the green led on success */
        BSP_LED_Toggle(LED_GREEN);
      }
      else
      {
       /* connection lost with the server, exit the loop */
       break;
      }
   }

  if (count == MAX_PACKET_COUNT + 1)
  {
    printf("\n-------------------------------------\n\tSUCCESS : %u / %u packets sent\n-------------------------------------\n", count - 1, MAX_PACKET_COUNT);
    Success_Handler();
  }
  else
  {
    printf("\n-------------------------------------\n\tFAIL : %u / %u packets sent\n-------------------------------------\n", count - 1, MAX_PACKET_COUNT);
    Error_Handler();
  }
}
/* USER CODE END 1 */
