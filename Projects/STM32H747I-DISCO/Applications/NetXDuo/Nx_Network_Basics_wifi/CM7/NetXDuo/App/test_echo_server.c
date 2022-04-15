/**
  ******************************************************************************
  * @file    test_echo_server.c
  * @author  MCD Application Team
  * @brief   Test a echo with a server.
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

/* Includes ------------------------------------------------------------------*/
#include <inttypes.h>
#include "msg.h"
#include "main.h"
#include "app_netxduo.h"
#include "nxd_bsd.h"

/* Using the ARM Mbed echo server. */
#define REMOTE_IP_ADDR     "52.215.34.155"
#define REMOTE_PORT        7

#define TRANSFER_SIZE      2000
#define ITERATION_COUNT    10

static uint8_t buffer_out[TRANSFER_SIZE + ITERATION_COUNT];
static uint8_t buffer_in[TRANSFER_SIZE + ITERATION_COUNT];


int32_t test_echo_server(int32_t argc, char **argv);

static void fillbuffer(uint8_t *buff, uint32_t n);
static uint32_t checkbuffer(uint8_t *buff, uint32_t n, uint32_t offset);
static void EchoServerTest(uint32_t loop, uint32_t n);


static void fillbuffer(uint8_t *buff, uint32_t n)
{
  for (uint32_t i = 0; i < n; i++)
  {
    buff[i] = (uint8_t)i;
  }
}

static uint32_t checkbuffer(uint8_t *buff, uint32_t n, uint32_t offset)
{
  uint32_t error_count = 0;
  for (uint32_t i = 0; i < n; i++)
  {
    if (buff[i] != ((i + offset) & 0xff))
    {
      MSG_ERROR("Received data are different from data sent %"PRId32" <> %d at index %"PRId32"\n", i & 0xff, buff[i], i);
      error_count++;
    }
  }
  return error_count;
}

static void EchoServerTest(uint32_t loop, uint32_t n)
{
  struct sockaddr_in addr = {0};
  addr.sin_port = htons(REMOTE_PORT);
  addr.sin_family = AF_INET;

  const INT convert_status = inet_aton(REMOTE_IP_ADDR, &addr.sin_addr);

  if (NX_SOC_ERROR == convert_status)
  {
    MSG_ERROR("inet_aton failed\n");
    return;
  }

  const INT sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

  if (NX_SOC_ERROR == sock)
  {
    MSG_ERROR("socket() failed\n");
    return;
  }

  {
    struct timeval timeout = {0};
    timeout.tv_sec = 15;

    INT sock_status = setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
    if (NX_SOC_ERROR == sock_status)
    {
      MSG_ERROR("setsockopt failed\n");
    }

    sock_status = setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));
    if (NX_SOC_ERROR == sock_status)
    {
      MSG_ERROR("setsockopt failed\n");
    }
  }

  MSG_INFO("- Trying to connect to echo server %s\n", REMOTE_IP_ADDR);

  const INT connect_status = connect(sock, (struct sockaddr *)&addr, sizeof(addr));

  if (NX_SOC_OK == connect_status)
  {
    uint32_t transfer = 0;
    uint32_t error_count = 0;
    MSG_INFO("- Device connected to the %s\n", REMOTE_IP_ADDR);
    MSG_INFO("- Starting transfer ");

    fillbuffer(buffer_out, n + loop);

    const uint32_t tstart = HAL_GetTick();
    uint32_t tstop = HAL_GetTick();

    for (uint32_t i = 0; i < loop; i++)
    {
      {
        uint32_t transfer_out = 0;
        do
        {
          INT count_done = send(sock, (const CHAR *)&buffer_out[i + transfer_out], (INT)(n - transfer_out), 0);

          MSG_INFO(".");
          /*MSG_INFO("%"PRId32"", (int32_t)count_done);*/

          if (count_done < 0)
          {
            MSG_INFO("Failed to send data to echo server (%"PRId32"), try again\n", (int32_t)count_done);
            count_done = 0;
          }
          transfer_out += (uint32_t)count_done;
        } while (transfer_out < n);

        /* Update cumulative number with data that have been sent.*/
        transfer += transfer_out;
      }

      /* Reset the buffer of data reception. */
      memset(buffer_in, 0x00, sizeof(buffer_in));

      {
        uint32_t transfer_in = 0;
        do
        {
          INT count_done = recv(sock, &buffer_in[transfer_in], (INT)(n - transfer_in), 0);
          if (count_done < 0)
          {
            MSG_ERROR("\nReceive failed with %"PRId32"\n", (int32_t)count_done);
            goto end;
          }
          transfer_in += (uint32_t)count_done;
        } while (transfer_in < n);

        tstop = HAL_GetTick();

        error_count = checkbuffer(buffer_in, n, i);
        MSG_INFO("x");
        /* Update cumulative number with data that have been received.*/
        transfer += transfer_in;
      }
    }
    if (error_count == 0)
    {
      MSG_INFO("\nSuccessfull Echo Transfer and receive %"PRId32" x %"PRId32" with %"PRId32" bytes in %"PRId32" ms, br = %"PRId32" Kbit/sec\n",
               loop, n, transfer, tstop - tstart, (transfer * 8) / (tstop - tstart));
    }
    else
    {
      MSG_INFO("\nError: Echo transfer, find %"PRId32" different bytes\n", error_count);
    }
  }
  else
  {
    MSG_ERROR("Failed to connect to %s (%"PRId32")\n", REMOTE_IP_ADDR, (int32_t)connect_status);
  }

end:
  soc_close(sock);
}


int32_t test_echo_server(int32_t argc, char **argv)
{
  UNUSED(argc);
  UNUSED(argv);

  for (uint32_t transfer_size = 1000; transfer_size <= TRANSFER_SIZE; transfer_size += 200)
  {
    EchoServerTest(ITERATION_COUNT, transfer_size);
  }
  return 0;
}
