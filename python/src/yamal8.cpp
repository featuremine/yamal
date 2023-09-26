#include <fmc/platform.h>
#include <fmc/python/common.h>

#include <ytp/version.h>

#include <Python.h>

#include <ytp++/yamal.hpp>

struct Yamal;

struct Stream {
  PyObject_HEAD;
  ytp::stream_t s_;
  Yamal *yamal_;
};

static int Stream_init(Stream *self, PyObject *args, PyObject *kwds) {}

static void Stream_dealloc(Stream *self) {}

PyObject *Stream_id(Stream *self, void *) {
  return PyLong_FromLong(self->s_.id());
}

static PyGetSetDef Stream_getset[] = {
    {(char *)"id", (getter)Stream_id, NULL,
     (char *)"Returns the numerical identifier associated with the stream.", NULL},
    {NULL, NULL, NULL, NULL, NULL} /* Sentinel */
};

static PyTypeObject StreamType = {
    PyVarObject_HEAD_INIT(NULL, 0) "yamal.yamal8.stream", /* tp_name */
    sizeof(Stream),                                       /* tp_basicsize */
    0,                                                   /* tp_itemsize */
    (destructor)Stream_dealloc,                           /* tp_dealloc */
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
    "Stream object",                                      /* tp_doc */
    0,                                                   /* tp_traverse */
    0,                                                   /* tp_clear */
    0,                                                   /* tp_richcompare */
    0,                                                   /* tp_weaklistoffset */
    0,                                                   /* tp_iter */
    0,                                                   /* tp_iternext */
    0,                                       /* tp_methods */
    0,                                                   /* tp_members */
    Stream_getset,                                                   /* tp_getset */
    0,                                                   /* tp_base */
    0,                                                   /* tp_dict */
    0,                                                   /* tp_descr_get */
    0,                                                   /* tp_descr_set */
    0,                                                   /* tp_dictoffset */
    (initproc)Stream_init,                                /* tp_init */
    0,                                                   /* tp_alloc */
    0,                                                   /* tp_new */
};

struct Streams {
  PyObject_HEAD;
};

static int Streams_init(Streams *self, PyObject *args, PyObject *kwds) {}

static void Streams_dealloc(Streams *self) {}

static PyObject *Streams_announce(Streams *self, PyObject *args, PyObject *kwds) {
}

static PyObject *Streams_lookup(Streams *self, PyObject *args, PyObject *kwds) {
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
    0,                                                   /* tp_itemsize */
    (destructor)Streams_dealloc,                           /* tp_dealloc */
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
    "Streams object",                                      /* tp_doc */
    0,                                                   /* tp_traverse */
    0,                                                   /* tp_clear */
    0,                                                   /* tp_richcompare */
    0,                                                   /* tp_weaklistoffset */
    0,                                                   /* tp_iter */
    0,                                                   /* tp_iternext */
    Streams_methods,                                       /* tp_methods */
    0,                                                   /* tp_members */
    0,                                                   /* tp_getset */
    0,                                                   /* tp_base */
    0,                                                   /* tp_dict */
    0,                                                   /* tp_descr_get */
    0,                                                   /* tp_descr_set */
    0,                                                   /* tp_dictoffset */
    (initproc)Streams_init,                                /* tp_init */
    0,                                                   /* tp_alloc */
    0,                                                   /* tp_new */
};

struct Data {
  PyObject_HEAD;
};

static int Data_init(Data *self, PyObject *args, PyObject *kwds) {}

static void Data_dealloc(Data *self) {}

static PyObject *Data_closable(Data *self) {
}

static PyObject *Data_close(Data *self) {
}

static PyObject *Data_closed(Data *self) {
}

static PyMethodDef Data_methods[] = {
  {"closable", (PyCFunction)Data_closable, METH_NOARGS, "Check if data is closable."},
  {"close", (PyCFunction)Data_close, METH_NOARGS, "Close data."},
  {"closed", (PyCFunction)Data_closed, METH_NOARGS, "Check if data is closed."},
  {NULL, NULL, 0, NULL} /* Sentinel */
};

static PyObject *Data_new(PyTypeObject *subtype, PyObject *args,
                          PyObject *kwds);

static PyTypeObject DataType = {
    PyVarObject_HEAD_INIT(NULL, 0) "yamal.yamal8.yamal", /* tp_name */
    sizeof(Data),                                       /* tp_basicsize */
    0,                                                   /* tp_itemsize */
    (destructor)Data_dealloc,                           /* tp_dealloc */
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
    "Data object",                                      /* tp_doc */
    0,                                                   /* tp_traverse */
    0,                                                   /* tp_clear */
    0,                                                   /* tp_richcompare */
    0,                                                   /* tp_weaklistoffset */
    0,                                                   /* tp_iter */
    0,                                                   /* tp_iternext */
    Data_methods,                                       /* tp_methods */
    0,                                                   /* tp_members */
    0,                                                   /* tp_getset */
    0,                                                   /* tp_base */
    0,                                                   /* tp_dict */
    0,                                                   /* tp_descr_get */
    0,                                                   /* tp_descr_set */
    0,                                                   /* tp_dictoffset */
    (initproc)Data_init,                                /* tp_init */
    0,                                                   /* tp_alloc */
    Data_new,                                                   /* tp_new */
};

static PyObject *Data_new(PyTypeObject *subtype, PyObject *args,
                          PyObject *kwds) {
  auto *self = (Data *)subtype->tp_alloc(subtype, 0);
  if (!self) {
    return nullptr;
  }
  return (PyObject *)self;
}


struct Yamal {
  PyObject_HEAD;
  ytp::yamal_t yamal_;
};

static int Yamal_init(Yamal *self, PyObject *args, PyObject *kwds) {

  static char *kwlist[] = {
    (char *)"file",
    (char *)"closable",
    (char *)"enable_thread",
    NULL /* Sentinel */
  };

  PyObject *file = NULL;
  bool closable = false;
  bool enable_thread = true;
  if (!PyArg_ParseTupleAndKeywords(args, kwds, "O|pp", kwlist, &file, &closable, &enable_thread)) {
    return NULL;
  }

  fmc_fd fd = PyObject_AsFileDescriptor(file);

  if (!fmc_fvalid(fd)) {
    return -1;
  }

  self->yamal_ = ytp::yamal_t(fd, closable, enable_thread);

  return 0;

}

static void Yamal_dealloc(Yamal *self) {}

static PyObject *Yamal_data(Yamal *self) {
}

static PyObject *Yamal_streams(Yamal *self) {


}

static PyObject *Yamal_announcement(Yamal *self, PyObject *args, PyObject *kwds) {
}

static PyMethodDef Yamal_methods[] = {
  {"data", (PyCFunction)Yamal_data, METH_NOARGS, "Obtain data object."},
  {"streams", (PyCFunction)Yamal_streams, METH_NOARGS, "Obtain streams object"},
  {"announcement", (PyCFunction)Yamal_announcement, METH_VARARGS | METH_KEYWORDS,
    "Obtain the announcement details for the desired stream"},
  {NULL, NULL, 0, NULL} /* Sentinel */
};

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
    0,                                                   /* tp_new */
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
