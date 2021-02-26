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
/*    _ux_hcd_stm32_request_control_transfer              PORTABLE C      */
/*                                                           6.x          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Chaoqiong Xiao, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*     This function performs a control transfer from a transfer request. */
/*     The USB control transfer is in 3 phases (setup, data, status).     */
/*     This function will chain all phases of the control sequence before */
/*     setting the stm32 endpoint as a candidate for transfer.            */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    hcd_stm32                             Pointer to STM32 controller   */
/*    transfer_request                      Pointer to transfer request   */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    Completion Status                                                   */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _ux_hcd_stm32_regular_td_obtain       Obtain regular TD             */
/*    _ux_host_stack_transfer_request_abort Abort transfer request        */
/*    _ux_utility_memory_allocate           Allocate memory block         */
/*    _ux_utility_memory_free               Release memory block          */
/*    _ux_utility_semaphore_get             Get semaphore                 */
/*    _ux_utility_short_put                 Write 16-bit value            */
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
/*  xx-xx-xxxx     Chaoqiong Xiao           Modified comment(s),          */
/*                                            prefixed UX to MS_TO_TICK,  */
/*                                            resulting in version 6.x    */
/*                                                                        */
/**************************************************************************/
UINT  _ux_hcd_stm32_request_control_transfer(UX_HCD_STM32 *hcd_stm32, UX_TRANSFER *transfer_request)
{

UX_ENDPOINT             *endpoint;
UCHAR                   *setup_request;
UX_HCD_STM32_ED         *ed;
UINT                    status;
UINT                    device_speed;
UINT                    saved_requested_length;
UINT                    saved_actual_length;
UINT                    saved_request_type;
UCHAR *                 saved_request_data_pointer;

    /* Get the pointer to the Endpoint.  */
    endpoint =  (UX_ENDPOINT *) transfer_request -> ux_transfer_request_endpoint;

    /* Set device speed.  */
    switch (endpoint -> ux_endpoint_device -> ux_device_speed)
    {
    case UX_HIGH_SPEED_DEVICE:
        device_speed =  HCD_DEVICE_SPEED_HIGH;
        break;
    case UX_FULL_SPEED_DEVICE:
        device_speed =  HCD_DEVICE_SPEED_FULL;
        break;
    case UX_LOW_SPEED_DEVICE:
        device_speed =  HCD_DEVICE_SPEED_LOW;
        break;
    default:
        return(UX_ERROR);
    }

    /* Now get the physical ED attached to this endpoint.  */
    ed =  endpoint -> ux_endpoint_ed;

    /* Save the pending transfer in the ED.  */
    ed -> ux_stm32_ed_transfer_request = transfer_request;

    /* Build the SETUP packet (phase 1 of the control transfer).  */
    setup_request =  _ux_utility_memory_allocate(UX_NO_ALIGN, UX_REGULAR_MEMORY, UX_SETUP_SIZE);
    if (setup_request == UX_NULL)
        return(UX_MEMORY_INSUFFICIENT);

    /* Build the SETUP request.  */
    *setup_request =                            transfer_request -> ux_transfer_request_function;
    *(setup_request + UX_SETUP_REQUEST_TYPE) =  transfer_request -> ux_transfer_request_type;
    *(setup_request + UX_SETUP_REQUEST) =       transfer_request -> ux_transfer_request_function;
    _ux_utility_short_put(setup_request + UX_SETUP_VALUE, transfer_request -> ux_transfer_request_value);
    _ux_utility_short_put(setup_request + UX_SETUP_INDEX, transfer_request -> ux_transfer_request_index);
    _ux_utility_short_put(setup_request + UX_SETUP_LENGTH, (USHORT) transfer_request -> ux_transfer_request_requested_length);

    /* Save the original transfer parameter.  */
    saved_requested_length = transfer_request -> ux_transfer_request_requested_length;
    saved_request_data_pointer = transfer_request -> ux_transfer_request_data_pointer;

    /* Reset requested length for SETUP packet.  */
    transfer_request -> ux_transfer_request_requested_length = 0;

    /* Set the packet length for SETUP packet.  */
    transfer_request -> ux_transfer_request_packet_length = 8;

    /* Change data pointer to setup data buffer.  */
    transfer_request -> ux_transfer_request_data_pointer = setup_request;

    /* Set the current status.  */
    ed -> ux_stm32_ed_status = UX_HCD_STM32_ED_STATUS_CONTROL_SETUP;

    /* Initialize the host channel for SETUP phase.  */
    HAL_HCD_HC_Init(hcd_stm32 -> hcd_handle,
                    ed -> ux_stm32_ed_channel,
                    0,
                    endpoint -> ux_endpoint_device -> ux_device_address,
                    device_speed,
                    EP_TYPE_CTRL,
                    endpoint -> ux_endpoint_descriptor.wMaxPacketSize);

    /* Send the SETUP packet.  */
    HAL_HCD_HC_SubmitRequest(hcd_stm32 -> hcd_handle, ed -> ux_stm32_ed_channel, 0, EP_TYPE_CTRL, USBH_PID_SETUP, setup_request, 8, 0);

    /* Wait for the completion of the transfer request.  */
    status =  _ux_utility_semaphore_get(&transfer_request -> ux_transfer_request_semaphore, MS_TO_TICK(UX_CONTROL_TRANSFER_TIMEOUT));

    /* Restore original data buffer pointer.  */
    transfer_request -> ux_transfer_request_data_pointer = saved_request_data_pointer;

    /* Free the resources.  */
    _ux_utility_memory_free(setup_request);

    /* If the semaphore did not succeed we probably have a time out.  */
    if (status != UX_SUCCESS)
    {

        /* All transfers pending need to abort. There may have been a partial transfer.  */
        _ux_host_stack_transfer_request_abort(transfer_request);

        /* There was an error, return to the caller.  */
        transfer_request -> ux_transfer_request_completion_code =  UX_TRANSFER_TIMEOUT;

        /* If trace is enabled, insert this event into the trace buffer.  */
        UX_TRACE_IN_LINE_INSERT(UX_TRACE_ERROR, UX_TRANSFER_TIMEOUT, transfer_request, 0, 0, UX_TRACE_ERRORS, 0, 0)

        return(UX_TRANSFER_TIMEOUT);
    }

    /* Check the transfer request completion code.  */
    if (transfer_request -> ux_transfer_request_completion_code != UX_SUCCESS)
    {

        /* Return completion to caller.  */
        return(transfer_request -> ux_transfer_request_completion_code);
    }

    /* Check if there is data phase.  */
    if (saved_requested_length)
    {

        /* Check the direction of the transaction.  */
        if ((transfer_request -> ux_transfer_request_type & UX_REQUEST_DIRECTION) == UX_REQUEST_IN)
        {

            /* Re-initialize the host channel to IN direction.  */
            HAL_HCD_HC_Init(hcd_stm32 -> hcd_handle,
                            ed -> ux_stm32_ed_channel,
                            0x80,
                            endpoint -> ux_endpoint_device -> ux_device_address,
                            device_speed,
                            EP_TYPE_CTRL,
                            endpoint -> ux_endpoint_descriptor.wMaxPacketSize);

            /* Set the current status to data IN.  */
            ed -> ux_stm32_ed_status = UX_HCD_STM32_ED_STATUS_CONTROL_DATA_IN;
        }
        else
        {

            /* Set the current status to data OUT.  */
            ed -> ux_stm32_ed_status = UX_HCD_STM32_ED_STATUS_CONTROL_DATA_OUT;
        }

        /* Save the pending transfer in the ED.  */
        ed -> ux_stm32_ed_transfer_request = transfer_request;

        /* Set the transfer to pending.  */
        transfer_request -> ux_transfer_request_completion_code =  UX_TRANSFER_STATUS_PENDING;

        /* Restore requested length.  */
        transfer_request -> ux_transfer_request_requested_length = saved_requested_length;

        /* If the direction is OUT, request size is larger than MPS, and DMA is not used, we need to set transfer length to MPS.  */
        if (((transfer_request -> ux_transfer_request_type & UX_REQUEST_DIRECTION) == UX_REQUEST_OUT) &&
             (transfer_request -> ux_transfer_request_requested_length > endpoint -> ux_endpoint_descriptor.wMaxPacketSize) &&
             (hcd_stm32 -> hcd_handle -> Init.dma_enable == 0))
        {

            /* Set transfer length to MPS.  */
            transfer_request -> ux_transfer_request_packet_length = endpoint -> ux_endpoint_descriptor.wMaxPacketSize;
        }
        else
        {

            /* Keep the original transfer length.  */
            transfer_request -> ux_transfer_request_packet_length = transfer_request -> ux_transfer_request_requested_length;
        }

        /* Reset actual length.  */
        transfer_request -> ux_transfer_request_actual_length = 0;

        /* Submit the transfer request.  */
        HAL_HCD_HC_SubmitRequest(hcd_stm32 -> hcd_handle, ed -> ux_stm32_ed_channel,
                                 (transfer_request -> ux_transfer_request_type & UX_REQUEST_DIRECTION) == UX_REQUEST_IN ? 1 : 0,
                                 EP_TYPE_CTRL, USBH_PID_DATA,
                                 transfer_request -> ux_transfer_request_data_pointer,
                                 transfer_request -> ux_transfer_request_packet_length, 0);

        /* Wait for the completion of the transfer request.  */
        status =  _ux_utility_semaphore_get(&transfer_request -> ux_transfer_request_semaphore, MS_TO_TICK(UX_CONTROL_TRANSFER_TIMEOUT));

        /* If the semaphore did not succeed we probably have a time out.  */
        if (status != UX_SUCCESS)
        {

            /* All transfers pending need to abort. There may have been a partial transfer.  */
            _ux_host_stack_transfer_request_abort(transfer_request);

            /* There was an error, return to the caller.  */
            transfer_request -> ux_transfer_request_completion_code =  UX_TRANSFER_TIMEOUT;

            /* If trace is enabled, insert this event into the trace buffer.  */
            UX_TRACE_IN_LINE_INSERT(UX_TRACE_ERROR, UX_TRANSFER_TIMEOUT, transfer_request, 0, 0, UX_TRACE_ERRORS, 0, 0)

            return(UX_TRANSFER_TIMEOUT);

        }

        /* Check the transfer request completion code.  */
        if (transfer_request -> ux_transfer_request_completion_code != UX_SUCCESS)
        {

            /* Return completion to caller.  */
            return(transfer_request -> ux_transfer_request_completion_code);
        }

        if ((transfer_request -> ux_transfer_request_type & UX_REQUEST_DIRECTION) == UX_REQUEST_IN)
        {

            /* Get the actual transfer length.  */
            transfer_request -> ux_transfer_request_actual_length = HAL_HCD_HC_GetXferCount(hcd_stm32 -> hcd_handle, ed -> ux_stm32_ed_channel);
        }
    }

    /* Setup status phase direction.  */
    HAL_HCD_HC_Init(hcd_stm32 -> hcd_handle,
                ed -> ux_stm32_ed_channel,
                (transfer_request -> ux_transfer_request_type & UX_REQUEST_DIRECTION) == UX_REQUEST_IN ? 0 : 0x80,
                endpoint -> ux_endpoint_device -> ux_device_address,
                device_speed,
                EP_TYPE_CTRL,
                endpoint -> ux_endpoint_descriptor.wMaxPacketSize);

    /* Save the pending transfer in the ED.  */
    ed -> ux_stm32_ed_transfer_request = transfer_request;

    /* Set the transfer to pending.  */
    transfer_request -> ux_transfer_request_completion_code =  UX_TRANSFER_STATUS_PENDING;

    /* Save the original transfer parameter.  */
    saved_requested_length = transfer_request -> ux_transfer_request_requested_length;
    transfer_request -> ux_transfer_request_requested_length = 0;
    saved_actual_length = transfer_request -> ux_transfer_request_actual_length;
    transfer_request -> ux_transfer_request_actual_length = 0;
    saved_request_type = transfer_request -> ux_transfer_request_type;
    transfer_request -> ux_transfer_request_type = (transfer_request -> ux_transfer_request_type & UX_REQUEST_DIRECTION) == UX_REQUEST_IN ? UX_REQUEST_OUT : UX_REQUEST_IN;

    /* Reset the packet length.  */
    transfer_request -> ux_transfer_request_packet_length = 0;

    /* Set the current status to data OUT.  */
    ed -> ux_stm32_ed_status = ((transfer_request -> ux_transfer_request_type & UX_REQUEST_DIRECTION) == UX_REQUEST_IN) ?
                                UX_HCD_STM32_ED_STATUS_CONTROL_STATUS_IN : UX_HCD_STM32_ED_STATUS_CONTROL_STATUS_OUT;

    /* Submit the request for status phase.  */
    HAL_HCD_HC_SubmitRequest(hcd_stm32 -> hcd_handle, ed -> ux_stm32_ed_channel,
                             (transfer_request -> ux_transfer_request_type & UX_REQUEST_DIRECTION) == UX_REQUEST_IN ? 1 : 0,
                             EP_TYPE_CTRL, USBH_PID_DATA, 0, 0, 0);

    /* Wait for the completion of the transfer request.  */
    status =  _ux_utility_semaphore_get(&transfer_request -> ux_transfer_request_semaphore, UX_MS_TO_TICK(UX_CONTROL_TRANSFER_TIMEOUT));

    /* Restore the original transfer parameter.  */
    transfer_request -> ux_transfer_request_requested_length = saved_requested_length;
    transfer_request -> ux_transfer_request_actual_length    = saved_actual_length;
    transfer_request -> ux_transfer_request_type             = saved_request_type;

    /* If the semaphore did not succeed we probably have a time out.  */
    if (status != UX_SUCCESS)
    {

        /* All transfers pending need to abort. There may have been a partial transfer.  */
        _ux_host_stack_transfer_request_abort(transfer_request);

        /* There was an error, return to the caller.  */
        transfer_request -> ux_transfer_request_completion_code =  UX_TRANSFER_TIMEOUT;

        /* If trace is enabled, insert this event into the trace buffer.  */
        UX_TRACE_IN_LINE_INSERT(UX_TRACE_ERROR, UX_TRANSFER_TIMEOUT, transfer_request, 0, 0, UX_TRACE_ERRORS, 0, 0)

    }

    /* Return completion to caller.  */
    return(transfer_request -> ux_transfer_request_completion_code);
}

