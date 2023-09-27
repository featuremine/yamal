/******************************************************************************
        COPYRIGHT (c) 2019-2023 by Featuremine Corporation.

        This Source Code Form is subject to the terms of the Mozilla Public
        License, v. 2.0. If a copy of the MPL was not distributed with this
        file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *****************************************************************************/

#include <Python.h>

#define ADD_PY_CLASS(C, N, MOD)                                                \
  if (PyType_Ready(&C) < 0)                                                    \
    return NULL;                                                               \
  Py_INCREF(&C);                                                               \
  PyModule_AddObject(MOD, N, (PyObject *)&C)
