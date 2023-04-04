/**
  ******************************************************************************
  * @file    msg.h
  * @author  MCD Application Team
  * @brief   Trace message API.
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
#ifndef MSG_H
#define MSG_H

/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>

#define MSG_LOCK()
#define MSG_UNLOCK()

#ifdef ENABLE_IOT_INFO
#define MSG_INFO_ON
#endif /* ENABLE_IOT_INFO */

#ifdef ENABLE_IOT_WARNING
#define MSG_WARNING_ON
#endif /* ENABLE_IOT_WARNING */

#ifdef ENABLE_IOT_ERROR
#define MSG_ERROR_ON
#endif /* ENABLE_IOT_ERROR */

#ifdef ENABLE_IOT_DEBUG
#define MSG_DEBUG_ON
#endif /* ENABLE_IOT_DEBUG */


/**
  * @brief Debug level logging macro.
  *
  * Macro to expose function, line number as well as desired log message.
  */
#ifdef MSG_DEBUG_ON
#define MSG_DEBUG(...)                                \
do                                                    \
{                                                     \
    MSG_LOCK();                                       \
    printf("DEBUG:   %s L#%4d ", __func__, __LINE__); \
    printf(__VA_ARGS__);                              \
    MSG_UNLOCK();                                     \
} while(0) /*;*/
#else
#define MSG_DEBUG(...)
#endif /* MSG_DEBUG_ON */


/**
  * @brief Info level logging macro.
  *
  * Macro to expose desired log message.  Info messages do not include automatic function names and line numbers.
  */
#ifdef MSG_INFO_ON
#define MSG_INFO(...)    \
do                       \
{                        \
    MSG_LOCK();          \
    printf(__VA_ARGS__); \
    MSG_UNLOCK();        \
} while(0) /*;*/
#else
#define MSG_INFO(...)
#endif /* MSG_INFO_ON */

/**
  * @brief Warn level logging macro.
  *
  * Macro to expose function, line number as well as desired log message.
  */
#ifdef MSG_WARNING_ON
#define MSG_WARNING(...)                            \
do                                                  \
{                                                   \
    MSG_LOCK();                                     \
    printf("WARN:  %s L#%d ", __func__, __LINE__);  \
    printf(__VA_ARGS__);                            \
    MSG_UNLOCK();                                   \
} while(0) /*;*/
#else
#define MSG_WARNING(...)
#endif /* MSG_WARNING_ON */

/**
  * @brief Error level logging macro.
  *
  * Macro to expose function, line number as well as desired log message.
  */
#ifdef MSG_ERROR_ON
#define MSG_ERROR(...)                             \
do                                                 \
{                                                  \
    MSG_LOCK();                                    \
    printf("ERROR: %s L#%d ", __func__, __LINE__); \
    printf(__VA_ARGS__);                           \
    MSG_UNLOCK();                                  \
} while(0) /*;*/
#else
#define MSG_ERROR(...)
#endif /* MSG_WARNING_ON */

#endif /* MSG_H */
