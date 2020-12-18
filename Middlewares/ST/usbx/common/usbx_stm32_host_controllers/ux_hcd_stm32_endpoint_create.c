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
/*    _ux_hcd_stm32_endpoint_create                       PORTABLE C      */ 
/*                                                           6.0          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Chaoqiong Xiao, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */ 
/*    This function will create an endpoint.                              */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    hcd_stm32                               Pointer to STM32 controller */ 
/*    endpoint                                Pointer to endpoint         */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    Completion Status                                                   */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _ux_hcd_stm32_ed_obtain                 Obtain STM32 ED             */ 
/*    _ux_hcd_stm32_regular_td_obtain         Obtain STM32 regular TD     */ 
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
UINT  _ux_hcd_stm32_endpoint_create(UX_HCD_STM32 *hcd_stm32, UX_ENDPOINT *endpoint)
{

UX_HCD_STM32_ED        *ed;
UX_DEVICE              *device;
ULONG                   channel_index;
UINT                    device_speed;
UINT                    endpoint_type;


    /* Get the pointer to the device.  */
    device =  endpoint -> ux_endpoint_device;

    switch (device -> ux_device_speed)
    {
    case UX_HIGH_SPEED_DEVICE:
        device_speed =  HAL_HCD_SPEED_HIGH;
        break;
    case UX_FULL_SPEED_DEVICE:
        device_speed =  HAL_HCD_SPEED_FULL;
        break;
    case UX_LOW_SPEED_DEVICE:
        device_speed =  HAL_HCD_SPEED_LOW;
        break;
    default:
        return(UX_ERROR);
    }
    
    switch ((endpoint -> ux_endpoint_descriptor.bmAttributes) & UX_MASK_ENDPOINT_TYPE)
    {
    case UX_CONTROL_ENDPOINT:
        endpoint_type =  EP_TYPE_CTRL;
        break;
    case UX_BULK_ENDPOINT:
        endpoint_type =  EP_TYPE_BULK;
        break;
    case UX_INTERRUPT_ENDPOINT:
        endpoint_type =  EP_TYPE_INTR;
       break;
    case UX_ISOCHRONOUS_ENDPOINT:
        endpoint_type =  EP_TYPE_ISOC;
        break;
    default:
        return(UX_FUNCTION_NOT_SUPPORTED);
    }

    /* Obtain a ED for this new endpoint. This ED will live as long as the endpoint is active 
       and will be the container for the tds.  */
    ed =  _ux_hcd_stm32_ed_obtain(hcd_stm32);
    if (ed == UX_NULL)
        return(UX_NO_ED_AVAILABLE);

    /* And get a channel. */
    for (channel_index = 0; channel_index < hcd_stm32 -> ux_hcd_stm32_nb_channels; channel_index++)
    {

        /* Check if that Channel is free.  */
        if (hcd_stm32 -> ux_hcd_stm32_channels_ed[channel_index]  == UX_NULL)
        {
    
            /* We have a channel. Save it. */
            hcd_stm32 -> ux_hcd_stm32_channels_ed[channel_index] = ed;
            
            /* And in the endpoint too. */
            ed -> ux_stm32_ed_channel = channel_index;
            
            /* Done here.  */
            break;

        }
    }
    
    /* Check for channel assignment.  */
    if (ed -> ux_stm32_ed_channel ==  UX_HCD_STM32_NO_CHANNEL_ASSIGNED)
    {
        
        /* Free the ED.  */
        ed -> ux_stm32_ed_status =  UX_UNUSED;

        /* Could not allocate a channel.  */
        return(UX_NO_ED_AVAILABLE);
    }

    if ((endpoint_type == EP_TYPE_INTR) || (endpoint_type == EP_TYPE_ISOC))
    {
        if (endpoint_type == EP_TYPE_ISOC || device_speed == HAL_HCD_SPEED_HIGH)
        {
            ed -> ux_stm32_ed_interval_mask =  (UCHAR)(1 << (endpoint -> ux_endpoint_descriptor.bInterval - 1)) - 1;
        }
        else
        {
            ed -> ux_stm32_ed_interval_mask = endpoint -> ux_endpoint_descriptor.bInterval;
            ed -> ux_stm32_ed_interval_mask |= ed -> ux_stm32_ed_interval_mask >> 1;
            ed -> ux_stm32_ed_interval_mask |= ed -> ux_stm32_ed_interval_mask >> 2;
            ed -> ux_stm32_ed_interval_mask |= ed -> ux_stm32_ed_interval_mask >> 4;
            ed -> ux_stm32_ed_interval_mask >>= 1;
        }

        ed -> ux_stm32_ed_interval_position =  (UCHAR)_ux_hcd_stm32_least_traffic_list_get(hcd_stm32);

        ed -> ux_stm32_ed_next_ed = hcd_stm32 -> ux_hcd_stm32_periodic_ed_head;
        hcd_stm32 -> ux_hcd_stm32_periodic_ed_head = ed;

        hcd_stm32 -> ux_hcd_stm32_periodic_scheduler_active ++;
    }

    /* Attach the ED to the endpoint container.  */
    endpoint -> ux_endpoint_ed =  (VOID *) ed;

    /* Now do the opposite, attach the ED container to the physical ED.  */
    ed -> ux_stm32_ed_endpoint =  endpoint;

    HAL_HCD_HC_Init(hcd_stm32->hcd_handle, 
                    channel_index, 
                    endpoint -> ux_endpoint_descriptor.bEndpointAddress,
                    device -> ux_device_address,
                    device_speed,
                    endpoint_type,
                    endpoint -> ux_endpoint_descriptor.wMaxPacketSize);

    hcd_stm32 -> hcd_handle -> hc[ed -> ux_stm32_ed_channel].toggle_in = 0;
    hcd_stm32 -> hcd_handle -> hc[ed -> ux_stm32_ed_channel].toggle_out = 0;

    /* Return successful completion.  */
    return(UX_SUCCESS);
}

