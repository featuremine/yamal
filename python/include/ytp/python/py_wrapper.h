/******************************************************************************
        COPYRIGHT (c) 2019-2023 by Featuremine Corporation.

        This Source Code Form is subject to the terms of the Mozilla Public
        License, v. 2.0. If a copy of the MPL was not distributed with this
        file, You can obtain one at https://mozilla.org/MPL/2.0/.
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
