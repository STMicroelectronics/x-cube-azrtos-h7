/**
  ******************************************************************************
  * @file    iap_client.h
  * @author  MCD Application Team
  * @brief   Header file for iap_client.c
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
#ifndef IAP_CLIENT_H
#define IAP_CLIENT_H

/* Includes ------------------------------------------------------------------*/
#include "nx_api.h"

VOID StartWiFiIAP(TX_BYTE_POOL *byte_pool, NX_IP *ip_instance);

#endif /* IAP_CLIENT_H */
