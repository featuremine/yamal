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

#include <apr.h> // apr_size_t APR_DECLARE
#include <apr_errno.h> // apr_status_t
#include <apr_strings.h> // apr_cpystrn
#include <ytp/errno.h> // ytp_status_t

static const struct {
  ytp_status_t code;
  const char *msg;
} ytp_error_list[] = {
  {YTP_STATUS_EINVFORMAT,    "Invalid yamal file format"},
  {YTP_STATUS_EEOF,          "Unexpected EOF"},
  {YTP_STATUS_EINVSIZE,      "Invalid size"},
  {YTP_STATUS_EREADONLY,     "Yamal file is readonly"},
  {YTP_STATUS_EMEM,          "Memory error"},
  {YTP_STATUS_EINVOFFSET,    "Invalid offset"},
  {YTP_STATUS_EPEERNOTFOUND, "Peer not found"},
  {YTP_STATUS_ECHNOTFOUND,   "Channel not found"},
  {YTP_STATUS_EPEERANN,      "Invalid peer announcement"},
  {YTP_STATUS_ECHANN,        "Invalid channel announcement"},
  {0,                        NULL}
};

/*
 * stuffbuffer - like apr_cpystrn() but returns the address of the
 * dest buffer instead of the address of the terminating '\0'
 */
static char *stuffbuffer(char *buf, apr_size_t bufsize, const char *s)
{
    apr_cpystrn(buf,s,bufsize);
    return buf;
}

APR_DECLARE(char *) ytp_strerror(ytp_status_t statcode, char *buf, 
                                 apr_size_t bufsize)
{
  if (YTP_STATUS_START <= statcode && statcode < YTP_STATUS_END) {
    for (unsigned i = 0; ytp_error_list[i].msg; ++i)
    {
        if(ytp_error_list[i].code == statcode)
        {
            return stuffbuffer(buf, bufsize, ytp_error_list[i].msg);
        }
    }
    return stuffbuffer(buf, bufsize, "Error string not specified yet");
  }
  else {
    return apr_strerror((apr_status_t)statcode, buf, bufsize);
  }
}