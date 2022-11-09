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
 * @file py_wrapper.h
 * @author Federico Ravchina
 * @date 13 Dec 2021
 * @brief File contains C definitions of the python ytp interface
 */

#pragma once

#include <fmc/platform.h>
#include <ytp/sequence.h>

#include <Python.h>

#ifdef __cplusplus
extern "C" {
#endif

FMMODFUNC bool PyYTPSequence_Check(PyObject *obj);
FMMODFUNC ytp_sequence_shared_t *PyYTPSequence_Shared(PyObject *obj);

FMMODFUNC bool PyYTPPeer_Check(PyObject *obj);
FMMODFUNC ytp_sequence_shared_t *PyYTPPeer_Shared(PyObject *obj);
FMMODFUNC ytp_peer_t PyYTPPeer_Id(PyObject *obj);

FMMODFUNC bool PyYTPChannel_Check(PyObject *obj);
FMMODFUNC ytp_sequence_shared_t *PyYTPChannel_Shared(PyObject *obj);
FMMODFUNC ytp_channel_t PyYTPChannel_Id(PyObject *obj);

FMMODFUNC bool PyYTPStream_Check(PyObject *obj);
FMMODFUNC ytp_sequence_shared_t *PyYTPStream_Shared(PyObject *obj);
FMMODFUNC ytp_peer_t PyYTPStream_PeerId(PyObject *obj);
FMMODFUNC ytp_channel_t PyYTPStream_ChannelId(PyObject *obj);

PyMODINIT_FUNC fm_ytp_py_init(void) FMMODFUNC;

#ifdef __cplusplus
}
#endif
