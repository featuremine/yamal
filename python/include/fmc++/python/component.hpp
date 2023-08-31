/******************************************************************************
        COPYRIGHT (c) 2019-2023 by Featuremine Corporation.

        This Source Code Form is subject to the terms of the Mozilla Public
        License, v. 2.0. If a copy of the MPL was not distributed with this
        file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *****************************************************************************/

#pragma once

#include "fmc++/config/config.hpp"
#include "fmc++/python/config.hpp"
#include <fmc++/python/wrapper.hpp>

#include "fmc++/component.hpp"

#include "Python.h"

namespace fmc {

namespace python {

class component {
public:
  typedef struct {
    PyObject_HEAD;
    fmc::component *comp_;
  } py_component;

  struct PyComponentTypeObject {
    PyTypeObject type_;
    std::function<void(fmc_fd, fmc::configs::interface::node &)> f_;
  };

  component(std::function<void(fmc_fd, fmc::configs::interface::node &)> f,
            std::string name, PyObject *mod) {

    component_methods[0] = PyMethodDef{"stop", (PyCFunction)&component_stop,
                                       METH_NOARGS, "Stop component."};
    component_methods[1] =
        PyMethodDef{"alive", (PyCFunction)&component_alive, METH_NOARGS,
                    "Verify if component is alive."};
    component_methods[2] = PyMethodDef{NULL, NULL, 0, NULL};

    component_type.type_ = PyTypeObject{
        PyVarObject_HEAD_INIT(NULL, 0) "component", /* tp_name */
        sizeof(component),                          /* tp_basicsize */
        0,                                          /* tp_itemsize */
        (destructor)&component_dealloc,             /* tp_dealloc */
        0,                                          /* tp_print */
        0,                                          /* tp_getattr */
        0,                                          /* tp_setattr */
        0,                                          /* tp_reserved */
        0,                                          /* tp_repr */
        0,                                          /* tp_as_number */
        0,                                          /* tp_as_sequence */
        0,                                          /* tp_as_mapping */
        0,                                          /* tp_hash  */
        0,                                          /* tp_call */
        0,                                          /* tp_str */
        0,                                          /* tp_getattro */
        0,                                          /* tp_setattro */
        0,                                          /* tp_as_buffer */
        Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,   /* tp_flags */
        "component objects",                        /* tp_doc */
        0,                                          /* tp_traverse */
        0,                                          /* tp_clear */
        0,                                          /* tp_richcompare */
        0,                                          /* tp_weaklistoffset */
        0,                                          /* tp_iter */
        0,                                          /* tp_iternext */
        component_methods,                          /* tp_methods */
        0,                                          /* tp_members */
        0,                                          /* tp_getset */
        0,                                          /* tp_base */
        0,                                          /* tp_dict */
        0,                                          /* tp_descr_get */
        0,                                          /* tp_descr_set */
        0,                                          /* tp_dictoffset */
        0,                                          /* tp_init */
        0,                                          /* tp_alloc */
        component_new,                              /* tp_new */
    };

    component_type.f_ = f; /* My_func */

    PyDateTime_IMPORT;

    if (PyType_Ready(&component_type.type_) < 0) {
      PyErr_SetString(PyExc_RuntimeError,
                      "Unable to initialize component type");
      return;
    }

    Py_INCREF(&component_type.type_);

    if (PyModule_AddObject(mod, name.c_str(),
                           (PyObject *)&component_type.type_) < 0) {
      PyErr_SetString(PyExc_RuntimeError, "Unable to add component to module");
      return;
    }
  }

  static void component_dealloc(PyObject *self);

  static PyObject *component_stop(PyObject *self);

  static PyObject *component_alive(PyObject *self);

  static PyObject *component_new(PyTypeObject *subtype, PyObject *args,
                                 PyObject *kwds);

private:
  std::function<void(fmc_fd, fmc::configs::interface::node &)> f_;
  PyMethodDef component_methods[3];
  PyComponentTypeObject component_type;
};

void component::component_dealloc(PyObject *self) {
  auto typed_self = (py_component *)self;
  if (typed_self->comp_) {
    delete typed_self->comp_;
  }
  Py_TYPE(self)->tp_free(self);
}

PyObject *component::component_stop(PyObject *self) {
  ((py_component *)self)->comp_->stop();
  Py_RETURN_NONE;
};

PyObject *component::component_alive(PyObject *self) {
  fmc_error_t *err;
  if (((py_component *)self)->comp_->alive(&err)) {
    Py_RETURN_TRUE;
  }
  if (err) {
    PyErr_SetString(PyExc_RuntimeError, fmc_error_msg(err));
    return nullptr;
  }
  Py_RETURN_FALSE;
};

PyObject *component::component_new(PyTypeObject *subtype, PyObject *args,
                                   PyObject *kwds) {
  static char *kwlist[] = {(char *)"cfg", NULL};
  PyObject *cfg = nullptr;

  if (!PyArg_ParseTupleAndKeywords(args, kwds, "O", kwlist, &cfg)) {
    return nullptr;
  }

  try {
    fmc::python::configs::node n(fmc::python::object::from_borrowed(cfg));

    auto *self = (component::py_component *)subtype->tp_alloc(subtype, 0);
    if (!self) {
      return nullptr;
    }

    self->comp_ = new fmc::component(
        reinterpret_cast<PyComponentTypeObject *>(subtype)->f_, n);

    return (PyObject *)self;
  } catch (const std::exception &e) {
    PyErr_SetString(PyExc_RuntimeError, e.what());
    return nullptr;
  }
};

} // namespace python

} // namespace fmc
