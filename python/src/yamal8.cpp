#include <fmc/platform.h>
#include <fmc/python/common.h>

#include <ytp/version.h>

#include <Python.h>

#include <ytp++/yamal.hpp>

// TODO:
// - Handle exceptions
// - Finish stream attributes
// - Confirm if announcement should be removed

struct Yamal;
struct Streams;

struct Stream {
  PyObject_HEAD;
  ytp::stream_t stream_;
  Yamal *yamal_;
};

static int Stream_init(Stream *self, PyObject *args, PyObject *kwds) {
  PyErr_SetString(PyExc_RuntimeError, "Stream objects are not standalone, use the Streams object to obtain an instance");
  return -1;
}

static void Stream_dealloc(Stream *self) {
  self->stream_.~stream_t();
  Py_XDECREF(self->yamal_);
}

PyObject *Stream_id(Stream *self, void *) {
  return PyLong_FromLong(self->stream_.id());
}

static PyGetSetDef Stream_getset[] = {
    {(char *)"id", (getter)Stream_id, NULL,
     (char *)"Returns the numerical identifier associated with the stream.",
     NULL},
    {NULL, NULL, NULL, NULL, NULL} /* Sentinel */
};

static PyTypeObject StreamType = {
    PyVarObject_HEAD_INIT(NULL, 0) "yamal.yamal8.stream", /* tp_name */
    sizeof(Stream),                                       /* tp_basicsize */
    0,                                                    /* tp_itemsize */
    (destructor)Stream_dealloc,                           /* tp_dealloc */
    0,                                                    /* tp_print */
    0,                                                    /* tp_getattr */
    0,                                                    /* tp_setattr */
    0,                                                    /* tp_reserved */
    0,                                                    /* tp_repr */
    0,                                                    /* tp_as_number */
    0,                                                    /* tp_as_sequence */
    0,                                                    /* tp_as_mapping */
    0,                                                    /* tp_hash  */
    0,                                                    /* tp_call */
    0,                                                    /* tp_str */
    0,                                                    /* tp_getattro */
    0,                                                    /* tp_setattro */
    0,                                                    /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,             /* tp_flags */
    "Stream object",                                      /* tp_doc */
    0,                                                    /* tp_traverse */
    0,                                                    /* tp_clear */
    0,                                                    /* tp_richcompare */
    0,                     /* tp_weaklistoffset */
    0,                     /* tp_iter */
    0,                     /* tp_iternext */
    0,                     /* tp_methods */
    0,                     /* tp_members */
    Stream_getset,         /* tp_getset */
    0,                     /* tp_base */
    0,                     /* tp_dict */
    0,                     /* tp_descr_get */
    0,                     /* tp_descr_set */
    0,                     /* tp_dictoffset */
    (initproc)Stream_init, /* tp_init */
    0,                     /* tp_alloc */
    0                      /* tp_new */
};

struct Streams {
  PyObject_HEAD;
  ytp::streams_t streams_;
  Yamal *yamal_;
};

static PyObject *Stream_new(Yamal *yamal, ytp::stream_t stream) {
  auto *self = (Stream *)StreamType.tp_alloc(&StreamType, 0);
  if (!self) {
    return nullptr;
  }
  self->stream_ = stream;
  self->yamal_ = yamal;
  Py_INCREF(yamal);
  return (PyObject *)self;
}

static int Streams_init(Streams *self, PyObject *args, PyObject *kwds) {
  PyErr_SetString(PyExc_RuntimeError, "Streams objects are not standalone, use "
                                      "the Yamal object to obtain an instance");
  return -1;
}

static void Streams_dealloc(Streams *self) {
  self->streams_.~streams_t();
  Py_XDECREF(self->yamal_);
}

static PyObject *Streams_announce(Streams *self, PyObject *args,
                                  PyObject *kwds) {
  static char *kwlist[] = {
      (char *)"peer", (char *)"channel", (char *)"encoding",
      NULL /* Sentinel */
  };

  char *peer = NULL;
  char *channel = NULL;
  char *encoding = NULL;
  if (!PyArg_ParseTupleAndKeywords(args, kwds, "sss", kwlist, &peer, &channel, &encoding)) {
    return NULL;
  }

  auto sl = self->streams_.announce(peer, channel, encoding);

  return Stream_new(self->yamal_, sl);
}

static PyObject *Streams_lookup(Streams *self, PyObject *args, PyObject *kwds) {

  static char *kwlist[] = {
      (char *)"peer", (char *)"channel",
      NULL /* Sentinel */
  };

  char *peer = NULL;
  char *channel = NULL;
  if (!PyArg_ParseTupleAndKeywords(args, kwds, "ss", kwlist, &peer, &channel)) {
    return NULL;
  }

  auto sl = self->streams_.lookup(peer, channel);

  if (!sl) {
    PyErr_SetString(PyExc_KeyError, "Unable to find stream");
    return NULL;
  }

  PyObject *s = Stream_new(self->yamal_, sl->first);
  if (!s) {
    return NULL;
  }

  PyObject *encoding = PyUnicode_FromStringAndSize(sl->second.data(), sl->second.size());
  if (!encoding) {
    Py_XDECREF(s);
    return NULL;
  }

  auto *obj = PyTuple_New(2);
  PyTuple_SET_ITEM(obj, 0, s);
  PyTuple_SET_ITEM(obj, 1, encoding);

  return obj;
}

static PyMethodDef Streams_methods[] = {
    {"announce", (PyCFunction)Streams_announce, METH_VARARGS | METH_KEYWORDS,
     "Announce a new stream and obtain it"},
    {"lookup", (PyCFunction)Streams_lookup, METH_VARARGS | METH_KEYWORDS,
     "Obtain the desired stream"},
    {NULL, NULL, 0, NULL} /* Sentinel */
};

static PyTypeObject StreamsType = {
    PyVarObject_HEAD_INIT(NULL, 0) "yamal.yamal8.streams", /* tp_name */
    sizeof(Streams),                                       /* tp_basicsize */
    0,                                                     /* tp_itemsize */
    (destructor)Streams_dealloc,                           /* tp_dealloc */
    0,                                                     /* tp_print */
    0,                                                     /* tp_getattr */
    0,                                                     /* tp_setattr */
    0,                                                     /* tp_reserved */
    0,                                                     /* tp_repr */
    0,                                                     /* tp_as_number */
    0,                                                     /* tp_as_sequence */
    0,                                                     /* tp_as_mapping */
    0,                                                     /* tp_hash  */
    0,                                                     /* tp_call */
    0,                                                     /* tp_str */
    0,                                                     /* tp_getattro */
    0,                                                     /* tp_setattro */
    0,                                                     /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,              /* tp_flags */
    "Streams object",                                      /* tp_doc */
    0,                                                     /* tp_traverse */
    0,                                                     /* tp_clear */
    0,                                                     /* tp_richcompare */
    0,                      /* tp_weaklistoffset */
    0,                      /* tp_iter */
    0,                      /* tp_iternext */
    Streams_methods,        /* tp_methods */
    0,                      /* tp_members */
    0,                      /* tp_getset */
    0,                      /* tp_base */
    0,                      /* tp_dict */
    0,                      /* tp_descr_get */
    0,                      /* tp_descr_set */
    0,                      /* tp_dictoffset */
    (initproc)Streams_init, /* tp_init */
    0,                      /* tp_alloc */
    0                       /* tp_new */
};

struct Data {
  PyObject_HEAD;
  ytp::data_t data_;
  Yamal *yamal_;
};

static int Data_init(Data *self, PyObject *args, PyObject *kwds) {
  PyErr_SetString(PyExc_RuntimeError, "Data objects are not standalone, use "
                                      "the Yamal object to obtain an instance");
  return -1;
}

static void Data_dealloc(Data *self) {
  self->data_.~data_t();
  Py_XDECREF(self->yamal_);
}

static PyObject *Data_closable(Data *self) {
  return PyBool_FromLong(self->data_.closable());
}

static PyObject *Data_close(Data *self) {
  self->data_.close();
  Py_RETURN_NONE;
}

static PyObject *Data_closed(Data *self) {
  return PyBool_FromLong(self->data_.closed());
}

static PyMethodDef Data_methods[] = {
    {"closable", (PyCFunction)Data_closable, METH_NOARGS,
     "Check if data is closable."},
    {"close", (PyCFunction)Data_close, METH_NOARGS, "Close data."},
    {"closed", (PyCFunction)Data_closed, METH_NOARGS,
     "Check if data is closed."},
    {NULL, NULL, 0, NULL} /* Sentinel */
};

static PyTypeObject DataType = {
    PyVarObject_HEAD_INIT(NULL, 0) "yamal.yamal8.yamal", /* tp_name */
    sizeof(Data),                                        /* tp_basicsize */
    0,                                                   /* tp_itemsize */
    (destructor)Data_dealloc,                            /* tp_dealloc */
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
    "Data object",                                       /* tp_doc */
    0,                                                   /* tp_traverse */
    0,                                                   /* tp_clear */
    0,                                                   /* tp_richcompare */
    0,                                                   /* tp_weaklistoffset */
    0,                                                   /* tp_iter */
    0,                                                   /* tp_iternext */
    Data_methods,                                        /* tp_methods */
    0,                                                   /* tp_members */
    0,                                                   /* tp_getset */
    0,                                                   /* tp_base */
    0,                                                   /* tp_dict */
    0,                                                   /* tp_descr_get */
    0,                                                   /* tp_descr_set */
    0,                                                   /* tp_dictoffset */
    (initproc)Data_init,                                 /* tp_init */
    0,                                                   /* tp_alloc */
    0                                                    /* tp_new */
};

struct Yamal {
  PyObject_HEAD;
  ytp::yamal_t yamal_;
};

static PyObject *Data_new(Yamal *yamal) {
  auto *self = (Data *)DataType.tp_alloc(&DataType, 0);
  if (!self) {
    return nullptr;
  }
  self->data_ = yamal->yamal_.data();
  self->yamal_ = yamal;
  Py_INCREF(yamal);
  return (PyObject *)self;
}

static PyObject *Streams_new(Yamal *yamal) {
  auto *self = (Streams *)StreamsType.tp_alloc(&StreamsType, 0);
  if (!self) {
    return nullptr;
  }
  self->streams_ = yamal->yamal_.streams();
  self->yamal_ = yamal;
  Py_INCREF(yamal);
  return (PyObject *)self;
}

static int Yamal_init(Yamal *self, PyObject *args, PyObject *kwds) {

  static char *kwlist[] = {
      (char *)"file", (char *)"closable", (char *)"enable_thread",
      NULL /* Sentinel */
  };

  PyObject *file = NULL;
  bool closable = false;
  bool enable_thread = true;
  if (!PyArg_ParseTupleAndKeywords(args, kwds, "O|pp", kwlist, &file, &closable,
                                   &enable_thread)) {
    return -1;
  }

  fmc_fd fd = PyObject_AsFileDescriptor(file);

  if (!fmc_fvalid(fd)) {
    return -1;
  }

  self->yamal_ = ytp::yamal_t(fd, closable, enable_thread);

  return 0;
}

static void Yamal_dealloc(Yamal *self) {}

static PyObject *Yamal_data(Yamal *self) { return Data_new(self); }

static PyObject *Yamal_streams(Yamal *self) { return Streams_new(self); }

static PyObject *Yamal_announcement(Yamal *self, PyObject *args,
                                    PyObject *kwds) {

  static char *kwlist[] = {
      (char *)"stream", NULL /* Sentinel */
  };

  Stream *stream = NULL;
  if (!PyArg_ParseTupleAndKeywords(args, kwds, "O", kwlist, &stream)) {
    return NULL;
  }

  if (!PyObject_TypeCheck(stream, &StreamType)) {
    PyErr_SetString(PyExc_RuntimeError, "Argument must be of Stream type");
    return NULL;
  }

  self->yamal_.announcement(stream->stream_);

  // TODO: Return proper object
  Py_RETURN_NONE;
}

static PyMethodDef Yamal_methods[] = {
    {"data", (PyCFunction)Yamal_data, METH_NOARGS, "Obtain data object."},
    {"streams", (PyCFunction)Yamal_streams, METH_NOARGS,
     "Obtain streams object"},
    {"announcement", (PyCFunction)Yamal_announcement,
     METH_VARARGS | METH_KEYWORDS,
     "Obtain the announcement details for the desired stream"},
    {NULL, NULL, 0, NULL} /* Sentinel */
};

static PyObject *Yamal_new(PyTypeObject *subtype, PyObject *args,
                           PyObject *kwds) {
  auto *self = (Yamal *)subtype->tp_alloc(subtype, 0);
  if (!self) {
    return nullptr;
  }
  return (PyObject *)self;
}

static PyTypeObject YamalType = {
    PyVarObject_HEAD_INIT(NULL, 0) "yamal.yamal8.yamal", /* tp_name */
    sizeof(Yamal),                                       /* tp_basicsize */
    0,                                                   /* tp_itemsize */
    (destructor)Yamal_dealloc,                           /* tp_dealloc */
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
    "Yamal object",                                      /* tp_doc */
    0,                                                   /* tp_traverse */
    0,                                                   /* tp_clear */
    0,                                                   /* tp_richcompare */
    0,                                                   /* tp_weaklistoffset */
    0,                                                   /* tp_iter */
    0,                                                   /* tp_iternext */
    Yamal_methods,                                       /* tp_methods */
    0,                                                   /* tp_members */
    0,                                                   /* tp_getset */
    0,                                                   /* tp_base */
    0,                                                   /* tp_dict */
    0,                                                   /* tp_descr_get */
    0,                                                   /* tp_descr_set */
    0,                                                   /* tp_dictoffset */
    (initproc)Yamal_init,                                /* tp_init */
    0,                                                   /* tp_alloc */
    Yamal_new                                            /* tp_new */
};

static PyModuleDef Yamal8Module = {PyModuleDef_HEAD_INIT, "yamal8",
                                   "yamal8 module", -1, NULL};

PyMODINIT_FUNC PyInit_yamal8(void) FMMODFUNC FMPYMODPUB;

PyMODINIT_FUNC PyInit_yamal8(void) {
  if (auto *module = PyModule_Create(&Yamal8Module); module) {
    ADD_PY_CLASS(StreamType, "stream", module);
    ADD_PY_CLASS(StreamsType, "streams", module);
    ADD_PY_CLASS(DataType, "data", module);
    ADD_PY_CLASS(YamalType, "yamal", module);
    if (PyModule_AddStringConstant(module, "__version__", YTP_VERSION) == -1)
      return NULL;
    return module;
  }

  return NULL;
}
