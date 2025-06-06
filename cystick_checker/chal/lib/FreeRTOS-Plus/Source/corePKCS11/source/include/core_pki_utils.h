/*
 * corePKCS11 v3.6.2
 * Copyright (C) 2024 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * SPDX-License-Identifier: MIT
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef _CORE_PKI_UTILS_H_
#define _CORE_PKI_UTILS_H_

#include <stdint.h>
#include <stddef.h>

/* *INDENT-OFF* */
#ifdef __cplusplus
    extern "C" {
#endif
/* *INDENT-ON* */

/**
 * @file core_pki_utils.h
 * @brief Helper functions for PKCS #11
 */

/**
 * @brief Converts an ECDSA P-256 signature from the format provided by mbedTLS
 * to the format expected by PKCS #11.
 *
 * For P-256 signatures, PKCS #11 expects a 64 byte signature, in the
 * format of 32 byte R component followed by 32 byte S component.
 *
 * mbedTLS provides signatures in DER encoded, zero-padded format.
 *
 * @param[out] pxSignaturePKCS            Pointer to a 64 byte buffer
 *                                        where PKCS #11 formatted signature
 *                                        will be placed.  Caller must
 *                                        allocate 64 bytes of memory.
 * @param[in] pxMbedSignature             Pointer to DER encoded ECDSA
 *                                        signature. Buffer size is expected to be 72 bytes.
 *
 * \return 0 on success, -1 on failure.
 */
/* @[declare_pkcs11_utils_pkimbedtlssignaturetopkcs11signature] */
int8_t PKI_mbedTLSSignatureToPkcs11Signature( uint8_t * pxSignaturePKCS,
                                              const uint8_t * pxMbedSignature );
/* @[declare_pkcs11_utils_pkimbedtlssignaturetopkcs11signature] */



/**
 * @brief Converts and ECDSA P-256 signature from the format provided by PKCS #11
 * to an ASN.1 formatted signature.
 *
 * For P-256 signature, ASN.1 formatting has the format
 *
 * SEQUENCE LENGTH
 *   INTEGER LENGTH R-VALUE
 *   INTEGER LENGTH S-VALUE
 *
 * @param[in,out] pucSig     This pointer serves dual purpose.
 *                           It should both contain the 64-byte PKCS #11
 *                           style signature on input, and will be modified
 *                           to hold the ASN.1 formatted signature (max length
 *                           72 bytes).  It is the responsibility of the caller
 *                           to guarantee that this pointer is large enough to
 *                           hold the (longer) formatted signature.
 *@param[out] pxSigLen       Pointer to the length of the ASN.1 formatted signature.
 *
 * \return 0 if successful, -1 on failure.
 *
 */
/* @[declare_pkcs11_utils_pkipkcs11signaturetombedtlssignature] */
int8_t PKI_pkcs11SignatureTombedTLSSignature( uint8_t * pucSig,
                                              size_t * pxSigLen );
/* @[declare_pkcs11_utils_pkipkcs11signaturetombedtlssignature] */

/* *INDENT-OFF* */
#ifdef __cplusplus
    }
#endif
/* *INDENT-ON* */

#endif /* ifndef _CORE_PKI_UTILS_H_ */
