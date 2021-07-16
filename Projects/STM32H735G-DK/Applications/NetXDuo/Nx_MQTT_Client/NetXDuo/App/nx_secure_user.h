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
/**    Transport Layer Security (TLS)                                     */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

/**************************************************************************/
/*                                                                        */
/*  PORT SPECIFIC C INFORMATION                            RELEASE        */
/*                                                                        */
/*    nx_secure_user.h                                    PORTABLE C      */
/*                                                           6.0          */
/*                                                                        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This file contains user defines for configuring NetX Secure in      */
/*    specific ways. This file will have an effect only if the            */
/*    application and NetX Secure library are built with                  */
/*    NX_SECURE_INCLUDE_USER_DEFINE_FILE defined.                         */
/*    Note that all the defines in this file may also be made on the      */
/*    command line when building NetX library and application objects.    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Timothy Stapko           Initial Version 6.0           */
/*                                                                        */
/**************************************************************************/

#ifndef NX_SECURE_USER_H
#define NX_SECURE_USER_H

/* Define various build options for the NetX Secure port.  The application should either make changes
   here by commenting or un-commenting the conditional compilation defined OR supply the defines
   though the compiler's equivalent of the -D option.  */

/* Override various options with default values already assigned in nx_secure_tls.h */

/* Defined, this option gives the maximum RSA modulus expected, in bits. The
   default value is 4096 for a 4096-bit modulus. Other values can be 3072,
   2048, or 1024 (not recommended). */
/*
#define NX_CRYPTO_MAX_RSA_MODULUS_SIZE      4096
*/

/* Defined, this option enables extra security features required for
   FIPS-Compliant usage. This option is not enabled for non-FIPS build. */
/*
#define NX_CRYPTO_FIPS
*/

/* This option defines cbc max block size. The default value is 16.*/
/*
#define NX_CRYPTO_CBC_MAX_BLOCK_SIZE         16
*/

/* This option defines the drbg block length.
   The default value is NX_CRYPTO_DRBG_BLOCK_LENGTH_AES.*/
/*
#define NX_CRYPTO_DRBG_BLOCK_LENGTH         NX_CRYPTO_DRBG_BLOCK_LENGTH_AES
*/

/* This option defines the drbg seed buffer length. The default value is 256.*/
/*
#define NX_CRYPTO_DRBG_SEED_BUFFER_LEN         256
*/

/* This option defines the drbg max entropy length. The default value is 125.*/
/*
#define NX_CRYPTO_DRBG_MAX_ENTROPY_LEN         125
*/

/* This option defines the drbg max seed life. The default value is 100000.*/
/*
#define NX_CRYPTO_DRBG_MAX_SEED_LIFE         100000
*/

/* Defined, this option enables drbg mutex get.*/
/*
#define NX_CRYPTO_DRBG_MUTEX_GET
*/

/* Defined, this option enables drbg mutex put.*/
/*
#define NX_CRYPTO_DRBG_MUTEX_PUT
*/

/* This option define drbg use df.*/
/*
#define NX_CRYPTO_DRBG_USE_DF         1
*/

/* This option defines drbg prediction resistance.*/
/*
#define NX_CRYPTO_DRBG_PREDICTION_RESISTANCE         1
*/

/* Defined, this option enables crypto hardware random initialize.*/
/*
#define NX_CRYPTO_HARDWARE_RAND_INITIALIZE
*/

/* This option defines crypto ecdsa scratch buffer size.*/
/*
#define NX_CRYPTO_ECDSA_SCRATCH_BUFFER_SIZE         3016
*/

/* This option defines ecdh scratch buffer size.*/
/*
#define NX_CRYPTO_ECDH_SCRATCH_BUFFER_SIZE         2464
*/

/* This option defines the ecjpake scratch buffer size.*/
/*
#define NX_CRYPTO_ECJPAKE_SCRATCH_BUFFER_SIZE         4096
*/

/* This option defines the hmac max pad size.*/
/*
#define NX_CRYPTO_HMAC_MAX_PAD_SIZE         128
*/

/* This option defines the huge number bits.*/
/*
#define NX_CRYPTO_HUGE_NUMBER_BITS         32
*/

/* Defined, this option allows TLS to accept self-signed certificates from
   a remote host. By default, TLS will reject self-signed server certificates
   as a security precaution. If this macro is defined, self-signed certificates
   must still be added to the trusted store to be accepted. */
/*
#define NX_SECURE_ALLOW_SELF_SIGNED_CERTIFICATES
*/

/* Defined, this option enables the optional X.509 Client Certificate
   Verification for TLS Servers4. */
/*
#define NX_SECURE_ENABLE_CLIENT_CERTIFICATE_VERIFY
*/

/* Defined, this option removes all TLS logic for Elliptic Curve Cryptography
  (ECC) ciphersuites. These ciphersuites are optional in TLS 1.2 and earlier
  and disabling them can result in significant code and data size reduction. */
/*
#define NX_SECURE_DISABLE_ECC_CIPHERSUITE
*/

/* Defined, this option disables TLSv1.1 mode. It is defined by default.
   TLSv1.1 is disabled in favor of using only the more-secure TLSv1.25. */
/*
#define NX_SECURE_TLS_DISABLE_TLS_1_1
*/

/* Defined, this option enables the legacy TLSv1.0 mode. TLSv1.0 is considered
   obsolete so it should only be enabled for backward-compatibility with older
   applications. */
/*
#define NX_SECURE_TLS_ENABLE_TLS_1_0
*/

/* Defined, this option enables the legacy TLSv1.1 mode. TLSv1.1 is considered
   obsolete so it should only be enabled for backward-compatibility with older
   applications.*/
/*
#define NX_SECURE_TLS_ENABLE_TLS_1_1
*/

/* Defined, this option enables TLSv1.3 mode. TLS 1.3 is the newest version of
   TLS and is disabled by default.*/
/*
#define NX_SECURE_TLS_ENABLE_TLS_1_3
*/

/* Defined, this option enables protocol version downgrade for TLS client.*/
/*
#define NX_SECURE_TLS_DISABLE_PROTOCOL_VERSION_DOWNGRADE
*/

/* Defined, this option enables AEAD ciphersuites other than
   AES-CCM or AES-GCM working.*/
/*
#define NX_SECURE_AEAD_CIPHER_CHECK
*/

/* Defined, this option disables X509 feature.*/
/*
#define NX_SECURE_DISABLE_X509
*/

/* This option defines the length of DTLS cookie. */
/*
#define NX_SECURE_DTLS_COOKIE_LENGTH         32
*/

/* This option defines the maximum re-transmit retries for
   DTLS handshake packet. */
/*
#define NX_SECURE_DTLS_MAXIMUM_RETRANSMIT_RETRIES         10
*/

/* This option defines the maximum DTLS re-transmit rate. The default value is
   60 * NX_IP_PERIODIC_RATE. */
/*
#define NX_SECURE_DTLS_MAXIMUM_RETRANSMIT_TIMEOUT         (60 * NX_IP_PERIODIC_RATE)
*/

/* This option defines how the re-transmit timeout period changes between
   successive retries.   If this value is 0, the initial re-transmit timeout
   is the same as subsequent re-transmit timeouts. If this value is 1,
   each successive re-transmit is twice as long.  */
/*
#define NX_SECURE_DTLS_RETRANSMIT_RETRY_SHIFT         1
*/

/* This option defines the initial DTLS re-transmit rate. */
/*
#define NX_SECURE_DTLS_RETRANSMIT_TIMEOUT        NX_IP_PERIODIC_RATE
*/

/* Defined, this option enables AEAD ciphersuites. */
/*
#define NX_SECURE_ENABLE_AEAD_CIPHER
*/

/* Defined, this option enables ECJPAKE ciphersuites for DTLS. */
/*
#define NX_SECURE_ENABLE_ECJPAKE_CIPHERSUITE
*/

/* Defined, this option enables key related materials cleanup
   when they are not used anymore. */
/*
#define NX_SECURE_KEY_CLEAR
*/

/* This option defines the memory compare function. */
/*
#define NX_SECURE_MEMCMP         "memcmp"
*/

/* This option defines the memory copy function. */
/*
#define NX_SECURE_MEMCPY         "memcpy"
*/

/* This option defines the memory move function. */
/*
#define NX_SECURE_MEMMOVE         "memmove"
*/

/* This option defines the memory set function. */
/*
#define NX_SECURE_MEMSET         "memset"
*/

/* This option enables module integrity self test. */
/*
#define NX_SECURE_POWER_ON_SELF_TEST_MODULE_INTEGRITY_CHECK
*/

/* This option defines the random number check for duplication. */
/*
#define NX_SECURE_RNG_CHECK_COUNT         3
*/

/* This option defines the maximum size of PSK ID. */
/*
#define NX_SECURE_TLS_MAX_PSK_ID_SIZE         20
*/

/* This option defines the maximum PSK keys. */
/*
#define NX_SECURE_TLS_MAX_PSK_KEYS         5
*/

/* This option defines the maximum size of PSK. */
/*
#define NX_SECURE_TLS_MAX_PSK_SIZE         64
*/

/* This option defines a minimum reasonable size for a TLS X509 certificate.
   This is used in checking for * errors in allocating certificate space.
   The size is determined by assuming a 512-bit RSA key, MD5 hash, and
   a rough estimate of other data. It is theoretically possible for a
   real certificate to be smaller, but in that case, bypass the error
   checking by re-defining this macro.
   Approximately: 64(RSA) + 16(MD5) + 176(ASN.1 + text data, common name, etc) */
/*
#define NX_SECURE_TLS_MINIMUM_CERTIFICATE_SIZE         256
*/

/* This option defines the minimum size for the TLS message buffer.
   It is determined by a number of factors, but primarily the expected size
   of the TLS handshake Certificate message (sent by the TLS server)
   that may contain multiple certificates of 1-2KB each. The upper limit
   is determined by the length field in the TLS header (16 bit), and is 64KB. */
/*
#define NX_SECURE_TLS_MINIMUM_MESSAGE_BUFFER_SIZE         4000
*/

/* This option defines the sie of pre-master secret.
   The pre-master secret should be at least 66 bytes for ECDH/ECDHE with
   secp521r1.
   The pre-master secret is 48 bytes, except for PSK ciphersuites for which
   it may be more.*/
/*
#define NX_SECURE_TLS_PREMASTER_SIZE         48
*/

/* This option disables Server Name Indication (SNI) extension. */
/*
#define NX_SECURE_TLS_SNI_EXTENSION_DISABLED
*/

/* This option enables SCSV ciphersuite in ClientHello message. */
/*
#define NX_SECURE_TLS_USE_SCSV_CIPHPERSUITE
*/

/* This option disables X509 Certificate Revocation List check. */
/*
#define NX_SECURE_X509_DISABLE_CRL
*/

/* This macro must be defined to enable DTLS logic in NetX Secure. */
/*
#define NX_SECURE_ENABLE_DTLS
*/

/* Defined, this option removes the basic NetX Secure error checking.
   It is typically used after the application has been debugged. */
/*
#define NX_SECURE_DISABLE_ERROR_CHECKING
*/

/* Defined, this option removes all TLS/DTLS stack code related to
   Client mode, reducing code and data usage. */
/*
#define NX_SECURE_TLS_CLIENT_DISABLED
*/

/* Defined, this option enables Pre-Shared Key (PSK) functionality.
   It does not disable digital certificates. */
/*
#define NX_SECURE_ENABLE_PSK_CIPHERSUITES
*/

/* Defined, this option enables strict distinguished name comparison for
   X.509 certificates for certificate searching and verification. The default
   is to only compare the Common Name fields of the Distinguished Names. */
/*
#define NX_SECURE_X509_STRICT_NAME_COMPARE
*/

/* Defined, this option enables the optional X.509 Distinguished Name fields,
   at the expense of extra memory use for X.509 certificates. */
/*
#define NX_SECURE_X509_USE_EXTENDED_DISTINGUISHED_NAMES
*/

/* Defined, this option enables TLS initialisation. */
/*
#define NX_SECURE_DTLS_INIT
*/

/* Defined, this option enables the ECC support in TLS. */
/*
#define NX_SECURE_ENABLE_ECC_CIPHERSUITE
*/

/* Defined, this option enables SSL 3.0 . */
/*
#define NX_SECURE_TLS_ENABLE_SSL_3_0
*/

/* Defined, this option enables TLS 1.2 .*/
#define NX_SECURE_TLS_TLS_1_2_ENABLED

/* Defined, this option enables TLS 1.3 .*/
/*
*#define NX_SECURE_TLS_TLS_1_3_ENABLED
*/

/* This option defines the TLS maximum psk monce size.
   The default value is 255.*/
/*
#define NX_SECURE_TLS_MAX_PSK_NONCE_SIZE         255
*/

/* Defined, this option enables secure renegotiation.*/
/*
#define NX_SECURE_TLS_ENABLE_SECURE_RENEGOTIATION
*/

/* Defined, this option enables TLS initialisation.*/
/*
#define NX_SECURE_TLS_INIT
*/

/* Defined, this option enables certificate verify extension.*/
/*
#define NX_SECURE_X509_CERTIFICATE_VERIFY_EXTENSION
*/

/* Defined, this option enables X509 parse crl extension.*/
/*
#define NX_SECURE_X509_PARSE_CRL_EXTENSION
*/

/* Defined, this option enables X509 crl verify extension.*/
/*
#define NX_SECURE_X509_CRL_VERIFY_EXTENSION
*/

/* Defined, this option enables X509 certificate initialize extension.*/
/*
#define NX_SECURE_X509_CERTIFICATE_INITIALIZE_EXTENSION
*/

/* Defined, this option removes all TLS stack code
   related to TLS Server mode, reducing code and data usage.*/
/*
#define NX_SECURE_TLS_SERVER_DISABLED
*/

#endif /* NX_SECURE_USER_H */
