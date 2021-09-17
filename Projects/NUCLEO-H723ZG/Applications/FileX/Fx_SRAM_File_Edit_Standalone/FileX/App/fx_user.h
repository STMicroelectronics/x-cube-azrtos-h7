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
/** FileX Component                                                       */
/**                                                                       */
/**   User Specific                                                       */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

/**************************************************************************/
/*                                                                        */
/*  PORT SPECIFIC C INFORMATION                            RELEASE        */
/*                                                                        */
/*    fx_user.h                                           PORTABLE C      */
/*                                                           6.0          */
/*                                                                        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This file contains user defines for configuring FileX in specific   */
/*    ways. This file will have an effect only if the application and     */
/*    FileX library are built with FX_INCLUDE_USER_DEFINE_FILE defined.   */
/*    Note that all the defines in this file may also be made on the      */
/*    command line when building FileX library and application objects.   */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     William E. Lamie         Initial Version 6.0           */
/*                                                                        */
/**************************************************************************/

#ifndef FX_USER_H
#define FX_USER_H

/* Define various build options for the FileX port.  The application should either make changes
   here by commenting or un-commenting the conditional compilation defined OR supply the defines though the compiler's equivalent of the -D option.  */

/* Override various options with default values already assigned in fx_api.h or fx_port.h.
  Please also refer to fx_port.h for descriptions on each of these options.  */

#define FX_STANDALONE_ENABLE
/* Defined, the direct read sector update of cache is disabled.  */

/* #define FX_DISABLE_DIRECT_DATA_READ_CACHE_FILL */

/* Determine if error checking is desired.  If so, map API functions
   to the appropriate error checking front-ends.  Otherwise, map API
   functions to the core functions that actually perform the work.
   Note: error checking is enabled by default.  */

/* #define FX_DISABLE_ERROR_CHECKING */

/* Defined, FileX does not update already opened files.  */

/* #define FX_DONT_UPDATE_OPEN_FILES */

/* Defined, enables 64-bits sector addresses used in I/O driver.  */

/* #define FX_DRIVER_USE_64BIT_LBA */

/* Defined, FileX is able to access exFAT file system.

   FileX supports the Microsoft exFAT file system format.
   Your use of exFAT technology in your products requires a separate
   license from Microsoft. Please see the following link for further
   details on exFAT licensing:

   https://www.microsoft.com/en-us/legal/intellectualproperty/mtl/exfat-licensing.aspx
*/

/* #define FX_ENABLE_EXFAT */

/* Defined, enables FileX fault tolerant service.  */

/* #define FX_ENABLE_FAULT_TOLERANT */

/* Defines the size in bytes of the bit map used to update the secondary FAT sectors.
   The larger the value the less unnecessary secondary FAT sector writes.   */

/* #define FX_FAT_MAP_SIZE         128 */

/* Defined, data sector write requests are flushed immediately to the driver.  */

/* #define FX_FAULT_TOLERANT */

/* Define byte offset in boot sector where the cluster number of the Fault Tolerant Log file is.
   Note that this field (byte 116 to 119) is marked as reserved by FAT 12/16/32/exFAT specification. */

/* #define FX_FAULT_TOLERANT_BOOT_INDEX         116 */

/* Defined, data sector write requests are flushed immediately to the driver.  */

/* #define FX_FAULT_TOLERANT_DATA */

/* Defines the number of entries in the FAT cache.  */

/* #define FX_MAX_FAT_CACHE         16 */

/* Defines the maximum size of long file names supported by FileX.
   The minimum value is 13 and the maximum value is 256.  */

/* #define FX_MAX_LAST_NAME_LEN         256 */
/* #define FX_MAX_LONG_NAME_LEN         256 */

/* Defines the maximum number of logical sectors that can be cached by FileX. The cache memory
   supplied to FileX at fx_media_open determines how many sectors can actually be cached.  */

/* #define FX_MAX_SECTOR_CACHE         256 */

/* Defined, the file search cache optimization is disabled.  */

/* #define FX_MEDIA_DISABLE_SEARCH_CACHE */

/* Defined, gathering of media statistics is disabled.  */

/* #define FX_MEDIA_STATISTICS_DISABLE */

/* Defined, local path logic is disabled.  */

/* #define FX_NO_LOCAL_PATH */

/* Defined, FileX is built without update to the time parameters.  */

/* #define FX_NO_TIMER */

/* Defined, renaming inherits path information.  */

/* #define FX_RENAME_PATH_INHERIT */

/* Defined, legacy single open logic for the same file is enabled.  */

/* #define FX_SINGLE_OPEN_LEGACY */

/* Define FileX internal protection macros.  If FX_SINGLE_THREAD is defined,
   these protection macros are effectively disabled.  However, for multi-thread
   uses, the macros are setup to utilize a ThreadX mutex for multiple thread
   access control into an open media.  */

#define FX_SINGLE_THREAD

/* Defines the number of seconds the time parameters are updated in FileX.  */

/* #define FX_UPDATE_RATE_IN_SECONDS         10 */

/* Defines the number of ThreadX timer ticks required to achieve the update rate specified by
   FX_UPDATE_RATE_IN_SECONDS defined previously. By default, the ThreadX timer tick is 10ms,
   so the default value for this constant is 1000.  */

/* #define FX_UPDATE_RATE_IN_TICKS         1000 */

#endif
