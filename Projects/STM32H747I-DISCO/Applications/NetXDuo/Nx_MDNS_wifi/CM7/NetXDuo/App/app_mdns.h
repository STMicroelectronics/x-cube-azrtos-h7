/**
  ******************************************************************************
  * @file    app_mdns.h
  * @author  MCD Application Team
  * @brief   ThreadX applicative header file
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
#ifndef __APP_MDNS_H
#define __APP_MDNS_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "tx_api.h"

/* Private includes ----------------------------------------------------------*/

/* Exported types ------------------------------------------------------------*/



typedef enum
{
  MDNS_START,
  MDNS_ANNOUNCE,
  MDNS_ANNOUNCE_INVALID_PARAMS,
  MDNS_DEANNOUNCE,
  MDNS_DEANNOUNCE_INVALID_PARAMS,
  MDNS_DEANNOUNCE_ALL,
  MDNS_SET_HOSTNAME,
  MDNS_UPDATE_TXT_RECORD,
  MDNS_STOP,
  MDNS_RESERVE
} MDNS_TESTCASES;


/* Exported constants --------------------------------------------------------*/

/* Exported macro ------------------------------------------------------------*/
CHAR  *mdns_host_name = "mDNS-HOST";
#define SERVICE1_TTL                      (USHORT)120
#define SERVICE1_PRIORITY                 (USHORT)0
#define SERVICE1_WEIGHTS                  (USHORT)0
#define SERVICE1_PORT                     (USHORT)8000
#define QUERY_TIMEOUT                     (USHORT)500

#define SERVICE_INSTANCE_NAME   "test_announce"
#define SERVICE1_INSTANCE_NAME  "test_announce1"
#define SERVICE2_INSTANCE_NAME  "test_announce2"
#define SERVICE3_INSTANCE_NAME  "test_announce3"
#define SERVICE1_TYPE_TEST_ST   "_test_st._tcp"
#define SERVICE1_TXT_INFO       "txtver=1/.0/.0.info=mdns-st"

#define SERVICE_INSTANCE_NULL   NX_NULL
#define SERVICE_TYPE_NULL       NX_NULL
#define SERVICE_SUBTYPE_NULL    NX_NULL
#define SERVICE_TXT_NULL        NX_NULL

/* Exported functions prototypes ---------------------------------------------*/
VOID cache_full_notify(NX_MDNS *mdns_ptr, UINT state, UINT cache_type);
VOID service_change_notify(NX_MDNS *mdns_ptr, NX_MDNS_SERVICE *service_ptr, UINT state);
void register_local_service(UCHAR *instance, UCHAR *type, UCHAR *subtype, UCHAR *txt, UINT ttl,
                            USHORT priority, USHORT weight, USHORT port, UCHAR is_unique);
void perform_oneshot_query(UCHAR *instance, UCHAR *type, UCHAR *subtype, UINT timeout);
void start_continous_query(UCHAR *instance, UCHAR *type, UCHAR *subtype);
VOID probing_notify(struct NX_MDNS_STRUCT *mdns_ptr, UCHAR *name, UINT state);
void delete_local_service(UCHAR *instance, UCHAR *type, UCHAR *subtype);
void delete_all_services(UCHAR *instance, UCHAR *type, UCHAR *subtype);

/* Private defines -----------------------------------------------------------*/


#ifdef __cplusplus
}
#endif
#endif /* __APP_MDNS_H__ */
