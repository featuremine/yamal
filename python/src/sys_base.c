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

#define PY_SSIZE_T_CLEAN

#include <Python.h>

#include <fmc/component.h>
#include <fmc/config.h>
#include <fmc/files.h>
#include <fmc/reactor.h>

#include <ytp/version.h>

struct Sys {
  PyObject_HEAD;
  struct fmc_component_sys sys;
  bool initialized;
};

struct Component {
  PyObject_HEAD;
  struct fmc_component *component;
};

struct Component_list {
  struct Component *comp;
  struct Component_list *next;
};

struct Reactor {
  PyObject_HEAD;
  struct fmc_reactor reactor;
  struct Component_list *comp_list;
  bool initialized;
};

static PyObject *Sys_new(PyTypeObject *subtype, PyObject *args, PyObject *kwds);
static int Sys_init(struct Sys *self, PyObject *args, PyObject *kwds);
static void Sys_dealloc(struct Sys *self);
static PyObject *Sys_get_paths(struct Sys *self, PyObject *args,
                               PyObject *kwds);
static PyObject *Sys_set_paths(struct Sys *self, PyObject *args,
                               PyObject *kwds);
static PyObject *Sys_get_module(struct Sys *self, PyObject *args,
                                PyObject *kwds);
static PyObject *Sys_get_component_type(struct Sys *self, PyObject *args,
                                        PyObject *kwds);
static PyObject *Sys_get_component_type_spec(struct Sys *self, PyObject *args,
                                             PyObject *kwds);
static PyObject *Sys_get_module_filepath(struct Sys *self, PyObject *args,
                                         PyObject *kwds);

static PyObject *Component_new(PyTypeObject *subtype, PyObject *args,
                               PyObject *kwds);
static int Component_init(struct Component *self, PyObject *args,
                          PyObject *kwds);
static void Component_dealloc(struct Component *self);
bool Component_Check(PyObject *obj);
static PyObject *Component_out_idx(struct Component *self, PyObject *args,
                                   PyObject *kwds);
static PyObject *Component_out_sz(struct Component *self);

static PyObject *Reactor_new(PyTypeObject *subtype, PyObject *args,
                             PyObject *kwds);
static int Reactor_init(struct Reactor *self, PyObject *args, PyObject *kwds);
static void Reactor_dealloc(struct Reactor *self);
static PyObject *Reactor_run(struct Reactor *self, PyObject *args,
                             PyObject *kwds);
static PyObject *Reactor_run_once(struct Reactor *self, PyObject *args,
                                  PyObject *kwds);
static PyObject *Reactor_stop(struct Reactor *self, PyObject *args,
                              PyObject *kwds);
static PyObject *Reactor_sched(struct Reactor *self, PyObject *args,
                               PyObject *kwds);

static PyMethodDef Sys_methods[] = {
    {"get_paths", (PyCFunction)Sys_get_paths, METH_VARARGS | METH_KEYWORDS,
     "Not implemented."},
    {"set_paths", (PyCFunction)Sys_set_paths, METH_VARARGS | METH_KEYWORDS,
     "Not implemented."},
    {"get_module", (PyCFunction)Sys_get_module, METH_VARARGS | METH_KEYWORDS,
     "Not implemented."},
    {"get_component_type", (PyCFunction)Sys_get_component_type,
     METH_VARARGS | METH_KEYWORDS, "Not implemented."},
    {"get_component_type_spec", (PyCFunction)Sys_get_component_type_spec,
     METH_VARARGS | METH_KEYWORDS, "Not implemented."},
    {"get_module_filepath", (PyCFunction)Sys_get_module_filepath,
     METH_VARARGS | METH_KEYWORDS, "Not implemented."},
    {NULL, NULL, 0, NULL} /* Sentinel */
};

static PyMethodDef Reactor_methods[] = {
    {"run", (PyCFunction)Reactor_run, METH_VARARGS | METH_KEYWORDS,
     "Not implemented."},
    {"run_once", (PyCFunction)Reactor_run_once, METH_VARARGS | METH_KEYWORDS,
     "Not implemented."},
    {"stop", (PyCFunction)Reactor_stop, METH_VARARGS | METH_KEYWORDS,
     "Not implemented."},
    {"sched", (PyCFunction)Reactor_sched, METH_VARARGS | METH_KEYWORDS,
     "Not implemented."},
    {NULL, NULL, 0, NULL} /* Sentinel */
};

static PyMethodDef Component_methods[] = {
    {"out_idx", (PyCFunction)Component_out_idx, METH_VARARGS | METH_KEYWORDS,
     "Not implemented."},
    {"out_sz", (PyCFunction)Component_out_sz, METH_NOARGS, "Not implemented."},
    {NULL, NULL, 0, NULL} /* Sentinel */
};

static PyTypeObject SysType = {
    PyVarObject_HEAD_INIT(NULL, 0) "yamal.sys_base.Sys", /* tp_name */
    sizeof(struct Sys),                                  /* tp_basicsize */
    0,                                                   /* tp_itemsize */
    (destructor)Sys_dealloc,                             /* tp_dealloc */
    0,                                                   /* tp_print */
    0,                                                   /* tp_getattr */
    0,                                                   /* tp_setattr */
    0,                                                   /* tp_reserved */
    0,                                                   /* tp_repr */
    0,                                                   /* tp_as_number */
    0,                                                   /* tp_as_sequence */
    0,                                                   /* tp_as_mapping */
    0,                                                   /* tp_hash  */
    0,                                                   /* tp_call */
    0,                                                   /* tp_str */
    0,                                                   /* tp_getattro */
    0,                                                   /* tp_setattro */
    0,                                                   /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,            /* tp_flags */
    "FMC Component sys class",                           /* tp_doc */
    0,                                                   /* tp_traverse */
    0,                                                   /* tp_clear */
    0,                                                   /* tp_richcompare */
    0,                                                   /* tp_weaklistoffset */
    0,                                                   /* tp_iter */
    0,                                                   /* tp_iternext */
    Sys_methods,                                         /* tp_methods */
    0,                                                   /* tp_members */
    0,                                                   /* tp_getset */
    0,                                                   /* tp_base */
    0,                                                   /* tp_dict */
    0,                                                   /* tp_descr_get */
    0,                                                   /* tp_descr_set */
    0,                                                   /* tp_dictoffset */
    (initproc)Sys_init,                                  /* tp_init */
    0,                                                   /* tp_alloc */
    Sys_new,                                             /* tp_new */
};

static PyTypeObject ComponentType = {
    PyVarObject_HEAD_INIT(NULL, 0) "yamal.sys_base.Component", /* tp_name */
    sizeof(struct Component),                 /* tp_basicsize */
    0,                                        /* tp_itemsize */
    (destructor)Component_dealloc,            /* tp_dealloc */
    0,                                        /* tp_print */
    0,                                        /* tp_getattr */
    0,                                        /* tp_setattr */
    0,                                        /* tp_reserved */
    0,                                        /* tp_repr */
    0,                                        /* tp_as_number */
    0,                                        /* tp_as_sequence */
    0,                                        /* tp_as_mapping */
    0,                                        /* tp_hash  */
    0,                                        /* tp_call */
    0,                                        /* tp_str */
    0,                                        /* tp_getattro */
    0,                                        /* tp_setattro */
    0,                                        /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /* tp_flags */
    "FMC Component class",                    /* tp_doc */
    0,                                        /* tp_traverse */
    0,                                        /* tp_clear */
    0,                                        /* tp_richcompare */
    0,                                        /* tp_weaklistoffset */
    0,                                        /* tp_iter */
    0,                                        /* tp_iternext */
    Component_methods,                        /* tp_methods */
    0,                                        /* tp_members */
    0,                                        /* tp_getset */
    0,                                        /* tp_base */
    0,                                        /* tp_dict */
    0,                                        /* tp_descr_get */
    0,                                        /* tp_descr_set */
    0,                                        /* tp_dictoffset */
    (initproc)Component_init,                 /* tp_init */
    0,                                        /* tp_alloc */
    Component_new,                            /* tp_new */
};

static PyTypeObject ReactorType = {
    PyVarObject_HEAD_INIT(NULL, 0) "yamal.sys_base.Reactor", /* tp_name */
    sizeof(struct Reactor),                                  /* tp_basicsize */
    0,                                                       /* tp_itemsize */
    (destructor)Reactor_dealloc,                             /* tp_dealloc */
    0,                                                       /* tp_print */
    0,                                                       /* tp_getattr */
    0,                                                       /* tp_setattr */
    0,                                                       /* tp_reserved */
    0,                                                       /* tp_repr */
    0,                                                       /* tp_as_number */
    0,                                        /* tp_as_sequence */
    0,                                        /* tp_as_mapping */
    0,                                        /* tp_hash  */
    0,                                        /* tp_call */
    0,                                        /* tp_str */
    0,                                        /* tp_getattro */
    0,                                        /* tp_setattro */
    0,                                        /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /* tp_flags */
    "FMC Reactor class",                      /* tp_doc */
    0,                                        /* tp_traverse */
    0,                                        /* tp_clear */
    0,                                        /* tp_richcompare */
    0,                                        /* tp_weaklistoffset */
    0,                                        /* tp_iter */
    0,                                        /* tp_iternext */
    Reactor_methods,                          /* tp_methods */
    0,                                        /* tp_members */
    0,                                        /* tp_getset */
    0,                                        /* tp_base */
    0,                                        /* tp_dict */
    0,                                        /* tp_descr_get */
    0,                                        /* tp_descr_set */
    0,                                        /* tp_dictoffset */
    (initproc)Reactor_init,                   /* tp_init */
    0,                                        /* tp_alloc */
    Reactor_new,                              /* tp_new */
};

static PyObject *Sys_new(PyTypeObject *subtype, PyObject *args,
                         PyObject *kwds) {
  struct Sys *self = (struct Sys *)subtype->tp_alloc(subtype, 0);
  if (!self) {
    return NULL;
  }
  return (PyObject *)self;
}

static int Sys_init(struct Sys *self, PyObject *args, PyObject *kwds) {
  self->initialized = false;

  static char *kwlist[] = {NULL /* Sentinel */};
  if (!PyArg_ParseTupleAndKeywords(args, kwds, "", kwlist)) {
    return -1;
  }

  fmc_component_sys_init(&self->sys);
  self->initialized = true;

  fmc_error_t *err;
  fmc_component_sys_paths_set_default(&self->sys, &err);
  if (err) {
    PyErr_SetString(PyExc_RuntimeError, fmc_error_msg(err));
    goto do_cleanup;
  }

  return 0;

do_cleanup:
  fmc_component_sys_destroy(&self->sys);
  return -1;
}

static void Sys_dealloc(struct Sys *self) {
  if (self->initialized) {
    fmc_component_sys_destroy(&self->sys);
  }
  Py_TYPE(self)->tp_free((PyObject *)self);
}

static PyObject *Sys_get_paths(struct Sys *self, PyObject *args,
                               PyObject *kwds) {
  static char *kwlist[] = {NULL /* Sentinel */};

  if (!PyArg_ParseTupleAndKeywords(args, kwds, "", kwlist)) {
    return NULL;
  }

  struct fmc_ext_searchpath_t *list = fmc_component_sys_paths_get(&self->sys);
  struct fmc_ext_searchpath_t *p = NULL;
  size_t count = 0;

  p = list;
  while (p) {
    ++count;
    p = p->next;
  }

  PyObject *paths = PyList_New(count);

  p = list;
  for (size_t i = 0; i < count; ++i) {
    PyList_SetItem(paths, i, PyUnicode_FromString(p->path));
    p = p->next;
  }

  return paths;
}

static PyObject *Sys_set_paths(struct Sys *self, PyObject *args,
                               PyObject *kwds) {
  static char *kwlist[] = {(char *)"paths", NULL /* Sentinel */};

  PyObject *paths_obj = NULL;
  if (!PyArg_ParseTupleAndKeywords(args, kwds, "O", kwlist, &paths_obj)) {
    return NULL;
  }

  if (!PyList_Check(paths_obj)) {
    PyErr_SetString(PyExc_RuntimeError, "paths must be a list");
    return NULL;
  }

  Py_ssize_t sz = PyList_Size(paths_obj);

  const char **paths = (const char **)calloc(sz + 1, sizeof(char *));
  if (!paths) {
    PyErr_SetString(PyExc_RuntimeError, "unable to allocate memory");
    goto do_cleanup;
  }

  for (Py_ssize_t i = 0; i < sz; ++i) {
    PyObject *value_obj = PyList_GetItem(paths_obj, i);
    if (!PyUnicode_Check(value_obj)) {
      PyErr_SetString(PyExc_RuntimeError, "array of string was expected");
      goto do_cleanup;
    }
    paths[i] = PyUnicode_AsUTF8(value_obj);
  }

  fmc_error_t *err;
  fmc_component_sys_paths_set(&self->sys, paths, &err);
  if (err) {
    PyErr_SetString(PyExc_RuntimeError, fmc_error_msg(err));
    goto do_cleanup;
  }
  free(paths);
  Py_RETURN_NONE;

do_cleanup:
  free(paths);
  return NULL;
}

static PyObject *Sys_get_module(struct Sys *self, PyObject *args,
                                PyObject *kwds) {
  static char *kwlist[] = {(char *)"name", NULL /* Sentinel */};

  const char *name = NULL;
  if (!PyArg_ParseTupleAndKeywords(args, kwds, "s", kwlist, &name)) {
    return NULL;
  }

  fmc_error_t *err;
  struct fmc_component_module *mod =
      fmc_component_module_get(&self->sys, name, &err);

  if (err) {
    PyErr_SetString(PyExc_RuntimeError, fmc_error_msg(err));
    return NULL;
  }

  return PyLong_FromVoidPtr(mod);
}

static PyObject *Sys_get_component_type(struct Sys *self, PyObject *args,
                                        PyObject *kwds) {
  static char *kwlist[] = {(char *)"module", (char *)"name",
                           NULL /* Sentinel */};
  PyObject *module = NULL;
  const char *name = NULL;
  if (!PyArg_ParseTupleAndKeywords(args, kwds, "Os", kwlist, &module, &name)) {
    return NULL;
  }

  struct fmc_component_module *mod =
      (struct fmc_component_module *)PyLong_AsVoidPtr(module);

  fmc_error_t *err;
  struct fmc_component_type *type =
      fmc_component_module_type_get(mod, name, &err);

  if (err) {
    PyErr_SetString(PyExc_RuntimeError, fmc_error_msg(err));
    return NULL;
  }

  return PyLong_FromVoidPtr(type);
}

static PyObject *Sys_get_component_type_spec(struct Sys *self, PyObject *args,
                                             PyObject *kwds) {
  static char *kwlist[] = {(char *)"component_type", NULL /* Sentinel */};
  PyObject *type_obj = NULL;

  if (!PyArg_ParseTupleAndKeywords(args, kwds, "O", kwlist, &type_obj)) {
    return NULL;
  }

  struct fmc_component_type *type =
      (struct fmc_component_type *)PyLong_AsVoidPtr(type_obj);
  struct fmc_cfg_node_spec *spec = type->tp_cfgspec;

  Py_ssize_t count = 0;
  while (spec[count].key) {
    ++count;
  }

  PyObject *list = PyList_New(count);
  for (Py_ssize_t i = 0; i < count; ++i) {
    PyObject *item = PyTuple_New(3);
    PyTuple_SetItem(item, 0, PyUnicode_FromString(spec[i].key));
    PyObject *itemtype = NULL;
    switch (spec[i].type.type) {
    case FMC_CFG_NONE:
      itemtype = PyUnicode_FromString("NoneType");
      break;
    case FMC_CFG_BOOLEAN:
      itemtype = PyUnicode_FromString("bool");
      break;
    case FMC_CFG_INT64:
      itemtype = PyUnicode_FromString("int");
      break;
    case FMC_CFG_FLOAT64:
      itemtype = PyUnicode_FromString("float");
      break;
    case FMC_CFG_STR:
      itemtype = PyUnicode_FromString("str");
      break;
    case FMC_CFG_SECT:
      itemtype = PyUnicode_FromString("dict");
      break;
    case FMC_CFG_ARR:
      itemtype = PyUnicode_FromString("list");
      break;
    }
    PyTuple_SetItem(item, 1, itemtype);
    PyTuple_SetItem(item, 2, PyBool_FromLong(spec[i].required));
    PyList_SetItem(list, i, item);
  }

  return list;
}

static PyObject *Sys_get_module_filepath(struct Sys *self, PyObject *args,
                                         PyObject *kwds) {
  static char *kwlist[] = {(char *)"module", NULL /* Sentinel */};
  PyObject *module = NULL;
  if (!PyArg_ParseTupleAndKeywords(args, kwds, "O", kwlist, &module)) {
    return NULL;
  }

  struct fmc_component_module *mod =
      (struct fmc_component_module *)PyLong_AsVoidPtr(module);

  return PyUnicode_FromString(fmc_component_module_file(mod));
}

static PyObject *Component_new(PyTypeObject *subtype, PyObject *args,
                               PyObject *kwds) {
  struct Component *self = (struct Component *)subtype->tp_alloc(subtype, 0);
  if (!self) {
    return NULL;
  }
  return (PyObject *)self;
}

static struct fmc_cfg_arr_item *PyList_to_cfg(PyObject *cfg_obj,
                                              fmc_error_t **err);
static struct fmc_cfg_sect_item *PyDict_to_cfg(PyObject *cfg_obj,
                                               fmc_error_t **err);

static struct fmc_cfg_arr_item *PyList_to_cfg(PyObject *cfg_obj,
                                              fmc_error_t **err) {
  fmc_error_clear(err);
  PyObject *value_obj;
  struct fmc_cfg_arr_item *ret = NULL;
  Py_ssize_t sz = PyList_Size(cfg_obj);

  for (Py_ssize_t i = sz; i-- > 0;) {
    value_obj = PyList_GetItem(cfg_obj, i);
    if (PyBool_Check(value_obj)) {
      ret = fmc_cfg_arr_item_add_boolean(ret, value_obj == Py_True, err);
      if (*err) {
        goto do_cleanup;
      }
    } else if (PyLong_Check(value_obj)) {
      ret = fmc_cfg_arr_item_add_int64(ret, PyLong_AsLongLong(value_obj), err);
      if (*err) {
        goto do_cleanup;
      }
    } else if (PyFloat_Check(value_obj)) {
      ret = fmc_cfg_arr_item_add_float64(ret, PyFloat_AsDouble(value_obj), err);
      if (*err) {
        goto do_cleanup;
      }
    } else if (PyUnicode_Check(value_obj)) {
      ret = fmc_cfg_arr_item_add_str(ret, PyUnicode_AsUTF8(value_obj), err);
      if (*err) {
        goto do_cleanup;
      }
    } else if (PyDict_Check(value_obj)) {
      struct fmc_cfg_sect_item *subdict = PyDict_to_cfg(value_obj, err);
      if (*err) {
        goto do_cleanup;
      }
      ret = fmc_cfg_arr_item_add_sect(ret, subdict, err);
      if (*err) {
        fmc_cfg_sect_del(subdict);
        goto do_cleanup;
      }
    } else if (PyList_Check(value_obj)) {
      struct fmc_cfg_arr_item *sublist = PyList_to_cfg(value_obj, err);
      if (*err) {
        goto do_cleanup;
      }
      ret = fmc_cfg_arr_item_add_arr(ret, sublist, err);
      if (*err) {
        fmc_cfg_arr_del(sublist);
        goto do_cleanup;
      }
    } else if (value_obj == Py_None) {
      ret = fmc_cfg_arr_item_add_none(ret, err);
    }
  }

  return ret;

do_cleanup:
  fmc_cfg_arr_del(ret);
  return NULL;
}

static struct fmc_cfg_sect_item *PyDict_to_cfg(PyObject *cfg_obj,
                                               fmc_error_t **err) {
  fmc_error_clear(err);
  PyObject *key_obj, *value_obj;
  Py_ssize_t pos = 0;
  struct fmc_cfg_sect_item *ret = NULL;

  while (PyDict_Next(cfg_obj, &pos, &key_obj, &value_obj)) {
    if (!PyUnicode_Check(key_obj)) {
      fmc_error_set(err, "config keys must be string");
      return NULL;
    }
    const char *key = PyUnicode_AsUTF8(key_obj);

    if (PyBool_Check(value_obj)) {
      ret = fmc_cfg_sect_item_add_boolean(ret, key, value_obj == Py_True, err);
      if (*err) {
        goto do_cleanup;
      }
    } else if (PyLong_Check(value_obj)) {
      ret = fmc_cfg_sect_item_add_int64(ret, key, PyLong_AsLongLong(value_obj),
                                        err);
      if (*err) {
        goto do_cleanup;
      }
    } else if (PyFloat_Check(value_obj)) {
      ret = fmc_cfg_sect_item_add_float64(ret, key, PyFloat_AsDouble(value_obj),
                                          err);
      if (*err) {
        goto do_cleanup;
      }
    } else if (PyUnicode_Check(value_obj)) {
      ret =
          fmc_cfg_sect_item_add_str(ret, key, PyUnicode_AsUTF8(value_obj), err);
      if (*err) {
        goto do_cleanup;
      }
    } else if (PyDict_Check(value_obj)) {
      struct fmc_cfg_sect_item *subdict = PyDict_to_cfg(value_obj, err);
      if (*err) {
        goto do_cleanup;
      }
      ret = fmc_cfg_sect_item_add_sect(ret, key, subdict, err);
      if (*err) {
        fmc_cfg_sect_del(subdict);
        goto do_cleanup;
      }
    } else if (PyList_Check(value_obj)) {
      struct fmc_cfg_arr_item *sublist = PyList_to_cfg(value_obj, err);
      if (*err) {
        goto do_cleanup;
      }
      ret = fmc_cfg_sect_item_add_arr(ret, key, sublist, err);
      if (*err) {
        fmc_cfg_arr_del(sublist);
        goto do_cleanup;
      }
    } else if (value_obj == Py_None) {
      ret = fmc_cfg_sect_item_add_none(ret, key, err);
    }
  }

  return ret;

do_cleanup:
  fmc_cfg_sect_del(ret);
  return NULL;
}

static int Component_init(struct Component *self, PyObject *args,
                          PyObject *kwds) {
  self->component = NULL;

  static char *kwlist[] = {(char *)"reactor", (char *)"component_type",
                           (char *)"inputs", (char *)"config",
                           NULL /* Sentinel */};
  PyObject *reactor_obj = NULL;
  PyObject *type_obj = NULL;
  PyObject *inps_obj = NULL;
  PyObject *cfg_obj = NULL;

  if (!PyArg_ParseTupleAndKeywords(args, kwds, "OOOO", kwlist, &reactor_obj,
                                   &type_obj, &inps_obj, &cfg_obj)) {
    return -1;
  }

  if (!PyObject_TypeCheck(reactor_obj, &ReactorType)) {
    PyErr_SetString(PyExc_RuntimeError, "invalid reactor type");
    return -1;
  }

  if (!PyList_Check(inps_obj)) {
    PyErr_SetString(PyExc_RuntimeError, "inputs must be a list");
    return -1;
  }

  struct Reactor *reactor_py = (struct Reactor *)reactor_obj;

  if (!PyDict_Check(cfg_obj)) {
    PyErr_SetString(PyExc_RuntimeError, "config must be a dictionary");
    return -1;
  }

  struct fmc_component_type *type =
      (struct fmc_component_type *)PyLong_AsVoidPtr(type_obj);

  fmc_error_t *err;
  struct fmc_cfg_sect_item *cfg = PyDict_to_cfg(cfg_obj, &err);
  if (err) {
    PyErr_SetString(PyExc_RuntimeError, fmc_error_msg(err));
    return -1;
  }

  Py_ssize_t inps_sz = PyList_Size(inps_obj);
  struct fmc_component_input inps[inps_sz + 1];
  for (Py_ssize_t i = 0; i < inps_sz; ++i) {
    PyObject *elem = PyList_GET_ITEM(inps_obj, i);
    if (!PyTuple_Check(elem)) {
      PyErr_SetString(PyExc_RuntimeError, "All inputs must be tuples");
      return -1;
    }
    if (PyTuple_Size(elem) != 2) {
      PyErr_SetString(
          PyExc_RuntimeError,
          "All input tuples must contain only the input component object and "
          "the index of the desired output in the provided input");
      return -1;
    }
    PyObject *comp = PyTuple_GetItem(elem, 0);
    if (!Component_Check(comp)) {
      PyErr_SetString(PyExc_RuntimeError, "Invalid component type");
      return -1;
    }
    PyObject *idx = PyTuple_GetItem(elem, 1);
    inps[i].comp = ((struct Component *)comp)->component;
    inps[i].idx = PyLong_AsSize_t(idx);
    if (inps[i].idx == (size_t)-1 && PyErr_Occurred()) {
      return -1;
    }
  }
  inps[inps_sz].comp = NULL;

  self->component =
      fmc_component_new(&reactor_py->reactor, type, cfg, inps, &err);
  fmc_cfg_sect_del(cfg);
  if (err) {
    PyErr_SetString(PyExc_RuntimeError, fmc_error_msg(err));
    return -1;
  }

  struct Component_list *new_node = malloc(sizeof(struct Component_list));
  if (!new_node)
    goto cleanup;
  new_node->comp = self;
  new_node->next = reactor_py->comp_list;
  reactor_py->comp_list = new_node;

  Py_INCREF(self);
  return 0;
cleanup:
  PyErr_SetString(PyExc_RuntimeError, "unable to allocate memory");
  return -1;
}

static void Component_dealloc(struct Component *self) {
  Py_TYPE(self)->tp_free((PyObject *)self);
}

bool Component_Check(PyObject *obj) {
  return PyObject_TypeCheck(obj, &ComponentType);
}

static PyObject *Component_out_idx(struct Component *self, PyObject *args,
                                   PyObject *kwds) {
  static char *kwlist[] = {(char *)"name", NULL /* Sentinel */};

  const char *name = NULL;
  if (!PyArg_ParseTupleAndKeywords(args, kwds, "s", kwlist, &name)) {
    return NULL;
  }

  fmc_error_t *err;
  size_t idx = fmc_component_out_idx(self->component, name, &err);
  if (err) {
    PyErr_SetString(PyExc_RuntimeError, fmc_error_msg(err));
    return NULL;
  }

  return PyLong_FromSize_t(idx);
}

static PyObject *Component_out_sz(struct Component *self) {
  return PyLong_FromSize_t(fmc_component_out_sz(self->component));
}

static PyObject *Reactor_new(PyTypeObject *subtype, PyObject *args,
                             PyObject *kwds) {
  struct Reactor *self = (struct Reactor *)subtype->tp_alloc(subtype, 0);
  if (!self) {
    return NULL;
  }
  return (PyObject *)self;
}

static int Reactor_init(struct Reactor *self, PyObject *args, PyObject *kwds) {
  self->initialized = false;
  self->comp_list = NULL;

  static char *kwlist[] = {NULL /* Sentinel */};
  if (!PyArg_ParseTupleAndKeywords(args, kwds, "", kwlist)) {
    return -1;
  }

  fmc_reactor_init(&self->reactor);
  self->initialized = true;

  return 0;
}

static void Reactor_dealloc(struct Reactor *self) {
  if (self->initialized) {
    while (self->comp_list) {
      Py_DECREF(self->comp_list->comp);
      struct Component_list *next = self->comp_list->next;
      free(self->comp_list);
      self->comp_list = next;
    }
    fmc_reactor_destroy(&self->reactor);
  }
  Py_TYPE(self)->tp_free((PyObject *)self);
}

static void pyreactor_run(struct fmc_reactor *reactor, bool live,
                          fmc_error_t **error) {
  fmc_error_clear(error);
  do {
    fmc_time64_t now = live ? fmc_time64_from_nanos(fmc_cur_time_ns())
                            : fmc_reactor_sched(reactor);
    if (!fmc_reactor_run_once(reactor, now, error))
      break;
    PyGILState_STATE gstate;
    gstate = PyGILState_Ensure();
    PyErr_CheckSignals();
    PyGILState_Release(gstate);
  } while (true);
}

static PyObject *Reactor_run(struct Reactor *self, PyObject *args,
                             PyObject *kwds) {
  static char *kwlist[] = {(char *)"live", (char *)"checksignals",
                           NULL /* Sentinel */};

  int live = 0;
  int checksignals = 0;
  if (!PyArg_ParseTupleAndKeywords(args, kwds, "pp", kwlist, &live,
                                   &checksignals)) {
    return NULL;
  }

  fmc_error_t *err;
  Py_BEGIN_ALLOW_THREADS;
  if (checksignals) {
    pyreactor_run(&self->reactor, live, &err);
  } else {
    fmc_reactor_run(&self->reactor, live, &err);
  }
  Py_END_ALLOW_THREADS;

  if (err) {
    PyErr_SetString(PyExc_RuntimeError, fmc_error_msg(err));
    return NULL;
  }

  Py_RETURN_NONE;
}

static PyObject *Reactor_run_once(struct Reactor *self, PyObject *args,
                                  PyObject *kwds) {
  static char *kwlist[] = {"now", NULL /* Sentinel */};

  long long now = 0;
  if (!PyArg_ParseTupleAndKeywords(args, kwds, "L", kwlist, &now)) {
    return NULL;
  }

  fmc_error_t *err;
  fmc_error_clear(&err);
  bool ret =
      fmc_reactor_run_once(&self->reactor, fmc_time64_from_nanos(now), &err);

  if (err) {
    PyErr_SetString(PyExc_RuntimeError, fmc_error_msg(err));
    return NULL;
  }

  return PyBool_FromLong(ret);
}

static PyObject *Reactor_stop(struct Reactor *self, PyObject *args,
                              PyObject *kwds) {
  static char *kwlist[] = {NULL /* Sentinel */};

  if (!PyArg_ParseTupleAndKeywords(args, kwds, "", kwlist)) {
    return NULL;
  }

  fmc_reactor_stop(&self->reactor);
  Py_RETURN_NONE;
}

static PyObject *Reactor_sched(struct Reactor *self, PyObject *args,
                               PyObject *kwds) {
  static char *kwlist[] = {NULL /* Sentinel */};

  if (!PyArg_ParseTupleAndKeywords(args, kwds, "", kwlist)) {
    return NULL;
  }

  return PyLong_FromLongLong(
      fmc_time64_to_nanos(fmc_reactor_sched(&self->reactor)));
}

static PyModuleDef ComponentsBaseModule = {PyModuleDef_HEAD_INIT, "sys_base",
                                           "sys base module", -1};

#define ADD_PY_CLASS(C, N, MOD)                                                \
  if (PyType_Ready(&C) < 0)                                                    \
    return NULL;                                                               \
  Py_INCREF(&C);                                                               \
  PyModule_AddObject(MOD, N, (PyObject *)&C)

PyMODINIT_FUNC PyInit_sys_base(void) FMMODFUNC FMPYMODPUB;

PyMODINIT_FUNC PyInit_sys_base(void) {
  PyObject *module = PyModule_Create(&ComponentsBaseModule);
  if (module) {
    ADD_PY_CLASS(SysType, "sys", module);
    ADD_PY_CLASS(ComponentType, "component", module);
    ADD_PY_CLASS(ReactorType, "reactor", module);
    if (PyModule_AddStringConstant(module, "__version__", YTP_VERSION) == -1)
      return NULL;
    return module;
  }

  return NULL;
}
