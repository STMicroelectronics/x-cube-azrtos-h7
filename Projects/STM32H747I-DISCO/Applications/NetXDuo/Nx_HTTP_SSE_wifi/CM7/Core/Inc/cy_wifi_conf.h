/**
  ******************************************************************************
  * @file    cy_wifi_conf.h
  * @author  MCD Application Team
  * @brief   Header for cypress_wifi module
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef CY_WIFI_CONF_H
#define CY_WIFI_CONF_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Includes ------------------------------------------------------------------*/

/* Exported macros -----------------------------------------------------------*/

/* Edit the defines below to suit the WiFi connection and the webserver to use. */
/* Access point connection parameters */
#define WIFI_SSID              "MyHotSpot"
#define WIFI_PASSWORD          " "


#define WIFI_USE_THREADX                             1
#define WIFI_USE_NETXDUO                             1

#define CYPRESS_WHD_BYTE_POOL_SIZE (1024 * 30)

#define CYPRESS_WHD_ASSERT(A) \
 do                           \
   {                          \
   } while(!(A))


/* #define WPRINT_ENABLE_WHD_DATA_LOG */

#endif /* CY_WIFI_CONF_H */
