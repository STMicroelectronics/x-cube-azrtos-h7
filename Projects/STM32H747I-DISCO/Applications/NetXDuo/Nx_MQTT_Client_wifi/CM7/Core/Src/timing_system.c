/**
  ******************************************************************************
  * @file    timing_system.c
  * @author  MCD Application Team
  * @brief   Wrapper to STM32 timing
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

/* Includes ------------------------------------------------------------------*/
#include <time.h>
#include "timing_system.h"

/* Private typedef -----------------------------------------------------------*/
/* Private defines -----------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Functions Definition ------------------------------------------------------*/

#if defined (__IAR_SYSTEMS_ICC__)
#if _DLIB_TIME_USES_64
__time64_t __time64(__time64_t *pointer)
{
  return (__time64_t)TimingSystemGetSystemTime();
}
#else
__time32_t __time32(__time32_t *pointer)
{
  return (__time32_t)TimingSystemGetSystemTime();
}
#endif /* _DLIB_TIME_USES_64 */

#elif defined (__GNUC__) || defined(__ARMCC_VERSION)
time_t time(time_t *pointer)
{
  const time_t the_time = (time_t)TimingSystemGetSystemTime();
  if (pointer != NULL)
  {
    *pointer = the_time;
  }
  return the_time;
}
#endif /* defined (__IAR_SYSTEMS_ICC__) */


#if defined(__ARMCC_VERSION)
/**
  * @brief gmtime conversion for ARM compiler
  * @param  time_t *time_ptr : File pointer to time_t structure
  * @retval struct tm * : date in struct tm format
  */
struct tm *gmtimeMDK(const time_t *time_ptr)
{
  static struct tm br_time;

  uint32_t dayclock = (uint32_t)(*time_ptr) % SSECS_DAY;
  uint32_t dayno    = (uint32_t)(*time_ptr) / SSECS_DAY;
  int32_t year = EEPOCH_YR;


  br_time.tm_sec = dayclock % 60;
  br_time.tm_min = (dayclock % 3600) / 60;
  br_time.tm_hour = dayclock / 3600;
  br_time.tm_wday = (dayno + 4) % 7;       /* day 0 was a Thursday */

  while (dayno >= YYEARSIZE(year))
  {
    dayno -= YYEARSIZE(year);
    year++;
  }

  br_time.tm_year = year - YYEAR0;
  br_time.tm_yday = dayno;
  br_time.tm_mon = 0;

  while (dayno >= yytab[LLEAPYEAR(year)][br_time.tm_mon])
  {
    dayno -= yytab[LLEAPYEAR(year)][br_time.tm_mon];
    br_time.tm_mon++;
  }

  br_time.tm_mday = dayno + 1;
  br_time.tm_isdst = 0;

  return &br_time;
}

struct tm *gmtime(const time_t *p)
{
  return gmtimeMDK(p);
}
#endif /* __ARMCC_VERSION */
