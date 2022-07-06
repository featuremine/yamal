/******************************************************************************

        COPYRIGHT (c) 2022 by Featuremine Corporation.
        This software has been provided pursuant to a License Agreement
        containing restrictions on its use.  This software contains
        valuable trade secrets and proprietary information of
        Featuremine Corporation and is protected by law.  It may not be
        copied or distributed in any form or medium, disclosed to third
        parties, reverse engineered or used in any manner not provided
        for in said License Agreement except with the prior written
        authorization from Featuremine Corporation.

 *****************************************************************************/

/**
 * @file errno.h
 * @author Featuremine Corporation
 * @date 10 May 2022
 * @brief File contains C declaration of Yamal error handling
 *
 * @see http://www.featuremine.com
 */

#ifndef __FM_YTP_ERRNO_H__
#define __FM_YTP_ERRNO_H__

#include <apr.h> // apr_size_t APR_DECLARE
#include <apr_errno.h> // apr_status_t

#ifdef __cplusplus
extern "C" {
#endif


typedef apr_status_t ytp_status_t;

// APR user errors start: APR_OS_START_USERERR = 120000
// APR user errors size: APR_OS_ERRSPACE_SIZE * 10 = 500000
// Other projects used error codes
// Subversion   - [APR_OS_START_USERERR:APR_OS_START_USERERR+120000]
// Apache HTTPD - [APR_OS_START_USERERR+2000:APR_OS_START_USERERR+2999]

#define YTP_STATUS_START          (APR_OS_START_USERERR + 200000)

#define YTP_STATUS_EINVFORMAT     (YTP_STATUS_START + 0)
#define YTP_STATUS_EEOF           (YTP_STATUS_START + 1)
#define YTP_STATUS_EINVSIZE       (YTP_STATUS_START + 2)
#define YTP_STATUS_EREADONLY      (YTP_STATUS_START + 3)
#define YTP_STATUS_EMEM           (YTP_STATUS_START + 4)
#define YTP_STATUS_EINVOFFSET     (YTP_STATUS_START + 5)
#define YTP_STATUS_EPEERNOTFOUND  (YTP_STATUS_START + 6)
#define YTP_STATUS_ECHNOTFOUND    (YTP_STATUS_START + 7)
#define YTP_STATUS_EPEERANN       (YTP_STATUS_START + 8)
#define YTP_STATUS_ECHANN         (YTP_STATUS_START + 9)

#define YTP_STATUS_END            (YTP_STATUS_START + 500)
#define YTP_STATUS_SIZE           (YTP_STATUS_END - YTP_STATUS_START)

#define YTP_STATUS_OK             0

/**
 * @brief Return a human readable string describing the specified error.
 *
 * @param[in] statcode The error code to get a string for.
 * @param[out] buf A buffer to hold the error string.
 * @param[in] bufsize Size of the buffer to hold the string.
 * @return the output buffer containing the error message string
 */
APR_DECLARE(char *) ytp_strerror(ytp_status_t statcode, char *buf, 
                                 apr_size_t bufsize);


#ifdef __cplusplus
}
#endif

#endif // __FM_YTP_ERRNO_H__
