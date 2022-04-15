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
/** ThreadX Component                                                     */ 
/**                                                                       */ 
/**   User Specific                                                       */ 
/**                                                                       */ 
/**************************************************************************/ 
/**************************************************************************/ 


/**************************************************************************/ 
/*                                                                        */ 
/*  APPLICATION INTERFACE DEFINITION                       RELEASE        */ 
/*                                                                        */ 
/*    txm_module_user.h                                   PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Scott Larson, Microsoft Corporation                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This file contains user defines for configuring the Module Manager  */ 
/*    in specific ways. This file will have an effect only if the Module  */ 
/*    Manager library is built with TXM_MODULE_INCLUDE_USER_DEFINE_FILE   */ 
/*    defined. Note that all the defines in this file may also be made on */ 
/*    the command line when building Modules library and application      */ 
/*    objects.                                                            */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  09-30-2020     Scott Larson             Initial Version 6.1           */
/*                                                                        */
/**************************************************************************/

#ifndef TXM_MODULE_USER_H
#define TXM_MODULE_USER_H

/* Defines the kernel stack size for a module thread. The default is 512, which is
   sufficient for applications only using ThreadX, however, if other libraries are
   used i.e. FileX, NetX, etc., then this value will most likely need to be increased.  */

/* #define TXM_MODULE_KERNEL_STACK_SIZE    2048 */

/* ARM Core speciic MPU configuration register settings */
/* For the following 3 access control settings, change TEX and C, B, S (bits 21 through 16 of MPU_RASR)
 * to reflect your system memory attributes (cache, shareable, memory type).  */

/* Code region access control: privileged read-only, outer & inner write-back, normal memory, not shareable.  */
#define TXM_MODULE_MPU_CODE_ACCESS_CONTROL      0x06030000

/* Data region access control: execute never, read/write, outer & inner write-back, normal memory, not shareable.  */
#define TXM_MODULE_MPU_DATA_ACCESS_CONTROL      0x13030000

/* Shared region access control: execute never, read-only, outer & inner write-back, normal memory, not shareable.  */
#define TXM_MODULE_MPU_SHARED_ACCESS_CONTROL    0x12030000

#endif
