/**
  ******************************************************************************
  * @file    filex_stub.c
  * @author  MCD Application Team
  * @brief   FileX applicative file
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


#include "filex_stub.h"

UINT fx_directory_attributes_read(FX_MEDIA *media_ptr, CHAR *directory_name, UINT *attributes_ptr)
{
  return 0;
}
UINT fx_directory_attributes_set(FX_MEDIA *media_ptr, CHAR *directory_name, UINT attributes)
{
  return 0;
}
UINT fx_directory_create(FX_MEDIA *media_ptr, CHAR *directory_name)
{
  return 0;
}
UINT fx_directory_delete(FX_MEDIA *media_ptr, CHAR *directory_name)
{
  return 0;
}
UINT fx_directory_rename(FX_MEDIA *media_ptr, CHAR *old_directory_name, CHAR *new_directory_name)
{
  return 0;
}
UINT fx_directory_first_entry_find(FX_MEDIA *media_ptr, CHAR *directory_name)
{
  return 0;
}
UINT fx_directory_first_full_entry_find(FX_MEDIA *media_ptr, CHAR *directory_name, UINT *attributes, ULONG *size,
                                        UINT *year, UINT *month, UINT *day, UINT *hour, UINT *minute, UINT *second)
{
  return 0;
}
UINT fx_directory_next_entry_find(FX_MEDIA *media_ptr, CHAR *directory_name)
{
  return 0;
}
UINT fx_directory_next_full_entry_find(FX_MEDIA *media_ptr, CHAR *directory_name, UINT *attributes, ULONG *size,
                                       UINT *year, UINT *month, UINT *day, UINT *hour, UINT *minute, UINT *second)
{
  return 0;
}
UINT fx_directory_name_test(FX_MEDIA *media_ptr, CHAR *directory_name)
{
  return 0;
}
UINT fx_directory_information_get(FX_MEDIA *media_ptr, CHAR *directory_name, UINT *attributes, ULONG *size,
                                  UINT *year, UINT *month, UINT *day, UINT *hour, UINT *minute, UINT *second)
{
  return 0;
}
UINT fx_directory_default_set(FX_MEDIA *media_ptr, CHAR *new_path_name)
{
  return 0;
}
UINT fx_directory_default_get(FX_MEDIA *media_ptr, CHAR **return_path_name)
{
  return 0;
}

UINT fx_file_best_effort_allocate(FX_FILE *file_ptr, ULONG size, ULONG *actual_size_allocated)
{
  return 0;
}
UINT fx_file_create(FX_MEDIA *media_ptr, CHAR *file_name)
{
  return 0;
}
UINT fx_file_delete(FX_MEDIA *media_ptr, CHAR *file_name)
{
  return 0;
}
UINT fx_file_rename(FX_MEDIA *media_ptr, CHAR *old_file_name, CHAR *new_file_name)
{
  return 0;
}
UINT fx_file_attributes_set(FX_MEDIA *media_ptr, CHAR *file_name, UINT attributes)
{
  return 0;
}
UINT fx_file_attributes_read(FX_MEDIA *media_ptr, CHAR *file_name, UINT *attributes_ptr)
{
  return 0;
}
UINT fx_file_open(FX_MEDIA *media_ptr, FX_FILE *file_ptr, CHAR *file_name,
                  UINT open_type)
{
  return 0;
}
UINT fx_file_close(FX_FILE *file_ptr)
{
  return 0;
}
UINT fx_file_read(FX_FILE *file_ptr, VOID *buffer_ptr, ULONG request_size, ULONG *actual_size)
{
  return 0;
}
UINT fx_file_write(FX_FILE *file_ptr, VOID *buffer_ptr, ULONG size)
{
  return 0;
}
UINT fx_file_allocate(FX_FILE *file_ptr, ULONG size)
{
  return 0;
}
UINT fx_file_relative_seek(FX_FILE *file_ptr, ULONG byte_offset, UINT seek_from)
{
  return 0;
}
UINT fx_file_seek(FX_FILE *file_ptr, ULONG byte_offset)
{
  return 0;
}
UINT fx_file_truncate(FX_FILE *file_ptr, ULONG size)
{
  return 0;
}
UINT fx_file_truncate_release(FX_FILE *file_ptr, ULONG size)
{
  return 0;
}
UINT fx_directory_local_path_restore(FX_MEDIA *media_ptr, FX_LOCAL_PATH *local_path_ptr)
{
  return 0;
}
UINT fx_directory_local_path_set(FX_MEDIA *media_ptr, FX_LOCAL_PATH *local_path_ptr, CHAR *new_path_name)
{
  return 0;
}
UINT fx_directory_local_path_get(FX_MEDIA *media_ptr, CHAR **return_path_name)
{
  return 0;
}
UINT fx_media_format(FX_MEDIA *media_ptr, VOID (*driver)(FX_MEDIA *media), VOID *driver_info_ptr,
                     UCHAR *memory_ptr, UINT memory_size,
                     CHAR *volume_name, UINT number_of_fats, UINT directory_entries, UINT hidden_sectors,
                     ULONG total_sectors, UINT bytes_per_sector, UINT sectors_per_cluster,
                     UINT heads, UINT sectors_per_track)
{
  return 0;
}
UINT fx_media_open(FX_MEDIA *media_ptr, CHAR *media_name,
                   VOID (*media_driver)(FX_MEDIA *), VOID *driver_info_ptr,
                   VOID *memory_ptr, ULONG memory_size)
{
  return 0;
}
