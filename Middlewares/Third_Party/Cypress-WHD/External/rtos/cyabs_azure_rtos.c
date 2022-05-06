/*********************************************************************************/
/**
  * \file cyabs_azure_rtos.c
  *
  * \brief
  * Implementation for AZURE RTOS abstraction
  *
  ********************************************************************************
  * \copyright
  * Copyright 2018-2021 Cypress Semiconductor Corporation (an Infineon company) or
  * an affiliate of Cypress Semiconductor Corporation
  * Copyright (c) 2021 STMicroelectronics. All rights reserved.
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

#include "cyabs_rtos.h"
#include "tx_api.h"
#include <stdlib.h>
#include <inttypes.h>
#include <assert.h>
#include "whd_config.h"
#include "nx_stm32_cypress_whd_driver.h"
#include "cy_wifi_conf.h"

/* Indicate that driver source is being compiled.  */
#define NX_DRIVER_SOURCE

#include "nx_driver_framework.h"


static const uint32_t WRAPPER_IDENT          = 0xABCDEF01U;

/* static const uint32_t MAX_QUEUE_MESSAGE_SIZE = 16; */
#define MAX_QUEUE_MESSAGE_SIZE               (uint32_t)16

#define ALL_EVENT_FLAGS (0xFFFFFFFFU)
#define MILLISECONDS_PER_SECOND (1000)

static cy_rtos_error_t last_error;

#if defined(__cplusplus)
extern "C" {
#endif


static ULONG cypress_whd_byte_pool_buffer[CYPRESS_WHD_BYTE_POOL_SIZE / sizeof(ULONG)];

static UINT cypress_whd_thread_deinit(TX_THREAD *thread_ptr);

static TX_BYTE_POOL CypressWhdBytePool;

/*--------------------------------------------------------------------------------------------------*/
/* convert_ms_to_ticks                                                                              */
/*--------------------------------------------------------------------------------------------------*/
static cy_time_t convert_ms_to_ticks(cy_time_t timeout_ms)
{
  if (timeout_ms == CY_RTOS_NEVER_TIMEOUT)
  {
    return TX_WAIT_FOREVER;
  }
  else if (timeout_ms == 0)
  {
    return 0;
  }
  else
  {
    uint64_t ticks = (uint64_t)timeout_ms * (uint64_t)TX_TIMER_TICKS_PER_SECOND /
                     (uint64_t)MILLISECONDS_PER_SECOND;
    if (ticks == 0)
    {
      ticks = 1;
    }
    else if (ticks >= UINT32_MAX)
    {
      /* if ticks if more than 32 bits, change ticks to max possible value that isn't */
      /* TX_WAIT_FOREVER.                                                             */
      ticks = UINT32_MAX - 1;
    }
    return (cy_time_t)ticks;
  }
}

#if 0
/*--------------------------------------------------------------------------------------------------*/
/* convert_ticks_to_ms                                                                              */
/*--------------------------------------------------------------------------------------------------*/
static inline cy_time_t convert_ticks_to_ms(cy_time_t timeout_ticks)
{
  return (cy_time_t)((uint64_t)timeout_ticks * (uint64_t)MILLISECONDS_PER_SECOND /
                     (uint64_t)TX_TIMER_TICKS_PER_SECOND);
}
#endif /* 0 */

/*--------------------------------------------------------------------------------------------------*/
/* convert_error                                                                                    */
/*--------------------------------------------------------------------------------------------------*/
static inline cy_rslt_t convert_error(cy_rtos_error_t error)
{
  if (error != TX_SUCCESS)
  {
    last_error = error;
    return CY_RTOS_GENERAL_ERROR;
  }
  return CY_RSLT_SUCCESS;
}


/**
  *****************************************************
  *                 Last Error
  *****************************************************
  */

/*--------------------------------------------------------------------------------------------------*/
/* cy_rtos_last_error                                                                               */
/*--------------------------------------------------------------------------------------------------*/
cy_rtos_error_t cy_rtos_last_error(void)
{
  return last_error;
}


static UINT cypress_whd_thread_deinit(TX_THREAD *thread_ptr)
{
  return tx_thread_delete(thread_ptr);
}

/**
  *****************************************************
  *                 Threads
  *****************************************************
  */


typedef struct
{
  TX_THREAD thread;
  uint32_t  magic;
  void     *memptr;
} cy_thread_wrapper_t;


/*--------------------------------------------------------------------------------------------------*/
/* cy_rtos_create_thread                                                                            */
/*--------------------------------------------------------------------------------------------------*/
cy_rslt_t cy_rtos_create_thread(cy_thread_t *thread, cy_thread_entry_fn_t entry_function,
                                const char *name, void *stack, uint32_t stack_size,
                                cy_thread_priority_t priority, cy_thread_arg_t arg)
{
  stack_size &= ~CY_RTOS_ALIGNMENT_MASK; /* make stack pointer 8-byte aligned */

  if ((thread == NULL) || (stack_size < CY_RTOS_MIN_STACK_SIZE))
  {
    return CY_RTOS_BAD_PARAM;
  }

  if ((stack != NULL) && (0 != (((uint32_t)stack) & CY_RTOS_ALIGNMENT_MASK)))
  {
    return CY_RTOS_ALIGNMENT_ERROR;
  }

  size_t malloc_size = sizeof(cy_thread_wrapper_t);
  if (stack == NULL)
  {
    malloc_size += stack_size;
  }
  /* Do not worry about alignment, it is handled by tx_thread_create */
  void *buffer = cypress_whd_malloc(malloc_size);
  if (buffer == NULL)
  {
    return CY_RTOS_NO_MEMORY;
  }

  cy_thread_wrapper_t *wrapper_ptr;
  if (stack == NULL)
  {
    stack = buffer;
    /* Have stack be in front of wrapper since stack size is x-byte aligned. */
    wrapper_ptr         = (cy_thread_wrapper_t *)((uint8_t *)buffer + stack_size);
    wrapper_ptr->memptr = stack;
  }
  else
  {
    wrapper_ptr         = buffer;
    wrapper_ptr->memptr = NULL;
  }
  wrapper_ptr->magic = WRAPPER_IDENT;

  *thread = (cy_thread_t)wrapper_ptr;

  /* Disable preemption-thresholding and time slicing */
  cy_rtos_error_t tx_rslt = tx_thread_create(*thread, (CHAR *)name, entry_function, arg, stack,
                                             stack_size, priority, priority, TX_NO_TIME_SLICE,
                                             TX_AUTO_START);

  if (TX_SUCCESS != tx_rslt)
  {
    last_error = tx_rslt;
    cypress_whd_free(buffer);
    return CY_RTOS_GENERAL_ERROR;
  }

  return CY_RSLT_SUCCESS;
}


/*--------------------------------------------------------------------------------------------------*/
/* cy_rtos_exit_thread                                                                              */
/*--------------------------------------------------------------------------------------------------*/
cy_rslt_t cy_rtos_exit_thread(void)
{
  cy_rslt_t status;
  extern TX_THREAD *_tx_thread_created_ptr;

  status = cypress_whd_thread_deinit(_tx_thread_created_ptr);

  return status;
}


/*--------------------------------------------------------------------------------------------------*/
/* cy_rtos_terminate_thread                                                                         */
/*--------------------------------------------------------------------------------------------------*/
cy_rslt_t cy_rtos_terminate_thread(cy_thread_t *thread)
{
  if (thread == NULL)
  {
    return CY_RTOS_BAD_PARAM;
  }

  return convert_error(tx_thread_terminate(*thread));
}


/*--------------------------------------------------------------------------------------------------*/
/* cy_rtos_is_thread_running                                                                        */
/*--------------------------------------------------------------------------------------------------*/
cy_rslt_t cy_rtos_is_thread_running(cy_thread_t *thread, bool *running)
{
  if ((thread == NULL) || (running == NULL))
  {
    return CY_RTOS_BAD_PARAM;
  }

  /* Only true when the given thread is the current one */
  *running = (*thread == tx_thread_identify());
  return CY_RSLT_SUCCESS;
}


/*--------------------------------------------------------------------------------------------------*/
/* cy_rtos_get_thread_state                                                                         */
/*--------------------------------------------------------------------------------------------------*/
cy_rslt_t cy_rtos_get_thread_state(cy_thread_t *thread, cy_thread_state_t *state)
{
  if ((thread == NULL) || (state == NULL))
  {
    return CY_RTOS_BAD_PARAM;
  }

  bool running;
  cy_rslt_t rslt = cy_rtos_is_thread_running(thread, &running);
  if (CY_RSLT_SUCCESS != rslt)
  {
    return rslt;
  }
  else if (running)
  {
    *state = CY_THREAD_STATE_RUNNING;
    return CY_RSLT_SUCCESS;
  }

  UINT thread_state;
  cy_rtos_error_t tx_rslt = tx_thread_info_get(*thread, TX_NULL, &thread_state, TX_NULL, TX_NULL,
                                               TX_NULL, TX_NULL, TX_NULL, TX_NULL);
  if (TX_SUCCESS != tx_rslt)
  {
    last_error = tx_rslt;
    return CY_RTOS_GENERAL_ERROR;
  }

  /* Descriptions of these states are not given in the ThreadX user guide - these are best guesses */
  /* as to their meanings                                                                          */
  switch (thread_state)
  {
    case TX_READY:
      *state = CY_THREAD_STATE_READY;
      break;

    case TX_COMPLETED:
    case TX_TERMINATED:
      *state = CY_THREAD_STATE_TERMINATED;
      break;

    case TX_SUSPENDED:
    case TX_SLEEP:
    case TX_QUEUE_SUSP:
    case TX_SEMAPHORE_SUSP:
    case TX_MUTEX_SUSP:
    case TX_EVENT_FLAG:    /* Likely waiting for event flags to be set (tx_event_flags_get)  */
    case TX_BLOCK_MEMORY:  /* Likely waiting to allocate a memory block (tx_block_allocate)  */
    case TX_BYTE_MEMORY:   /* Likely waiting to allocate a byte pool (tx_byte_allocate)      */
      *state = CY_THREAD_STATE_BLOCKED;
      break;

    default:
      *state = CY_THREAD_STATE_UNKNOWN;
      break;
  }

  return CY_RSLT_SUCCESS;
}


/*--------------------------------------------------------------------------------------------------*/
/* cy_rtos_join_thread                                                                              */
/*--------------------------------------------------------------------------------------------------*/
cy_rslt_t cy_rtos_join_thread(cy_thread_t *thread)
{
  if (thread == NULL)
  {
    return CY_RTOS_BAD_PARAM;
  }

  /* ThreadX doesn't have a join method itself, so just repeatedly check the thread's state until */
  /* it completes or is terminated.                                                               */
  /* Check if the thread we are joining has a higher priority, if it does, we need to lower our   */
  /* priority to that of the other thread.                                                        */
  UINT thread_state;

  cy_rtos_error_t tx_rslt = tx_thread_info_get(*thread, TX_NULL, &thread_state, TX_NULL, TX_NULL,
                                               TX_NULL, TX_NULL, TX_NULL, TX_NULL);
  if (TX_SUCCESS != tx_rslt)
  {
    last_error = tx_rslt;
    return CY_RTOS_GENERAL_ERROR;
  }

  while (TX_TERMINATED != thread_state && TX_COMPLETED != thread_state)
  {
    tx_rslt = tx_thread_sleep(1);
    if (TX_SUCCESS != tx_rslt)
    {
      last_error = tx_rslt;
      return CY_RTOS_GENERAL_ERROR;
    }

    tx_rslt = tx_thread_info_get(*thread, TX_NULL, &thread_state, TX_NULL, TX_NULL, TX_NULL,
                                 TX_NULL, TX_NULL, TX_NULL);
    if (TX_SUCCESS != tx_rslt)
    {
      last_error = tx_rslt;
      return CY_RTOS_GENERAL_ERROR;
    }
  }

  tx_rslt = tx_thread_delete(*thread);
  if (TX_SUCCESS != tx_rslt)
  {
    last_error = tx_rslt;
    return CY_RTOS_GENERAL_ERROR;
  }

  cy_thread_wrapper_t *wrapper_ptr = (cy_thread_wrapper_t *)(*thread);
  if (wrapper_ptr->magic == WRAPPER_IDENT)
  {
    if (wrapper_ptr->memptr != NULL)
    {
      cypress_whd_free(wrapper_ptr->memptr);
    }
    else
    {
      cypress_whd_free(wrapper_ptr);
    }
  }
  return CY_RSLT_SUCCESS;
}


/*--------------------------------------------------------------------------------------------------*/
/* cy_rtos_get_thread_handle                                                                        */
/*--------------------------------------------------------------------------------------------------*/
cy_rslt_t cy_rtos_get_thread_handle(cy_thread_t *thread)
{
  if (thread == NULL)
  {
    return CY_RTOS_BAD_PARAM;
  }

  *thread = tx_thread_identify();
  return CY_RSLT_SUCCESS;
}


/*--------------------------------------------------------------------------------------------------*/
/* cy_rtos_wait_thread_notification                                                                 */
/*--------------------------------------------------------------------------------------------------*/
cy_rslt_t cy_rtos_wait_thread_notification(cy_time_t timeout_ms)
{
  UINT ret;
  cy_rslt_t status = CY_RSLT_SUCCESS;

  ret = tx_thread_sleep(convert_ms_to_ticks(timeout_ms));
  /* Update the last known error status */
  last_error = (cy_rtos_error_t)ret;

  if (ret == TX_SUCCESS)
  {
    status = CY_RTOS_TIMEOUT;
  }
  else if (ret != TX_WAIT_ABORTED)
  {
    status = CY_RTOS_GENERAL_ERROR;
  }
  return status;
}


/*--------------------------------------------------------------------------------------------------*/
/* cy_rtos_set_thread_notification                                                                  */
/*--------------------------------------------------------------------------------------------------*/
cy_rslt_t cy_rtos_set_thread_notification(cy_thread_t *thread, bool in_isr)
{
  (void)in_isr;

  if (thread == NULL)
  {
    return CY_RTOS_BAD_PARAM;
  }
  /* According to ThreadX user guide, this function allowed to
   * be called from ISR
   */
  return convert_error(tx_thread_wait_abort(*thread));
}


/**
  *****************************************************
  *                 Mutexes
  *****************************************************
  */

/*--------------------------------------------------------------------------------------------------*/
/* cy_rtos_init_mutex2                                                                              */
/*--------------------------------------------------------------------------------------------------*/
cy_rslt_t cy_rtos_init_mutex2(cy_mutex_t *mutex, bool recursive)
{
  if (mutex == NULL)
  {
    return CY_RTOS_BAD_PARAM;
  }

  /* Non recursive mutex is not supported by ThreadX. A recursive mutex is returned    */
  /* even if a non-recursive mutex was requested. This is ok because in all the cases  */
  /* where the behavior of the two types differs would have ended in a deadlock. So    */
  /* the difference in behavior should not have a functional impact on application.    */
  (void)recursive;
  return convert_error(tx_mutex_create(mutex, "cy_rtos_mutex2", TX_INHERIT));
}


/*--------------------------------------------------------------------------------------------------*/
/* cy_rtos_get_mutex                                                                                */
/*--------------------------------------------------------------------------------------------------*/
cy_rslt_t cy_rtos_get_mutex(cy_mutex_t *mutex, cy_time_t timeout_ms)
{
  if (mutex == NULL)
  {
    return CY_RTOS_BAD_PARAM;
  }

  cy_rtos_error_t tx_rslt = tx_mutex_get(mutex, convert_ms_to_ticks(timeout_ms));
  if (TX_NOT_AVAILABLE == tx_rslt)
  {
    return CY_RTOS_TIMEOUT;
  }
  else
  {
    return convert_error(tx_rslt);
  }
}


/*--------------------------------------------------------------------------------------------------*/
/* cy_rtos_set_mutex                                                                                */
/*--------------------------------------------------------------------------------------------------*/
cy_rslt_t cy_rtos_set_mutex(cy_mutex_t *mutex)
{
  if (mutex == NULL)
  {
    return CY_RTOS_BAD_PARAM;
  }

  return convert_error(tx_mutex_put(mutex));
}


/*--------------------------------------------------------------------------------------------------*/
/* cy_rtos_deinit_mutex                                                                             */
/*--------------------------------------------------------------------------------------------------*/
cy_rslt_t cy_rtos_deinit_mutex(cy_mutex_t *mutex)
{
  if (mutex == NULL)
  {
    return CY_RTOS_BAD_PARAM;
  }

  return convert_error(tx_mutex_delete(mutex));
}


/**
  ****************************************************
  *                 Semaphores
  ****************************************************
  */

/*--------------------------------------------------------------------------------------------------*/
/* cy_rtos_init_semaphore                                                                           */
/*--------------------------------------------------------------------------------------------------*/
cy_rslt_t cy_rtos_init_semaphore(cy_semaphore_t *semaphore, uint32_t maxcount, uint32_t initcount)
{
  if (semaphore == NULL)
  {
    return CY_RTOS_BAD_PARAM;
  }
  semaphore->maxcount = maxcount;
  return convert_error(tx_semaphore_create(&(semaphore->tx_semaphore), "cy_rtos_semaphore", initcount));
}


/*--------------------------------------------------------------------------------------------------*/
/* cy_rtos_get_semaphore                                                                            */
/*--------------------------------------------------------------------------------------------------*/
cy_rslt_t cy_rtos_get_semaphore(cy_semaphore_t *semaphore, cy_time_t timeout_ms, bool in_isr)
{
  /* Based on documentation                                                   */
  /* http://www.ece.ualberta.ca/~cmpe490/documents/ghs/405/threadxug_g40c.pdf */
  /* pg 168 it specifies that the timeout must be zero when called from ISR.  */
  if ((semaphore == NULL) || (in_isr && (timeout_ms != 0)))
  {
    return CY_RTOS_BAD_PARAM;
  }
  cy_rtos_error_t tx_rslt =
    tx_semaphore_get(&(semaphore->tx_semaphore), convert_ms_to_ticks(timeout_ms));

  if (TX_NO_INSTANCE == tx_rslt)
  {
    return CY_RTOS_TIMEOUT;
  }
  else
  {
    return convert_error(tx_rslt);
  }
}


/*--------------------------------------------------------------------------------------------------*/
/* cy_rtos_set_semaphore                                                                            */
/*--------------------------------------------------------------------------------------------------*/
cy_rslt_t cy_rtos_set_semaphore(cy_semaphore_t *semaphore, bool in_isr)
{
  (void)in_isr;
  if (semaphore == NULL)
  {
    return CY_RTOS_BAD_PARAM;
  }
  return convert_error(tx_semaphore_ceiling_put(&(semaphore->tx_semaphore), semaphore->maxcount));
}


/*--------------------------------------------------------------------------------------------------*/
/* cy_rtos_get_count_semaphore                                                                      */
/*--------------------------------------------------------------------------------------------------*/
cy_rslt_t cy_rtos_get_count_semaphore(cy_semaphore_t *semaphore, size_t *count)
{
  if ((semaphore == NULL) || (count == NULL))
  {
    return CY_RTOS_BAD_PARAM;
  }
  return convert_error(tx_semaphore_info_get(&(semaphore->tx_semaphore), TX_NULL, (ULONG *)count,
                                             TX_NULL, TX_NULL, TX_NULL));
}


/*--------------------------------------------------------------------------------------------------*/
/* cy_rtos_deinit_semaphore                                                                         */
/*--------------------------------------------------------------------------------------------------*/
cy_rslt_t cy_rtos_deinit_semaphore(cy_semaphore_t *semaphore)
{
  if (semaphore == NULL)
  {
    return CY_RTOS_BAD_PARAM;
  }
  return convert_error(tx_semaphore_delete(&(semaphore->tx_semaphore)));
}


/**
  ****************************************************
  *                 Events
  ****************************************************
  */

/*--------------------------------------------------------------------------------------------------*/
/* cy_rtos_init_event                                                                               */
/*--------------------------------------------------------------------------------------------------*/
cy_rslt_t cy_rtos_init_event(cy_event_t *event)
{
  if (event == NULL)
  {
    return CY_RTOS_BAD_PARAM;
  }

  return convert_error(tx_event_flags_create(event, TX_NULL));
}


/*--------------------------------------------------------------------------------------------------*/
/* cy_rtos_setbits_event                                                                            */
/*--------------------------------------------------------------------------------------------------*/
cy_rslt_t cy_rtos_setbits_event(cy_event_t *event, uint32_t bits, bool in_isr)
{
  (void)in_isr;

  if (event == NULL)
  {
    return CY_RTOS_BAD_PARAM;
  }

  return convert_error(tx_event_flags_set(event, bits, TX_OR));
}


/*--------------------------------------------------------------------------------------------------*/
/* cy_rtos_clearbits_event                                                                          */
/*--------------------------------------------------------------------------------------------------*/
cy_rslt_t cy_rtos_clearbits_event(cy_event_t *event, uint32_t bits, bool in_isr)
{
  (void)in_isr;

  if (event == NULL)
  {
    return CY_RTOS_BAD_PARAM;
  }

  return convert_error(tx_event_flags_set(event, ~bits, TX_AND));
}


/*--------------------------------------------------------------------------------------------------*/
/* cy_rtos_getbits_event                                                                            */
/*--------------------------------------------------------------------------------------------------*/
cy_rslt_t cy_rtos_getbits_event(cy_event_t *event, uint32_t *bits)
{
  if ((event == NULL) || (bits == NULL))
  {
    return CY_RTOS_BAD_PARAM;
  }

  cy_rtos_error_t tx_rslt = tx_event_flags_get(event, ALL_EVENT_FLAGS, TX_OR, (ULONG *)bits, TX_NO_WAIT);
  if (TX_NO_EVENTS == tx_rslt) /* If timeout error occur with ALL_EVENT_FLAGS and TX_OR, then no flag is set */
  {
    *bits = 0;
    return CY_RSLT_SUCCESS;
  }
  else
  {
    return convert_error(tx_rslt);
  }
}


/*--------------------------------------------------------------------------------------------------*/
/* cy_rtos_waitbits_event                                                                           */
/*--------------------------------------------------------------------------------------------------*/
cy_rslt_t cy_rtos_waitbits_event(cy_event_t *event, uint32_t *bits, bool clear, bool all,
                                 cy_time_t timeout_ms)
{
  UINT get_option;

  if ((event == NULL) || (bits == NULL))
  {
    return CY_RTOS_BAD_PARAM;
  }

  if (all)
  {
    get_option = clear ? TX_AND_CLEAR : TX_AND;
  }
  else
  {
    get_option = clear ? TX_OR_CLEAR : TX_OR;
  }

  cy_rtos_error_t tx_rslt =
    tx_event_flags_get(event, *bits, get_option, (ULONG *)bits, convert_ms_to_ticks(timeout_ms));
  if (TX_NO_EVENTS == tx_rslt)
  {
    return CY_RTOS_TIMEOUT;
  }
  else
  {
    return convert_error(tx_rslt);
  }
}


/*--------------------------------------------------------------------------------------------------*/
/* cy_rtos_deinit_event                                                                             */
/*--------------------------------------------------------------------------------------------------*/
cy_rslt_t cy_rtos_deinit_event(cy_event_t *event)
{
  if (event == NULL)
  {
    return CY_RTOS_BAD_PARAM;
  }

  return convert_error(tx_event_flags_delete(event));
}


/**
  ***************************************************
  *                 Queues
  ***************************************************
  */

/*--------------------------------------------------------------------------------------------------*/
/* cy_rtos_init_queue                                                                               */
/*--------------------------------------------------------------------------------------------------*/
cy_rslt_t cy_rtos_init_queue(cy_queue_t *queue, size_t length, size_t itemsize)
{
  /* Valid message lengths are {1-ULONG, 2-ULONG, 4-ULONG, 8-ULONG, 16-ULONG} */
  static const uint32_t BYTES_PER_QUEUE_WORD = sizeof(ULONG);

  if ((queue == NULL) || (itemsize == 0) ||
      (itemsize > BYTES_PER_QUEUE_WORD * MAX_QUEUE_MESSAGE_SIZE))
  {
    return CY_RTOS_BAD_PARAM;
  }

  /* round message words to next power of 2 times word size. */
  UINT message_words = 1;
  while (message_words * BYTES_PER_QUEUE_WORD < itemsize)
  {
    message_words <<= 1;
  }

  queue->itemsize = itemsize;
  ULONG queue_size = length * message_words * BYTES_PER_QUEUE_WORD;
  queue->mem = cypress_whd_malloc(queue_size);

  if (queue->mem == NULL)
  {
    return CY_RTOS_NO_MEMORY;
  }

  cy_rtos_error_t tx_rslt = tx_queue_create(&(queue->tx_queue), TX_NULL, message_words,
                                            queue->mem, queue_size);
  if (TX_SUCCESS != tx_rslt)
  {
    last_error = tx_rslt;
    cypress_whd_free(queue->mem);
    return CY_RTOS_GENERAL_ERROR;
  }

  return CY_RSLT_SUCCESS;
}


/*--------------------------------------------------------------------------------------------------*/
/* cy_rtos_put_queue                                                                                */
/*--------------------------------------------------------------------------------------------------*/
cy_rslt_t cy_rtos_put_queue(cy_queue_t *queue, const void *item_ptr, cy_time_t timeout_ms,
                            bool in_isr)
{
  if ((queue == NULL) || (item_ptr == NULL) || (in_isr && (timeout_ms != 0)))
  {
    return CY_RTOS_BAD_PARAM;
  }

  cy_rtos_error_t tx_rslt = tx_queue_send(&(queue->tx_queue), (void *)item_ptr, convert_ms_to_ticks(
                                            timeout_ms));
  if (TX_QUEUE_FULL == tx_rslt)
  {
    return CY_RTOS_NO_MEMORY;
  }
  else
  {
    return convert_error(tx_rslt);
  }
}


/*--------------------------------------------------------------------------------------------------*/
/* cy_rtos_get_queue                                                                                */
/*--------------------------------------------------------------------------------------------------*/
cy_rslt_t cy_rtos_get_queue(cy_queue_t *queue, void *item_ptr, cy_time_t timeout_ms, bool in_isr)
{
  ULONG buffer[MAX_QUEUE_MESSAGE_SIZE];
  if ((queue == NULL) || (item_ptr == NULL) || (in_isr && (timeout_ms != 0)))
  {
    return CY_RTOS_BAD_PARAM;
  }

  cy_rtos_error_t tx_rslt =
    tx_queue_receive(&(queue->tx_queue), buffer, convert_ms_to_ticks(timeout_ms));
  if (TX_QUEUE_EMPTY == tx_rslt)
  {
    return CY_RTOS_TIMEOUT;
  }
  else if (tx_rslt == TX_SUCCESS)
  {
    memcpy(item_ptr, buffer, queue->itemsize);
    return CY_RSLT_SUCCESS;
  }
  else
  {
    last_error = tx_rslt;
    return CY_RTOS_GENERAL_ERROR;
  }
}


/*--------------------------------------------------------------------------------------------------*/
/* cy_rtos_count_queue                                                                              */
/*--------------------------------------------------------------------------------------------------*/
cy_rslt_t cy_rtos_count_queue(cy_queue_t *queue, size_t *num_waiting)
{
  if ((queue == NULL) || (num_waiting == NULL))
  {
    return CY_RTOS_BAD_PARAM;
  }
  return convert_error(tx_queue_info_get(&(queue->tx_queue), TX_NULL, (ULONG *)num_waiting,
                                         TX_NULL, TX_NULL, TX_NULL, TX_NULL));
}


/*--------------------------------------------------------------------------------------------------*/
/* cy_rtos_space_queue                                                                              */
/*--------------------------------------------------------------------------------------------------*/
cy_rslt_t cy_rtos_space_queue(cy_queue_t *queue, size_t *num_spaces)
{
  if ((queue == NULL) || (num_spaces == NULL))
  {
    return CY_RTOS_BAD_PARAM;
  }
  return convert_error(tx_queue_info_get(&(queue->tx_queue), TX_NULL, TX_NULL, (ULONG *)num_spaces,
                                         TX_NULL, TX_NULL, TX_NULL));
}


/*--------------------------------------------------------------------------------------------------*/
/* cy_rtos_reset_queue                                                                              */
/*--------------------------------------------------------------------------------------------------*/
cy_rslt_t cy_rtos_reset_queue(cy_queue_t *queue)
{
  if (queue == NULL)
  {
    return CY_RTOS_BAD_PARAM;
  }
  return convert_error(tx_queue_flush(&(queue->tx_queue)));
}


/*--------------------------------------------------------------------------------------------------*/
/* cy_rtos_deinit_queue                                                                             */
/*--------------------------------------------------------------------------------------------------*/
cy_rslt_t cy_rtos_deinit_queue(cy_queue_t *queue)
{
  if (queue == NULL)
  {
    return CY_RTOS_BAD_PARAM;
  }
  cy_rslt_t result = convert_error(tx_queue_delete(&(queue->tx_queue)));
  if (result == CY_RSLT_SUCCESS)
  {
    cypress_whd_free(queue->mem);
  }
  return result;
}


/**
  *****************************************************
  *                 Timers
  *****************************************************
  */

/*--------------------------------------------------------------------------------------------------*/
/* cy_rtos_init_timer                                                                               */
/*--------------------------------------------------------------------------------------------------*/
cy_rslt_t cy_rtos_init_timer(cy_timer_t *timer, cy_timer_trigger_type_t type,
                             cy_timer_callback_t fun, cy_timer_callback_arg_t arg)
{
  if ((timer == NULL) || (fun == NULL))
  {
    return CY_RTOS_BAD_PARAM;
  }
  timer->oneshot = (type == CY_TIMER_TYPE_ONCE);
  /* Use 1s here as default timeouts since these are going to get changed anyway */
  return convert_error(tx_timer_create(&(timer->tx_timer), TX_NULL, fun, arg, 1, 1,
                                       TX_NO_ACTIVATE));
}


/*--------------------------------------------------------------------------------------------------*/
/* cy_rtos_start_timer                                                                              */
/*--------------------------------------------------------------------------------------------------*/
cy_rslt_t cy_rtos_start_timer(cy_timer_t *timer, cy_time_t num_ms)
{
  if (timer == NULL)
  {
    return CY_RTOS_BAD_PARAM;
  }
  ULONG timer_ticks = convert_ms_to_ticks(num_ms);
  cy_rslt_t rslt =
    convert_error(tx_timer_change(&(timer->tx_timer), timer_ticks,
                                  timer->oneshot ? 0 : timer_ticks));
  if (CY_RSLT_SUCCESS != rslt)
  {
    return rslt;
  }
  return convert_error(tx_timer_activate(&(timer->tx_timer)));
}


/*--------------------------------------------------------------------------------------------------*/
/* cy_rtos_stop_timer                                                                               */
/*--------------------------------------------------------------------------------------------------*/
cy_rslt_t cy_rtos_stop_timer(cy_timer_t *timer)
{
  if (timer == NULL)
  {
    return CY_RTOS_BAD_PARAM;
  }
  return convert_error(tx_timer_deactivate(&(timer->tx_timer)));
}


/*--------------------------------------------------------------------------------------------------*/
/* cy_rtos_is_running_timer                                                                         */
/*--------------------------------------------------------------------------------------------------*/
cy_rslt_t cy_rtos_is_running_timer(cy_timer_t *timer, bool *state)
{
  if ((timer == NULL) || (state == NULL))
  {
    return CY_RTOS_BAD_PARAM;
  }

  UINT active;
  cy_rslt_t rslt =
    convert_error(tx_timer_info_get(&(timer->tx_timer), TX_NULL, &active, TX_NULL, TX_NULL,
                                    TX_NULL));
  if (CY_RSLT_SUCCESS == rslt)
  {
    *state = (active == TX_TRUE);
  }
  return rslt;
}


/*--------------------------------------------------------------------------------------------------*/
/* cy_rtos_deinit_timer                                                                             */
/*--------------------------------------------------------------------------------------------------*/
cy_rslt_t cy_rtos_deinit_timer(cy_timer_t *timer)
{
  if (timer == NULL)
  {
    return CY_RTOS_BAD_PARAM;
  }
  return convert_error(tx_timer_delete(&(timer->tx_timer)));
}


/**
  ****************************************************
  *                 Time
  ****************************************************
  */

/*--------------------------------------------------------------------------------------------------*/
/* cy_rtos_get_time                                                                                 */
/*--------------------------------------------------------------------------------------------------*/
cy_rslt_t cy_rtos_get_time(cy_time_t *tval)
{
  cy_rslt_t status = CY_RSLT_SUCCESS;

  /* Use HAL time based on the 1 ms interrupt.*/
  const uint32_t milliseconds = HAL_GetTick();

  if (milliseconds != 0)
  {
    *tval = milliseconds;
  }
  else
    status = CY_RTOS_GENERAL_ERROR;

  return status;
}


/*--------------------------------------------------------------------------------------------------*/
/* cy_rtos_delay_milliseconds                                                                       */
/*--------------------------------------------------------------------------------------------------*/
cy_rslt_t cy_rtos_delay_milliseconds(cy_time_t num_ms)
{
  cy_time_t ticks = convert_ms_to_ticks(num_ms);

  return convert_error(tx_thread_sleep(ticks));
}


UINT cypress_whd_alloc_init(VOID)
{
  if (tx_byte_pool_create(&CypressWhdBytePool, "CYPRESS WHD byte pool",
                          cypress_whd_byte_pool_buffer, CYPRESS_WHD_BYTE_POOL_SIZE))
  {
    return NX_DRIVER_ERROR;
  }

  return NX_SUCCESS;
}


void *cypress_whd_malloc(size_t size)
{
  UINT status;
  void *p = NULL;

  /* printf("%s()> %"PRIu32"\n", __FUNCTION__, (uint32_t)size); */
  status = tx_byte_allocate(&CypressWhdBytePool, &p, size, TX_NO_WAIT);

  if (status != TX_SUCCESS)
  {
    printf("tx_byte_allocate() failed in cypress_whd_malloc()\n");
    return NULL;
  }

  CYPRESS_WHD_ASSERT(p);

  return p;
}


void cypress_whd_free(void *p)
{
  UINT status;

  CYPRESS_WHD_ASSERT(p);

  status = tx_byte_release(p);

  CYPRESS_WHD_ASSERT(status == NX_SUCCESS);
}


void *cypress_whd_calloc(size_t n, size_t m)
{
  UINT status;
  const size_t size  = n * m;
  void *p = NULL;

  /* printf("%s()> %"PRIu32"\n", __FUNCTION__, (uint32_t)size); */

  status = tx_byte_allocate(&CypressWhdBytePool, &p, size, TX_NO_WAIT);

  if (status != TX_SUCCESS)
  {
    return NULL;
  }

  if (p != NULL)
  {
    (void) memset(p, 0, size);
  }
  return p;
}


#if defined(__cplusplus)
}
#endif
