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
#include "main.h"
#include "ux_api.h"
#include "app_filex.h"
#include "nxd_dhcp_client.h"
#include "nx_web_http_server.h"
#include "ux_network_driver.h"
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* Packet payload size */
#define PACKET_PAYLOAD_SIZE    1536

/* Packet pool size */
#define NX_PACKET_POOL_SIZE    ((1536 + sizeof(NX_PACKET)) * 60)

/* HTTP connection port */
#define CONNECTION_PORT        80

/* Server packet size */
#define SERVER_PACKET_SIZE     (NX_WEB_HTTP_SERVER_MIN_PACKET_SIZE * 2)

/* Server pool size */
#define SERVER_POOL_SIZE       (SERVER_PACKET_SIZE * 4)

#define PRINT_ADDRESS(addr)    do { \
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
VOID nx_server_thread_entry(ULONG thread_input);
/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

#ifdef __cplusplus
}
#endif
#endif /* __APP_NETXDUO_H__ */
