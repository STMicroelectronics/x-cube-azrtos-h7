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
/** USBX Component                                                        */
/**                                                                       */
/**   STM32 Controller Driver                                             */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/


/**************************************************************************/
/*                                                                        */
/*  COMPONENT DEFINITION                                   RELEASE        */
/*                                                                        */
/*    ux_hcd_stm32.h                                      PORTABLE C      */
/*                                                           6.0          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Chaoqiong Xiao, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This file contains all the header and extern functions used by the  */
/*    USBX host STM32 Controller.                                         */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Chaoqiong Xiao           Initial Version 6.0           */
/*                                                                        */
/**************************************************************************/

#ifndef UX_HCD_STM32_H
#define UX_HCD_STM32_H

#include "ux_stm32_config.h"

/* Define STM32 HCD generic definitions.  */


#define UX_HCD_STM32_CONTROLLER                                 6
#ifndef UX_HCD_STM32_MAX_NB_CHANNELS
#define UX_HCD_STM32_MAX_NB_CHANNELS                            12
#endif /* UX_HCD_STM32_MAX_NB_CHANNELS */
#define UX_HCD_STM32_NB_ROOT_PORTS                              1
#define UX_HCD_STM32_NO_CHANNEL_ASSIGNED                        0xff
#define UX_HCD_STM32_CONTROLLER_FLAG_DEVICE_ATTACHED            0x01
#define UX_HCD_STM32_CONTROLLER_FLAG_DEVICE_DETACHED            0x02
#define UX_HCD_STM32_CONTROLLER_FLAG_SOF                        0x04
#define UX_HCD_STM32_CONTROLLER_FLAG_TRANSFER_DONE              0x08
#define UX_HCD_STM32_CONTROLLER_FLAG_TRANSFER_ERROR             0x10
#define UX_HCD_STM32_CONTROLLER_LOW_SPEED_DEVICE                0x20
#define UX_HCD_STM32_CONTROLLER_FULL_SPEED_DEVICE               0x40
#define UX_HCD_STM32_CONTROLLER_HIGH_SPEED_DEVICE               0x80
#define UX_HCD_STM32_MAX_PACKET_COUNT                           256

#define UX_HCD_STM32_ED_STATUS_FREE                             0x00
#define UX_HCD_STM32_ED_STATUS_ALLOCATED                        0x01
#define UX_HCD_STM32_ED_STATUS_ABORTED                          0x02
#define UX_HCD_STM32_ED_STATUS_CONTROL_SETUP                    0x03
#define UX_HCD_STM32_ED_STATUS_CONTROL_DATA_IN                  0x04
#define UX_HCD_STM32_ED_STATUS_CONTROL_DATA_OUT                 0x05
#define UX_HCD_STM32_ED_STATUS_CONTROL_STATUS_IN                0x06
#define UX_HCD_STM32_ED_STATUS_CONTROL_STATUS_OUT               0x07
#define UX_HCD_STM32_ED_STATUS_BULK_IN                          0x08
#define UX_HCD_STM32_ED_STATUS_BULK_OUT                         0x09
#define UX_HCD_STM32_ED_STATUS_PERIODIC_TRANSFER                0x0A


/* Define STM32 static definition.  */

#define UX_HCD_STM32_AVAILABLE_BANDWIDTH                        6000


/* Define STM32 structure.  */

typedef struct UX_HCD_STM32_STRUCT
{

    struct UX_HCD_STRUCT                *ux_hcd_stm32_hcd_owner;
    struct UX_HCD_STM32_ED_STRUCT       *ux_hcd_stm32_ed_list;
    struct UX_HCD_STM32_ED_STRUCT       *ux_hcd_stm32_channels_ed[UX_HCD_STM32_MAX_NB_CHANNELS];
    ULONG                               ux_hcd_stm32_nb_channels;
    UINT                                ux_hcd_stm32_queue_empty;
    UINT                                ux_hcd_stm32_periodic_scheduler_active;
    ULONG                               ux_hcd_stm32_controller_flag;
    HCD_HandleTypeDef                   *hcd_handle;
    struct UX_HCD_STM32_ED_STRUCT       *ux_hcd_stm32_periodic_ed_head;
} UX_HCD_STM32;


/* Define STM32 ED structure.  */

typedef struct UX_HCD_STM32_ED_STRUCT
{

    struct UX_HCD_STM32_ED_STRUCT       *ux_stm32_ed_next_ed;
    struct UX_ENDPOINT_STRUCT           *ux_stm32_ed_endpoint;
    struct UX_TRANSFER_STRUCT           *ux_stm32_ed_transfer_request;
    UCHAR                               ux_stm32_ed_status;
    UCHAR                               ux_stm32_ed_channel;
    UCHAR                               ux_stm32_ed_interval_mask;
    UCHAR                               ux_stm32_ed_interval_position;
} UX_HCD_STM32_ED;


#define USBH_PID_SETUP                            0U
#define USBH_PID_DATA                             1U


/* Define STM32 function prototypes.  */

UINT                _ux_hcd_stm32_controller_disable(UX_HCD_STM32 *hcd_stm32);
UX_HCD_STM32_ED *   _ux_hcd_stm32_ed_obtain(UX_HCD_STM32 *hcd_stm32);
UINT                _ux_hcd_stm32_endpoint_create(UX_HCD_STM32 *hcd_stm32, UX_ENDPOINT *endpoint);
UINT                _ux_hcd_stm32_endpoint_destroy(UX_HCD_STM32 *hcd_stm32, UX_ENDPOINT *endpoint);
UINT                _ux_hcd_stm32_endpoint_reset(UX_HCD_STM32 *hcd_stm32, UX_ENDPOINT *endpoint);
UINT                _ux_hcd_stm32_entry(UX_HCD *hcd, UINT function, VOID *parameter);
UINT                _ux_hcd_stm32_frame_number_get(UX_HCD_STM32 *hcd_stm32, ULONG *frame_number);
UINT                _ux_hcd_stm32_initialize(UX_HCD *hcd);
VOID                _ux_hcd_stm32_interrupt_handler(VOID);
UINT                _ux_hcd_stm32_least_traffic_list_get(UX_HCD_STM32 *hcd_stm32);
UINT                _ux_hcd_stm32_periodic_schedule(UX_HCD_STM32 *hcd_stm32);
UINT                _ux_hcd_stm32_port_disable(UX_HCD_STM32 *hcd_stm32, ULONG port_index);
UINT                _ux_hcd_stm32_port_enable(UX_HCD_STM32 *hcd_stm32, ULONG port_index);
UINT                _ux_hcd_stm32_port_reset(UX_HCD_STM32 *hcd_stm32, ULONG port_index);
UINT                _ux_hcd_stm32_port_resume(UX_HCD_STM32 *hcd_stm32, UINT port_index);
ULONG               _ux_hcd_stm32_port_status_get(UX_HCD_STM32 *hcd_stm32, ULONG port_index);
UINT                _ux_hcd_stm32_port_suspend(UX_HCD_STM32 *hcd_stm32, ULONG port_index);
UINT                _ux_hcd_stm32_power_down_port(UX_HCD_STM32 *hcd_stm32, ULONG port_index);
UINT                _ux_hcd_stm32_power_on_port(UX_HCD_STM32 *hcd_stm32, ULONG port_index);
UINT                _ux_hcd_stm32_request_bulk_transfer(UX_HCD_STM32 *hcd_stm32, UX_TRANSFER *transfer_request);
UINT                _ux_hcd_stm32_request_control_transfer(UX_HCD_STM32 *hcd_stm32, UX_TRANSFER *transfer_request);
UINT                _ux_hcd_stm32_request_periodic_transfer(UX_HCD_STM32 *hcd_stm32, UX_TRANSFER *transfer_request);
UINT                _ux_hcd_stm32_request_transfer(UX_HCD_STM32 *hcd_stm32, UX_TRANSFER *transfer_request);
UINT                _ux_hcd_stm32_transfer_abort(UX_HCD_STM32 *hcd_stm32, UX_TRANSFER *transfer_request);

#define ux_hcd_stm32_initialize                      _ux_hcd_stm32_initialize
#define ux_hcd_stm32_interrupt_handler               _ux_hcd_stm32_interrupt_handler


#endif

