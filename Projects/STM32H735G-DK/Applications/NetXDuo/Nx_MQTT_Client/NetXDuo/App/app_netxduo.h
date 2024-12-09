/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    app_netxduo.h
  * @author  MCD Application Team
  * @brief   NetXDuo applicative header file
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
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __APP_NETXDUO_H__
#define __APP_NETXDUO_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "nx_api.h"

/* Private includes ----------------------------------------------------------*/
#include "nx_stm32_eth_driver.h"

/* USER CODE BEGIN Includes */
#include "main.h"
#include "nxd_dhcp_client.h"
#include "nxd_mqtt_client.h"
#include "nxd_sntp_client.h"
#include "nxd_dns.h"
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */
/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */
#define MOSQUITTO_CERT_FILE         "mosquitto.cert.h"

  /* Threads configuration */
#define PAYLOAD_SIZE                1536
#define NX_PACKET_POOL_SIZE         (( PAYLOAD_SIZE + sizeof(NX_PACKET)) * 10)
#define DEFAULT_MEMORY_SIZE         1024
#define ARP_MEMORY_SIZE             DEFAULT_MEMORY_SIZE
#define DEFAULT_MAIN_PRIORITY       10
#define MQTT_PRIORITY               3
#define SNTP_PRIORITY               5
#define LINK_PRIORITY               11
#define THREAD_MEMORY_SIZE          4 * DEFAULT_MEMORY_SIZE
#define APP_QUEUE_SIZE              10


/* MQTT Client configuration */
#define MQTT_CLIENT_STACK_SIZE      1024 * 10
#define CLIENT_ID_STRING            "MQTT_client_ID"
#define MQTT_THREAD_PRIORTY         2
#define MQTT_KEEP_ALIVE_TIMER       30000                /* Define the MQTT keep alive timer for 5 minutes */
#define CLEAN_SESSION               NX_TRUE
#define STRLEN(p)                   (sizeof(p) - 1)

#define TOPIC_NAME                  "Temperature"
#define NB_MESSAGE                  10                    /*  if NB_MESSAGE = 0, client will publish messages infinitely */

#define MQTT_BROKER_NAME            "test.mosquitto.org" /* MQTT Server */

#define MQTT_PORT                   NXD_MQTT_TLS_PORT

#define QOS0                        0
#define QOS1                        1

#define DEMO_MESSAGE_EVENT          1
#define DEMO_ALL_EVENTS             3

/* SNTP Client configuration */
#define SNTP_CLIENT_THREAD_MEMORY    6 * DEFAULT_MEMORY_SIZE
#define SNTP_SERVER_NAME             "time1.google.com"
#define SNTP_UPDATE_EVENT            1
/* Define how often the demo checks for SNTP updates. */
#define PERIODIC_CHECK_INTERVAL      (60 * NX_IP_PERIODIC_RATE)

/* Define how often we check on SNTP server status. */
#define CHECK_SNTP_UPDATES_TIMEOUT   (180 * NX_IP_PERIODIC_RATE)
#define EPOCH_TIME_DIFF              2208988800     /* is equivalent to 70 years in sec
                                                       calculated with www.epochconverter.com/date-difference */

#define NULL_ADDRESS                0
#define USER_DNS_ADDRESS            IP_ADDRESS(1, 1, 1, 1)   /* User should configure it with his DNS address */

#define DEFAULT_TIMEOUT             5 * NX_IP_PERIODIC_RATE

#define NX_APP_CABLE_CONNECTION_CHECK_PERIOD      (1 * NX_IP_PERIODIC_RATE)
/* USER CODE END EC */
/* The DEFAULT_PAYLOAD_SIZE should match with RxBuffLen configured via MX_ETH_Init */
#ifndef DEFAULT_PAYLOAD_SIZE
#define DEFAULT_PAYLOAD_SIZE      1536
#endif

#ifndef DEFAULT_ARP_CACHE_SIZE
#define DEFAULT_ARP_CACHE_SIZE    1024
#endif

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */
#define PRINT_IP_ADDRESS(addr)           do { \
                                              printf("STM32 %s: %lu.%lu.%lu.%lu \n", #addr, \
                                                (addr >> 24) & 0xff,                        \
                                                  (addr >> 16) & 0xff,                      \
                                                    (addr >> 8) & 0xff,                     \
                                                      (addr & 0xff));                       \
                                            } while(0)
#define PRINT_CNX_SUCC()          do { \
                                        printf("SNTP client connected to NTP server : < %s > \n", SNTP_SERVER_NAME);\
                                     } while(0)

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
UINT MX_NetXDuo_Init(VOID *memory_ptr);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

#define NX_APP_DEFAULT_TIMEOUT               (10 * NX_IP_PERIODIC_RATE)

#define NX_APP_PACKET_POOL_SIZE              ((DEFAULT_PAYLOAD_SIZE + sizeof(NX_PACKET)) * 10)

#define NX_APP_THREAD_STACK_SIZE             4 * 1024

#define Nx_IP_INSTANCE_THREAD_SIZE           2 * 1024

#define NX_APP_THREAD_PRIORITY               10

#ifndef NX_APP_INSTANCE_PRIORITY
#define NX_APP_INSTANCE_PRIORITY             NX_APP_THREAD_PRIORITY
#endif

#define NX_APP_DEFAULT_IP_ADDRESS                   0

#define NX_APP_DEFAULT_NET_MASK                     0

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

#ifdef __cplusplus
}
#endif
#endif /* __APP_NETXDUO_H__ */
