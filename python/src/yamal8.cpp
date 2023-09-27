/******************************************************************************
        COPYRIGHT (c) 2019-2023 by Featuremine Corporation.

        This Source Code Form is subject to the terms of the Mozilla Public
        License, v. 2.0. If a copy of the MPL was not distributed with this
        file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *****************************************************************************/

#define PY_SSIZE_T_CLEAN

#include <fmc/platform.h>
#include <fmc/python/common.h>

#include <ytp/version.h>

#include <Python.h>

#include <fmc++/python/wrapper.hpp>
#include <ytp++/yamal.hpp>

struct Yamal;

struct Streams {
  PyObject_HEAD;
  ytp::streams_t streams_;
  Yamal *yamal_;
};

struct Stream {
  PyObject_HEAD;
  ytp::stream_t stream_;
  Yamal *yamal_;
};

struct Yamal {
  PyObject_HEAD;
  ytp::yamal_t yamal_;
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

struct DataRevIter {
  PyObject_HEAD;
  ytp::data_t::reverse_iterator it_;
  Data *data_;
};

static void Stream_dealloc(Stream *self) {
  self->stream_.~stream_t();
  Py_XDECREF(self->yamal_);
}

PyObject *Stream_id(Stream *self, void *) {
  return PyLong_FromLong(self->stream_.id());
}

PyObject *Stream_seqno(Stream *self, void *) {
  try {
    auto [seqno, peer, channel, encoding] =
        self->yamal_->yamal_.announcement(self->stream_);
    return PyLong_FromLong(seqno);
  } catch (const std::exception &e) {
    PyErr_SetString(PyExc_RuntimeError, "Unable to obtain the stream seqno");
    return NULL;
  }
}

PyObject *Stream_peer(Stream *self, void *) {
  try {
    auto [seqno, peer, channel, encoding] =
        self->yamal_->yamal_.announcement(self->stream_);
    return PyUnicode_FromStringAndSize(peer.data(), peer.size());
  } catch (const std::exception &e) {
    PyErr_SetString(PyExc_RuntimeError, "Unable to obtain the stream peer");
    return NULL;
  }
}

PyObject *Stream_channel(Stream *self, void *) {
  try {
    auto [seqno, peer, channel, encoding] =
        self->yamal_->yamal_.announcement(self->stream_);
    return PyUnicode_FromStringAndSize(channel.data(), channel.size());
  } catch (const std::exception &e) {
    PyErr_SetString(PyExc_RuntimeError, "Unable to obtain the stream channel");
    return NULL;
  }
}

PyObject *Stream_encoding(Stream *self, void *) {
  try {
    auto [seqno, peer, channel, encoding] =
        self->yamal_->yamal_.announcement(self->stream_);
    return PyUnicode_FromStringAndSize(encoding.data(), encoding.size());
  } catch (const std::exception &e) {
    PyErr_SetString(PyExc_RuntimeError, "Unable to obtain the stream encoding");
    return NULL;
  }
}

Py_hash_t Stream_hash(Stream *self) {
  return std::hash<ytp::stream_t>{}(self->stream_);
}

static PyGetSetDef Stream_getset[] = {
    {(char *)"id", (getter)Stream_id, NULL,
     (char *)"Returns the numerical identifier associated with the stream.",
     NULL},
    {(char *)"seqno", (getter)Stream_seqno, NULL,
     (char *)"Returns the sequence number associated with the stream.", NULL},
    {(char *)"peer", (getter)Stream_peer, NULL,
     (char *)"Returns the peer associated with the stream.", NULL},
    {(char *)"channel", (getter)Stream_channel, NULL,
     (char *)"Returns the channel associated with the stream.", NULL},
    {(char *)"encoding", (getter)Stream_encoding, NULL,
     (char *)"Returns the encoding associated with the stream.", NULL},
    {NULL, NULL, NULL, NULL, NULL} /* Sentinel */
};

static PyObject *Stream_write(Stream *self, PyObject *args, PyObject *kwds);

static PyMethodDef Stream_methods[] = {
    {"write", (PyCFunction)Stream_write, METH_VARARGS | METH_KEYWORDS,
     "Write a new message"},
    {NULL, NULL, 0, NULL} /* Sentinel */
};

static PyObject *Stream_str(Stream *self) {
  try {
    std::ostringstream o;
    auto [seqno, peer, channel, encoding] =
        self->yamal_->yamal_.announcement(self->stream_);
    o << "stream_t(id=" << self->stream_.id() << ",seqno=" << seqno
      << ",peer=" << peer << ",channel=" << channel << ",encoding=" << encoding
      << ")";
    auto os = o.str();
    return PyUnicode_FromString(os.c_str());
  } catch (const std::exception &e) {
    PyErr_SetString(PyExc_RuntimeError,
                    "Unable to generate string representation of stream");
    return NULL;
  }
}

static PyObject *Stream_richcompare(Stream *obj1, Stream *obj2, int op);

static PyTypeObject StreamType = {
    .ob_base = PyVarObject_HEAD_INIT(NULL, 0).tp_name = "yamal.yamal8.stream",
    .tp_basicsize = sizeof(Stream),
    .tp_dealloc = (destructor)Stream_dealloc,
    .tp_repr = (reprfunc)Stream_str,
    .tp_hash = (hashfunc)Stream_hash,
    .tp_str = (reprfunc)Stream_str,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_doc = "Stream object",
    .tp_richcompare = (richcmpfunc)Stream_richcompare,
    .tp_methods = Stream_methods,
    .tp_getset = Stream_getset};

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

  try {
    auto sl = self->streams_.lookup(peer, channel);

    if (!sl) {
      PyErr_SetString(PyExc_KeyError, "Unable to find stream");
      return NULL;
    }

    auto pystream =
        fmc::python::object::from_new(Stream_new(self->yamal_, sl->first));
    if (!pystream) {
      return NULL;
    }

    return fmc::python::tuple::from_args(pystream, sl->second);
  } catch (const std::exception &e) {
    PyErr_SetString(PyExc_RuntimeError, e.what());
    return NULL;
  }
}

static PyMethodDef Streams_methods[] = {
    {"announce", (PyCFunction)Streams_announce, METH_VARARGS | METH_KEYWORDS,
     "Announce a new stream and obtain it"},
    {"lookup", (PyCFunction)Streams_lookup, METH_VARARGS | METH_KEYWORDS,
     "Obtain the desired stream"},
    {NULL, NULL, 0, NULL} /* Sentinel */
};

static PyTypeObject StreamsType = {
    .ob_base = PyVarObject_HEAD_INIT(NULL, 0).tp_name = "yamal.yamal8.streams",
    .tp_basicsize = sizeof(Streams),
    .tp_dealloc = (destructor)Streams_dealloc,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_doc = "Streams object",
    .tp_methods = Streams_methods};

PyObject *DataRevIter_iter(PyObject *self) {
  Py_INCREF(self);
  return self;
}

PyObject *DataRevIter_iternext(DataRevIter *self) {

  try {
    if (self->it_ == self->data_->data_.rend()) {
      PyErr_SetNone(PyExc_StopIteration);
      return NULL;
    }
    auto [seqno, ts, stream, data] = *self->it_;
    auto pyseqno =
        fmc::python::object::from_new(PyLong_FromUnsignedLongLong(seqno));
    if (!pyseqno) {
      return NULL;
    }
    auto pyts = fmc::python::object::from_new(PyLong_FromUnsignedLongLong(ts));
    if (!pyts) {
      return NULL;
    }
    auto pystream =
        fmc::python::object::from_new(Stream_new(self->data_->yamal_, stream));
    if (!pystream) {
      return NULL;
    }
    auto pydata = fmc::python::object::from_new(
        PyBytes_FromStringAndSize(data.data(), data.size()));
    if (!pydata) {
      return NULL;
    }
    auto obj = fmc::python::tuple::from_args(pyseqno, pyts, pystream, pydata);
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

PyObject *DataRevIter_nb_int(DataRevIter *self) {
  try {
    return PyLong_FromUnsignedLongLong(ytp_mmnode_offs(self->it_));
  } catch (const std::exception &e) {
    PyErr_SetString(PyExc_RuntimeError, e.what());
    return NULL;
  }
}

static PyNumberMethods DataRevIter_as_number = {
    .nb_int = (unaryfunc)DataRevIter_nb_int};

static PyTypeObject DataRevIterType = {
    .ob_base = PyVarObject_HEAD_INIT(NULL, 0).tp_name =
        "yamal.yamal8.data_rev_iter",
    .tp_basicsize = sizeof(DataRevIter),
    .tp_dealloc = (destructor)DataRevIter_dealloc,
    .tp_as_number = &DataRevIter_as_number,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_doc = "DataRevIter object",
    .tp_iter = DataRevIter_iter,
    .tp_iternext = (iternextfunc)DataRevIter_iternext};

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

PyObject *DataIter_iter(PyObject *self) {
  Py_INCREF(self);
  return self;
}

PyObject *DataIter_iternext(DataIter *self) {

  try {
    if (self->it_ == self->data_->data_.end()) {
      PyErr_SetNone(PyExc_StopIteration);
      return NULL;
    }
    auto [seqno, ts, stream, data] = *self->it_;
    auto pyseqno =
        fmc::python::object::from_new(PyLong_FromUnsignedLongLong(seqno));
    if (!pyseqno) {
      return NULL;
    }
    auto pyts = fmc::python::object::from_new(PyLong_FromUnsignedLongLong(ts));
    if (!pyts) {
      return NULL;
    }
    auto pystream =
        fmc::python::object::from_new(Stream_new(self->data_->yamal_, stream));
    if (!pystream) {
      return NULL;
    }
    auto pydata = fmc::python::object::from_new(
        PyBytes_FromStringAndSize(data.data(), data.size()));
    if (!pydata) {
      return NULL;
    }
    auto obj = fmc::python::tuple::from_args(pyseqno, pyts, pystream, pydata);
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

PyObject *DataIter_reversed(DataIter *self) {
  try {
    return DataRevIter_new(self->data_,
                           ytp::data_t::reverse_iterator(self->it_));
  } catch (const std::exception &e) {
    PyErr_SetString(PyExc_RuntimeError, e.what());
    return NULL;
  }
}

static PyMethodDef DataIter_methods[] = {
    {"__reversed__", (PyCFunction)DataIter_reversed, METH_NOARGS,
     "Obtain reverse iterator."},
    {NULL, NULL, 0, NULL} /* Sentinel */
};

PyObject *DataIter_nb_int(DataIter *self) {
  try {
    return PyLong_FromUnsignedLongLong(ytp_mmnode_offs(self->it_));
  } catch (const std::exception &e) {
    PyErr_SetString(PyExc_RuntimeError, e.what());
    return NULL;
  }
}

static PyNumberMethods DataIter_as_number = {.nb_int =
                                                 (unaryfunc)DataIter_nb_int};

static PyTypeObject DataIterType = {
    .ob_base = PyVarObject_HEAD_INIT(NULL, 0).tp_name =
        "yamal.yamal8.data_iter",
    .tp_basicsize = sizeof(DataIter),
    .tp_dealloc = (destructor)DataIter_dealloc,
    .tp_as_number = &DataIter_as_number,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_doc = "DataIter object",
    .tp_iter = DataIter_iter,
    .tp_iternext = (iternextfunc)DataIter_iternext,
    .tp_methods = DataIter_methods};

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
  try {
    return DataRevIter_new(self, self->data_.rbegin());
  } catch (const std::exception &e) {
    PyErr_SetString(PyExc_RuntimeError, e.what());
    return NULL;
  }
}

static PyObject *Data_seek(Data *self, PyObject *args, PyObject *kwds) {
  static char *kwlist[] = {
      (char *)"offset", NULL /* Sentinel */
  };

  unsigned long long offset;
  if (!PyArg_ParseTupleAndKeywords(args, kwds, "K", kwlist, &offset)) {
    return NULL;
  }

  try {
    return DataIter_new(self, self->data_.seek(offset));
  } catch (const std::exception &e) {
    PyErr_SetString(PyExc_RuntimeError, e.what());
    return NULL;
  }
}

static PyMethodDef Data_methods[] = {
    {"closable", (PyCFunction)Data_closable, METH_NOARGS,
     "Check if data is closable."},
    {"close", (PyCFunction)Data_close, METH_NOARGS, "Close data."},
    {"closed", (PyCFunction)Data_closed, METH_NOARGS,
     "Check if data is closed."},
    {"__reversed__", (PyCFunction)Data_reversed, METH_NOARGS,
     "Obtain reverse iterator."},
    {"seek", (PyCFunction)Data_seek, METH_VARARGS | METH_KEYWORDS,
     "Obtain the iterator for the desired position"},
    {NULL, NULL, 0, NULL} /* Sentinel */
};

PyObject *Data_iter(Data *self) {
  try {
    return DataIter_new(self, self->data_.begin());
  } catch (const std::exception &e) {
    PyErr_SetString(PyExc_RuntimeError, e.what());
    return NULL;
  }
}

static PyTypeObject DataType = {
    .ob_base = PyVarObject_HEAD_INIT(NULL, 0).tp_name = "yamal.yamal8.data",
    .tp_basicsize = sizeof(Data),
    .tp_dealloc = (destructor)Data_dealloc,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_doc = "Data object",
    .tp_iter = (getiterfunc)Data_iter,
    .tp_methods = Data_methods};

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
  auto pself = fmc::python::object::from_new((PyObject *)self);
  try {
    self->data_ = yamal->yamal_.data();
  } catch (const std::exception &e) {
    PyErr_SetString(PyExc_RuntimeError, e.what());
    return nullptr;
  }
  self->yamal_ = yamal;
  Py_INCREF(yamal);
  return (PyObject *)pself.steal_ref();
}

static PyObject *Streams_new(Yamal *yamal) {
  auto *self = (Streams *)StreamsType.tp_alloc(&StreamsType, 0);
  if (!self) {
    return NULL;
  }
  auto pself = fmc::python::object::from_new((PyObject *)self);
  try {
    self->streams_ = yamal->yamal_.streams();
  } catch (const std::exception &e) {
    PyErr_SetString(PyExc_RuntimeError, e.what());
    return nullptr;
  }
  self->yamal_ = yamal;
  Py_INCREF(yamal);
  return (PyObject *)pself.steal_ref();
}

static int Yamal_init(Yamal *self, PyObject *args, PyObject *kwds) {

  static char *kwlist[] = {
      (char *)"path", (char *)"readonly", (char *)"enable_thread",
      (char *)"closable", NULL /* Sentinel */
  };

  char *path;
  int readonly = false;
  int enable_thread = true;
  int closable = false;
  if (!PyArg_ParseTupleAndKeywords(args, kwds, "s|ppp", kwlist, &path,
                                   &readonly, &enable_thread, &closable)) {
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
    self->yamal_ = ytp::yamal_t(fd, enable_thread, closable);
  } catch (const std::exception &e) {
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
    return fmc::python::tuple::from_args(seqno, peer, channel, encoding);
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
    .ob_base = PyVarObject_HEAD_INIT(NULL, 0).tp_name = "yamal.yamal8.yamal",
    .tp_basicsize = sizeof(Yamal),
    .tp_dealloc = (destructor)Yamal_dealloc,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_doc = "Yamal object",
    .tp_methods = Yamal_methods,
    .tp_init = (initproc)Yamal_init,
    .tp_new = Yamal_new};

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
