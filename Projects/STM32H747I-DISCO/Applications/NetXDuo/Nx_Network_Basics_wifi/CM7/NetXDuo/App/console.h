/**
  ******************************************************************************
  * @file    console.h
  * @author  MCD Application Team
  * @brief   This file provides code for console application.
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
#ifndef _CONSOLE_H
#define _CONSOLE_H

/* Includes ------------------------------------------------------------------*/
#include <inttypes.h>

typedef struct
{
  char *name;
  char *comment;
  int32_t (*func)(int32_t argc, char *argv[]);
} cmd_t;

void console(const char *prompt, const cmd_t cmdlist[]);
int32_t help_cmd(int32_t argc, char **argv);
int32_t quit_cmd(int32_t argc, char **argv);

extern int32_t quit_msg;

#endif /* _CONSOLE_H */
