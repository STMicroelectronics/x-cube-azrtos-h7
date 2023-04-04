/**
  ******************************************************************************
  * @file    httpserver_sentevents_socket.h
  * @author  MCD Application Team
  * @brief   Http server declarations.
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

#ifndef HTTPSERVER_SENTEVENTS_SOCKET_H
#define HTTPSERVER_SENTEVENTS_SOCKET_H

#include <stddef.h>
#include <stdint.h>
#include "tx_port.h"
#include "nx_api.h"

typedef enum
{
  STATUS_UNKNOWN     = 0,
  STATUS_NEW         = 1,   /* New connection, no communication yet. */
  STATUS_COMM        = 2,   /* Connected. */
  STATUS_COMM_STREAM = 3,   /* Connected and stream mode. */
  STATUS_END         = 4    /* Communication terminated. */
} ClientStatusTypeDef;

typedef struct ClientInfos
{
  NX_TCP_SOCKET      *tcp_socket;       /* NetXDuo socket pointer. */
  char                info[64];         /* Ip & port */
  char                err;              /* Error status */
  ClientStatusTypeDef status;           /* Connection status */
  int32_t             message;          /* Message count */
  TX_THREAD           thread;           /* Thread that push event to connected peer. */
  CHAR                thread_name[16];  /* Corresponding name for the thread.*/
} ClientInfosTypeDef;

UINT http_server_socket_init(TX_BYTE_POOL *byte_pool);

#endif /* HTTPSERVER_SENTEVENTS_SOCKET_H */
