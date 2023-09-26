#define PY_SSIZE_T_CLEAN

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

static void Stream_dealloc(Stream *self) {
  self->stream_.~stream_t();
  Py_XDECREF(self->yamal_);
}

PyObject *Stream_id(Stream *self, void *) {
  return PyLong_FromLong(self->stream_.id());
}

Py_hash_t Stream_hash(Stream *self) {
  return std::hash<ytp::stream_t>{}(self->stream_);
}

static PyGetSetDef Stream_getset[] = {
    {(char *)"id", (getter)Stream_id, NULL,
     (char *)"Returns the numerical identifier associated with the stream.",
     NULL},
    {NULL, NULL, NULL, NULL, NULL} /* Sentinel */
};

static PyObject *Stream_write(Stream *self, PyObject *args, PyObject *kwds);

static PyMethodDef Stream_methods[] = {
    {"write", (PyCFunction)Stream_write, METH_VARARGS | METH_KEYWORDS,
     "Write a new message"},
    {NULL, NULL, 0, NULL} /* Sentinel */
};

static PyObject *Stream_str(Stream *self) {
  std::ostringstream o;
  o << self->stream_;
  auto os = o.str();
  return PyUnicode_FromString(os.c_str());
}

static PyObject *Stream_richcompare(Stream *obj1, Stream *obj2, int op);

static PyTypeObject StreamType = {
    PyVarObject_HEAD_INIT(NULL, 0) "yamal.yamal8.stream", /* tp_name */
    sizeof(Stream),                                       /* tp_basicsize */
    0,                                                    /* tp_itemsize */
    (destructor)Stream_dealloc,                           /* tp_dealloc */
    0,                                                    /* tp_print */
    0,                                                    /* tp_getattr */
    0,                                                    /* tp_setattr */
    0,                                                    /* tp_reserved */
    (reprfunc)Stream_str,                                 /* tp_repr */
    0,                                                    /* tp_as_number */
    0,                                                    /* tp_as_sequence */
    0,                                                    /* tp_as_mapping */
    (hashfunc)Stream_hash,                                /* tp_hash  */
    0,                                                    /* tp_call */
    (reprfunc)Stream_str,                                 /* tp_str */
    0,                                                    /* tp_getattro */
    0,                                                    /* tp_setattro */
    0,                                                    /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,             /* tp_flags */
    "Stream object",                                      /* tp_doc */
    0,                                                    /* tp_traverse */
    0,                                                    /* tp_clear */
    (richcmpfunc)Stream_richcompare,                      /* tp_richcompare */
    0,              /* tp_weaklistoffset */
    0,              /* tp_iter */
    0,              /* tp_iternext */
    Stream_methods, /* tp_methods */
    0,              /* tp_members */
    Stream_getset,  /* tp_getset */
    0,              /* tp_base */
    0,              /* tp_dict */
    0,              /* tp_descr_get */
    0,              /* tp_descr_set */
    0,              /* tp_dictoffset */
    0,              /* tp_init */
    0,              /* tp_alloc */
    0               /* tp_new */
};

static PyObject *Stream_richcompare(Stream *obj1, Stream *obj2, int op) {
  if (!PyObject_TypeCheck(obj1, &StreamType)) {
    PyErr_SetString(PyExc_RuntimeError,
                    "Invalid type of first argument, expected Stream");
    return NULL;
  }
  if (!PyObject_TypeCheck(obj2, &StreamType)) {
    PyErr_SetString(PyExc_RuntimeError,
                    "Invalid type of second argument, expected Stream");
    return NULL;
  }
  switch (op) {
  case Py_LT:
    break;
  case Py_LE:
    break;
  case Py_EQ:
    return PyBool_FromLong(obj1->stream_ == obj2->stream_);
    break;
  case Py_NE:
    return PyBool_FromLong(obj1->stream_ != obj2->stream_);
    break;
  case Py_GT:
    break;
  case Py_GE:
    break;
  }
  PyErr_SetString(PyExc_RuntimeError,
                  "Unsupported stream comparison operation");
  return NULL;
}

struct Streams {
  PyObject_HEAD;
  ytp::streams_t streams_;
  Yamal *yamal_;
};

static PyObject *Stream_new(Yamal *yamal, ytp::stream_t stream) {
  auto *self = (Stream *)StreamType.tp_alloc(&StreamType, 0);
  if (!self) {
    return NULL;
  }
  self->stream_ = stream;
  self->yamal_ = yamal;
  Py_INCREF(yamal);
  return (PyObject *)self;
}

static void Streams_dealloc(Streams *self) {
  self->streams_.~streams_t();
  Py_XDECREF(self->yamal_);
}

static PyObject *Streams_announce(Streams *self, PyObject *args,
                                  PyObject *kwds) {
  static char *kwlist[] = {
      (char *)"peer", (char *)"channel", (char *)"encoding", NULL /* Sentinel */
  };

  const char *peer = NULL;
  const char *channel = NULL;
  const char *encoding = NULL;
  if (!PyArg_ParseTupleAndKeywords(args, kwds, "sss", kwlist, &peer, &channel,
                                   &encoding)) {
    return NULL;
  }

  try {
    auto sl = self->streams_.announce(peer, channel, encoding);
    return Stream_new(self->yamal_, sl);
  } catch (const std::exception &e) {
    PyErr_SetString(PyExc_RuntimeError, "Unable to announce stream");
    return NULL;
  }
}

static PyObject *Streams_lookup(Streams *self, PyObject *args, PyObject *kwds) {

  static char *kwlist[] = {
      (char *)"peer", (char *)"channel", NULL /* Sentinel */
  };

  const char *peer = NULL;
  const char *channel = NULL;
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

  PyObject *encoding =
      PyUnicode_FromStringAndSize(sl->second.data(), sl->second.size());
  if (!encoding) {
    Py_XDECREF(s);
    return NULL;
  }

  auto *obj = PyTuple_New(2);
  if (!obj) {
    Py_XDECREF(s);
    Py_XDECREF(encoding);
    return NULL;
  }
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
    0,               /* tp_weaklistoffset */
    0,               /* tp_iter */
    0,               /* tp_iternext */
    Streams_methods, /* tp_methods */
    0,               /* tp_members */
    0,               /* tp_getset */
    0,               /* tp_base */
    0,               /* tp_dict */
    0,               /* tp_descr_get */
    0,               /* tp_descr_set */
    0,               /* tp_dictoffset */
    0,               /* tp_init */
    0,               /* tp_alloc */
    0                /* tp_new */
};

struct Data {
  PyObject_HEAD;
  ytp::data_t data_;
  Yamal *yamal_;
};

struct DataIter {
  PyObject_HEAD;
  ytp::data_t::iterator it_;
  Data *data_;
};

PyObject *DataIter_iter(PyObject *self) {
  Py_INCREF(self);
  return self;
}

PyObject *DataIter_iternext(DataIter *self) {

  if (self->it_ == self->data_->data_.end()) {
    PyErr_SetNone(PyExc_StopIteration);
    return NULL;
  }

  try {
    auto [seqno, ts, stream, data] = *self->it_;
    PyObject *pyseqno = PyLong_FromUnsignedLongLong(seqno);
    if (!pyseqno) {
      return NULL;
    }
    PyObject *pyts = PyLong_FromUnsignedLongLong(ts);
    if (!pyts) {
      Py_XDECREF(pyseqno);
      return NULL;
    }
    PyObject *pystream = Stream_new(self->data_->yamal_, stream);
    if (!pystream) {
      Py_XDECREF(pyseqno);
      Py_XDECREF(pyts);
      return NULL;
    }
    PyObject *pydata = PyBytes_FromStringAndSize(data.data(), data.size());
    if (!pydata) {
      Py_XDECREF(pyseqno);
      Py_XDECREF(pyts);
      Py_XDECREF(pystream);
      return NULL;
    }
    auto *obj = PyTuple_New(4);
    if (!obj) {
      Py_XDECREF(pyseqno);
      Py_XDECREF(pyts);
      Py_XDECREF(pystream);
      Py_XDECREF(pydata);
      return NULL;
    }
    PyTuple_SET_ITEM(obj, 0, pyseqno);
    PyTuple_SET_ITEM(obj, 1, pyts);
    PyTuple_SET_ITEM(obj, 2, pystream);
    PyTuple_SET_ITEM(obj, 3, pydata);
    ++self->it_;
    return obj;
  } catch (const std::exception &e) {
    PyErr_SetString(PyExc_RuntimeError, e.what());
    return NULL;
  }
}

static void DataIter_dealloc(DataIter *self) {
  self->it_.ytp::data_t::iterator::~iterator();
  Py_XDECREF(self->data_);
}

static PyTypeObject DataIterType = {
    PyVarObject_HEAD_INIT(NULL, 0) "yamal.yamal8.data_iter", /* tp_name */
    sizeof(DataIter),                                        /* tp_basicsize */
    0,                                                       /* tp_itemsize */
    (destructor)DataIter_dealloc,                            /* tp_dealloc */
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
    "DataIter object",                        /* tp_doc */
    0,                                        /* tp_traverse */
    0,                                        /* tp_clear */
    0,                                        /* tp_richcompare */
    0,                                        /* tp_weaklistoffset */
    DataIter_iter,                            /* tp_iter */
    (iternextfunc)DataIter_iternext,          /* tp_iternext */
    0,                                        /* tp_methods */
    0,                                        /* tp_members */
    0,                                        /* tp_getset */
    0,                                        /* tp_base */
    0,                                        /* tp_dict */
    0,                                        /* tp_descr_get */
    0,                                        /* tp_descr_set */
    0,                                        /* tp_dictoffset */
    0,                                        /* tp_init */
    0,                                        /* tp_alloc */
    0                                         /* tp_new */
};

PyObject *DataIter_new(Data *data, ytp::data_t::iterator it) {
  auto *self = (DataIter *)DataIterType.tp_alloc(&DataIterType, 0);
  if (!self) {
    return NULL;
  }
  self->it_ = it;
  self->data_ = data;
  Py_INCREF(data);
  return (PyObject *)self;
}

struct DataRevIter {
  PyObject_HEAD;
  ytp::data_t::reverse_iterator it_;
  Data *data_;
};

PyObject *DataRevIter_iter(PyObject *self) {
  Py_INCREF(self);
  return self;
}

PyObject *DataRevIter_iternext(DataRevIter *self) {

  if (self->it_ == self->data_->data_.rend()) {
    PyErr_SetNone(PyExc_StopIteration);
    return NULL;
  }

  try {
    auto [seqno, ts, stream, data] = *self->it_;
    PyObject *pyseqno = PyLong_FromUnsignedLongLong(seqno);
    if (!pyseqno) {
      return NULL;
    }
    PyObject *pyts = PyLong_FromUnsignedLongLong(ts);
    if (!pyts) {
      Py_XDECREF(pyseqno);
      return NULL;
    }
    PyObject *pystream = Stream_new(self->data_->yamal_, stream);
    if (!pystream) {
      Py_XDECREF(pyseqno);
      Py_XDECREF(pyts);
      return NULL;
    }
    PyObject *pydata = PyBytes_FromStringAndSize(data.data(), data.size());
    if (!pydata) {
      Py_XDECREF(pyseqno);
      Py_XDECREF(pyts);
      Py_XDECREF(pystream);
      return NULL;
    }
    auto *obj = PyTuple_New(4);
    PyTuple_SET_ITEM(obj, 0, pyseqno);
    PyTuple_SET_ITEM(obj, 1, pyts);
    PyTuple_SET_ITEM(obj, 2, pystream);
    PyTuple_SET_ITEM(obj, 3, pydata);
    ++self->it_;
    return obj;
  } catch (const std::exception &e) {
    PyErr_SetString(PyExc_RuntimeError, e.what());
    return NULL;
  }
}

static void DataRevIter_dealloc(DataRevIter *self) {
  self->it_.ytp::data_t::reverse_iterator::~reverse_iterator();
  Py_XDECREF(self->data_);
}

static PyTypeObject DataRevIterType = {
    PyVarObject_HEAD_INIT(NULL, 0) "yamal.yamal8.data_iter", /* tp_name */
    sizeof(DataRevIter),                                     /* tp_basicsize */
    0,                                                       /* tp_itemsize */
    (destructor)DataRevIter_dealloc,                         /* tp_dealloc */
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
    "DataRevIter object",                     /* tp_doc */
    0,                                        /* tp_traverse */
    0,                                        /* tp_clear */
    0,                                        /* tp_richcompare */
    0,                                        /* tp_weaklistoffset */
    DataRevIter_iter,                         /* tp_iter */
    (iternextfunc)DataRevIter_iternext,       /* tp_iternext */
    0,                                        /* tp_methods */
    0,                                        /* tp_members */
    0,                                        /* tp_getset */
    0,                                        /* tp_base */
    0,                                        /* tp_dict */
    0,                                        /* tp_descr_get */
    0,                                        /* tp_descr_set */
    0,                                        /* tp_dictoffset */
    0,                                        /* tp_init */
    0,                                        /* tp_alloc */
    0                                         /* tp_new */
};

PyObject *DataRevIter_new(Data *data, ytp::data_t::reverse_iterator it) {
  auto *self = (DataRevIter *)DataRevIterType.tp_alloc(&DataRevIterType, 0);
  if (!self) {
    return NULL;
  }
  self->it_ = it;
  self->data_ = data;
  Py_INCREF(data);
  return (PyObject *)self;
}

static void Data_dealloc(Data *self) {
  self->data_.~data_t();
  Py_XDECREF(self->yamal_);
}

static PyObject *Data_closable(Data *self) {
  try {
    return PyBool_FromLong(self->data_.closable());
  } catch (const std::exception &e) {
    PyErr_SetString(PyExc_RuntimeError, e.what());
    return NULL;
  }
}

static PyObject *Data_close(Data *self) {
  try {
    self->data_.close();
    Py_RETURN_NONE;
  } catch (const std::exception &e) {
    PyErr_SetString(PyExc_RuntimeError, e.what());
    return NULL;
  }
}

static PyObject *Data_closed(Data *self) {
  try {
    return PyBool_FromLong(self->data_.closed());
  } catch (const std::exception &e) {
    PyErr_SetString(PyExc_RuntimeError, e.what());
    return NULL;
  }
}

PyObject *Data_reversed(Data *self) {
  return DataRevIter_new(self, self->data_.rbegin());
}

static PyMethodDef Data_methods[] = {
    {"closable", (PyCFunction)Data_closable, METH_NOARGS,
     "Check if data is closable."},
    {"close", (PyCFunction)Data_close, METH_NOARGS, "Close data."},
    {"closed", (PyCFunction)Data_closed, METH_NOARGS,
     "Check if data is closed."},
    {"__reversed__", (PyCFunction)Data_reversed, METH_NOARGS,
     "Obtain reverse iterator."},
    {NULL, NULL, 0, NULL} /* Sentinel */
};

PyObject *Data_iter(Data *self) {
  return DataIter_new(self, self->data_.begin());
}

static PyTypeObject DataType = {
    PyVarObject_HEAD_INIT(NULL, 0) "yamal.yamal8.data", /* tp_name */
    sizeof(Data),                                       /* tp_basicsize */
    0,                                                  /* tp_itemsize */
    (destructor)Data_dealloc,                           /* tp_dealloc */
    0,                                                  /* tp_print */
    0,                                                  /* tp_getattr */
    0,                                                  /* tp_setattr */
    0,                                                  /* tp_reserved */
    0,                                                  /* tp_repr */
    0,                                                  /* tp_as_number */
    0,                                                  /* tp_as_sequence */
    0,                                                  /* tp_as_mapping */
    0,                                                  /* tp_hash  */
    0,                                                  /* tp_call */
    0,                                                  /* tp_str */
    0,                                                  /* tp_getattro */
    0,                                                  /* tp_setattro */
    0,                                                  /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,           /* tp_flags */
    "Data object",                                      /* tp_doc */
    0,                                                  /* tp_traverse */
    0,                                                  /* tp_clear */
    0,                                                  /* tp_richcompare */
    0,                                                  /* tp_weaklistoffset */
    (getiterfunc)Data_iter,                             /* tp_iter */
    0,                                                  /* tp_iternext */
    Data_methods,                                       /* tp_methods */
    0,                                                  /* tp_members */
    0,                                                  /* tp_getset */
    0,                                                  /* tp_base */
    0,                                                  /* tp_dict */
    0,                                                  /* tp_descr_get */
    0,                                                  /* tp_descr_set */
    0,                                                  /* tp_dictoffset */
    0,                                                  /* tp_init */
    0,                                                  /* tp_alloc */
    0                                                   /* tp_new */
};

struct Yamal {
  PyObject_HEAD;
  ytp::yamal_t yamal_;
};

static PyObject *Stream_write(Stream *self, PyObject *args, PyObject *kwds) {
  static char *kwlist[] = {(char *)"time", (char *)"data", NULL /* Sentinel */};
  unsigned long long time;
  const char *src = nullptr;
  Py_ssize_t sz;
  if (!PyArg_ParseTupleAndKeywords(args, kwds, "Ky#", kwlist, &time, &src,
                                   &sz)) {
    return nullptr;
  }

  try {
    auto data = self->yamal_->yamal_.data();
    auto dst = data.reserve(sz);
    memcpy(dst.data(), src, sz);
    data.commit(time, self->stream_, dst);
    Py_RETURN_NONE;
  } catch (const std::exception &e) {
    PyErr_SetString(PyExc_RuntimeError, e.what());
    return NULL;
  }
}

static PyObject *Data_new(Yamal *yamal) {
  auto *self = (Data *)DataType.tp_alloc(&DataType, 0);
  if (!self) {
    return NULL;
  }
  self->data_ = yamal->yamal_.data();
  self->yamal_ = yamal;
  Py_INCREF(yamal);
  return (PyObject *)self;
}

static PyObject *Streams_new(Yamal *yamal) {
  auto *self = (Streams *)StreamsType.tp_alloc(&StreamsType, 0);
  if (!self) {
    return NULL;
  }
  self->streams_ = yamal->yamal_.streams();
  self->yamal_ = yamal;
  Py_INCREF(yamal);
  return (PyObject *)self;
}

static int Yamal_init(Yamal *self, PyObject *args, PyObject *kwds) {

  static char *kwlist[] = {
      (char *)"path", (char *)"readonly", (char *)"closable",
      (char *)"enable_thread", NULL /* Sentinel */
  };

  char *path;
  int readonly = false;
  int closable = false;
  int enable_thread = true;
  if (!PyArg_ParseTupleAndKeywords(args, kwds, "s|ppp", kwlist, &path,
                                   &readonly, &closable, &enable_thread)) {
    return -1;
  }

  fmc_error_t *err = NULL;

  fmc_fd fd = fmc_fopen(
      path, fmc_fmode(fmc_fmode::READ | (fmc_fmode::WRITE * !readonly)), &err);

  if (err) {
    PyErr_SetString(PyExc_RuntimeError,
                    "Unable to open file in specified path with permissions");
    return -1;
  }

  try {
    self->yamal_ = ytp::yamal_t(fd, closable, enable_thread);
  } catch (const std::exception &e) {
    Py_XDECREF(self);
    PyErr_SetString(PyExc_RuntimeError, e.what());
    return -1;
  }

  return 0;
}

static void Yamal_dealloc(Yamal *self) {
  fmc_fd fd = self->yamal_.fd();
  self->yamal_.~yamal_t();
  if (fmc_fvalid(fd)) {
    fmc_error_t *err = NULL;
    fmc_fclose(fd, &err);
  }
}

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

  try {
    auto [seqno, peer, channel, encoding] =
        self->yamal_.announcement(stream->stream_);
    PyObject *pyseqno = PyLong_FromUnsignedLongLong(seqno);
    if (!pyseqno) {
      return NULL;
    }
    PyObject *pypeer = PyUnicode_FromStringAndSize(peer.data(), peer.size());
    if (!pypeer) {
      Py_XDECREF(pyseqno);
      return NULL;
    }
    PyObject *pychannel =
        PyUnicode_FromStringAndSize(channel.data(), channel.size());
    if (!pychannel) {
      Py_XDECREF(pyseqno);
      Py_XDECREF(pypeer);
      return NULL;
    }
    PyObject *pyencoding =
        PyUnicode_FromStringAndSize(encoding.data(), encoding.size());
    if (!pyencoding) {
      Py_XDECREF(pyseqno);
      Py_XDECREF(pypeer);
      Py_XDECREF(pychannel);
      return NULL;
    }
    auto *obj = PyTuple_New(4);
    if (!obj) {
      Py_XDECREF(pyseqno);
      Py_XDECREF(pypeer);
      Py_XDECREF(pychannel);
      Py_XDECREF(pyencoding);
      return NULL;
    }
    PyTuple_SET_ITEM(obj, 0, pyseqno);
    PyTuple_SET_ITEM(obj, 1, pypeer);
    PyTuple_SET_ITEM(obj, 2, pychannel);
    PyTuple_SET_ITEM(obj, 3, pyencoding);
    return obj;
  } catch (const std::exception &e) {
    PyErr_SetString(PyExc_KeyError, e.what());
    return NULL;
  }
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
    return NULL;
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
    ADD_PY_CLASS(DataIterType, "data_iterator", module);
    ADD_PY_CLASS(DataRevIterType, "data_reverse_iterator", module);
    ADD_PY_CLASS(DataType, "data", module);
    ADD_PY_CLASS(YamalType, "yamal", module);
    if (PyModule_AddStringConstant(module, "__version__", YTP_VERSION) == -1)
      return NULL;
    return module;
  }

  return NULL;
}
