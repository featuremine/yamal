/******************************************************************************
        COPYRIGHT (c) 2019-2023 by Featuremine Corporation.

        This Source Code Form is subject to the terms of the Mozilla Public
        License, v. 2.0. If a copy of the MPL was not distributed with this
        file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *****************************************************************************/

#include <Python.h>

#include <fmc/platform.h>

PyMODINIT_FUNC fm_ytp_py_init(void) FMMODFUNC FMPYMODPUB;
PyMODINIT_FUNC PyInit_ytp(void) FMMODFUNC FMPYMODPUB;

PyMODINIT_FUNC PyInit_ytp(void) { return fm_ytp_py_init(); }
