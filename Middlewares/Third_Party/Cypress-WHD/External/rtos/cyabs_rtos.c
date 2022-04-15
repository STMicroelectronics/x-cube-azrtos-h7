/*********************************************************************************/
/**
  * \file cyabs_rtos.c
  *
  * \brief
  * Implementation for CMSIS RTOS v2 abstraction
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

#include <stdlib.h>
#include <inttypes.h>
#include <assert.h>
#include "whd_config.h"
#include "cyabs_rtos.h"
#include "cmsis_compiler.h"

#include "FreeRTOS.h"
#include "task.h"
#include "event_groups.h"
#include "semphr.h"



#if defined(__cplusplus)
extern "C" {
#endif

#define osThreadCbSize           sizeof(StaticTask_t)
#define osMutexCbSize            sizeof(StaticSemaphore_t)
#define osSemaphoreCbSize        sizeof(StaticSemaphore_t)
#define osTimerCbSize            sizeof(StaticTimer_t)

#define osEventFlagsCbSize       sizeof(StaticEventGroup_t)
#define osMessageQueueCbSize     sizeof(StaticQueue_t)
/**
  ****************************************************
  *                 Error Converter
  ****************************************************
  */

/* Last received error status */
static cy_rtos_error_t dbgErr;

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
  return dbgErr;
}

/* Converts internal error type to external error type */
static cy_rslt_t error_converter(cy_rtos_error_t internalError)
{
  cy_rslt_t value;

  switch (internalError)
  {
    case osOK:
      value = CY_RSLT_SUCCESS;
      break;

    case osErrorTimeout:
      value = CY_RTOS_TIMEOUT;
      break;

    case osErrorParameter:
      value = CY_RTOS_BAD_PARAM;
      break;

    case osErrorNoMemory:
      value = CY_RTOS_NO_MEMORY;
      break;

    case osError:
    case osErrorResource:
    case osErrorISR:
    default:
      value = CY_RTOS_GENERAL_ERROR;
      break;
  }

  /* Update the last known error status */
  dbgErr = internalError;
  return value;
}

#if 0
/*--------------------------------------------------------------------------------------------------*/
/* convert_ms_to_ticks                                                                              */
/*--------------------------------------------------------------------------------------------------*/
static uint32_t convert_ms_to_ticks(cy_time_t timeout_ms)
{
  if (timeout_ms == CY_RTOS_NEVER_TIMEOUT)
  {
    return osWaitForever;
  }
  else if (timeout_ms == 0)
  {
    return 0;
  }
  else
  {
    /* Get number of ticks per second */
    uint32_t tick_freq = osKernelGetTickFreq();
    uint32_t ticks = (uint32_t)(((uint64_t)timeout_ms * tick_freq) / 1000);

    if (ticks == 0)
    {
      ticks = 1;
    }
    else if (ticks >= UINT32_MAX)
    {
      /* if ticks if more than 32 bits, change ticks to max possible value that isn't */
      /* osWaitForever.                                                               */
      ticks = UINT32_MAX - 1;
    }
    return ticks;
  }
}
#endif /* 0 */

/**
  *****************************************************
  *                 Threads
  *****************************************************
  */

/*--------------------------------------------------------------------------------------------------*/
/* cy_rtos_create_thread                                                                            */
/*--------------------------------------------------------------------------------------------------*/
cy_rslt_t cy_rtos_create_thread(cy_thread_t *thread, cy_thread_entry_fn_t entry_function,
                                const char *name, void *stack, uint32_t stack_size,
                                cy_thread_priority_t priority, cy_thread_arg_t arg)
{
  cy_rslt_t      status = CY_RSLT_SUCCESS;
  osThreadAttr_t attr;

  if ((thread == NULL) || (stack_size < CY_RTOS_MIN_STACK_SIZE))
  {
    status = CY_RTOS_BAD_PARAM;
  }
  else if ((stack != NULL) && (0 != (((uint32_t)stack) & CY_RTOS_ALIGNMENT_MASK)))
  {
    status = CY_RTOS_ALIGNMENT_ERROR;
  }
  else
  {
    attr.name       = name;
    attr.attr_bits  = 0;  /* Not supported in FreeRTOS ? osThreadJoinable; */
    attr.cb_mem     = NULL;
    attr.cb_size    = osThreadCbSize;
    attr.stack_size = stack_size;
    attr.priority   = (osPriority_t)priority;
    attr.tz_module  = 0;
    attr.reserved   = 0;

    /* Allocate stack if NULL was passed */
    if ((uint32_t *)stack == NULL)
    {
      /* Note: 1 malloc so that it can be freed with 1 call when terminating */
      uint32_t cb_mem_pad = (~osThreadCbSize + 1) & CY_RTOS_ALIGNMENT_MASK;
      attr.cb_mem = WHD_MALLOC(osThreadCbSize + cb_mem_pad + stack_size);

      if (attr.cb_mem != NULL)
      {
        attr.stack_mem = (uint32_t *)((uint32_t)attr.cb_mem + osThreadCbSize + cb_mem_pad);
      }
    }
    else
    {
      attr.cb_mem    = WHD_MALLOC(osThreadCbSize);
      attr.stack_mem = stack;
    }

    if (attr.cb_mem == NULL)
    {
      status = CY_RTOS_NO_MEMORY;
    }
    else
    {
      assert(((uint32_t)attr.cb_mem & CY_RTOS_ALIGNMENT_MASK) == 0UL);
      assert(((uint32_t)attr.stack_mem & CY_RTOS_ALIGNMENT_MASK) == 0UL);
      *thread = osThreadNew((osThreadFunc_t)entry_function, (void *)arg, &attr);
      assert((*thread == attr.cb_mem) || (*thread == NULL));
      status = (*thread == NULL) ? CY_RTOS_GENERAL_ERROR : CY_RSLT_SUCCESS;
    }
  }

  return status;
}


/*--------------------------------------------------------------------------------------------------*/
/* cy_rtos_exit_thread                                                                              */
/*--------------------------------------------------------------------------------------------------*/
cy_rslt_t cy_rtos_exit_thread(void)
{
  osThreadExit();
}


/*--------------------------------------------------------------------------------------------------*/
/* cy_rtos_terminate_thread                                                                         */
/*--------------------------------------------------------------------------------------------------*/
cy_rslt_t cy_rtos_terminate_thread(cy_thread_t *thread)
{
  cy_rslt_t       status;
  cy_rtos_error_t statusInternal;

  if (thread == NULL)
  {
    status = CY_RTOS_BAD_PARAM;
  }
  else
  {
    statusInternal = osThreadTerminate(*thread);
    status         = error_converter(statusInternal);
  }

  return status;
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

  *running = (osThreadGetState(*thread) == osThreadRunning) ? true : false;

  return CY_RSLT_SUCCESS;
}


/*--------------------------------------------------------------------------------------------------*/
/* cy_rtos_get_thread_state                                                                         */
/*--------------------------------------------------------------------------------------------------*/
cy_rslt_t cy_rtos_get_thread_state(cy_thread_t *thread, cy_thread_state_t *state)
{
  cy_rslt_t status = CY_RSLT_SUCCESS;

  if ((thread == NULL) || (state == NULL))
  {
    status = CY_RTOS_BAD_PARAM;
  }
  else
  {
    switch (osThreadGetState(*thread))
    {
      case osThreadInactive:
        *state = CY_THREAD_STATE_INACTIVE;
        break;

      case osThreadReady:
        *state = CY_THREAD_STATE_READY;
        break;

      case osThreadRunning:
        *state = CY_THREAD_STATE_RUNNING;
        break;

      case osThreadBlocked:
        *state = CY_THREAD_STATE_BLOCKED;
        break;

      case osThreadTerminated:
        *state = CY_THREAD_STATE_TERMINATED;
        break;

      case osThreadError:
      case osThreadReserved:
      default:
        *state = CY_THREAD_STATE_UNKNOWN;
        break;
    }
  }

  return status;
}


/*--------------------------------------------------------------------------------------------------*/
/* cy_rtos_join_thread                                                                              */
/*--------------------------------------------------------------------------------------------------*/
cy_rslt_t cy_rtos_join_thread(cy_thread_t *thread)
{
  cy_rslt_t       status = CY_RSLT_SUCCESS;

  if (thread == NULL)
  {
    status = CY_RTOS_BAD_PARAM;
  }
  else
  {
    if (status == CY_RSLT_SUCCESS)
    {
      WHD_FREE(*thread);
      *thread = NULL;
    }
  }

  return status;
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
  cy_rslt_t     status;
  osMutexAttr_t attr;

  if (mutex == NULL)
  {
    status = CY_RTOS_BAD_PARAM;
  }
  else
  {
    attr.name      = "cy_rtos_mutex2";
    attr.attr_bits = osMutexRecursive | osMutexPrioInherit;
    attr.cb_mem    = WHD_MALLOC(osMutexCbSize);
    attr.cb_size   = osMutexCbSize;

    if (attr.cb_mem == NULL)
    {
      status = CY_RTOS_NO_MEMORY;
    }
    else
    {
      assert(((uint32_t)attr.cb_mem & CY_RTOS_ALIGNMENT_MASK) == 0UL);
      *mutex = osMutexNew(&attr);
      assert((*mutex == attr.cb_mem) || (*mutex == NULL));
      status = (*mutex == NULL) ? CY_RTOS_GENERAL_ERROR : CY_RSLT_SUCCESS;
    }
  }

  return status;
}


/*--------------------------------------------------------------------------------------------------*/
/* cy_rtos_get_mutex                                                                                */
/*--------------------------------------------------------------------------------------------------*/
cy_rslt_t cy_rtos_get_mutex(cy_mutex_t *mutex, cy_time_t timeout_ms)
{
  cy_rslt_t       status;
  cy_rtos_error_t statusInternal;

  if (mutex == NULL)
  {
    status = CY_RTOS_BAD_PARAM;
  }
  else
  {
    statusInternal = osMutexAcquire(*mutex, timeout_ms);
    status         = error_converter(statusInternal);
  }

  return status;
}


/*--------------------------------------------------------------------------------------------------*/
/* cy_rtos_set_mutex                                                                                */
/*--------------------------------------------------------------------------------------------------*/
cy_rslt_t cy_rtos_set_mutex(cy_mutex_t *mutex)
{
  cy_rslt_t       status;
  cy_rtos_error_t statusInternal;

  if (mutex == NULL)
  {
    status = CY_RTOS_BAD_PARAM;
  }
  else
  {
    statusInternal = osMutexRelease(*mutex);
    status         = error_converter(statusInternal);
  }

  return status;
}


/*--------------------------------------------------------------------------------------------------*/
/* cy_rtos_deinit_mutex                                                                             */
/*--------------------------------------------------------------------------------------------------*/
cy_rslt_t cy_rtos_deinit_mutex(cy_mutex_t *mutex)
{
  cy_rslt_t       status;
  cy_rtos_error_t statusInternal;

  if (mutex == NULL)
  {
    status = CY_RTOS_BAD_PARAM;
  }
  else
  {
    statusInternal = osMutexDelete(*mutex);
    status         = error_converter(statusInternal);

    if (status == CY_RSLT_SUCCESS)
    {
      WHD_FREE(*mutex);
      *mutex = NULL;
    }
  }

  return status;
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
  cy_rslt_t         status;
  osSemaphoreAttr_t attr;

  if (semaphore == NULL)
  {
    status = CY_RTOS_BAD_PARAM;
  }
  else
  {
    attr.name = NULL;
    attr.attr_bits = 0U;
    attr.cb_size = osSemaphoreCbSize;
    attr.cb_mem = WHD_MALLOC(osSemaphoreCbSize);

    if (attr.cb_mem == NULL)
    {
      status = CY_RTOS_NO_MEMORY;
    }
    else
    {
      assert(((uint32_t)attr.cb_mem & CY_RTOS_ALIGNMENT_MASK) == 0UL);
      *semaphore = osSemaphoreNew(maxcount, initcount, &attr);
      assert((*semaphore == attr.cb_mem) || (*semaphore == NULL));
      status = (*semaphore == NULL) ? CY_RTOS_GENERAL_ERROR : CY_RSLT_SUCCESS;
    }
  }

  return status;
}


/*--------------------------------------------------------------------------------------------------*/
/* cy_rtos_get_semaphore                                                                            */
/*--------------------------------------------------------------------------------------------------*/
cy_rslt_t cy_rtos_get_semaphore(cy_semaphore_t *semaphore, cy_time_t timeout_ms, bool in_isr)
{
  cy_rslt_t       status = CY_RSLT_SUCCESS;
  cy_rtos_error_t statusInternal;

  /* Based on documentation when osSemaphoreAcquire is called from ISR timeout must be zero.                                  */
  /* https://www.keil.com/pack/doc/CMSIS/RTOS2/html/group__CMSIS__RTOS__SemaphoreMgmt.html#ga7e94c8b242a0c81f2cc79ec22895c87b */
  if ((semaphore == NULL) || (in_isr && (timeout_ms != 0)))
  {
    status = CY_RTOS_BAD_PARAM;
  }
  else
  {
    statusInternal = osSemaphoreAcquire(*semaphore, timeout_ms);
    status = error_converter(statusInternal);
  }

  return status;
}


/*--------------------------------------------------------------------------------------------------*/
/* cy_rtos_set_semaphore                                                                            */
/*--------------------------------------------------------------------------------------------------*/
cy_rslt_t cy_rtos_set_semaphore(cy_semaphore_t *semaphore, bool in_isr)
{
  cy_rslt_t       status = CY_RSLT_SUCCESS;
  cy_rtos_error_t statusInternal;
  (void)in_isr;

  if (semaphore == NULL)
  {
    status = CY_RTOS_BAD_PARAM;
  }
  else
  {
    statusInternal = osSemaphoreRelease(*semaphore);
    status         = error_converter(statusInternal);
  }

  return status;
}


/*--------------------------------------------------------------------------------------------------*/
/* cy_rtos_get_count_semaphore                                                                      */
/*--------------------------------------------------------------------------------------------------*/



/*--------------------------------------------------------------------------------------------------*/
/* cy_rtos_deinit_semaphore                                                                         */
/*--------------------------------------------------------------------------------------------------*/
cy_rslt_t cy_rtos_deinit_semaphore(cy_semaphore_t *semaphore)
{
  cy_rslt_t       status;
  cy_rtos_error_t statusInternal;

  if (semaphore == NULL)
  {
    status = CY_RTOS_BAD_PARAM;
  }
  else
  {
    statusInternal = osSemaphoreDelete(*semaphore);
    status         = error_converter(statusInternal);
    if (status == CY_RSLT_SUCCESS)
    {
      WHD_FREE(*semaphore);
      *semaphore = NULL;
    }
  }

  return status;
}


/**
  ****************************************************
  *                 Events
  ****************************************************
  */

#define CY_RTOS_EVENT_ERRORFLAG     0x80000000UL
#define CY_RTOS_EVENT_FLAGS         0x7FFFFFFFUL

/*--------------------------------------------------------------------------------------------------*/
/* cy_rtos_init_event                                                                               */
/*--------------------------------------------------------------------------------------------------*/
cy_rslt_t cy_rtos_init_event(cy_event_t *event)
{
  cy_rslt_t          status;
  osEventFlagsAttr_t attr;

  if (event == NULL)
  {
    status = CY_RTOS_BAD_PARAM;
  }
  else
  {
    attr.name      = NULL;
    attr.attr_bits = 0U;
    attr.cb_mem    = WHD_MALLOC(osEventFlagsCbSize);
    attr.cb_size   = osEventFlagsCbSize;

    if (attr.cb_mem == NULL)
    {
      status = CY_RTOS_NO_MEMORY;
    }
    else
    {
      assert(((uint32_t)attr.cb_mem & CY_RTOS_ALIGNMENT_MASK) == 0UL);
      *event = osEventFlagsNew(&attr);
      assert((*event == attr.cb_mem) || (*event == NULL));
      status = (*event == NULL) ? CY_RTOS_GENERAL_ERROR : CY_RSLT_SUCCESS;
    }
  }

  return status;
}


/*--------------------------------------------------------------------------------------------------*/
/* cy_rtos_setbits_event                                                                            */
/*--------------------------------------------------------------------------------------------------*/
cy_rslt_t cy_rtos_setbits_event(cy_event_t *event, uint32_t bits, bool in_isr)
{
  cy_rslt_t status = CY_RSLT_SUCCESS;
  cy_rtos_error_t statusInternal;
  (void)in_isr;

  if (event == NULL)
  {
    status = CY_RTOS_BAD_PARAM;
  }
  else
  {
    statusInternal = (osStatus_t)osEventFlagsSet(*event, bits);
    if ((statusInternal & CY_RTOS_EVENT_ERRORFLAG) != 0UL)
    {
      status = error_converter(statusInternal);
    }
  }

  return status;
}


/*--------------------------------------------------------------------------------------------------*/
/* cy_rtos_clearbits_event                                                                          */
/*--------------------------------------------------------------------------------------------------*/
cy_rslt_t cy_rtos_clearbits_event(cy_event_t *event, uint32_t bits, bool in_isr)
{
  cy_rslt_t status = CY_RSLT_SUCCESS;
  cy_rtos_error_t statusInternal;
  (void)in_isr;

  if (event == NULL)
  {
    status = CY_RTOS_BAD_PARAM;
  }
  else
  {
    statusInternal = (osStatus_t)osEventFlagsClear(*event, bits);
    if ((statusInternal & CY_RTOS_EVENT_ERRORFLAG) != 0UL)
    {
      status = error_converter(statusInternal);
    }
  }

  return status;
}


/*--------------------------------------------------------------------------------------------------*/
/* cy_rtos_getbits_event                                                                            */
/*--------------------------------------------------------------------------------------------------*/
cy_rslt_t cy_rtos_getbits_event(cy_event_t *event, uint32_t *bits)
{
  cy_rslt_t status = CY_RSLT_SUCCESS;

  if ((event == NULL) || (bits == NULL))
  {
    status = CY_RTOS_BAD_PARAM;
  }
  else
  {
    *bits = osEventFlagsGet(*event);
  }

  return status;
}


/*--------------------------------------------------------------------------------------------------*/
/* cy_rtos_waitbits_event                                                                           */
/*--------------------------------------------------------------------------------------------------*/
cy_rslt_t cy_rtos_waitbits_event(cy_event_t *event, uint32_t *bits, bool clear, bool all,
                                 cy_time_t timeout)
{
  cy_rslt_t       status = CY_RSLT_SUCCESS;
  cy_rtos_error_t statusInternal;
  uint32_t        flagOption;

  if ((event == NULL) || (bits == NULL))
  {
    status = CY_RTOS_BAD_PARAM;
  }
  else
  {
    flagOption = (all) ? osFlagsWaitAll : osFlagsWaitAny;
    if (!clear)
    {
      flagOption |= osFlagsNoClear;
    }

    statusInternal = (osStatus_t)osEventFlagsWait(*event, *bits, flagOption, timeout);
    if ((statusInternal & CY_RTOS_EVENT_ERRORFLAG) == 0UL)
    {
      *bits = statusInternal;
    }
    else
    {
      status = error_converter(statusInternal);
    }
  }

  return status;
}


/*--------------------------------------------------------------------------------------------------*/
/* cy_rtos_deinit_event                                                                             */
/*--------------------------------------------------------------------------------------------------*/
cy_rslt_t cy_rtos_deinit_event(cy_event_t *event)
{
  cy_rslt_t       status;
  cy_rtos_error_t statusInternal;

  if (event == NULL)
  {
    status = CY_RTOS_BAD_PARAM;
  }
  else
  {
    statusInternal = osEventFlagsDelete(*event);
    status         = error_converter(statusInternal);
    if (status == CY_RSLT_SUCCESS)
    {
      WHD_FREE(*event);
      *event = NULL;
    }
  }

  return status;
}



/**
  ***************************************************
  *                 Queues
  ***************************************************
  */

cy_rslt_t cy_rtos_init_queue(cy_queue_t *queue, size_t length, size_t itemsize)
{
  cy_rslt_t            status;
  osMessageQueueAttr_t attr;

  if (queue == NULL)
  {
    status = CY_RTOS_BAD_PARAM;
  }
  else
  {
    attr.name = NULL;
    attr.attr_bits = 0U;
    attr.cb_size = osMessageQueueCbSize;
    uint32_t blockSize = ((itemsize + 3U) & ~3UL) + osMessageQueueCbSize;
    attr.mq_size = blockSize * length;

    /* Note: 1 malloc for both so that they can be freed with 1 call */
    uint32_t cb_mem_pad = (8 - (osMessageQueueCbSize & 0x07)) & 0x07;
    attr.cb_mem = WHD_MALLOC(osMessageQueueCbSize + cb_mem_pad + attr.mq_size);
    if (attr.cb_mem != NULL)
    {
      attr.mq_mem = (uint32_t *)((uint32_t)attr.cb_mem + osMessageQueueCbSize + cb_mem_pad);
    }

    if (attr.cb_mem == NULL)
    {
      status = CY_RTOS_NO_MEMORY;
    }
    else
    {
      assert(((uint32_t)attr.cb_mem & CY_RTOS_ALIGNMENT_MASK) == 0UL);
      assert(((uint32_t)attr.mq_mem & CY_RTOS_ALIGNMENT_MASK) == 0UL);
      *queue = osMessageQueueNew(length, itemsize, &attr);
      assert((*queue == attr.cb_mem) || (*queue == NULL));
      status = (*queue == NULL) ? CY_RTOS_GENERAL_ERROR : CY_RSLT_SUCCESS;
    }
  }

  return status;
}


/*--------------------------------------------------------------------------------------------------*/
/* cy_rtos_put_queue                                                                                */
/*--------------------------------------------------------------------------------------------------*/
cy_rslt_t cy_rtos_put_queue(cy_queue_t *queue, const void *item_ptr, cy_time_t timeout_ms,
                            bool in_isr)
{
  cy_rslt_t       status;
  cy_rtos_error_t statusInternal;

  if ((queue == NULL) || (item_ptr == NULL))
  {
    status = CY_RTOS_BAD_PARAM;
  }
  else
  {
    /* Not allowed to be called in ISR if timeout != 0 */
    if ((!in_isr) || (in_isr && (timeout_ms == 0U)))
    {
      statusInternal = osMessageQueuePut(*queue, (uint8_t *)item_ptr, 0u, timeout_ms);
    }
    else
    {
      statusInternal = osErrorISR;
    }

    status = error_converter(statusInternal);
  }

  return status;
}


/*--------------------------------------------------------------------------------------------------*/
/* cy_rtos_get_queue                                                                                */
/*--------------------------------------------------------------------------------------------------*/
cy_rslt_t cy_rtos_get_queue(cy_queue_t *queue, void *item_ptr, cy_time_t timeout_ms, bool in_isr)
{
  cy_rslt_t       status;
  cy_rtos_error_t statusInternal;

  if ((queue == NULL) || (item_ptr == NULL))
  {
    status = CY_RTOS_BAD_PARAM;
  }
  else
  {
    /* Not allowed to be called in ISR if timeout != 0 */
    if ((!in_isr) || (in_isr && (timeout_ms == 0U)))
    {
      statusInternal = osMessageQueueGet(*queue, (uint8_t *)item_ptr, 0u, timeout_ms);
    }
    else
    {
      statusInternal = osErrorISR;
    }

    status = error_converter(statusInternal);
  }

  return status;
}


/*--------------------------------------------------------------------------------------------------*/
/* cy_rtos_count_queue                                                                              */
/*--------------------------------------------------------------------------------------------------*/
cy_rslt_t cy_rtos_count_queue(cy_queue_t *queue, size_t *num_waiting)
{
  cy_rslt_t status = CY_RSLT_SUCCESS;

  if ((queue == NULL) || (num_waiting == NULL))
  {
    status = CY_RTOS_BAD_PARAM;
  }
  else
  {
    *num_waiting = osMessageQueueGetCount(*queue);
  }

  return status;
}


/*--------------------------------------------------------------------------------------------------*/
/* cy_rtos_space_queue                                                                              */
/*--------------------------------------------------------------------------------------------------*/
cy_rslt_t cy_rtos_space_queue(cy_queue_t *queue, size_t *num_spaces)
{
  cy_rslt_t status = CY_RSLT_SUCCESS;

  if ((queue == NULL) || (num_spaces == NULL))
  {
    status = CY_RTOS_BAD_PARAM;
  }
  else
  {
    *num_spaces = osMessageQueueGetSpace(*queue);
  }

  return status;
}


/*--------------------------------------------------------------------------------------------------*/
/* cy_rtos_reset_queue                                                                              */
/*--------------------------------------------------------------------------------------------------*/
cy_rslt_t cy_rtos_reset_queue(cy_queue_t *queue)
{
  cy_rslt_t       status;
  cy_rtos_error_t statusInternal;

  if (queue == NULL)
  {
    status = CY_RTOS_BAD_PARAM;
  }
  else
  {
    statusInternal = osMessageQueueReset(*queue);
    status         = error_converter(statusInternal);
  }

  return status;
}


/*--------------------------------------------------------------------------------------------------*/
/* cy_rtos_deinit_queue                                                                             */
/*--------------------------------------------------------------------------------------------------*/
cy_rslt_t cy_rtos_deinit_queue(cy_queue_t *queue)
{
  cy_rslt_t       status;
  cy_rtos_error_t statusInternal;

  if (queue == NULL)
  {
    status = CY_RTOS_BAD_PARAM;
  }
  else
  {
    statusInternal = osMessageQueueDelete(*queue);
    status         = error_converter(statusInternal);

    if (status == CY_RSLT_SUCCESS)
    {
      WHD_FREE(*queue);
      *queue = NULL;
    }
  }

  return status;
}



/**
  ****************************************************
  *                 Timers
  ****************************************************
  */
cy_rslt_t cy_rtos_init_timer(cy_timer_t *timer, cy_timer_trigger_type_t type,
                             cy_timer_callback_t fun, cy_timer_callback_arg_t arg)
{
  cy_rslt_t     status;
  osTimerAttr_t attr;

  if (timer == NULL)
  {
    status = CY_RTOS_BAD_PARAM;
  }
  else
  {
    attr.name = NULL;
    attr.attr_bits = 0U;
    attr.cb_mem = WHD_MALLOC(osTimerCbSize);
    attr.cb_size = osTimerCbSize;

    if (attr.cb_mem == NULL)
    {
      status = CY_RTOS_NO_MEMORY;
    }
    else
    {
      osTimerType_t osTriggerType = (CY_TIMER_TYPE_PERIODIC == type)
                                    ? osTimerPeriodic
                                    : osTimerOnce;

      assert(((uint32_t)attr.cb_mem & CY_RTOS_ALIGNMENT_MASK) == 0UL);
      *timer = osTimerNew((osTimerFunc_t)fun, osTriggerType, (void *)arg, &attr);
      assert((*timer == attr.cb_mem) || (*timer == NULL));
      status = (*timer == NULL) ? CY_RTOS_GENERAL_ERROR : CY_RSLT_SUCCESS;
    }
  }

  return status;
}


/*--------------------------------------------------------------------------------------------------*/
/* cy_rtos_start_timer                                                                              */
/*--------------------------------------------------------------------------------------------------*/
cy_rslt_t cy_rtos_start_timer(cy_timer_t *timer, cy_time_t num_ms)
{
  cy_rslt_t       status;
  cy_rtos_error_t statusInternal;

  if (timer == NULL)
  {
    status = CY_RTOS_BAD_PARAM;
  }
  else
  {
    /* Get Number of ticks per second */
    uint32_t tick_freq = osKernelGetTickFreq();

    /* Convert ticks count to time in milliseconds */
    if (tick_freq != 0)
    {
      uint32_t ticks = ((num_ms * tick_freq) / 1000);
      statusInternal = osTimerStart(*timer, ticks);
      status = error_converter(statusInternal);
    }
    else
      status = CY_RTOS_GENERAL_ERROR;
  }

  return status;
}

cy_rslt_t cy_rtos_stop_timer(cy_timer_t *timer)
{
  cy_rslt_t       status;
  cy_rtos_error_t statusInternal;

  if (timer == NULL)
  {
    status = CY_RTOS_BAD_PARAM;
  }
  else
  {
    statusInternal = osTimerStop(*timer);
    status         = error_converter(statusInternal);
  }

  return status;
}

cy_rslt_t cy_rtos_is_running_timer(cy_timer_t *timer, bool *state)
{
  cy_rslt_t status = CY_RSLT_SUCCESS;

  if ((timer == NULL) || (state == NULL))
  {
    status = CY_RTOS_BAD_PARAM;
  }
  else
    *state = osTimerIsRunning(*timer);

  return status;
}

cy_rslt_t cy_rtos_deinit_timer(cy_timer_t *timer)
{
  cy_rslt_t       status;
  cy_rtos_error_t statusInternal;

  if (timer == NULL)
  {
    status = CY_RTOS_BAD_PARAM;
  }
  else
  {
    statusInternal = osTimerDelete(*timer);
    status = error_converter(statusInternal);

    if (status == CY_RSLT_SUCCESS)
    {
      WHD_FREE(*timer);
      *timer = NULL;
    }
  }

  return status;
}


/**
  ****************************************************
  *                 Time
  ****************************************************
  */

cy_rslt_t cy_rtos_get_time(cy_time_t *tval)
{
  cy_rslt_t status = CY_RSLT_SUCCESS;
  uint32_t tick_freq;

  if (tval == NULL)
  {
    status = CY_RTOS_BAD_PARAM;
  }
  else
  {
    /* Get Number of ticks per second */
    tick_freq = osKernelGetTickFreq();

    /* Convert ticks count to time in milliseconds */
    if (tick_freq != 0)
    {
      *tval = ((osKernelGetTickCount() * 1000) / tick_freq);
    }
    else
      status = CY_RTOS_GENERAL_ERROR;
  }

  return status;
}

/*--------------------------------------------------------------------------------------------------*/
/* cy_rtos_delay_milliseconds                                                                       */
/*--------------------------------------------------------------------------------------------------*/
cy_rslt_t cy_rtos_delay_milliseconds(cy_time_t num_ms)
{
  cy_rslt_t       status;
  cy_rtos_error_t statusInternal;

  statusInternal = osDelay(num_ms);
  status         = error_converter(statusInternal);

  return status;
}


#if defined(__cplusplus)
}
#endif
