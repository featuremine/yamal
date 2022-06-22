/******************************************************************************

        COPYRIGHT (c) 2017 by Featuremine Corporation.
        This software has been provided pursuant to a License Agreement
        containing restrictions on its use.  This software contains
        valuable trade secrets and proprietary information of
        Featuremine Corporation and is protected by law.  It may not be
        copied or distributed in any form or medium, disclosed to third
        parties, reverse engineered or used in any manner not provided
        for in said License Agreement except with the prior written
        authorization from Featuremine Corporation.

 *****************************************************************************/

#include <Python.h>

#include <fmc/platform.h>

PyMODINIT_FUNC fm_ytp_py_init(void) FMMODFUNC FMPYMODPUB;
PyMODINIT_FUNC PyInit_ytp(void) FMMODFUNC FMPYMODPUB;

PyMODINIT_FUNC PyInit_ytp(void) { return fm_ytp_py_init(); }
