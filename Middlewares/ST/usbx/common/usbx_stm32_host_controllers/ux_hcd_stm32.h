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

/* Define STM32 generic definitions.  */
#define UX_HCD_STM32_FS_NB_CHANNELS                             8

#define UX_HCD_STM32_HS_NB_CHANNELS                             12
#define UX_HCD_STM32_CONTROLLER                                 6
#define UX_HCD_STM32_MAX_NB_CHANNELS                            12
#define UX_HCD_STM32_FRAME_DELAY                                4
#define UX_HCD_STM32_COMMAND_RETRY                              3
#define UX_HCD_STM32_PERIODIC_ENTRY_NB                          32
#define UX_HCD_STM32_PERIODIC_ENTRY_MASK                        0x1f
#define UX_HCD_STM32_NB_ROOT_PORTS                              1
#define UX_HCD_STM32_PORT_ENABLE_TIMEOUT                        5
#define UX_HCD_STM32_NO_CHANNEL_ASSIGNED                        0xffffffff
#define UX_HCD_STM32_CONTROLLER_FLAG_DEVICE_ATTACHED            0x01
#define UX_HCD_STM32_CONTROLLER_FLAG_DEVICE_DETACHED            0x02
#define UX_HCD_STM32_CONTROLLER_FLAG_SOF                        0x04
#define UX_HCD_STM32_CONTROLLER_FLAG_TRANSFER_DONE              0x08
#define UX_HCD_STM32_CONTROLLER_FLAG_TRANSFER_ERROR             0x10
#define UX_HCD_STM32_CONTROLLER_LOW_SPEED_DEVICE                0x20
#define UX_HCD_STM32_CONTROLLER_FULL_SPEED_DEVICE               0x40
#define UX_HCD_STM32_CONTROLLER_HIGH_SPEED_DEVICE               0x80

/* Define STM32 static definition.  */

#define UX_HCD_STM32_AVAILABLE_BANDWIDTH                        6000


/* Define STM32 HAL device speeds.  */

#define HAL_HCD_SPEED_HIGH                                      0
#define HAL_HCD_SPEED_FULL                                      1
#define HAL_HCD_SPEED_LOW                                       2


/* Define STM32 completion code errors.  */

#define UX_HCD_STM32_NO_ERROR                                   0x00

/* Define STM32 structure.  */

typedef struct UX_HCD_STM32_STRUCT
{

    struct UX_HCD_STRUCT                *ux_hcd_stm32_hcd_owner;
    struct UX_HCD_STM32_ED_STRUCT       *ux_hcd_stm32_ed_list;
    struct UX_HCD_STM32_TD_STRUCT       *ux_hcd_stm32_td_list;
    struct UX_HCD_STM32_ISO_TD_STRUCT   *ux_hcd_stm32_iso_td_list;
    struct UX_HCD_STM32_ED_STRUCT       *ux_hcd_stm32_asynch_head_ed;
    struct UX_HCD_STM32_ED_STRUCT       *ux_hcd_stm32_asynch_current_ed;
    struct UX_HCD_STM32_ED_STRUCT       *ux_hcd_stm32_periodic_current_ed;
    struct UX_HCD_STM32_ED_STRUCT       *ux_hcd_stm32_iso_head_ed;
    struct UX_HCD_STM32_ED_STRUCT       *ux_hcd_stm32_interrupt_ed_list[32];
    struct UX_HCD_STM32_ED_STRUCT       *ux_hcd_stm32_channels_ed[UX_HCD_STM32_MAX_NB_CHANNELS];
    ULONG                               ux_hcd_stm32_nb_channels;
    UINT                                ux_hcd_stm32_queue_empty;
    UINT                                ux_hcd_stm32_periodic_scheduler_active;
    UINT                                ux_hcd_stm32_interruptible;
    ULONG                               ux_hcd_stm32_interrupt_count;
    ULONG                               ux_hcd_stm32_controller_flag;
    ULONG                               ux_hcd_stm32_base;
    ULONG                               *ux_hcd_stm32_bdt;
    ULONG                               ux_hcd_stm32_ping_pong_mode;
    ULONG                               ux_hcd_stm32_host_channel_interrupt;
    ULONG                               ux_hcd_stm32_frame_number;
    struct UX_HCD_STM32_ED_STRUCT       *ux_hcd_stm32_scheduled_ed;

    ULONG                               ux_hcd_stm32_port_status;

    HCD_HandleTypeDef                   *hcd_handle;
    struct UX_HCD_STM32_ED_STRUCT       *ux_hcd_stm32_periodic_ed_head;
} UX_HCD_STM32;


/* Define STM32 ED structure.  */

typedef struct UX_HCD_STM32_ED_STRUCT
{

    struct UX_HCD_STM32_TD_STRUCT       *ux_stm32_ed_tail_td;
    struct UX_HCD_STM32_TD_STRUCT       *ux_stm32_ed_head_td;
    struct UX_HCD_STM32_ED_STRUCT       *ux_stm32_ed_next_ed;
    struct UX_HCD_STM32_ED_STRUCT       *ux_stm32_ed_previous_ed;
    ULONG                               ux_stm32_ed_status;
    struct UX_ENDPOINT_STRUCT           *ux_stm32_ed_endpoint;
    ULONG                               ux_stm32_ed_frame;
    ULONG                               ux_stm32_ed_channel;
    ULONG                               ux_stm32_ed_command_retry;
    struct UX_TRANSFER_STRUCT           *ux_stm32_ed_transfer_request;
    UCHAR                               ux_stm32_ed_interval_mask;
    UCHAR                               ux_stm32_ed_interval_position;
    UCHAR                               ux_stm32_ed_type;
} UX_HCD_STM32_ED;


/* Define STM32 ED bitmap.  */

#define UX_HCD_STM32_ED_STATIC                                  0x80000000
#define UX_HCD_STM32_ED_SKIP                                    0x40000000

#define UX_HCD_STM32_ED_CHANNEL_RESET                           0x00000000
#define UX_HCD_STM32_ED_CHANNEL_HALTED                          0x00000001


/* Define STM32 TD structure.  */

typedef struct UX_HCD_STM32_TD_STRUCT
{

    UCHAR                               *ux_stm32_td_buffer;
    ULONG                               ux_stm32_td_length;
    struct UX_HCD_STM32_TD_STRUCT       *ux_stm32_td_next_td;
    struct UX_TRANSFER_STRUCT           *ux_stm32_td_transfer_request;
    struct UX_HCD_STM32_TD_STRUCT       *ux_stm32_td_next_td_transfer_request;
    struct UX_HCD_STM32_ED_STRUCT       *ux_stm32_td_ed;
    ULONG                               ux_stm32_td_actual_length;
    ULONG                               ux_stm32_td_status;
    ULONG                               ux_stm32_td_direction;
    ULONG                               ux_stm32_td_toggle;
} UX_HCD_STM32_TD;


/* Define STM32 TD bitmap.  */

#define UX_HCD_STM32_TD_PHASE_MASK                            0x00070000
#define UX_HCD_STM32_TD_SETUP_PHASE                           0x00010000
#define UX_HCD_STM32_TD_DATA_PHASE                            0x00020000
#define UX_HCD_STM32_TD_STATUS_PHASE                          0x00040000
#define UX_HCD_STM32_TD_OUT                                   0x00000800
#define UX_HCD_STM32_TD_IN                                    0x00001000
#define UX_HCD_STM32_TD_ACK_PENDING                           0x00080000
#define UX_HCD_STM32_TD_TOGGLE_FROM_ED                        0x80000000


#define USBH_PID_SETUP                            0U
#define USBH_PID_DATA                             1U

/* Define STM32 ISOCHRONOUS TD structure.  */

typedef struct UX_HCD_STM32_ISO_TD_STRUCT
{

    UCHAR                               *ux_stm32_iso_td_buffer;
    ULONG                               ux_stm32_iso_td_length;
    struct UX_HCD_STM32_ISO_TD_STRUCT   *ux_stm32_iso_td_next_td;
    struct UX_TRANSFER_STRUCT           *ux_stm32_iso_td_transfer_request;
    struct UX_HCD_STM32_ISO_TD_STRUCT   *ux_stm32_iso_td_next_td_transfer_request;
    struct UX_HCD_STM32_ED_STRUCT       *ux_stm32_iso_td_ed;
    ULONG                               ux_stm32_iso_td_actual_length;
    ULONG                               ux_stm32_iso_td_status;
    ULONG                               ux_stm32_iso_td_direction;
} UX_HCD_STM32_ISO_TD;


/* Define STM32 function prototypes.  */

VOID                _ux_hcd_stm32_asynch_queue_process(UX_HCD_STM32 *hcd_stm32, ULONG channel_index, ULONG channel_interrupt_source);
UINT                _ux_hcd_stm32_asynch_schedule(UX_HCD_STM32 *hcd_stm32);
UINT                _ux_hcd_stm32_controller_disable(UX_HCD_STM32 *hcd_stm32);
UX_HCD_STM32_ED *   _ux_hcd_stm32_ed_obtain(UX_HCD_STM32 *hcd_stm32);
VOID                _ux_hcd_stm32_ed_td_clean(UX_HCD_STM32_ED *ed);
UINT                _ux_hcd_stm32_endpoint_create(UX_HCD_STM32 *hcd_stm32, UX_ENDPOINT *endpoint);
UINT                _ux_hcd_stm32_endpoint_destroy(UX_HCD_STM32 *hcd_stm32, UX_ENDPOINT *endpoint);
UINT                _ux_hcd_stm32_endpoint_reset(UX_HCD_STM32 *hcd_stm32, UX_ENDPOINT *endpoint);
UINT                _ux_hcd_stm32_entry(UX_HCD *hcd, UINT function, VOID *parameter);
UINT                _ux_hcd_stm32_frame_number_get(UX_HCD_STM32 *hcd_stm32, ULONG *frame_number);
VOID                _ux_hcd_stm32_frame_number_set(UX_HCD_STM32 *hcd_stm32, ULONG frame_number);
UINT                _ux_hcd_stm32_initialize(UX_HCD *hcd);
VOID                _ux_hcd_stm32_interrupt_handler(VOID);
VOID                _ux_hcd_stm32_iso_queue_process(UX_HCD_STM32 *hcd_stm32);
UINT                _ux_hcd_stm32_iso_schedule(UX_HCD_STM32 *hcd_stm32);
UINT                _ux_hcd_stm32_isochronous_endpoint_create(UX_HCD_STM32 *hcd_stm32, UX_ENDPOINT *endpoint);
UX_HCD_STM32_ISO_TD *  _ux_hcd_stm32_isochronous_td_obtain(UX_HCD_STM32 *hcd_stm32);
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
VOID                _ux_hcd_stm32_power_root_hubs(UX_HCD_STM32 *hcd_stm32);
UINT                _ux_hcd_stm32_request_bulk_transfer(UX_HCD_STM32 *hcd_stm32, UX_TRANSFER *transfer_request);
UINT                _ux_hcd_stm32_request_control_transfer(UX_HCD_STM32 *hcd_stm32, UX_TRANSFER *transfer_request);
UINT                _ux_hcd_stm32_request_interrupt_transfer(UX_HCD_STM32 *hcd_stm32, UX_TRANSFER *transfer_request);
UINT                _ux_hcd_stm32_request_isochronous_transfer(UX_HCD_STM32 *hcd_stm32, UX_TRANSFER *transfer_request);
UINT                _ux_hcd_stm32_request_transfer(UX_HCD_STM32 *hcd_stm32, UX_TRANSFER *transfer_request);
UINT                _ux_hcd_stm32_transfer_abort(UX_HCD_STM32 *hcd_stm32, UX_TRANSFER *transfer_request);
VOID                _ux_hcd_stm32_channel_halt(UX_HCD_STM32 *hcd_stm32, UX_HCD_STM32_ED *ed);

#define ux_hcd_stm32_initialize_fscore               _ux_hcd_stm32_initialize_fscore
#define ux_hcd_stm32_initialize_hscore               _ux_hcd_stm32_initialize_hscore
#define ux_hcd_stm32_interrupt_handler               _ux_hcd_stm32_interrupt_handler


#endif

