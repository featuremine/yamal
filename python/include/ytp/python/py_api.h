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
 * @file py_api.h
 * @date 4 Oct 2022
 * @brief File contains C declaration of yamal sequence Python API
 *
 * File contains C declaration of yamal sequence Python API
 * @see http://www.featuremine.com
 */

#pragma once

#include <Python.h>
#include <ytp/api.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef shared_sequence *(*pysharedseqfunc_get)(PyObject *);

struct py_ytp_sequence_api_v1 {
  bool (*channel_check)(PyObject *);
  bool (*peer_check)(PyObject *);
  bool (*sequence_check)(PyObject *);
  bool (*stream_check)(PyObject *);
  pysharedseqfunc_get sequence_shared;
  pysharedseqfunc_get stream_shared;
  ytp_peer_t (*stream_peer_id)(PyObject *);
  ytp_channel_t (*stream_channel_id)(PyObject *);
  pysharedseqfunc_get peer_shared;
  ytp_peer_t (*peer_id)(PyObject *);
  pysharedseqfunc_get channel_shared;
  ytp_channel_t (*channel_id)(PyObject *);
};

struct PyAPIWrapper {
  PyObject_HEAD ytp_sequence_api_v1 *api;
  py_ytp_sequence_api_v1 *py_api;
};

#ifdef __cplusplus
}
#endif
