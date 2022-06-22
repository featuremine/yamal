
#pragma GCC visibility push(default)
#define PY_SSIZE_T_CLEAN

#include <Python.h>
#include <algorithm>
#include <fcntl.h>
#include <string>
#include <sys/stat.h>
#include <vector>

#include <fmc/files.h>

#include <ytp/control.h>
#include <ytp/sequence.h>
#include <ytp/version.h>

using namespace std;

string gen_error(string prefix, fmc_error_t *error) {
  if (error) {
    return prefix + " with error: " + fmc_error_msg(error);
  }
  return prefix;
}

typedef struct {
  PyObject_HEAD;
  ytp_yamal_t *yamal;
  fmc_fd fd;
  ytp_iterator_t it;
} YTPPeer;

static PyObject *YTPPeer_write(YTPPeer *self, PyObject *args, PyObject *kwds) {
  static char *kwlist[] = {(char *)"peer", (char *)"data", NULL /* Sentinel */};
  ytp_peer_t peer;
  const char *data = nullptr;
  Py_ssize_t sz;
  if (!PyArg_ParseTupleAndKeywords(args, kwds, "Ky#", kwlist, &peer, &data,
                                   &sz)) {
    return nullptr;
  }

  fmc_error_t *error = nullptr;
  char *dst = ytp_peer_reserve(self->yamal, sz, &error);

  if (!dst) {
    PyErr_SetString(
        PyExc_RuntimeError,
        gen_error("unable to reserve data in yamaluence", error).c_str());
  }

  memcpy(dst, data, sz);

  if (!ytp_peer_commit(self->yamal, peer, dst, &error)) {
    PyErr_SetString(
        PyExc_RuntimeError,
        gen_error("unable to commit data in sequence", error).c_str());
  }

  Py_RETURN_NONE;
}

static PyObject *YTPPeer_read(YTPPeer *self) {
  ytp_peer_t peer;
  const char *data = nullptr;
  size_t sz;
  fmc_error_t *error = nullptr;

  if (!ytp_yamal_term(self->it)) {
    ytp_peer_read(self->yamal, self->it, &peer, &sz, &data, &error);
    if (!error) {
      self->it = ytp_yamal_next(self->yamal, self->it, &error);
      if (error) {
        PyErr_SetString(PyExc_RuntimeError,
                        gen_error("unable to advance iterator", error).c_str());
        return nullptr;
      }
      return Py_BuildValue("(Ky#)", peer, data, Py_ssize_t(sz));
    }
  }

  if (error) {
    PyErr_SetString(PyExc_RuntimeError,
                    gen_error("unable to read data", error).c_str());
    return nullptr;
  }

  Py_RETURN_NONE;
}

static PyMethodDef YTPPeer_methods[] = {
    {"write", (PyCFunction)YTPPeer_write, METH_VARARGS | METH_KEYWORDS,
     "Not implemented."},
    {"read", (PyCFunction)YTPPeer_read, METH_NOARGS, "Not implemented."},
    {NULL, NULL, 0, NULL} /* Sentinel */
};

static int YTPPeer_init(YTPPeer *self, PyObject *args, PyObject *kwds) {
  static char *kwlist[] = {(char *)"filename", NULL /* Sentinel */};
  char *filename = nullptr;
  if (!PyArg_ParseTupleAndKeywords(args, kwds, "s", kwlist, &filename)) {
    return -1;
  }

  fmc_error_t *error = nullptr;
  fmc_fd fd = fmc_fopen(filename, fmc_fmode::READWRITE, &error);
  if (!fmc_fvalid(fd)) {
    PyErr_SetString(PyExc_RuntimeError,
                    gen_error("unable to open provided file", error).c_str());
    return -1;
  }

  ytp_yamal_t *yamal = ytp_yamal_new(fd, &error);
  if (!yamal) {
    close(fd);
    PyErr_SetString(PyExc_RuntimeError,
                    gen_error("unable to create new Peer", error).c_str());
    return -1;
  }

  ytp_iterator_t it = ytp_yamal_begin(yamal, &error);

  if (error) {
    auto err = gen_error("unable to obtain iterator", error);
    ytp_yamal_del(yamal, &error);
    close(fd);
    PyErr_SetString(PyExc_RuntimeError, err.c_str());
    return -1;
  }

  self->yamal = yamal;
  self->fd = fd;
  self->it = it;
  return 0;
}

PyObject *YTPPeer_new(PyTypeObject *subtype, PyObject *args, PyObject *kwds);

static void YTPPeer_dealloc(YTPPeer *self) {
  fmc_error_t *error = nullptr;
  if (self->yamal) {
    ytp_yamal_del(self->yamal, &error);
    if (error) {
      PyErr_SetString(PyExc_RuntimeError,
                      gen_error("unable to delete YTP yamal", error).c_str());
    }
  }
  if (self->fd) {
    fmc_fclose(self->fd, &error);
    if (error) {
      PyErr_SetString(PyExc_RuntimeError,
                      gen_error("unable to close file", error).c_str());
    }
  }
  Py_TYPE(self)->tp_free((PyObject *)self);
}

static PyTypeObject YtpPeerType = {
    PyVarObject_HEAD_INIT(NULL, 0) "ytp_base.peer", /* tp_name */
    sizeof(YTPPeer),                                /* tp_basicsize */
    0,                                              /* tp_itemsize */
    (destructor)YTPPeer_dealloc,                    /* tp_dealloc */
    0,                                              /* tp_print */
    0,                                              /* tp_getattr */
    0,                                              /* tp_setattr */
    0,                                              /* tp_reserved */
    0,                                              /* tp_repr */
    0,                                              /* tp_as_number */
    0,                                              /* tp_as_Peer */
    0,                                              /* tp_as_mapping */
    0,                                              /* tp_hash  */
    0,                                              /* tp_call */
    0,                                              /* tp_str */
    0,                                              /* tp_getattro */
    0,                                              /* tp_setattro */
    0,                                              /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,       /* tp_flags */
    "YTP Peer",                                     /* tp_doc */
    0,                                              /* tp_traverse */
    0,                                              /* tp_clear */
    0,                                              /* tp_richcompare */
    0,                                              /* tp_weaklistoffset */
    0,                                              /* tp_iter */
    0,                                              /* tp_iternext */
    YTPPeer_methods,                                /* tp_methods */
    0,                                              /* tp_members */
    0,                                              /* tp_getset */
    0,                                              /* tp_base */
    0,                                              /* tp_dict */
    0,                                              /* tp_descr_get */
    0,                                              /* tp_descr_set */
    0,                                              /* tp_dictoffset */
    (initproc)YTPPeer_init,                         /* tp_init */
    0,                                              /* tp_alloc */
    YTPPeer_new,                                    /* tp_new */
};

PyObject *YTPPeer_new(PyTypeObject *subtype, PyObject *args, PyObject *kwds) {
  auto *self = (YTPPeer *)subtype->tp_alloc(subtype, 0);
  if (!self) {
    return nullptr;
  }
  self->yamal = nullptr;
  self->fd = 0;
  return (PyObject *)self;
}

static PyModuleDef YtpBaseModule = {PyModuleDef_HEAD_INIT, "ytp_base",
                                    "ytp_base module", -1};

// TODO: move to common

#define ADD_PY_CLASS(C, N, MOD)                                                \
  if (PyType_Ready(&C) < 0)                                                    \
    return NULL;                                                               \
  Py_INCREF(&C);                                                               \
  PyModule_AddObject(MOD, N, (PyObject *)&C)

PyMODINIT_FUNC PyInit_ytp_base(void) FMPYMODPUB;

PyMODINIT_FUNC PyInit_ytp_base(void) {
  if (auto *module = PyModule_Create(&YtpBaseModule); module) {
    ADD_PY_CLASS(YtpPeerType, "peer", module);
    if (PyModule_AddStringConstant(module, "__version__", YTP_VERSION) == -1)
      return NULL;
    return module;
  }

  return NULL;
}
