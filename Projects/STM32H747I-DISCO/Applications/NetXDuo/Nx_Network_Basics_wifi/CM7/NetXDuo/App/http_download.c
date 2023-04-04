/**
  ******************************************************************************
  * @file      http_download.c
  * @author    MCD Application Team
  * @brief     This file provides code for HTTP download service.
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
#include <inttypes.h>
#include <stdbool.h>
#include "msg.h"
#include "main.h"
#include "nxd_bsd.h"
#include "nxd_dns.h"
#include "nx_tcp.h"

#include "nx_driver_framework.h"

#define MAX_BUFFER              (NX_DRIVER_PACKET_SIZE + 1)
#define MAX_LINES               10
#define REPORT_TIMEPERIOD_MS    (uint32_t)10000

#define HTTPS_PORT              443
#define HTTP_PORT               80

#define HTTP_RESPONSE_OK                 (uint32_t)200
#define HTTP_RESPONSE_MULTIPLE_CHOICES   (uint32_t)300
#define HTTP_RESPONSE_BAD_REQUEST        (uint32_t)400

#define DOWNLOAD_LIMITED_SIZE            (uint32_t)140000
#define SOCKET_TIMEOUT_S                 (uint32_t)10

extern TX_BYTE_POOL *AppBytePool;

int32_t http_download_cmd(int32_t argc, char *argv[]);

static const char DOWNLOAD_HEADER[] =
  "GET  %s HTTP/1.1\r\n"
  "Host: %s\r\n"\
  "User-Agent: STSTVS\r\n"\
  "\r\n";

typedef struct t_http_context
{
  int32_t    hSocket ;
  uint32_t   status;
  uint64_t   contentLen;
  uint64_t   posFile;
} http_context_t;

static void http_split_host_request(const char *pURL, char **pHttpHost, char **pHttpReq, bool *secure);
static int32_t http_read_response(int32_t sock, unsigned char *pHeader, uint32_t maxSize, uint32_t *pRetSize);
static uint32_t service_lines(unsigned char *pBuffer, char *tLines[], uint32_t maxLines);
static char *http_seek_while(char *pString, char key);
static int32_t http_check_response(http_context_t *pCtx, unsigned char *pBuffer);
static char *http_seek_to(char *pString, char key);

#ifdef ENABLE_IOT_DEBUG
static void tcp_infos(void);
#endif /* ENABLE_IOT_DEBUG */


static void http_split_host_request(const char *pURL, char **pHttpHost, char **pHttpReq, bool *secure)
{
  static char host[50];
  const char *pStr = NULL;

  memset(host, '\0', sizeof(host));

  *pHttpHost = NULL;
  *pHttpReq = NULL;

  if (strncmp(pURL, "http://", strlen("http://")) == 0)
  {
    pStr = pURL + strlen("http://");
    *secure = false;
  }
  else if (strncmp(pURL, "https://", strlen("https://")) == 0)
  {
    pStr = pURL + strlen("https://");
    *secure = true;
  }
  else
  {
    pStr = NULL;
  }

  if (pStr != NULL)
  {
    char *pHost = host;
    while (*pStr && *pStr != '/')
    {
      *pHost++ = *pStr++;
    }
    *pHost = 0;
    *pHttpHost = host;
    *pHttpReq  = (char *)pStr;
  }
}

static int32_t http_read_response(int32_t sock, unsigned char *pHeader, uint32_t maxSize, uint32_t *pRetSize)
{
  const char *pString = (const char *)pHeader;

  int32_t headerSize = maxSize;
  uint32_t count = 0;

  if (pRetSize)
  {
    *pRetSize = 0;
  }
  else
  {
    return -1;
  }

  while (headerSize > 0)
  {
    INT ret = recv((INT)sock, (VOID *)pString, 1, 0);
    if (ret < 0)
    {
      return ret;
    }

    pString++;
    headerSize--;
    count++;
    if (count > 4 && strncmp(pString - 4, "\r\n\r\n", 4) == 0)
    {
      break;
    }
  }

  if (headerSize == 0)
  {
    return -1;
  }

  *pRetSize = count;

  return 0;
}


/* Fill a table with each line entries */
static uint32_t service_lines(unsigned char *pBuffer, char *tLines[], uint32_t maxLines)
{
  uint32_t index = 0;

  if (*pBuffer == 0)
  {
    return 0;
  }

  while (index  < maxLines && *pBuffer)
  {
    tLines[index] = (char *)pBuffer;
    /* Seek to the new line. */
    while (*pBuffer && *pBuffer != '\n')
    {
      pBuffer++;
    }
    index++;
    if (*pBuffer)
    {
      pBuffer++;
    }

  }
  return index;
}

/* move to char ... */
static char *http_seek_to(char *pString, char key)
{
  while (*pString && *pString != key)
  {
    pString++;
  }
  if (*pString)
  {
    pString ++;
  }
  return pString;
}

/* move to char ... */
static char *http_seek_while(char *pString, char key)
{
  if (pString)
  {
    while (*pString && *pString == key)
    {
      pString++;
    }
  }
  return pString;
}

/* move to char ... */
static char *http_seek_while_not(char *pString, char key)
{
  if (pString)
  {
    while (*pString && *pString != key)
    {
      pString++;
    }
  }
  return pString;
}

/* Check the response. */
static int32_t http_check_response(http_context_t *pCtx, unsigned char *pBuffer)
{
  char *tLines[MAX_LINES] = {0};
  uint32_t nbLines = service_lines(pBuffer, &tLines[0], MAX_LINES);

  pCtx->contentLen = (uint64_t) -1;
  pCtx->status     = HTTP_RESPONSE_BAD_REQUEST;
  pCtx->posFile    = 0;

  if (nbLines < 1)
  {
    return -1;
  }

  /* Extract the status from the first line. */
  char *pString = tLines[0];
  pString = http_seek_while_not(pString, ' ');
  pString = http_seek_while(pString, ' ');
  sscanf(pString, "%"PRIu32"", &pCtx->status);

  /* Parses each header lines and only manage mandatory information. */
  for (uint32_t a = 0 ; a < nbLines ; a++)
  {
    if (strncmp(tLines[a], "Content-Length", strlen("Content-Length")) == 0)
    {
      /* move to the size */
      char *pParam = http_seek_to(tLines[a], ':');
      pParam = http_seek_while(pParam, ' ');
      uint64_t len;
      sscanf(pParam, "%"PRIu64"", &len);
      /* fill len */
      pCtx->contentLen = len;
    }
  }

  /* Check error status. */
  if (pCtx->status < HTTP_RESPONSE_OK || pCtx->status >= HTTP_RESPONSE_MULTIPLE_CHOICES)
  {
    return -1;
  }
  return 0;
}

static int32_t http_file_download(int32_t socket, const char *pHost, char *pReq)
{
  CHAR          *pBuffer = 0;
  uint32_t       startTime;
  uint32_t       localStartTime;
  int32_t        ret = 0;
  uint32_t       cptCount = 0;
  uint32_t       retSize;
  uint32_t       len;
  http_context_t httpCtx;
  const uint32_t limitedSize = DOWNLOAD_LIMITED_SIZE;

#ifdef ENABLE_IOT_DEBUG
  static struct
  {
    int32_t done;
    uint32_t when;
    uint32_t duration;
  } history[101];
  static const uint32_t historyCount = sizeof(history) / sizeof(history[0]);
  uint32_t historyIndex = 0;
#endif /* ENABLE_IOT_DEBUG */

  /* Construct the header from the host and the request. */
  if (tx_byte_allocate(AppBytePool, (VOID **) &pBuffer, MAX_BUFFER, TX_NO_WAIT) != TX_SUCCESS)
  {
    MSG_ERROR("alloc failed\n");
    return -1;
  }

  const uint32_t bufferSize = MAX_BUFFER - 1;
  pBuffer[bufferSize] = '\0';

  snprintf((char *)pBuffer, bufferSize, DOWNLOAD_HEADER, pReq, pHost);

  /* Send GET HTTP request. */
  if (send((INT)socket, pBuffer, (INT)strlen(pBuffer), 0) < 0)
  {
    MSG_ERROR("send() failed\n");
  }

  /* Wait for the response. */
  if (http_read_response(socket, (unsigned char *) pBuffer, bufferSize, &retSize) != 0)
  {
    MSG_ERROR("No answer from HTTP server\n");
    ret = -1;
    goto exit;
  }

  if (http_check_response(&httpCtx, (unsigned char *)pBuffer) != 0)
  {
    MSG_DEBUG("  <--- \"%s\"\n\n", pBuffer);
    MSG_ERROR("Incorrect HTTP server response\n");
    ret = -1;
    goto exit;
  }

  len = httpCtx.contentLen;
  MSG_INFO("File size %"PRIu32" bytes\n", len);


  if (len > limitedSize)
  {
    MSG_INFO("Limiting transfer to first %"PRIu32" bytes with report time of %"PRIu32" ms\n",
             limitedSize, REPORT_TIMEPERIOD_MS);
    len = limitedSize;
  }

  localStartTime = startTime = HAL_GetTick();

  while (len)
  {
    INT count;
    uint32_t szInSByte;
    uint32_t elapsedTime;

    szInSByte = (bufferSize < len) ? bufferSize : len;

#ifdef ENABLE_IOT_DEBUG
    uint32_t measureStart = HAL_GetTick();
#endif /* ENABLE_IOT_DEBUG */

    count = recv(socket, (VOID *)pBuffer, (INT)szInSByte, 0);
    if (count == -1)
    {
      ret = -1;
      break;
    }
    if (count == 0)
    {
      break;
    }

    MSG_INFO(".");

    cptCount += (uint32_t)count;
    elapsedTime = HAL_GetTick() - localStartTime;

#ifdef ENABLE_IOT_DEBUG
    {
      uint32_t measureEnd = HAL_GetTick();
      history[historyIndex].done = count;
      history[historyIndex].when = measureEnd;
      history[historyIndex++].duration = measureEnd - measureStart;
      if (historyIndex > historyCount)
      {
        historyIndex = 0;
      }
    }
#endif /* ENABLE_IOT_DEBUG */

    if (elapsedTime  > REPORT_TIMEPERIOD_MS)
    {
      uint32_t refBw = (8 * cptCount) / elapsedTime;
      MSG_INFO("\nTransfer %"PRIu32" bytes, remain %"PRIu32" bytes, bitrate %"PRIu32" kbit/s\n",
               cptCount, len, refBw);
      cptCount = 0;
      localStartTime = startTime = HAL_GetTick();
    }

    len -= count;
  }

  /* Actually close the receive data stream. */
  soc_close(socket);

  {
    const uint32_t duration = HAL_GetTick() - startTime;
    uint32_t refBw  = (8 * cptCount) / duration;
    MSG_INFO("\nTransfer %"PRIu32" bytes, duration %"PRIu32" ms, bitrate %"PRIu32" kbit/s\n",
             cptCount, duration, refBw);
  }
#ifdef ENABLE_IOT_DEBUG
  for (uint32_t i = 0; i < historyCount; i++)
  {
    MSG_DEBUG("[%4"PRId32"] - [%4"PRIu32" - %4"PRIu32"]\n", history[i].done, history[i].when, history[i].duration);
  }
#endif /* ENABLE_IOT_DEBUG */

exit:
  tx_byte_release(pBuffer);
  return ret;
}


static const char HTTP_FILE_URL[] = "http://public.st.free.fr/500MO.bin";

int32_t http_download_cmd(int32_t argc, char *argv[])
{
  CHAR    *hostname;
  char    *pHttpReq;
  bool    secure;
  const char *url;
  uint16_t port;
  int32_t ret = 0;

  struct sockaddr addr = {0};
  struct sockaddr_in *saddr;
  struct addrinfo *hostinfo = {0};
  struct addrinfo hints = {0};

  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_DGRAM;
  hints.ai_flags = AI_PASSIVE;

  if (argc == 1)
  {
    url = HTTP_FILE_URL;
  }
  else
  {
    url = argv[1];
  }

  http_split_host_request(url, &hostname, &pHttpReq, &secure);

  if ((hostname == NULL) || (pHttpReq == NULL))
  {
    MSG_ERROR("Invalid URL %s\n", url);
    return -1;
  }
  MSG_INFO("host %s\n req %s\n", hostname, pHttpReq);

  if (getaddrinfo(hostname, NULL, &hints, &hostinfo) != 0)
  {
    MSG_ERROR("Could not find hostname %s from URL %s\n", hostname, url);
    return -1;
  }

  MSG_DEBUG("getaddrinfo() done\n");

  if (hostinfo->ai_family == AF_INET)
  {
    saddr = (struct sockaddr_in *) &addr;
    (VOID) memset((VOID *)saddr, 0, sizeof(struct sockaddr_in));
    saddr->sin_family = AF_INET;
    (VOID) memcpy(&saddr->sin_addr, &((struct sockaddr_in *)(hostinfo->ai_addr))->sin_addr, sizeof(saddr->sin_addr));
    freeaddrinfo(hostinfo);
  }

  port = HTTP_PORT;
  if (secure)
  {
    port = HTTPS_PORT;
  }

  saddr = (struct sockaddr_in *) &addr;
  saddr->sin_port = htons(port);

  const INT sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (sock < 0)
  {
    MSG_ERROR("Could not create the socket.\n");
    return -1;
  }

  MSG_DEBUG("socket() done\n");

  {
    struct sock_winsize tcp_size = {0};

    /*
     * NX_BSD_TCP_WINDOW / 16   ->  319 kbit/s
     * NX_BSD_TCP_WINDOW / 8    -> 1210 kbit/s
     * NX_BSD_TCP_WINDOW / 4    -> 1347 kbit/s
     * NX_BSD_TCP_WINDOW / 2    -> 1346 kbit/s
     * NX_BSD_TCP_WINDOW / 1    -> 1019 kbit/s
     */

    tcp_size.winsize = NX_BSD_TCP_WINDOW / 4;

    INT sock_status = setsockopt(sock, SOL_SOCKET, SO_RCVBUF, &tcp_size, sizeof(tcp_size));
    if (NX_SOC_ERROR == sock_status)
    {
      MSG_ERROR("setsockopt failed\n");
    }
    MSG_DEBUG("setsockopt() done with %"PRIu32"\n", (uint32_t)tcp_size.winsize);
  }

  {
    struct timeval timeout = {0};
    timeout.tv_sec = SOCKET_TIMEOUT_S;

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

  MSG_DEBUG("setsockopt() done\n");

  if (connect(sock, &addr, sizeof(addr)) == 0)
  {
    MSG_INFO("Downloading file %s from %s\n", &pHttpReq[1], hostname);
    ret = http_file_download(sock, hostname, pHttpReq);
#ifdef ENABLE_IOT_DEBUG
    tcp_infos();
#endif /* ENABLE_IOT_DEBUG */

  }
  else
  {
    MSG_ERROR("Cannot connect to %s port: %"PRIu32"\n", hostname, (uint32_t)port);
    soc_close(sock);
    ret =  -1;
  }

  return ret;
}

#ifdef ENABLE_IOT_DEBUG
void tcp_infos(void)
{
  extern NX_IP IpInstance;
  ULONG packets_sent = 0;
  ULONG bytes_sent = 0;
  ULONG packets_received = 0;
  ULONG bytes_received = 0;
  ULONG invalid_packets = 0;
  ULONG receive_packets_dropped = 0;
  ULONG checksum_errors = 0;
  ULONG connections = 0;
  ULONG disconnections = 0;
  ULONG connections_dropped = 0;
  ULONG retransmit_packets = 0;

  _nx_tcp_info_get(&IpInstance, &packets_sent, &bytes_sent,
                   &packets_received, &bytes_received,
                   &invalid_packets, &receive_packets_dropped,
                   &checksum_errors, &connections,
                   &disconnections, &connections_dropped,
                   &retransmit_packets);

  MSG_INFO("\npacket sent             : %"PRIu32"\nbytes sent              : %"PRIu32"\n",
           (uint32_t)packets_sent, (uint32_t)bytes_sent);
  MSG_INFO("packet received         : %"PRIu32"\nbytes received          : %"PRIu32"\n",
           (uint32_t)packets_received, (uint32_t)bytes_received);
  MSG_INFO("invalid packets         : %"PRIu32"\nreceive packets dropped : %"PRIu32"\n",
           (uint32_t)invalid_packets,
           (uint32_t)receive_packets_dropped);
  MSG_INFO("checksum errors         : %"PRIu32"\nconnections             : %"PRIu32"\n",
           (uint32_t)checksum_errors,
           (uint32_t)connections);
  MSG_INFO("disconnections          : %"PRIu32"\nconnections dropped     : %"PRIu32"\n",
           (uint32_t)disconnections, (uint32_t)connections_dropped);
  MSG_INFO("retransmit packets      : %"PRIu32"\n", (uint32_t)retransmit_packets);
}
#endif /* ENABLE_IOT_DEBUG */
