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

/* USER CODE BEGIN Includes */
#include <inttypes.h>
#include "main.h"
#include "nx_ip.h"
#include "nxd_mqtt_client.h"
#include "nxd_dhcp_client.h"
#include "nxd_dns.h"
#include "nx_stm32_cypress_whd_driver.h"
#include "cyabs_rtos_impl.h"
#include "msg.h"
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */
/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */
#define BROKER_CONF1
/*#define BROKER_CONF2*/

#if defined(BROKER_CONF1)
#define MOSQUITTO_CERT_FILE                  "mosquitto.cert.h"
#define MQTT_BROKER_NAME                     "test.mosquitto.org" /* The MQTT Server. */

#elif defined(BROKER_CONF2)
#define BROKER_EMQX_IO_CERT_FILE             "broker.emqx.io.cert.h"
#define MQTT_BROKER_NAME                     "broker.emqx.io"     /* The MQTT Server. */

#else
#endif /* BROKER_CONF1 */


/* Adding more than needed due to some extra memory needed inside the Cypress driver. */

/**
 *  HD_BUS_SDIO_MAX_BACKPLANE_TRANSFER_SIZE(1536) +
 *                         MAX_BUS_HEADER_SIZE(4) +
 *                 sizeof(whd_buffer_header_t)(8) = 1548
 */
#define PAYLOAD_SIZE                         1548
#define NX_PACKET_POOL_SIZE                  ((PAYLOAD_SIZE + sizeof(NX_PACKET)) *  10)

#define DEFAULT_MEMORY_SIZE                  1024
#define ARP_MEMORY_SIZE                      DEFAULT_MEMORY_SIZE

/* relative values compare to the WHD_THREAD_PRIORITY priority. */
#define MAIN_THREAD_STACK_SIZE               3 * DEFAULT_MEMORY_SIZE
#define MAIN_THREAD_PRIORITY                 CY_RTOS_PRIORITY_BELOWNORMAL

#define NETX_IP_THREAD_STACK_SIZE            5 * DEFAULT_MEMORY_SIZE
#define NETX_IP_THREAD_PRIORITY              CY_RTOS_PRIORITY_NORMAL

/* MQTT Client configuration */
#define MQTT_CLIENT_THREAD_STACK_SIZE        10 * DEFAULT_MEMORY_SIZE
#define MQTT_CLIENT_THREAD_PRIORITY          CY_RTOS_PRIORITY_BELOWNORMAL

#define MQTT_CLIENT_STACK_SIZE               12 * DEFAULT_MEMORY_SIZE

#define CLIENT_ID_STRING                     "MQTT_client_ID3659"
#define MQTT_THREAD_PRIORTY                  CY_RTOS_PRIORITY_BELOWNORMAL
#define MQTT_KEEP_ALIVE_TIMER                30000         /* Define the MQTT keep alive timer for 5 minutes */
#define CLEAN_SESSION                        NX_TRUE
#define STRLEN(p)                            (sizeof(p) - 1)

#define TOPIC_NAME                           "stm32_rng_value"
#define NB_MESSAGE                           10            /*  if NB_MESSAGE = 0, client will publish messages infinitely */

#define QOS0                                 0
#define QOS1                                 1

#define DEMO_MESSAGE_EVENT                   1
#define DEMO_ALL_EVENTS                      3

#define DEFAULT_TIMEOUT                      5 * NX_IP_PERIODIC_RATE

#define NULL_ADDRESS                         (void*)0

#define DHCP_IP_TRANSITION_TIMEOUT           20000

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */
#define PRINT_IP_ADDRESS(addr) \
  do { \
    MSG_INFO("%"PRIu32".%"PRIu32".%"PRIu32".%"PRIu32"\n", \
            ((uint32_t)(addr) >> 24) & 0xff, \
            ((uint32_t)(addr) >> 16) & 0xff, \
            ((uint32_t)(addr) >> 8) & 0xff, \
            ((uint32_t)(addr) & 0xff)); \
  } while(0)

#define PRINT_SERVER_ADDRESS(addr, port) \
  do { \
    MSG_INFO("%"PRIu32".%"PRIu32".%"PRIu32".%"PRIu32":%"PRIu32"\n", \
            ((uint32_t)(addr) >> 24) & 0xff, \
            ((uint32_t)(addr) >> 16) & 0xff, \
            ((uint32_t)(addr) >> 8) & 0xff, \
            ((uint32_t)(addr) & 0xff), \
            (uint32_t)(port)); \
  } while(0)
/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
UINT MX_NetXDuo_Init(VOID *memory_ptr);

/* USER CODE BEGIN EFP */

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
