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
/** NetX Secure Component                                                 */
/**                                                                       */
/**    Datagram Transport Layer Security (DTLS)                           */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define NX_SECURE_SOURCE_CODE


/* Include necessary system files.  */

#include "nx_secure_dtls.h"

/* Bring in externs for caller checking code.  */

NX_SECURE_CALLER_CHECKING_EXTERNS

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxe_secure_dtls_session_delete                     PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for errors in the DTLS session delete call.    */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    dtls_session                          DTLS session control block    */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_secure_dtls_session_delete        Actual DTLS session delete    */
/*                                            call                        */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Timothy Stapko           Initial Version 6.0           */
/*  09-30-2020     Timothy Stapko           Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT _nxe_secure_dtls_session_delete(NX_SECURE_DTLS_SESSION *dtls_session)
{
#ifdef NX_SECURE_ENABLE_DTLS
UINT status;
NX_SECURE_DTLS_SESSION *session_ptr;

    if (dtls_session == NX_NULL)
    {
        return(NX_PTR_ERROR);
    }

    session_ptr = _nx_secure_dtls_created_ptr;
    for (;;)
    {
        if (session_ptr == NX_NULL)
        {

            /* No session created. */
            return(NX_PTR_ERROR);
        }

        if (session_ptr == dtls_session)
        {

            /* Found the session to delete. */
            break;
        }

        session_ptr = session_ptr -> nx_secure_dtls_created_next;

        if (session_ptr == _nx_secure_dtls_created_ptr)
        {

            /* This session is not created. */
            return(NX_PTR_ERROR);
        }
    }

    status =  _nx_secure_dtls_session_delete(dtls_session);

    /* Return completion status.  */
    return(status);
#else
    NX_PARAMETER_NOT_USED(dtls_session);

    return(NX_NOT_SUPPORTED);
#endif /* NX_SECURE_ENABLE_DTLS */
}

