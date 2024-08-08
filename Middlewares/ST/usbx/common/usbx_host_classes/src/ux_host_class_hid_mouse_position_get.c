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
/**   HID Mouse Client Class                                              */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/


/* Include necessary system files.  */

#define UX_SOURCE_CODE

#include "ux_api.h"
#include "ux_host_class_hid.h"
#include "ux_host_class_hid_mouse.h"
#include "ux_host_stack.h"


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _ux_host_class_hid_mouse_position_get               PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Chaoqiong Xiao, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */ 
/*    This function reads the mouse position and reports it to the user.  */
/*    We have the X and Y coordinates passed by the application.          */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    mouse_instance                        Pointer to mouse instance     */ 
/*    mouse_x_position                      Current Mouse X Position      */ 
/*    mouse_y_position                      Current Mouse Y Position      */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    Completion Status                                                   */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    HID Mouse Class                                                     */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */ 
/*                                                                        */ 
/*  05-19-2020     Chaoqiong Xiao           Initial Version 6.0           */
/*  09-30-2020     Chaoqiong Xiao           Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT  _ux_host_class_hid_mouse_position_get(UX_HOST_CLASS_HID_MOUSE *mouse_instance, 
                                            SLONG *mouse_x_position, 
                                            SLONG *mouse_y_position)
{

UX_HOST_CLASS_HID   *hid;

    /* Get the HID class associated with the HID client. */
    hid = mouse_instance -> ux_host_class_hid_mouse_hid;
    
    /* Ensure the instance is valid.  */
    if (_ux_host_stack_class_instance_verify(_ux_system_host_class_hid_name, (VOID *) hid) != UX_SUCCESS)
    {        

        /* Error trap. */
        _ux_system_error_handler(UX_SYSTEM_LEVEL_THREAD, UX_SYSTEM_CONTEXT_CLASS, UX_HOST_CLASS_INSTANCE_UNKNOWN);

        /* If trace is enabled, insert this event into the trace buffer.  */
        UX_TRACE_IN_LINE_INSERT(UX_TRACE_ERROR, UX_HOST_CLASS_INSTANCE_UNKNOWN, hid, 0, 0, UX_TRACE_ERRORS, 0, 0)

        return(UX_HOST_CLASS_INSTANCE_UNKNOWN);
    }

    /* Report the mouse X position.  */
    *mouse_x_position = mouse_instance -> ux_host_class_hid_mouse_x_position;

    /* Report the mouse Y position.  */
    *mouse_y_position = mouse_instance -> ux_host_class_hid_mouse_y_position;

    /* The status will tell the application there is something valid in the usage/value.  */
    return(UX_SUCCESS);    
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _uxe_host_class_hid_mouse_position_get              PORTABLE C      */
/*                                                           6.3.0        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Chaoqiong Xiao, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks errors in HID mouse position get function call.*/
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    mouse_instance                        Pointer to mouse instance     */
/*    mouse_x_position                      Current Mouse X Position      */
/*    mouse_y_position                      Current Mouse Y Position      */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    Status                                                              */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _ux_host_class_hid_mouse_position_get Get mouse position            */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application                                                         */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  10-31-2023     Chaoqiong Xiao           Initial Version 6.3.0         */
/*                                                                        */
/**************************************************************************/
UINT  _uxe_host_class_hid_mouse_position_get(UX_HOST_CLASS_HID_MOUSE *mouse_instance, 
                                            SLONG *mouse_x_position, 
                                            SLONG *mouse_y_position)
{

    /* Sanity checks.  */
    if ((mouse_instance == UX_NULL) ||
        (mouse_x_position == UX_NULL) || (mouse_y_position == UX_NULL) ||
        (mouse_x_position == mouse_y_position))
    {
        return(UX_INVALID_PARAMETER);
    }

    /* Invoke position get function.  */
    return(_ux_host_class_hid_mouse_position_get(mouse_instance,
                                        mouse_x_position, mouse_y_position));
}
