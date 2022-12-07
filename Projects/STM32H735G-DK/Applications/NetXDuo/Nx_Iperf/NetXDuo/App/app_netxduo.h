/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    app_netxduo.h
  * @author  MCD Application Team
  * @brief   NetXDuo applicative header file
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
#include <stdio.h>
#include "main.h"
#include "nx_web_http_server.h"
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

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
#define PRINT_IP_ADDRESS(addr)             do { \
                                                printf("STM32 %s: %lu.%lu.%lu.%lu \n", #addr, \
                                                (addr >> 24) & 0xff, \
                                                (addr >> 16) & 0xff, \
                                                (addr >> 8) & 0xff, \
                                                addr& 0xff);\
                                           }while(0)

#define PRINT_DATA(addr, port, data)       do { \
                                                printf("[%lu.%lu.%lu.%lu:%u] -> '%s' \n", \
                                                (addr >> 24) & 0xff, \
                                                (addr >> 16) & 0xff, \
                                                (addr >> 8) & 0xff,  \
                                                (addr & 0xff), port, data); \
                                           } while(0)

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
UINT MX_NetXDuo_Init(VOID *memory_ptr);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
/* USER CODE BEGIN PD */


/* Define demo stack size.   */
#define                 PACKET_PAYLOAD_SIZE        1536
#define                 NX_PACKET_POOL_SIZE        ((PACKET_PAYLOAD_SIZE + sizeof(NX_PACKET)) * 40)
#define                 DEMO_STACK_SIZE            2048
#define                 HTTP_STACK_SIZE            2048
#define                 IPERF_STACK_SIZE           2048


#define                 PAYLOAD_SIZE               1536 + sizeof(NX_PACKET)

#define                 DEFAULT_MEMORY_SIZE        1024
#define                 DEFAULT_PRIORITY           10
#define LINK_PRIORITY                              11

#define                 NULL_ADDRESS               0

#define SERVER_PACKET_SIZE                        (1024 * 2)

/* Server pool size */
#define SERVER_POOL_SIZE                          (SERVER_PACKET_SIZE * 8)

#define NX_APP_INSTANCE_PRIORITY                  1

#define NX_APP_CABLE_CONNECTION_CHECK_PERIOD      (6 * NX_IP_PERIODIC_RATE)
/* USER CODE END PD */

#define NX_APP_DEFAULT_TIMEOUT               (10 * NX_IP_PERIODIC_RATE)

#define NX_APP_PACKET_POOL_SIZE              ((DEFAULT_PAYLOAD_SIZE + sizeof(NX_PACKET)) * 40)

#define NX_APP_THREAD_STACK_SIZE             2 * 1024

#define Nx_IP_INSTANCE_THREAD_SIZE           2 * 1024

#define NX_APP_THREAD_PRIORITY               4

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
