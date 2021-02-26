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
/*    ux_dcd_stm32.h                                      PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Chaoqiong Xiao, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */ 
/*    This file defines the USB OTG device equivalences for the STM32     */ 
/*    controller.                                                         */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */ 
/*                                                                        */ 
/*  05-19-2020     Chaoqiong Xiao           Initial Version 6.0           */
/*  09-30-2020     Chaoqiong Xiao           Modified comment(s), used ST  */
/*                                            HAL library to drive the    */
/*                                            controller,                 */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/

#ifndef UX_DCD_STM32_H
#define UX_DCD_STM32_H

#include "ux_stm32_config.h"

/* Define STM32 generic equivalences.  */

#define UX_DCD_STM32_SLAVE_CONTROLLER                           0x80
#ifndef UX_DCD_STM32_MAX_ED
#define UX_DCD_STM32_MAX_ED                                     4
#endif /* UX_DCD_STM32_MAX_ED */
#define UX_DCD_STM32_IN_FIFO                                    3


#define UX_DCD_STM32_FLUSH_RX_FIFO                              0x00000010
#define UX_DCD_STM32_FLUSH_TX_FIFO                              0x00000020
#define UX_DCD_STM32_FLUSH_FIFO_ALL                             0x00000010
#define UX_DCD_STM32_ENDPOINT_SPACE_SIZE                        0x00000020
#define UX_DCD_STM32_ENDPOINT_CHANNEL_SIZE                      0x00000020


/* Define USB STM32 physical endpoint status definition.  */

#define UX_DCD_STM32_ED_STATUS_UNUSED                            0
#define UX_DCD_STM32_ED_STATUS_USED                              1
#define UX_DCD_STM32_ED_STATUS_TRANSFER                          2
#define UX_DCD_STM32_ED_STATUS_STALLED                           4

/* Define USB STM32 physical endpoint state machine definition.  */

#define UX_DCD_STM32_ED_STATE_IDLE                               0
#define UX_DCD_STM32_ED_STATE_DATA_TX                            1
#define UX_DCD_STM32_ED_STATE_DATA_RX                            2
#define UX_DCD_STM32_ED_STATE_STATUS_TX                          3
#define UX_DCD_STM32_ED_STATE_STATUS_RX                          4

/* Define USB STM32 endpoint transfer status definition.  */

#define UX_DCD_STM32_ED_TRANSFER_STATUS_IDLE                     0
#define UX_DCD_STM32_ED_TRANSFER_STATUS_SETUP                    1
#define UX_DCD_STM32_ED_TRANSFER_STATUS_IN_COMPLETION            2
#define UX_DCD_STM32_ED_TRANSFER_STATUS_OUT_COMPLETION           3

/* Define USB STM32 physical endpoint structure.  */

typedef struct UX_DCD_STM32_ED_STRUCT 
{

    UCHAR           ux_dcd_stm32_ed_status;
    UCHAR           ux_dcd_stm32_ed_state;
    UCHAR           ux_dcd_stm32_ed_index;
    UCHAR           ux_dcd_stm32_ed_direction;
    struct UX_SLAVE_ENDPOINT_STRUCT             
                    *ux_dcd_stm32_ed_endpoint;
} UX_DCD_STM32_ED;


/* Define USB STM32 DCD structure definition.  */

typedef struct UX_DCD_STM32_STRUCT
{

    struct UX_SLAVE_DCD_STRUCT
                        *ux_dcd_stm32_dcd_owner;
    struct UX_DCD_STM32_ED_STRUCT
                        ux_dcd_stm32_ed[UX_DCD_STM32_MAX_ED];
    PCD_HandleTypeDef   *pcd_handle;
} UX_DCD_STM32;


/* Define USB STM32 DCD prototypes.  */

UINT    _ux_dcd_stm32_endpoint_create(UX_DCD_STM32 *dcd_stm32, UX_SLAVE_ENDPOINT *endpoint);
UINT    _ux_dcd_stm32_endpoint_destroy(UX_DCD_STM32 *dcd_stm32, UX_SLAVE_ENDPOINT *endpoint);
UINT    _ux_dcd_stm32_endpoint_reset(UX_DCD_STM32 *dcd_stm32, UX_SLAVE_ENDPOINT *endpoint);
UINT    _ux_dcd_stm32_endpoint_stall(UX_DCD_STM32 *dcd_stm32, UX_SLAVE_ENDPOINT *endpoint);
UINT    _ux_dcd_stm32_endpoint_status(UX_DCD_STM32 *dcd_stm32, ULONG endpoint_index);
UINT    _ux_dcd_stm32_frame_number_get(UX_DCD_STM32 *dcd_stm32, ULONG *frame_number);
UINT    _ux_dcd_stm32_function(UX_SLAVE_DCD *dcd, UINT function, VOID *parameter);
UINT    _ux_dcd_stm32_initialize_complete(VOID);
VOID    _ux_dcd_stm32_interrupt_handler(VOID);
UINT    _ux_dcd_stm32_transfer_request(UX_DCD_STM32 *dcd_stm32, UX_SLAVE_TRANSFER *transfer_request);
UINT    _ux_dcd_stm32_initialize(ULONG dcd_io, ULONG parameter);
UINT    _ux_dcd_stm32_uninitialize(ULONG dcd_io, ULONG parameter);


#define ux_dcd_stm32_initialize                      _ux_dcd_stm32_initialize
#define ux_dcd_stm32_interrupt_handler               _ux_dcd_stm32_interrupt_handler

#endif

