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
/*    _ux_hcd_stm32_isochronous_endpoint_create           PORTABLE C      */ 
/*                                                           6.0          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Chaoqiong Xiao, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */ 
/*     This function creates an isochronous endpoint.                     */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    hcd_stm32                             Pointer to STM32 controller   */ 
/*    endpoint                              Pointer to endpoint           */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    Completion Status                                                   */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _ux_hcd_stm32_ed_obtain                 Obtain STM32 ED             */ 
/*    _ux_hcd_stm32_isochronous_td_obtain     Obtain STM32 ISO ED         */ 
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
UINT  _ux_hcd_stm32_isochronous_endpoint_create(UX_HCD_STM32 *hcd_stm32, UX_ENDPOINT *endpoint)
{

UX_HCD_STM32_ED           *ed;
UX_HCD_STM32_ED           *head_ed;
UX_HCD_STM32_ISO_TD       *td;


    /* Obtain a ED for this new endpoint. This ED will live as long as the endpoint is 
       active and will be the container for the TDs.  */
    ed =  _ux_hcd_stm32_ed_obtain(hcd_stm32);
    if (ed == UX_NULL)
        return(UX_NO_ED_AVAILABLE);

    /* Obtain a dummy isoch TD for terminating the ED transfer chain.  */
    td =  _ux_hcd_stm32_isochronous_td_obtain(hcd_stm32);
    if (td == UX_NULL)
    {

        ed -> ux_stm32_ed_status =  UX_UNUSED;
        return(UX_NO_TD_AVAILABLE);
    }

    /* Attach the ED to the endpoint container.  */
    endpoint -> ux_endpoint_ed =  (VOID *) ed;
    
    /* Hook the TD to both the tail and head of the ED.  */
    ed -> ux_stm32_ed_tail_td =  (UX_HCD_STM32_TD *) ((void *) td);
    ed -> ux_stm32_ed_head_td =  (UX_HCD_STM32_TD *) ((void *) td);

    /* Attach this ED to the iso list.  */
    head_ed =  hcd_stm32 -> ux_hcd_stm32_iso_head_ed;
    ed -> ux_stm32_ed_next_ed =  head_ed;
    hcd_stm32 -> ux_hcd_stm32_iso_head_ed =  ed;

    /* Build the back chaining pointer. The previous head ED needs to know about the
       inserted ED. */
    if (head_ed != UX_NULL)
        head_ed -> ux_stm32_ed_previous_ed =  ed;
    
    /* Return successful completion.  */
    return(UX_SUCCESS);         
}

