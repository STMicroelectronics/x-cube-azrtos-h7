/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    app_netxduo.h
  * @author  MCD Application Team
  * @brief   NetXDuo applicative header file
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
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
#include "nxd_sntp_client.h"
#include "nxd_dns.h"
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */
/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */
#define SNTP_SERVER_NAME             "0.fr.pool.ntp.org"

#define SNTP_UPDATE_EVENT            1

/* Define how often the demo checks for SNTP updates. */
#define PERIODIC_CHECK_INTERVAL      (60 * NX_IP_PERIODIC_RATE)

/* Define how often we check on SNTP server status. */
#define CHECK_SNTP_UPDATES_TIMEOUT   (180 * NX_IP_PERIODIC_RATE)

#define PAYLOAD_SIZE                 1536
#define NX_PACKET_POOL_SIZE          (( PAYLOAD_SIZE + sizeof(NX_PACKET)) * 10)
#define DEFAULT_MEMORY_SIZE          1024
#define ARP_MEMORY_SIZE              DEFAULT_MEMORY_SIZE
#define SNTP_CLIENT_THREAD_MEMORY    6 * DEFAULT_MEMORY_SIZE
#define MAIN_THREAD_MEMORY           2 * DEFAULT_MEMORY_SIZE

#define DEFAULT_MAIN_PRIORITY        10
#define DEFAULT_PRIORITY             5

#define NULL_ADDRESS                 0
#define DNS_SERVER_ADDRESS           IP_ADDRESS(192, 168, 1, 1)

#define DEFAULT_TIMEOUT              10 * NX_IP_PERIODIC_RATE

/* EPOCH_TIME_DIFF is equivalent to 70 years in sec
   calculated with www.epochconverter.com/date-difference
   This constant is used to delete difference between :
   Epoch converter (referenced to 1970) and SNTP (referenced to 1900) */
#define EPOCH_TIME_DIFF             2208988800


/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */
#define PRINT_IP_ADDRESS(addr)    do { \
                                       printf("STM32 %s: %lu.%lu.%lu.%lu \n", #addr, \
                                       (addr >> 24) & 0xff,                          \
                                       (addr >> 16) & 0xff,                          \
                                       (addr >> 8) & 0xff,                           \
                                       (addr & 0xff));                               \
                                     } while(0)

#define PRINT_SNTP_SERVER(addr)  do { \
                                       printf("Client connected to SNTP server: [%lu.%lu.%lu.%lu] \n", \
                                       (addr >> 24) & 0xff,                      \
                                       (addr >> 16) & 0xff,                    \
                                       (addr >> 8) & 0xff,                   \
                                       (addr & 0xff));                     \
                                    } while(0)

#define PRINT_CNX_SUCC()          do { \
                                        printf("SNTP client connected to NTP server : < %s > \n", SNTP_SERVER_NAME);\
                                     } while(0)
/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
UINT App_NetXDuo_Init(VOID *memory_ptr);

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

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
