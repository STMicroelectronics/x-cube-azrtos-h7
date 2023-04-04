/**
  ******************************************************************************
  * @file    iap_client.c
  * @author  MCD Application Team
  * @brief   This sample code shows how to use the WiFi module to download
  *          a fw from a HTTP server, write it into the flash then jump to run it.
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

/* Includes ------------------------------------------------------------------*/
#include "iap_client.h"
#include "msg.h"
#include "main.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>

#include "flash_if.h"

#include "nx_api.h"
#include "nx_ip.h"
#include "nx_tcp.h"
#include "nxd_http_client.h"
#include "app_netxduo.h"
#include "nx_driver_framework.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define  HTTP_CLIENT_RX_WINDOW_SIZE     (ULONG)(NX_DRIVER_PACKET_SIZE * 4)

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

/* Private typedef -----------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
static void halt_on_error(NX_HTTP_CLIENT *Client);

static NX_HTTP_CLIENT HttpClient;
static NX_PACKET_POOL HttpClientPool;


#ifdef ENABLE_IOT_DEBUG
static void task_list(char *buffer_string);
#endif /* ENABLE_IOT_DEBUG */

/* Exported functions --------------------------------------------------------*/
VOID StartWiFiIAP(TX_BYTE_POOL *byte_pool, NX_IP *ip_instance)
{
  UINT status = NX_SUCCESS;
  ULONG bytes_copied;

  /* Reset the HTTP data field to 0. */
  static uint8_t http_data[DATA_MAX_SIZE] = {0};
  /* The actual size we can write in RAM buffer. */
  uint32_t http_data_remaining_size = sizeof(http_data);
  uint8_t *http_data_pos = http_data;

  MSG_DEBUG(">\n");

#if defined(ENABLE_IOT_DEBUG)
  {
    char task_list_string[1000] = "";
    task_list(task_list_string);
    MSG_DEBUG("\n%s\n", task_list_string);
  }
#endif /* ENABLE_IOT_DEBUG */

  /* Allocate the memory for packet_pool.  */
  {
    const ULONG pool_size = HTTP_CLIENT_POOL_SIZE;
    VOID *pool_start;

    if (tx_byte_allocate(byte_pool, &pool_start, pool_size, TX_NO_WAIT) != TX_SUCCESS)
    {
      /* Leave the access point. */
      halt_on_error(NULL);
    }

    /* Create the Http Client Packet pool to be used for TCP packet allocation */
    status = nx_packet_pool_create(&HttpClientPool, "HTTP Client Pool", HTTP_CLIENT_PACKET_SIZE, pool_start, pool_size);

    if (status != NX_SUCCESS)
    {
      /* Leave the access point. */
      halt_on_error(NULL);
    }
    MSG_DEBUG("nx_packet_pool_create() done\n");
  }

  /* Create a HTTP Client Socket. */
  status = nx_http_client_create(&HttpClient, "IAP_HTTP_Client",
                                 ip_instance, &HttpClientPool, HTTP_CLIENT_RX_WINDOW_SIZE);

  if (status != NX_SUCCESS)
  {
    MSG_ERROR("HTTP Client create Failed!\n");
    halt_on_error(NULL);
  }

  /* Change port */
  status = nx_http_client_set_connect_port(&HttpClient, HOST_PORT);

  if (status != NX_SUCCESS)
  {
    MSG_ERROR("Set Port Failed!\n");
    halt_on_error(&HttpClient);
  }

  MSG_INFO("Connecting to: ");
  PRINT_SERVER_ADDRESS(HOST_ADDRESS, HOST_PORT);

  /* Set up the server's IPv4 address here. */
  NXD_ADDRESS server_ip_address = {0};
  server_ip_address.nxd_ip_version = NX_IP_VERSION_V4;
  server_ip_address.nxd_ip_address.v4 = HOST_ADDRESS;

  /* Send a get request to get the specified content of the web server. */
  /* Use the 'NetX' service to send a GET request to the server (can only use IPv4 addresses). */
  status = nxd_http_client_get_start(&HttpClient, &server_ip_address,
                                     "/"FW_FILE,
                                     NX_NULL, 0, NX_NULL, NX_NULL, DEFAULT_TIMEOUT);

  if (status != NX_SUCCESS)
  {
    if (status == NX_NOT_CONNECTED)
    {
      MSG_ERROR("Connecting to %"PRIu32".%"PRIu32".%"PRIu32".%"PRIu32":%"PRIu32" Failed!\n", \
                ((uint32_t)(HOST_ADDRESS) >> 24) & 0xff, \
                ((uint32_t)(HOST_ADDRESS) >> 16) & 0xff, \
                ((uint32_t)(HOST_ADDRESS) >> 8) & 0xff, \
                ((uint32_t)(HOST_ADDRESS) & 0xff), \
                (uint32_t)(HOST_PORT));
    }
    else
    {
      MSG_ERROR("Downloading the firmware \"%s\" failed with error (0x%x)\n", FW_FILE, status);
    }
    halt_on_error(&HttpClient);
  }
  else
  {
    MSG_INFO("O.K!\n\n");
    MSG_INFO("Requesting the firmware ' %s '...\n", FW_FILE);

    http_data_pos = http_data;

#ifdef ENABLE_IOT_DEBUG
    uint32_t pkt_counter = 0;
#endif /* ENABLE_IOT_DEBUG */

    do
    {
      /* Data to be received from server. */
      NX_PACKET *packet;

      /* The request is successful, get the data. */
      status = nx_http_client_get_packet(&HttpClient, &packet, DEFAULT_TIMEOUT);

      if (status == NX_SUCCESS)
      {
        /* Get the length of the data packet. */

        /* length of single TCP packet in packet_ptr */
        ULONG len = 0;
        nx_packet_length_get(packet, &len);

        MSG_DEBUG("[%06"PRIu32"] packet%02"PRIu32", length: %"PRIu32"\n",
                  HAL_GetTick(), pkt_counter++, (uint32_t)len);

        if (len > 0 && (len < http_data_remaining_size))
        {
          /* Retrieve data from packet pointer. */
          nx_packet_data_retrieve(packet, http_data_pos, &bytes_copied);
          http_data_pos += bytes_copied;
          http_data_remaining_size -= bytes_copied;
        }
        else
        {
          http_data_remaining_size = 0;
        }

        /* Release the data package. */
        nx_packet_release(packet);
      }
      else if (status == NX_HTTP_GET_DONE)
      {
        break;
      }
      else
      {
        MSG_ERROR("Get packet failed (0x%"PRIx32")\n", (uint32_t)status);
        break;
      }
    } while (http_data_remaining_size > 0);
  }

  if (status != NX_HTTP_GET_DONE)
  {
    MSG_ERROR("The firmware ' %s ' is not fully received\n", FW_FILE);

    /* Leave the access point. */
    halt_on_error(&HttpClient);
  }

  const uint8_t *const fw_data = (uint8_t *)http_data;
  int32_t fw_size = ((char *)http_data_pos - (char *)http_data);

#ifdef IAP_DEBUG_PRINT_FW_DATA
  /* print fw_data hexdump */
  MSG_DEBUG("\n");

  for (int32_t i = 0; i < fw_size; i++)
  {
    MSG_DEBUG("%02x ", fw_data[i]);

    if ((i + 1) % 16 == 0)
    {
      MSG_DEBUG("\n");
    }
  }

  MSG_DEBUG("\n");
#endif /* IAP_DEBUG_PRINT_FW_DATA */


  MSG_INFO("Downloaded firmware ' %s ', size = %"PRIi32" Bytes\n\n", FW_FILE, fw_size);

  /* Now the firmware can be written into Flash memory. */
  MSG_INFO("State: Programming...\n");

  /* Initialize the Flash memory module for write operations. */
  FLASH_If_Init();

  /* Erase 256 Ko of flash. */
  if (FLASH_If_Erase(USER_FLASH_START_ADDRESS) != 0)
  {
    /* Leave the access point. */
    halt_on_error(&HttpClient);
  }

  /* Start writing the binary file into the Flash memory. */
  if (FLASH_If_Write(USER_FLASH_START_ADDRESS, fw_data, fw_size) != 0)
  {
    MSG_ERROR("Programming Failed !\n");

    /* Leave the access point. */
    halt_on_error(&HttpClient);
  }
  else
  {
    MSG_INFO("Programming Done !\n");
  }

  {
    /* Delete the http client */
    const UINT status = nx_http_client_delete(&HttpClient);
    MSG_INFO("nx_http_client_delete() done with 0x%"PRIx32"\n", (uint32_t)status);
  }

  {
    ULONG total_packets = 0;
    ULONG free_packets = 0;
    ULONG empty_pool_requests = 0;
    ULONG empty_pool_suspensions = 0;
    ULONG invalid_packet_releases = 0;

    nx_packet_pool_info_get(&HttpClientPool,
                            &total_packets, &free_packets,
                            &empty_pool_requests, &empty_pool_suspensions,
                            &invalid_packet_releases);
    MSG_INFO("Packet Pool \"%s\" (%"PRIu32")\n",
            HttpClientPool.nx_packet_pool_name,
            (uint32_t)HttpClientPool.nx_packet_pool_payload_size);
    MSG_INFO(" - total packets          : %"PRIu32"\n", (uint32_t)total_packets);
    MSG_INFO(" - free packets           : %"PRIu32"\n", (uint32_t)free_packets);
    MSG_INFO(" - invalid packet releases: %"PRIu32"\n", (uint32_t)invalid_packet_releases);
  }

  NetXDuo_DeInit();

  /* Blink forever the LED. */
  blink_led(LED_STATUS_OK);
}


static void halt_on_error(NX_HTTP_CLIENT *Client)
{
  MSG_INFO("Quitting Access Point...\n\n");

  /* Leave the access point. */
  /* Uninitialize the WiFi module. */
  /* Clean up the network interface. */

  if(NULL != Client)
  {
    /* Delete the http client */
    const UINT status = nx_http_client_delete(Client);
    MSG_INFO("nx_http_client_delete() done with 0x%"PRIx32"\n", (uint32_t)status);
  }

  NetXDuo_DeInit();

  /* Blink forever the LED error. */
  blink_led(LED_STATUS_KO);
}

#ifdef ENABLE_IOT_DEBUG
static void task_list(char *buffer_string)
{
  extern ULONG _tx_thread_created_count;
  extern TX_THREAD *_tx_thread_created_ptr;

  const uint32_t current_number_of_threads = (uint32_t) _tx_thread_created_count;
  const TX_THREAD *thread_ptr = _tx_thread_created_ptr;
  static const char *to_state_str[] =
  {
    "TX_READY          ",
    "TX_COMPLETED      ",
    "TX_TERMINATED     ",
    "TX_SUSPENDED      ",
    "TX_SLEEP          ",
    "TX_QUEUE_SUSP     ",
    "TX_SEMAPHORE_SUSP ",
    "TX_EVENT_FLAG     ",
    "TX_BLOCK_MEMORY   ",
    "TX_BYTE_MEMORY    ",
    "TX_IO_DRIVER      ",
    "TX_FILE           ",
    "TX_TCP_IP         ",
    "TX_MUTEX_SUSP     ",
    "TX_PRIORITY_CHANGE"
  };

  sprintf(buffer_string, "%-24s %-15s\tPrio\tStack Start\tStack End\tSize\tStack Ptr\n", "Name", "State");
  strcat(buffer_string, "--------------------------------------------------------------------------------------\n");

  /* The list of tasks and their status. */
  for (uint32_t count = 0; count < current_number_of_threads; count++)
  {
    sprintf(buffer_string + strlen(buffer_string), "%-24s %-15s\t%3"PRIu32"\t%p\t%p\t%"PRIu32"\t%p\r\n",
            thread_ptr->tx_thread_name,
            to_state_str[thread_ptr->tx_thread_state],
            (uint32_t)thread_ptr->tx_thread_priority,
            thread_ptr->tx_thread_stack_start,
            thread_ptr->tx_thread_stack_end,
            (uint32_t)thread_ptr->tx_thread_stack_size,
            thread_ptr->tx_thread_stack_ptr);

    thread_ptr = thread_ptr->tx_thread_created_next;
  }

  strcat(buffer_string, "--------------------------------------------------------------------------------------\n");
}
#endif /* ENABLE_IOT_DEBUG */
