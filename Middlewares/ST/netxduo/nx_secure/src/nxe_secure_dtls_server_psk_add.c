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

#include "nx_secure_dtls.h"

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxe_secure_dtls_server_psk_add                     PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for errors when adding a Pre-Shared Key (PSK)  */
/*    to a DTLS server instance.                                          */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    server_ptr                            DTLS server control block     */
/*    pre_shared_key                        Pointer to PSK data           */
/*    psk_length                            Length of PSK data            */
/*    psk_identity                          PSK id data                   */
/*    identity_length                       Length of PSK id data         */
/*    hint                                  PSK hint data                 */
/*    hint_length                           Length of PSK hint data       */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_secure_dtls_server_psk_add        Actual function call          */
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
#if defined(NX_SECURE_ENABLE_PSK_CIPHERSUITES) || defined(NX_SECURE_ENABLE_ECJPAKE_CIPHERSUITE)
UINT _nxe_secure_dtls_server_psk_add(NX_SECURE_DTLS_SERVER *server_ptr, UCHAR *pre_shared_key,
                                    UINT psk_length, UCHAR *psk_identity, UINT identity_length, UCHAR *hint,
                                    UINT hint_length)
{
#ifdef NX_SECURE_ENABLE_DTLS
UINT status;

    if(server_ptr == NX_NULL)
    {
        return(NX_PTR_ERROR);
    }

    /* Call actual function. */
    status = _nx_secure_dtls_server_psk_add(server_ptr, pre_shared_key, psk_length, psk_identity, identity_length, hint, hint_length);

    return(status);
#else
    NX_PARAMETER_NOT_USED(server_ptr);
    NX_PARAMETER_NOT_USED(pre_shared_key);
    NX_PARAMETER_NOT_USED(psk_length);
    NX_PARAMETER_NOT_USED(psk_identity);
    NX_PARAMETER_NOT_USED(identity_length);
    NX_PARAMETER_NOT_USED(hint);
    NX_PARAMETER_NOT_USED(hint_length);

    return(NX_NOT_SUPPORTED);
#endif /* NX_SECURE_ENABLE_DTLS */
}
#endif
