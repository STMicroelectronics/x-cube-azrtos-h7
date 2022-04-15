/***************************************************************************
  * \file cyabs_rtos_impl.h
  *
  * \brief
  * Internal definitions for RTOS abstraction layer.
  *
  ********************************************************************************
  * \copyright
  * Copyright 2019-2021 Cypress Semiconductor Corporation (an Infineon company) or
  * an affiliate of Cypress Semiconductor Corporation
  *
  * SPDX-License-Identifier: Apache-2.0
  *
  * Licensed under the Apache License, Version 2.0 (the "License");
  * you may not use this file except in compliance with the License.
  * You may obtain a copy of the License at
  *
  *     http://www.apache.org/licenses/LICENSE-2.0
  *
  * Unless required by applicable law or agreed to in writing, software
  * distributed under the License is distributed on an "AS IS" BASIS,
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *******************************************************************************/

#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "cy_wifi_conf.h"

#if (defined(WIFI_USE_CMSIS_OS) && (WIFI_USE_CMSIS_OS == 1))
#include "cmsis_os.h"

/******************************************************
  *                 Constants
  *****************************************************
  */
#define CY_RTOS_MIN_STACK_SIZE      200               /** Minimum stack size in bytes */
#define CY_RTOS_ALIGNMENT           0x00000001UL      /** Minimum alignment for RTOS objects */
#define CY_RTOS_ALIGNMENT_MASK      0x00000001UL      /** Mask for checking the alignment of
                                                          created RTOS objects */


/******************************************************
  *                 Type Definitions
  *****************************************************
  */

/* RTOS thread priority */
typedef enum
{
  CY_RTOS_PRIORITY_MIN         = osPriorityNone,
  CY_RTOS_PRIORITY_LOW         = osPriorityLow,
  CY_RTOS_PRIORITY_BELOWNORMAL = osPriorityBelowNormal,
  CY_RTOS_PRIORITY_NORMAL      = osPriorityNormal,
  CY_RTOS_PRIORITY_ABOVENORMAL = osPriorityAboveNormal,
  CY_RTOS_PRIORITY_HIGH        = osPriorityHigh,
  CY_RTOS_PRIORITY_REALTIME    = osPriorityRealtime,
  CY_RTOS_PRIORITY_MAX         = osPriorityRealtime7
} cy_thread_priority_t;

typedef osThreadId_t       cy_thread_t;             /** CMSIS definition of a thread handle */
typedef uint32_t           cy_thread_arg_t;         /** Argument passed to the entry function of a thread */
typedef osMutexId_t        cy_mutex_t;              /** CMSIS definition of a mutex */
typedef osSemaphoreId_t    cy_semaphore_t;          /** CMSIS definition of a semaphore */
typedef osEventFlagsId_t   cy_event_t;              /** CMSIS definition of an event */
typedef osMessageQueueId_t cy_queue_t;              /** CMSIS definition of a message queue */
typedef osTimerId_t        cy_timer_t;              /** CMSIS definition of a timer */
typedef uint32_t           cy_timer_callback_arg_t; /** Argument passed to the timer callback function */
typedef uint32_t           cy_time_t;               /** Time in milliseconds */
typedef osStatus_t         cy_rtos_error_t;         /** CMSIS definition of a error status */



#elif (defined(WIFI_USE_THREADX) && (WIFI_USE_THREADX==1))
#include "tx_api.h"


#ifdef __cplusplus
extern "C"
{
#endif
/******************************************************
  *                 Constants
  *****************************************************
  */

#define CY_RTOS_MIN_STACK_SIZE  TX_MINIMUM_STACK /** Minimum stack size in bytes */
#define CY_RTOS_ALIGNMENT       0x00000004UL     /** Minimum alignment for RTOS objects */
#define CY_RTOS_ALIGNMENT_MASK  0x00000003UL     /** Checks for 4-byte alignment */


/******************************************************
  *                 Type Definitions
  *****************************************************
  */

/* RTOS thread priority */
typedef enum
{
  CY_RTOS_PRIORITY_MIN         = TX_MAX_PRIORITIES - 1,
  CY_RTOS_PRIORITY_LOW         = (TX_MAX_PRIORITIES * 6 / 7),
  CY_RTOS_PRIORITY_BELOWNORMAL = (TX_MAX_PRIORITIES * 5 / 7),
  CY_RTOS_PRIORITY_NORMAL      = (TX_MAX_PRIORITIES * 4 / 7),
  CY_RTOS_PRIORITY_ABOVENORMAL = (TX_MAX_PRIORITIES * 3 / 7),
  CY_RTOS_PRIORITY_HIGH        = (TX_MAX_PRIORITIES * 2 / 7),
  CY_RTOS_PRIORITY_REALTIME    = (TX_MAX_PRIORITIES * 1 / 7),
  CY_RTOS_PRIORITY_MAX         = 0
} cy_thread_priority_t;

typedef struct
{
  uint32_t     maxcount;
  TX_SEMAPHORE tx_semaphore;
} cy_semaphore_t;

typedef struct
{
  ULONG *mem;
  /* ThreadX buffer size is a power of 2 times word size, */
  /* this is used to prevent memory corruption when get message from queue. */
  size_t   itemsize;
  TX_QUEUE tx_queue;
} cy_queue_t;

typedef struct
{
  bool     oneshot;
  TX_TIMER tx_timer;
} cy_timer_t;

typedef TX_THREAD              *cy_thread_t;
typedef ULONG                   cy_thread_arg_t;           /** Argument passed to the entry function of a thread */
typedef TX_MUTEX                cy_mutex_t;                /** ThreadX definition of a mutex */
typedef TX_EVENT_FLAGS_GROUP    cy_event_t;
typedef ULONG                   cy_timer_callback_arg_t;   /** Argument passed to the timer callback function */
typedef uint32_t                cy_time_t;                 /** Time in milliseconds */
typedef UINT                    cy_rtos_error_t;           /** ThreadX definition of a error status */


/* Declare the memory handlers. */
void *cypress_whd_malloc(size_t size);
void *cypress_whd_calloc(size_t n, size_t m);
void  cypress_whd_free(void *p);


#define WHD_MALLOC(SIZE)     cypress_whd_malloc((SIZE))
#define WHD_CALLOC(SIZE, X)  cypress_whd_calloc((SIZE), (X))
#define WHD_FREE(P)          cypress_whd_free((P))

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* WIFI_USE_CMSIS_OS */
