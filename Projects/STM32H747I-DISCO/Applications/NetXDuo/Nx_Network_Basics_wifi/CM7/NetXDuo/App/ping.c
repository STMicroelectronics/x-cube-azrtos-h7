/**
  ******************************************************************************
  * @file    ping.c
  * @author  MCD Application Team
  * @brief   Ping command.
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
#include "msg.h"
#include <string.h>
#include <stdio.h>
#include <inttypes.h>
#include "main.h"
#include "nxd_bsd.h"
#include "nx_icmp.h"

#ifndef IPPROTO_ICMP
#define IPPROTO_ICMP      NX_PROTOCOL_ICMP
#endif /* IPPROTO_ICMP */


int32_t ping_cmd(int32_t argc, char **argv);

#define TIME_SOURCE_HTTP_HOST   "www.st.com"

#define PING_ITERATION  (uint32_t)10
#define PING_DELAY_S    (uint32_t)1

/** ping identifier - must fit on a 16 bit word. */
#ifndef PING_ID
#define PING_ID        (USHORT)0xAFAF
#endif /* PING_ID */

/** ping additional data size to include in the packet */
#ifndef PING_DATA_SIZE
#define PING_DATA_SIZE (size_t)32
#endif /* PING_DATA_SIZE */

#define FOLD_U32T(u)          ((ULONG)(((u) >> 16) + ((u) & 0x0000ffffUL)))
#define SWAP_BYTES_IN_WORD(w) (((w) & 0xff) << 8) | (((w) & 0xff00) >> 8)


/* Private variables ---------------------------------------------------------*/

extern TX_BYTE_POOL *AppBytePool;

static uint16_t standard_chksum(const void *dataptr, int32_t len);
static void ping_prepare_echo(NX_ICMPV4_ECHO *iecho, uint16_t len, uint16_t ping_seq_num);
static int32_t icmp_ping(struct sockaddr *addr, uint32_t count, uint32_t timeout, int32_t response[]);

static int32_t ping_gethostbyname(struct sockaddr *addr, const char *name);



/* Functions Definition ------------------------------------------------------*/

static uint16_t standard_chksum(const void *dataptr, int32_t len)
{
  const uint8_t *pb = (const uint8_t *)dataptr;
  const uint16_t *ps;
  uint16_t t = 0;
  uint32_t sum = 0;
  uint32_t odd = ((uintptr_t)pb & 1);

  /* Get aligned to uint16_t */
  if (odd && (len > 0))
  {
    ((uint8_t *)&t)[1] = *pb++;
    len--;
  }

  /* Add the bulk of the data */
  ps = (const uint16_t *)(const VOID *)pb;
  while (len > 1)
  {
    sum += *ps++;
    len -= 2;
  }

  /* Consume left-over byte, if any */
  if (len > 0)
  {
    ((uint8_t *)&t)[0] = *(const uint8_t *)ps;
  }

  /* Add end bytes */
  sum += t;

  /* Fold 32-bit sum to 16 bits
     calling this twice is probably faster than if statements... */
  sum = FOLD_U32T(sum);
  sum = FOLD_U32T(sum);

  /* Swap if alignment was odd */
  if (odd)
  {
    sum = SWAP_BYTES_IN_WORD(sum);
  }

  return ~(uint16_t)sum;
}

static void ping_prepare_echo(NX_ICMPV4_ECHO *iecho, uint16_t len, uint16_t ping_seq_num)
{
  const size_t data_len = len - sizeof(NX_ICMPV4_ECHO);

  iecho->nx_icmpv4_echo_header.nx_icmpv4_header_type = NX_ICMP_ECHO_REQUEST_TYPE;
  iecho->nx_icmpv4_echo_header.nx_icmpv4_header_code = NX_ICMP_ZERO_CODE;
  iecho->nx_icmpv4_echo_header.nx_icmpv4_header_checksum = 0;
  iecho->nx_icmpv4_echo_identifier = PING_ID;
  iecho->nx_icmpv4_echo_sequence_num  = (uint16_t) htons(ping_seq_num);

  /* fill the additional data buffer with some data */
  for (size_t i = 0; i < data_len; i++)
  {
    ((CHAR *)iecho)[sizeof(NX_ICMPV4_ECHO) + i] = (CHAR) i;
  }
  /* Ping data are sent in RAM mode , so LWIP is not computing the checksum by SW */
#ifndef CHECKSUM_BY_HARDWARE
  ((NX_ICMPV4_ECHO *)iecho)->nx_icmpv4_echo_header.nx_icmpv4_header_checksum = standard_chksum(iecho, len);
#endif /* CHECKSUM_BY_HARDWARE */
}


static int32_t icmp_ping(struct sockaddr *addr, uint32_t count, uint32_t timeout, int32_t response[])
{
  int32_t ret = 0;
  uint16_t ping_seq_num = 1;
  NX_ICMPV4_ECHO *iecho;
  NX_ICMPV4_ECHO *pecho = NULL;
  const size_t ping_size = sizeof(NX_ICMPV4_ECHO) + PING_DATA_SIZE;
  struct timeval timeout_opt = {0};
  timeout_opt.tv_sec = timeout;

  for (uint32_t i = 0; i < count; i++)
  {
    response[i] = -1;
  }

  const INT sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
  MSG_DEBUG("-> %"PRId32"\n", (int32_t)sock);

  if (NX_SOC_ERROR == sock)
  {
    MSG_ERROR("socket() failed\n");
    ret = -1;
  }
  else if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout_opt, sizeof(timeout_opt)) < 0)
  {
    MSG_ERROR("Setting option for socket failed\n");
    ret = -1;
  }
  else
  {
    if (tx_byte_allocate(AppBytePool, (VOID **) &pecho, ping_size, TX_NO_WAIT) != TX_SUCCESS)
    {
      MSG_ERROR("Allocation failed\n");
      ret = -1;
    }

    if (pecho == NULL)
    {
      MSG_ERROR("Allocation failed\n");
      ret = -1;
    }
  }

  if (ret == 0)
  {
    addr->sa_family = AF_INET;
    ((struct sockaddr_in *)addr)->sin_port = htons(0);

    for (uint32_t i = 0; i < count; i++)
    {
      /* add useless test for MISRA on pecho */
      if (pecho != NULL)
      {
        ping_prepare_echo(pecho, ping_size, ping_seq_num);
      }

      const uint32_t ping_start_time = HAL_GetTick();

      if (sendto(sock, (CHAR *)pecho, (INT) ping_size, 0, addr, (INT) sizeof(*addr)) < 0)
      {
        MSG_ERROR("send failed\n");
        break;
      }

      do
      {
        struct sockaddr from = {0};
        INT fromlen = sizeof(from);
        CHAR buf[64] = "";
        const INT len = recvfrom(sock, buf, (INT) ping_size, 0, &from, &fromlen);
        const uint32_t now = HAL_GetTick();

        if (len >= (INT)(sizeof(NX_IPV4_HEADER) + sizeof(NX_ICMPV4_ECHO)))
        {
          NX_IPV4_HEADER *iphdr = (NX_IPV4_HEADER *)buf;
          iecho = (NX_ICMPV4_ECHO *)(buf + ((((ntohl(iphdr->nx_ip_header_word_0)) >> 24) & 0x0f) * 4U));
          const USHORT seqnum = htons(ping_seq_num);

          if ((iecho->nx_icmpv4_echo_identifier == PING_ID) && (iecho->nx_icmpv4_echo_sequence_num == seqnum))
          {
            if (iecho->nx_icmpv4_echo_header.nx_icmpv4_header_type == (uint8_t) NX_ICMP_ECHO_REPLY_TYPE)
            {
              uint32_t delta;
              ret = 0;
              delta = now - ping_start_time;
              response[i] = (int32_t) delta;
              break;
            }
            else
            {
              MSG_INFO("ICMP Other Response received\n");
            }
          }
        }
        else
        {
          MSG_INFO("No data start %"PRId32" : %"PRIu32" .. %"PRIu32"\n", (int32_t)len, ping_start_time, now);
        }
      } while (HAL_GetTick() < (ping_start_time + timeout));
      ping_seq_num++;
    }
    tx_byte_release(pecho);
  }

  soc_close(sock);

  return ret;
}

static int32_t ping_gethostbyname(struct sockaddr *addr, const char *name)
{
  int32_t ret = -1;
  struct addrinfo *hostinfo = {0};
  struct addrinfo hints = {0};

  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_DGRAM;
  hints.ai_flags = AI_PASSIVE;

  if (0 == getaddrinfo((const CHAR *)name, NULL, &hints, &hostinfo))
  {
    if (hostinfo->ai_family == AF_INET)
    {
      struct sockaddr_in *saddr = (struct sockaddr_in *) addr;

      (void) memset((void *)saddr, 0, sizeof(struct sockaddr_in));
      saddr->sin_family = AF_INET;
      (void) memcpy(&saddr->sin_addr, &((struct sockaddr_in *)(hostinfo->ai_addr))->sin_addr, 4);

      ret = 0;
    }
    freeaddrinfo(hostinfo);
  }
  return ret;
}


int32_t ping_cmd(int32_t argc, char **argv)
{
  char *servername;
  int32_t ping_res[PING_ITERATION];
  struct sockaddr addr = {0};

  if (argc == 1)
  {
    servername = TIME_SOURCE_HTTP_HOST;
    MSG_INFO("ping <hostname>\n");
  }
  else
  {
    servername = argv[1];
  }

  if (ping_gethostbyname(&addr, servername) < 0)
  {
    MSG_INFO("Could not find hostname %s\n", servername);
    return -1;
  }
  else
  {
    struct in_addr ip_addr;
    MSG_INFO("Pinging hostname %s\n", servername);

    struct sockaddr_in *addr_ip4 = (struct sockaddr_in *) &addr;
    ip_addr.s_addr = addr_ip4->sin_addr.s_addr;

    MSG_INFO("Pinging %s\n", inet_ntoa(ip_addr));
  }

  if (icmp_ping((struct sockaddr *)&addr, PING_ITERATION, PING_DELAY_S, ping_res) >= 0)
  {
    for (uint32_t i = 0; i < PING_ITERATION; i++)
    {
      if (ping_res[i] >= 0)
      {
        MSG_INFO("Ping iteration #%"PRId32" roundtrip %"PRId32"\n", i, ping_res[i]);
      }
    }
  }
  else
  {
    MSG_INFO("Ping failure\n");
  }
  return 0;
}
