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


/* Include necessary system files.  */

#define UX_SOURCE_CODE
#define UX_HCD_STM32_SOURCE_CODE

#include "ux_api.h"
#include "ux_hcd_stm32.h"
#include "ux_host_stack.h"


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _ux_hcd_stm32_periodic_schedule                     PORTABLE C      */ 
/*                                                           6.0          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Chaoqiong Xiao, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */ 
/*     This function schedules new transfers from the periodic interrupt  */ 
/*     list.                                                              */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    hcd_stm32                           Pointer to STM32 controller     */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    TRUE or FALSE                                                       */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    HAL_HCD_GetCurrentFrame             Get frame number                */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    STM32 Controller Driver                                             */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */ 
/*                                                                        */ 
/*  05-19-2020     Chaoqiong Xiao           Initial Version 6.0           */
/*                                                                        */
/**************************************************************************/
UINT  _ux_hcd_stm32_periodic_schedule(UX_HCD_STM32 *hcd_stm32)
{

UX_HCD_STM32_ED     *ed;
ULONG               frame_number;
UX_TRANSFER         *transfer_request;


    /* Get the current frame number.  */
    frame_number = HAL_HCD_GetCurrentFrame(hcd_stm32 -> hcd_handle);

    /* Get the first ED in the periodic list.  */
    ed =  hcd_stm32 -> ux_hcd_stm32_periodic_ed_head;

    /* Search for an entry in the periodic tree.  */
    while (ed != UX_NULL) 
    {
        
        /* Check if the periodic transfer should be scheduled in this frame.  */
        if ((frame_number & ed -> ux_stm32_ed_interval_mask) == ed -> ux_stm32_ed_interval_position)
        {

            /* Get the transfer request.  */
            transfer_request = ed -> ux_stm32_ed_transfer_request;
            
            /* Check if there is transfer pending.  */
            if (transfer_request)
            {
                
                /* Call HAL driver to submit the transfer request.  */
                HAL_HCD_HC_SubmitRequest(hcd_stm32 -> hcd_handle, ed -> ux_stm32_ed_channel,
                                         (transfer_request -> ux_transfer_request_type & UX_REQUEST_DIRECTION) == UX_REQUEST_IN ? 1 : 0, 
                                         EP_TYPE_INTR, USBH_PID_DATA, 
                                         transfer_request -> ux_transfer_request_data_pointer, 
                                         transfer_request -> ux_transfer_request_requested_length, 0);
            }
        }

        /* Point to the next ED in the list.  */
        ed =  ed -> ux_stm32_ed_next_ed;
    }

    /* Return to caller.  */
    return(UX_FALSE);
}

