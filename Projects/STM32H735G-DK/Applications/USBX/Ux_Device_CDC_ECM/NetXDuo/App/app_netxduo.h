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

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */
#define PRINT_ADDRESS(addr) do { \
                                    printf("%s: %lu.%lu.%lu.%lu \n", #addr, \
                                    (addr >> 24) & 0xff, \
                                    (addr >> 16) & 0xff, \
                                    (addr >> 8) & 0xff, \
                                     addr& 0xff);\
                                  }while(0)
/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
UINT MX_NetXDuo_Init(VOID *memory_ptr);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* IP Memory size */
#define IP_MEMORY_SIZE                   2048
/* Pirority IP creation */
#define DEFAULT_PRIORITY                 10
/*Packet payload size */
#define PACKET_PAYLOAD_SIZE              1536
/* APP Cache size  */
#define ARP_CACHE_SIZE                   1024
/* Packet pool size */
#define NX_PACKET_POOL_SIZE              ((1536 + sizeof(NX_PACKET)) * 60)
/* Wait option for getting @IP */
#define WAIT_OPTION                      1000
/* Entry input for Main thread */
#define ENTRY_INPUT                      0
/* Main Thread priority */
#define THREAD_PRIO                      4
/* Main Thread preemption threshold */
#define THREAD_PREEMPT_THRESHOLD         10
/* Web application size */
#define WEB_STACK_SIZE                   2048
/* HTTP connection port */
#define CONNECTION_PORT                  80
/* Server packet size */
#define SERVER_PACKET_SIZE               (NX_WEB_HTTP_SERVER_MIN_PACKET_SIZE * 2)
/* Server stack */
#define SERVER_STACK                     4096
/* Server pool size */
#define SERVER_POOL_SIZE                 (SERVER_PACKET_SIZE * 4)
/* SD Driver information pointer */
#define SD_DRIVER_INFO_POINTER           0
/* Netx byte pool */
#define NETX_APP_BYTE_POOL_SIZE          ((1024 * 10) + (ARP_CACHE_SIZE + IP_MEMORY_SIZE + WEB_STACK_SIZE))
/* define the NULL ip address */
#define NULL_IP_ADDRESS                  IP_ADDRESS(0,0,0,0)
/* USER CODE END PD */

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

#ifdef __cplusplus
}
#endif
#endif /* __APP_NETXDUO_H__ */
