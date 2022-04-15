/**
  ******************************************************************************
  * @file    stm32cube_RTC_interface.c
  * @author  MCD Application Team
  * @brief   Std. time port to the RTC.
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

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "timing_system.h"
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <inttypes.h>

#include "msg.h"

#define CONVERSION_EPOCHFACTOR  2208988800ul

int32_t setting_rtc(const char *dow, int32_t day, const char *month, int32_t year,
                    int32_t hour, int32_t min, int32_t sec);
void RTC_CalendarShow(uint8_t *showtime, uint8_t *showdate);
time_t SynchronizationAgentConvertNTPTime2EpochTime(uint8_t *pBufferTimingAnswer, size_t sizeBuffer);


const int32_t yytab[2][12] =
{
  { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 },
  { 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 }
};

/**
  * @brief  Set RTC time
  * @param  time_t epochTimeNow : Epoch Time
  * @retval int value for success(1)/failure(0)
  */
int32_t TimingSystemSetSystemTime(time_t epochTimeNow)
{
  const time_t now = epochTimeNow;

  /* Initialize calendar time form the given Epoch Time. */
  struct tm *calendar = gmtime(&now);
  if (calendar == NULL)
  {
    return 0;
  }

  /* Configure the Date. */
  RTC_DateTypeDef sDate;
  sDate.Year = calendar->tm_year - 100;
  sDate.Month = calendar->tm_mon + 1;
  sDate.Date = calendar->tm_mday;
  sDate.WeekDay = calendar->tm_wday + 1;

  if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN) != HAL_OK)
  {
    /* Initialization Error. */
    return 0;
  }

  /* Configure the Time. */
  RTC_TimeTypeDef sTime;
  sTime.Hours = calendar->tm_hour;
  sTime.Minutes = calendar->tm_min;
  sTime.Seconds = calendar->tm_sec;
  sTime.TimeFormat = RTC_HOURFORMAT12_AM;
  sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
  sTime.StoreOperation = RTC_STOREOPERATION_RESET;

  if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN) != HAL_OK)
  {
    /* Initialization Error. */
    return 0;
  }

  /* Writes a data in a RTC Backup data Register0. */
  HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR0, 0x32F2);

  return 1;
}

/**
  * @brief  Display the Time on format hh:mm:ss mm-dd-yy
  * @param  uint8_t* showtime pointer to the string where save the time
  * @param  uint8_t* showdate pointer to the string where save the date
  * @retval None
  */
void RTC_CalendarShow(uint8_t *showtime, uint8_t *showdate)
{
  /* Get the RTC current Time. */
  {
    RTC_TimeTypeDef sTime;
    HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);

    /* Display time Format: hh:mm:ss. */
    sprintf((char *)showtime, "%02d:%02d:%02d", sTime.Hours, sTime.Minutes, sTime.Seconds);

#if defined(ENABLE_IOT_DEBUG)
    MSG_DEBUG("%s\n", showtime);
#endif /* ENABLE_IOT_DEBUG */
  }
  /* Get the RTC current Date. */
  {
    RTC_DateTypeDef sDate;
    HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);

    /* Display date Format: mm-dd-yy. */
    sprintf((char *)showdate, "%02d-%02d-%02d", sDate.Month, sDate.Date, 2000 + sDate.Year);

#if defined(ENABLE_IOT_DEBUG)
    MSG_DEBUG("%s\n", showdate);
#endif /* ENABLE_IOT_DEBUG */
  }
}

/**
  * @brief  Get RTC time
  * @param  void
  * @retval time_t : time retrieved from RTC
  */
time_t TimingSystemGetSystemTime(void)
{
  time_t returnTime = 0;
  RTC_DateTypeDef sDate;
  RTC_TimeTypeDef sTime;

  /** Compute default calendar.
    * Thu Jan  1 01:00:00 CET 197
    */
  const time_t time_0 = 0;
  struct tm *calendar = gmtime(&time_0);

  if ((HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN) == HAL_OK) &&
      (HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN) == HAL_OK))
  {
    calendar->tm_year = sDate.Year + 100;
    calendar->tm_mon = sDate.Month - 1;
    calendar->tm_mday = sDate.Date;
    calendar->tm_wday = sDate.WeekDay - 1;
    calendar->tm_hour = sTime.Hours;
    calendar->tm_min = sTime.Minutes;
    calendar->tm_sec = sTime.Seconds;
    calendar->tm_isdst = 0;
#if _DLIB_SUPPORT_FOR_AEABI
    calendar->__BSD_bug_filler1 = 0;
    calendar->__BSD_bug_filler2 = 0;
#endif /* _DLIB_SUPPORT_FOR_AEABI */

  }

  returnTime = mktime(calendar);

#if defined(ENABLE_IOT_DEBUG)
  MSG_DEBUG("Time is %s\n", ctime(&returnTime));
#endif /* ENABLE_IOT_DEBUG */

  return returnTime;
}

/**
  * @brief  Convert NTP time to epoch time
  * @param  uint8_t* pBufferTimingAnswer : pointer to buffer containing the NTP date
  * @param  size_t sizeBuffer : size of buffer
  * @retval time_t : epoch time after conversion
  */
time_t SynchronizationAgentConvertNTPTime2EpochTime(uint8_t *pBufferTimingAnswer, size_t sizeBuffer)
{
  time_t epochTime;

  epochTime = (time_t) - 1;

  if (sizeBuffer >= 4)
  {
    const uint32_t valueNumericExtracted = ((pBufferTimingAnswer[0] << 24) | (pBufferTimingAnswer[1] << 16) |
                                            (pBufferTimingAnswer[2] << 8) | pBufferTimingAnswer[3]);
    epochTime = (time_t)(valueNumericExtracted - CONVERSION_EPOCHFACTOR);
  }

  return epochTime;
}

int32_t setting_rtc(const char *dow, int32_t day, const char *month, int32_t year,
                    int32_t hour, int32_t min, int32_t sec)
{
  {
    RTC_TimeTypeDef sTime;

    sTime.Hours = hour;
    sTime.Minutes = min;
    sTime.Seconds = sec;
    sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
    sTime.StoreOperation = RTC_STOREOPERATION_RESET;

    if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN) != HAL_OK)
    {
      return -1;
    }
  }

  {
    RTC_DateTypeDef sDate;

    if (strcmp(dow, "Mon,") == 0)
    {
      sDate.WeekDay = RTC_WEEKDAY_MONDAY;
    }
    else if (strcmp(dow, "Tue,") == 0)
    {
      sDate.WeekDay = RTC_WEEKDAY_TUESDAY;
    }
    else if (strcmp(dow, "Wed,") == 0)
    {
      sDate.WeekDay = RTC_WEEKDAY_WEDNESDAY;
    }
    else if (strcmp(dow, "Thu,") == 0)
    {
      sDate.WeekDay = RTC_WEEKDAY_THURSDAY;
    }
    else if (strcmp(dow, "Fri,") == 0)
    {
      sDate.WeekDay = RTC_WEEKDAY_FRIDAY;
    }
    else if (strcmp(dow, "Sat,") == 0)
    {
      sDate.WeekDay = RTC_WEEKDAY_SATURDAY;
    }
    else if (strcmp(dow, "Sun,") == 0)
    {
      sDate.WeekDay = RTC_WEEKDAY_SUNDAY;
    }
    else
    {
      return  -1;
    }

    if (strcmp(month, "Jan") == 0)
    {
      sDate.Month = RTC_MONTH_JANUARY;
    }
    else if (strcmp(month, "Feb") == 0)
    {
      sDate.Month = RTC_MONTH_FEBRUARY;
    }
    else if (strcmp(month, "Mar") == 0)
    {
      sDate.Month = RTC_MONTH_MARCH;
    }
    else if (strcmp(month, "Apr") == 0)
    {
      sDate.Month = RTC_MONTH_APRIL;
    }
    else if (strcmp(month, "May") == 0)
    {
      sDate.Month = RTC_MONTH_MAY;
    }
    else if (strcmp(month, "Jun") == 0)
    {
      sDate.Month = RTC_MONTH_JUNE;
    }
    else if (strcmp(month, "Jul") == 0)
    {
      sDate.Month = RTC_MONTH_JULY;
    }
    else if (strcmp(month, "Aug") == 0)
    {
      sDate.Month = RTC_MONTH_AUGUST;
    }
    else if (strcmp(month, "Sep") == 0)
    {
      sDate.Month = RTC_MONTH_SEPTEMBER;
    }
    else if (strcmp(month, "Oct") == 0)
    {
      sDate.Month = RTC_MONTH_OCTOBER;
    }
    else if (strcmp(month, "Nov") == 0)
    {
      sDate.Month = RTC_MONTH_NOVEMBER;
    }
    else if (strcmp(month, "Dec") == 0)
    {
      sDate.Month = RTC_MONTH_DECEMBER;
    }
    else
    {
      return  -1;
    }

    sDate.Date = day;
    sDate.Year = year - 2000;
    if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN) != HAL_OK)
    {
      return -1;
    }
  }
  return 0;
}
