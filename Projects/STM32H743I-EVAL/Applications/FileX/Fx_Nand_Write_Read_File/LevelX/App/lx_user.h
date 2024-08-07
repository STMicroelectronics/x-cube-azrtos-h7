/**************************************************************************/
/*                                                                        */
/*       Copyright (c) Microsoft Corporation. All rights reserved.        */
/*                                                                        */
/*       This software is licensed under the Microsoft Software License   */
/*       Terms for Microsoft Azure RTOS. Full text of the license can be  */
/*       found in the LICENSE file at https://aka.ms/AzureRTOS_EULA       */
/*       and in the root directory of this software.                      */
/*                                                                        */
/**************************************************************************/

/**************************************************************************/
/**************************************************************************/
/**                                                                       */
/** LevelX Component                                                      */
/**                                                                       */
/**   User Specific                                                       */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

/**************************************************************************/
/*                                                                        */
/*  PORT SPECIFIC C INFORMATION                            RELEASE        */
/*                                                                        */
/*    lx_user.h                                           PORTABLE C      */
/*                                                           6.3.0        */
/*                                                                        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This file contains user defines for configuring LevelX in specific  */
/*    ways. This file will have an effect only if the application and     */
/*    LevelX library are built with LX_INCLUDE_USER_DEFINE_FILE defined.  */
/*    Note that all the defines in this file may also be made on the      */
/*    command line when building LevelX library and application objects.  */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  11-09-2020     William E. Lamie         Initial Version 6.1.2         */
/*  06-02-2021     Bhupendra Naphade        Modified comment(s), and      */
/*                                            added standalone support,   */
/*                                            resulting in version 6.1.7  */
/*  03-08-2023     Xiuwen Cai               Modified comment(s), and      */
/*                                            added new NAND options,     */
/*                                            resulting in version 6.2.1  */
/*  10-31-2023     Xiuwen Cai               Modified comment(s),          */
/*                                            added options for mapping , */
/*                                            bitmap cache and obsolete   */
/*                                            count cache,                */
/*                                            resulting in version 6.3.0  */
/*                                                                        */
/**************************************************************************/

#ifndef LX_USER_H
#define LX_USER_H

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/* By default this value is 4, which represents a maximum of 4 blocks that
   can be allocated for metadata.
*/

/* #define LX_NAND_FLASH_MAX_METADATA_BLOCKS         4 */
/* Defined, this makes LevelX thread-safe by using a ThreadX mutex object
   throughout the API.
*/

/* #define LX_THREAD_SAFE_ENABLE */

/* Defined, LevelX will be used in standalone mode (without Azure RTOS ThreadX) */

/* #define LX_STANDALONE_ENABLE */

/* USER CODE BEGIN 2 */

/* Define user extension for NAND flash control block.  */

/* #define LX_NAND_FLASH_USER_EXTENSION   ???? */

/* USER CODE END 2 */

#endif
