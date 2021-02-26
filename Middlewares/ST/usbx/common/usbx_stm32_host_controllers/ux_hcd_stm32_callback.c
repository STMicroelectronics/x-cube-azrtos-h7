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
/*    HAL_HCD_Connect_Callback                            PORTABLE C      */
/*                                                           6.0          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Chaoqiong Xiao, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function handles callback from HAL driver.                     */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    hhcd                                  Pointer to HCD handle         */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    Completion Status                                                   */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _ux_utility_semaphore_put             Put semaphore                 */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    stm32 Controller Driver                                             */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Chaoqiong Xiao           Initial Version 6.0           */
/*                                                                        */
/**************************************************************************/
void HAL_HCD_Connect_Callback(HCD_HandleTypeDef *hhcd)
{

UX_HCD              *hcd;
UX_HCD_STM32        *hcd_stm32;


    /* Get the pointer to the HCD & HCD_STM32.  */
    hcd = (UX_HCD*)hhcd -> pData;
    hcd_stm32 = (UX_HCD_STM32*)hcd -> ux_hcd_controller_hardware;

    /* Something happened on the root hub port. Signal it to the root hub     thread.  */
    hcd -> ux_hcd_root_hub_signal[0]++;

    /* The controller has issued a ATTACH Root HUB signal.  */
    hcd_stm32 -> ux_hcd_stm32_controller_flag |= UX_HCD_STM32_CONTROLLER_FLAG_DEVICE_ATTACHED;
    hcd_stm32 -> ux_hcd_stm32_controller_flag &= ~UX_HCD_STM32_CONTROLLER_FLAG_DEVICE_DETACHED;

    /* Wake up the root hub thread.  */
    _ux_utility_semaphore_put(&_ux_system_host -> ux_system_host_enum_semaphore);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    HAL_HCD_Disconnect_Callback                         PORTABLE C      */
/*                                                           6.0          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Chaoqiong Xiao, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function handles callback from HAL driver.                     */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    hhcd                                  Pointer to HCD handle         */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    Completion Status                                                   */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _ux_utility_semaphore_put             Put semaphore                 */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    stm32 Controller Driver                                             */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Chaoqiong Xiao           Initial Version 6.0           */
/*                                                                        */
/**************************************************************************/
void HAL_HCD_Disconnect_Callback(HCD_HandleTypeDef *hhcd)
{

UX_HCD              *hcd;
UX_HCD_STM32        *hcd_stm32;


    /* Get the pointer to the HCD & HCD_STM32.  */
    hcd = (UX_HCD*)hhcd -> pData;
    hcd_stm32 = (UX_HCD_STM32*)hcd -> ux_hcd_controller_hardware;

    /* Something happened on the root hub port. Signal it to the root hub     thread.  */
    hcd -> ux_hcd_root_hub_signal[0]++;

    /* The controller has issued a DETACH Root HUB signal.  */
    hcd_stm32 -> ux_hcd_stm32_controller_flag |= UX_HCD_STM32_CONTROLLER_FLAG_DEVICE_DETACHED;
    hcd_stm32 -> ux_hcd_stm32_controller_flag &= ~UX_HCD_STM32_CONTROLLER_FLAG_DEVICE_ATTACHED;

    /* Wake up the root hub thread.  */
    _ux_utility_semaphore_put(&_ux_system_host -> ux_system_host_enum_semaphore);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    HAL_HCD_Disconnect_Callback                         PORTABLE C      */
/*                                                           6.0          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Chaoqiong Xiao, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function handles callback from HAL driver.                     */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    hhcd                                  Pointer to HCD handle         */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    Completion Status                                                   */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _ux_utility_semaphore_put             Put semaphore                 */
/*    HAL_HCD_HC_SubmitRequest              Submit request                */
/*    HAL_HCD_HC_Halt                       Halt channel                  */
/*    HAL_HCD_HC_GetXferCount               Get transfer count            */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    stm32 Controller Driver                                             */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Chaoqiong Xiao           Initial Version 6.0           */
/*                                                                        */
/**************************************************************************/
void HAL_HCD_HC_NotifyURBChange_Callback(HCD_HandleTypeDef *hhcd, uint8_t chnum, HCD_URBStateTypeDef urb_state)
{

UX_HCD              *hcd;
UX_HCD_STM32        *hcd_stm32;
UX_HCD_STM32_ED     *ed;
UX_TRANSFER         *transfer_request;


    /* Check the URB state.  */
    if (urb_state == URB_DONE || urb_state == URB_STALL || urb_state == URB_ERROR || urb_state == URB_NOTREADY)
    {

        /* Get the pointer to the HCD & HCD_STM32.  */
        hcd = (UX_HCD*)hhcd -> pData;
        hcd_stm32 = (UX_HCD_STM32*)hcd -> ux_hcd_controller_hardware;

        /* Check if driver is still valid.  */
        if (hcd_stm32 == UX_NULL)
            return;

        /* Load the ED for the channel.  */
        ed =  hcd_stm32 -> ux_hcd_stm32_channels_ed[chnum];

        /* Check if ED is still valid.  */
        if (ed == UX_NULL)
        {

            /* Disable channel.  */
            HAL_HCD_HC_Halt(hhcd, chnum);
            return;
        }

        /* Get transfer request.  */
        transfer_request = ed -> ux_stm32_ed_transfer_request;

        /* Check if ED is still valid.  */
        if (transfer_request == UX_NULL)
        {

            /* Disable channel.  */
            HAL_HCD_HC_Halt(hhcd, chnum);
            return;
        }

        /* Check if URB state is not URB_NOTREADY.  */
        if (urb_state != URB_NOTREADY)
        {

            /* Handle URB states.  */
            switch (urb_state)
            {
            case URB_STALL:

                /* Set the completion code to stalled.  */
                transfer_request -> ux_transfer_request_completion_code =  UX_TRANSFER_STALLED;
                break;
            case URB_DONE:

                /* Check the request direction.  */
                if ((((ed->ux_stm32_ed_endpoint -> ux_endpoint_descriptor.bmAttributes) & UX_MASK_ENDPOINT_TYPE) != UX_CONTROL_ENDPOINT) &&
                    (transfer_request -> ux_transfer_request_type & UX_REQUEST_DIRECTION) == UX_REQUEST_IN)
                {

                    /* Get transfer size for receiving direction.  */
                    transfer_request -> ux_transfer_request_actual_length = HAL_HCD_HC_GetXferCount(hcd_stm32 -> hcd_handle, ed -> ux_stm32_ed_channel);
                }

                /* Check if the request is for bulk OUT or control OUT.  */
                if ((((ed -> ux_stm32_ed_endpoint -> ux_endpoint_descriptor.bmAttributes) & UX_MASK_ENDPOINT_TYPE) == UX_BULK_ENDPOINT ||
                     ((ed -> ux_stm32_ed_endpoint -> ux_endpoint_descriptor.bmAttributes) & UX_MASK_ENDPOINT_TYPE) == UX_CONTROL_ENDPOINT) &&
                     (transfer_request -> ux_transfer_request_type & UX_REQUEST_DIRECTION) == UX_REQUEST_OUT)
                {

                    /* Update actual transfer length.  */
                    transfer_request -> ux_transfer_request_actual_length += transfer_request -> ux_transfer_request_packet_length;

                    /* Check if there is more data to send.  */
                    if (transfer_request -> ux_transfer_request_requested_length > transfer_request -> ux_transfer_request_actual_length)
                    {

                        /* Adjust the transmit length.  */
                        transfer_request -> ux_transfer_request_packet_length = UX_MIN(ed->ux_stm32_ed_endpoint->ux_endpoint_descriptor.wMaxPacketSize,
                                transfer_request -> ux_transfer_request_requested_length - transfer_request -> ux_transfer_request_actual_length);

                        /* Submit the transmit request.  */
                        HAL_HCD_HC_SubmitRequest(hcd_stm32 -> hcd_handle, ed -> ux_stm32_ed_channel,
                                 0, EP_TYPE_BULK, USBH_PID_DATA,
                                 transfer_request->ux_transfer_request_data_pointer + transfer_request -> ux_transfer_request_actual_length,
                                 transfer_request -> ux_transfer_request_packet_length, 0);
                        return;
                    }
                }

                /* Set the completion code to SUCCESS.  */
                transfer_request -> ux_transfer_request_completion_code =  UX_SUCCESS;
                break;
            default:
                /* Set the completion code to transfer error.  */
                transfer_request -> ux_transfer_request_completion_code =  UX_TRANSFER_ERROR;
            }

            /* Move to next transfer.  */
            ed -> ux_stm32_ed_transfer_request = transfer_request -> ux_transfer_request_next_transfer_request;

            /* Invoke callback function.  */
            if (transfer_request -> ux_transfer_request_completion_function)
                transfer_request -> ux_transfer_request_completion_function(transfer_request);

            /* Wake up the transfer request thread.  */
            _ux_utility_semaphore_put(&transfer_request -> ux_transfer_request_semaphore);

        }
        else
        {

            /* Handle URB_NOTREADY state here.  */
            /* Check if we need to retry the transfer by checking the status.  */
            if ((ed -> ux_stm32_ed_status == UX_HCD_STM32_ED_STATUS_CONTROL_SETUP) ||
                (ed -> ux_stm32_ed_status == UX_HCD_STM32_ED_STATUS_CONTROL_DATA_OUT) ||
                (ed -> ux_stm32_ed_status == UX_HCD_STM32_ED_STATUS_CONTROL_STATUS_OUT) ||
                (ed -> ux_stm32_ed_status == UX_HCD_STM32_ED_STATUS_BULK_OUT))
                {
                    /* Submit the transmit request.  */
                       HAL_HCD_HC_SubmitRequest(hcd_stm32 -> hcd_handle, ed -> ux_stm32_ed_channel, 0,
                                        ((ed -> ux_stm32_ed_endpoint -> ux_endpoint_descriptor.bmAttributes) & UX_MASK_ENDPOINT_TYPE) == UX_BULK_ENDPOINT ? EP_TYPE_BULK : EP_TYPE_CTRL,
                                         ed -> ux_stm32_ed_status == UX_HCD_STM32_ED_STATUS_CONTROL_SETUP ? USBH_PID_SETUP : USBH_PID_DATA,
                                         transfer_request -> ux_transfer_request_data_pointer + transfer_request -> ux_transfer_request_actual_length,
                                         transfer_request -> ux_transfer_request_packet_length, 0);
                }

        }
    }
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    HAL_HCD_SOF_Callback                                PORTABLE C      */
/*                                                           6.0          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Chaoqiong Xiao, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function handles callback from HAL driver.                     */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    hhcd                                  Pointer to HCD handle         */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    Completion Status                                                   */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _ux_utility_semaphore_put             Put semaphore                 */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    stm32 Controller Driver                                             */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Chaoqiong Xiao           Initial Version 6.0           */
/*                                                                        */
/**************************************************************************/
void HAL_HCD_SOF_Callback(HCD_HandleTypeDef *hhcd)
{

UX_HCD              *hcd;
UX_HCD_STM32        *hcd_stm32;


    /* Get the pointer to the HCD & HCD_STM32.  */
    hcd = (UX_HCD*)hhcd -> pData;
    hcd_stm32 = (UX_HCD_STM32*)hcd -> ux_hcd_controller_hardware;

    if ((hcd_stm32 -> ux_hcd_stm32_controller_flag & UX_HCD_STM32_CONTROLLER_FLAG_SOF) == 0)
    {
        hcd_stm32 -> ux_hcd_stm32_controller_flag |= UX_HCD_STM32_CONTROLLER_FLAG_SOF;
        hcd -> ux_hcd_thread_signal++;

        /* Wake up the scheduler.  */
        _ux_utility_semaphore_put(&_ux_system_host -> ux_system_host_hcd_semaphore);
    }
}
