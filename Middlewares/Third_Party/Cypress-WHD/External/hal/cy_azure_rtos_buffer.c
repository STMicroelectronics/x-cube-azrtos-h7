/***************************************************************************//**
  * \file cy_azure_rtos_buffer.c
  *
  * \brief
  * Basic set of APIs for dealing with network packet buffers. This is used by WHD
  * for relaying data between the network stack and the connectivity chip.
  *
  ********************************************************************************
  * \copyright
  * Copyright 2018-2021 Cypress Semiconductor Corporation
  * Copyright (c) 2021 STMicroelectronics. All rights reserved.
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
#include <stdio.h>
#include <inttypes.h>

#include "whd.h"
#include "whd_types.h"
#include "whd_types_int.h"

#include "cy_network_buffer.h"
#include "nx_stm32_cypress_whd_driver.h"


/* Indicate that driver source is being compiled.  */
#define NX_DRIVER_SOURCE

#include "nx_driver_framework.h"


#define SDIO_BLOCK_SIZE (64U)

/**
  * The maximum size of the SDPCM + BDC header, including offsets and reserved space
  * 12 bytes - SDPCM header
  * 2 bytes  - Extra offset for SDPCM headers that come as 14 bytes
  * 4 bytes  - BDC header
  */
#define MAX_SDPCM_BDC_HEADER_LENGTH (18)

#define HOST_BUFFER_RELEASE_REMOVE_AT_FRONT_BUS_HEADER_SIZE  \
  (sizeof(whd_buffer_header_t) + MAX_SDPCM_BDC_HEADER_LENGTH)

#define HOST_BUFFER_RELEASE_REMOVE_AT_FRONT_FULL_SIZE        \
  (HOST_BUFFER_RELEASE_REMOVE_AT_FRONT_BUS_HEADER_SIZE + WHD_ETHERNET_SIZE)

#define WPRINT_NETWORK_DEBUG(...) /*printf(__VA_ARGS__)*/
#define WPRINT_NETWORK_ERROR(...) printf(__VA_ARGS__)

#define CY_ASSERT(...)

extern NX_DRIVER_INFORMATION nx_driver_information;

whd_buffer_funcs_t buffer_funcs_ops =
{
  .whd_host_buffer_get = cy_host_buffer_get,
  .whd_buffer_release = cy_buffer_release,
  .whd_buffer_get_current_piece_data_pointer = cy_buffer_get_current_piece_data_pointer,
  .whd_buffer_get_current_piece_size = cy_buffer_get_current_piece_size,
  .whd_buffer_set_size = cy_buffer_set_size,
  .whd_buffer_add_remove_at_front = cy_buffer_add_remove_at_front,
};


whd_netif_funcs_t netif_funcs_ops =
{
  .whd_network_process_ethernet_data = cy_network_process_ethernet_data,
};


/*--------------------------------------------------------------------------------------------------*/
/* cy_host_buffer_get                                                                               */
/*--------------------------------------------------------------------------------------------------*/
whd_result_t cy_host_buffer_get(whd_buffer_t *buffer, whd_buffer_dir_t direction,
                                unsigned short size, unsigned long timeout_ms)
{
  UINT status = NX_NO_PACKET;
  NX_PACKET *packet_ptr;

  UNUSED_PARAMETER(direction);
  UNUSED_PARAMETER(timeout_ms);

  if (!nx_driver_information.nx_driver_information_packet_pool_ptr)
  {
    /* printf("##### %d", __LINE__); */
    return 1;
  }

  if (size > nx_driver_information.nx_driver_information_packet_pool_ptr->nx_packet_pool_payload_size)
  {
    WPRINT_NETWORK_ERROR("cy_host_buffer_get(): failed with %"PRIu32"\n", (uint32_t)size);
    return 1;
  }

  status = nx_packet_allocate(nx_driver_information.nx_driver_information_packet_pool_ptr, &packet_ptr, NX_RECEIVE_PACKET, NX_NO_WAIT);

  /* No packet available from any pool */
  if (status == NX_NO_PACKET)
  {
    WPRINT_NETWORK_ERROR("cy_host_buffer_get(): No packet\n");
    return WHD_BUFFER_UNAVAILABLE_TEMPORARY;
  }

  if (status != NX_SUCCESS)
  {
    WPRINT_NETWORK_ERROR(("cy_host_buffer_get(): Packet allocation failed\n"));
    return WHD_BUFFER_ALLOC_FAIL;
  }

  packet_ptr->nx_packet_length = size;
  packet_ptr->nx_packet_append_ptr = packet_ptr->nx_packet_prepend_ptr + size;

  *buffer = (whd_buffer_t *)packet_ptr;

  WPRINT_NETWORK_DEBUG("cy_host_buffer_get(): returns %p\n", packet_ptr);

  return WHD_SUCCESS;
}


/*--------------------------------------------------------------------------------------------------*/
/* cy_buffer_release                                                                                */
/*--------------------------------------------------------------------------------------------------*/
void cy_buffer_release(whd_buffer_t buffer, whd_buffer_dir_t direction)
{
  CY_ASSERT(buffer != NULL);

  NX_PACKET *packet_ptr = (NX_PACKET *)buffer;

  WPRINT_NETWORK_DEBUG("cy_buffer_release(): with %p\n", packet_ptr);

  if (direction == WHD_NETWORK_TX)
  {

    /* TCP transmit packet isn't returned immediately to the pool. The stack holds the packet temporarily
     * until ACK is received. Otherwise, the same packet is used for re-transmission.
     * Return prepend pointer to the original location which the stack expects (the start of IP header).
     * For other packets, resetting prepend pointer isn't required.
     */
    if (packet_ptr->nx_packet_length > HOST_BUFFER_RELEASE_REMOVE_AT_FRONT_FULL_SIZE)
    {
      if (cy_buffer_add_remove_at_front(&buffer, HOST_BUFFER_RELEASE_REMOVE_AT_FRONT_FULL_SIZE) != WHD_SUCCESS)
      {
        WPRINT_NETWORK_DEBUG("cy_buffer_release(): Could not move packet pointer\n");
      }
    }

    if (NX_SUCCESS != nx_packet_transmit_release(packet_ptr))
    {
      WPRINT_NETWORK_ERROR("cy_buffer_release(): Could not release packet - leaking buffer\n");
    }
  }
  else
  {
    if (NX_SUCCESS != nx_packet_release(packet_ptr))
    {
      WPRINT_NETWORK_ERROR("cy_buffer_release(): Could not release packet - leaking buffer\n");
    }
  }
}


/*--------------------------------------------------------------------------------------------------*/
/* cy_buffer_get_current_piece_data_pointer                                                         */
/*--------------------------------------------------------------------------------------------------*/
uint8_t *cy_buffer_get_current_piece_data_pointer(whd_buffer_t buffer)
{
  NX_PACKET *packet_ptr = (NX_PACKET *)buffer;
  CY_ASSERT(buffer != NULL);

  return packet_ptr->nx_packet_prepend_ptr;
}


/*--------------------------------------------------------------------------------------------------*/
/* cy_buffer_get_current_piece_size                                                                 */
/*--------------------------------------------------------------------------------------------------*/
uint16_t cy_buffer_get_current_piece_size(whd_buffer_t buffer)
{
  NX_PACKET *packet_ptr = (NX_PACKET *)buffer;
  CY_ASSERT(buffer != NULL);

  return (uint16_t)packet_ptr->nx_packet_length;
}


/*--------------------------------------------------------------------------------------------------*/
/* cy_buffer_set_size                                                                               */
/*--------------------------------------------------------------------------------------------------*/
whd_result_t cy_buffer_set_size(whd_buffer_t buffer, unsigned short size)
{
  NX_PACKET *packet_ptr = (NX_PACKET *)buffer;
  CY_ASSERT(buffer != NULL);

  if (size > WHD_LINK_MTU)
  {
    WPRINT_NETWORK_DEBUG("cy_buffer_set_size(): Attempt to set a length larger than the MTU of the link\n");
    return WHD_BUFFER_SIZE_SET_ERROR;
  }

  if (packet_ptr->nx_packet_prepend_ptr + size >= packet_ptr->nx_packet_data_end)
  {
    WPRINT_NETWORK_ERROR("cy_buffer_set_size(): WHD_PMK_WRONG_LENGTH: %p with set size %"PRIu32"\n",
                         packet_ptr, (uint32_t)size);
    return WHD_PMK_WRONG_LENGTH;
  }

  packet_ptr->nx_packet_append_ptr = packet_ptr->nx_packet_prepend_ptr + size;
  packet_ptr->nx_packet_length = size;

  return WHD_SUCCESS;
}


/*--------------------------------------------------------------------------------------------------*/
/* cy_buffer_add_remove_at_front                                                                    */
/*--------------------------------------------------------------------------------------------------*/
whd_result_t cy_buffer_add_remove_at_front(whd_buffer_t *buffer, int32_t add_remove_amount)
{
  CY_ASSERT(buffer != NULL);

  NX_PACKET *packet_ptr = *((NX_PACKET **)buffer);
  UCHAR *new_start_ptr = packet_ptr->nx_packet_prepend_ptr + add_remove_amount;

  if (new_start_ptr <= packet_ptr->nx_packet_data_start)
  {
    WPRINT_NETWORK_DEBUG("cy_buffer_add_remove_at_front(): WHD_BUFFER_POINTER_MOVE_ERROR: %p with add/remove %"PRId32"\n",
                         packet_ptr, add_remove_amount);

    /* Trying to move to a location before start - not supported without buffer chaining */
    WPRINT_NETWORK_ERROR("cy_buffer_add_remove_at_front(): Attempt to move to a location before start - not supported without buffer chaining\n");
    return WHD_BUFFER_POINTER_MOVE_ERROR;
  }

  else if (new_start_ptr >= packet_ptr->nx_packet_data_end)
  {
    WPRINT_NETWORK_DEBUG("cy_buffer_add_remove_at_front(): WHD_BUFFER_POINTER_MOVE_ERROR: %p with add/remove %"PRId32"\n",
                         packet_ptr, add_remove_amount);

    /* Trying to move to a location after end of buffer - not supported without buffer chaining */
    WPRINT_NETWORK_ERROR("cy_buffer_add_remove_at_front(): Attempt to move to a location after end of buffer - not supported without buffer chaining\n");
    return WHD_BUFFER_POINTER_MOVE_ERROR;

  }
  else
  {
    packet_ptr->nx_packet_prepend_ptr = new_start_ptr;

    if (packet_ptr->nx_packet_append_ptr < packet_ptr->nx_packet_prepend_ptr)
    {
      packet_ptr->nx_packet_append_ptr = packet_ptr->nx_packet_prepend_ptr;
    }
    packet_ptr->nx_packet_length -= add_remove_amount;
  }

  return WHD_SUCCESS;
}
