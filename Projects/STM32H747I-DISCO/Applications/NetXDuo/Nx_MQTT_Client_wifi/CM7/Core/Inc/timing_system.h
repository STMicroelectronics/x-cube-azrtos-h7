/**
  ******************************************************************************
  * @file    timing_system.h
  * @author  MCD Application Team
  * @brief   Header file for timing_system.c
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

#ifndef TIMING_SYSTEM_H
#define TIMING_SYSTEM_H

/* Includes ------------------------------------------------------------------*/
#include <inttypes.h>
#include <time.h>

extern void TimingSystemInitialize(void);
extern int32_t TimingSystemSetSystemTime(time_t epochTimeNow);
extern time_t TimingSystemGetSystemTime(void);

#define YYEAR0           1900            /* The first year. */
#define EEPOCH_YR        1970            /* EPOCH = Jan 1 1970 00:00:00. */
#define SSECS_DAY        (24UL * 60UL * 60UL)
#define LLEAPYEAR(year)  (!((year) % 4) && (((year) % 100) || !((year) % 400)))
#define YYEARSIZE(year)  (LLEAPYEAR(year) ? 366 : 365)

/* MDK */
extern struct tm *gmtimeMDK(register const time_t *timer);

extern const int32_t yytab[2][12];
#endif /* TIMING_SYSTEM_H */
