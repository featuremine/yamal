/******************************************************************************
        COPYRIGHT (c) 2019-2023 by Featuremine Corporation.

        This Source Code Form is subject to the terms of the Mozilla Public
        License, v. 2.0. If a copy of the MPL was not distributed with this
        file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *****************************************************************************/

#define PY_SSIZE_T_CLEAN

#include <Python.h>
#include <algorithm>
#include <deque>
#include <fcntl.h>
#include <memory>
#include <optional>
#include <string>

#include <fmc/files.h>

#include <ytp/api.h>
#include <ytp/python/py_api.h>
#include <ytp/python/py_wrapper.h>
#include <ytp/sequence.h>
#include <ytp/version.h>

#include "fmc/python/common.h"

using namespace std;

struct YTPSequenceBase : std::enable_shared_from_this<YTPSequenceBase> {
  ~YTPSequenceBase();
  void clear();

  ytp_sequence_shared_t *shared_seq = nullptr;
  std::deque<std::pair<YTPSequenceBase *, PyObject *>> peer_cb;
  std::deque<std::pair<YTPSequenceBase *, PyObject *>> channel_cb;
  std::deque<std::tuple<YTPSequenceBase *, PyObject *, ytp_channel_t>> data_cb;
  std::deque<std::tuple<YTPSequenceBase *, PyObject *, std::string>> prfx_cb;
  std::deque<std::pair<struct YTPTransactions *, std::string>> transactions_cb;
};

struct YTPSequence {
  PyObject_HEAD;
  std::shared_ptr<YTPSequenceBase> seq;
};

struct YTPPeer {
  PyObject_HEAD;
  std::shared_ptr<YTPSequenceBase> seq;
  ytp_peer_t id;
};

struct YTPChannel {
  PyObject_HEAD;
  std::shared_ptr<YTPSequenceBase> seq;
  ytp_channel_t id;
};

struct YTPStream {
  PyObject_HEAD;
  std::shared_ptr<YTPSequenceBase> seq;
  ytp_peer_t peer_id;
  ytp_channel_t channel_id;
};

struct YTPTransaction {
  ytp_peer_t peer;
  ytp_channel_t channel;
  uint64_t time;
  std::string_view data;
};

struct YTPTransactions {
  PyObject_HEAD;
  std::shared_ptr<YTPSequenceBase> seq;
  std::optional<YTPTransaction> transaction;
};

static PyObject *YTPSequence_new(PyTypeObject *subtype, PyObject *args,
                                 PyObject *kwds);
static int YTPSequence_init(YTPSequence *self, PyObject *args, PyObject *kwds);
static void YTPSequence_dealloc(YTPSequence *self);
static PyObject *YTPSequence_peer_callback(YTPSequence *self, PyObject *args,
                                           PyObject *kwds);
static PyObject *YTPSequence_channel_callback(YTPSequence *self, PyObject *args,
                                              PyObject *kwds);
static PyObject *YTPSequence_data_callback(YTPSequence *self, PyObject *args,
                                           PyObject *kwds);
static PyObject *YTPSequence_peer(YTPSequence *self, PyObject *args,
                                  PyObject *kwds);
static PyObject *YTPSequence_poll(YTPSequence *self);
static PyObject *YTPSequence_remove_callbacks(YTPSequence *self);

static PyObject *YTPPeer_new(PyTypeObject *subtype, PyObject *args,
                             PyObject *kwds);
static int YTPPeer_init(YTPPeer *self, PyObject *args, PyObject *kwds);
static void YTPPeer_dealloc(YTPPeer *self);
static PyObject *YTPPeer_name(YTPPeer *self);
static PyObject *YTPPeer_id(YTPPeer *self);
static PyObject *YTPPeer_stream(YTPPeer *self, PyObject *args, PyObject *kwds);
static PyObject *YTPPeer_channel(YTPPeer *self, PyObject *args, PyObject *kwds);

static PyObject *YTPChannel_new(PyTypeObject *subtype, PyObject *args,
                                PyObject *kwds);
static int YTPChannel_init(YTPChannel *self, PyObject *args, PyObject *kwds);
static void YTPChannel_dealloc(YTPChannel *self);
static PyObject *YTPChannel_name(YTPChannel *self);
static PyObject *YTPChannel_id(YTPChannel *self);
static PyObject *YTPChannel_data_callback(YTPChannel *self, PyObject *args,
                                          PyObject *kwds);

static PyObject *YTPStream_new(PyTypeObject *subtype, PyObject *args,
                               PyObject *kwds);
static int YTPStream_init(YTPStream *self, PyObject *args, PyObject *kwds);
static void YTPStream_dealloc(YTPStream *self);
static PyObject *YTPStream_write(YTPStream *self, PyObject *args,
                                 PyObject *kwds);
static PyObject *YTPStream_peer(YTPStream *self);
static PyObject *YTPStream_channel(YTPStream *self);

static PyObject *YTPTransactions_new(PyTypeObject *subtype, PyObject *args,
                                     PyObject *kwds);
static int YTPTransactions_init(YTPTransactions *self, PyObject *args,
                                PyObject *kwds);
static void YTPTransactions_dealloc(YTPTransactions *self);
static PyObject *YTPTransactions_subscribe(YTPTransactions *self,
                                           PyObject *args, PyObject *kwds);
static PyObject *YTPTransactions_iter(YTPTransactions *self);
static PyObject *YTPTransactions_next(YTPTransactions *self);
static PyObject *PyAPIWrapper_new(PyTypeObject *subtype, PyObject *args,
                                  PyObject *kwds);
static int PyAPIWrapper_init(PyAPIWrapper *self, PyObject *args,
                             PyObject *kwds);
static void PyAPIWrapper_dealloc(PyAPIWrapper *self);

static PyMethodDef YTPSequence_methods[] = {
    {"peer_callback", (PyCFunction)YTPSequence_peer_callback,
     METH_VARARGS | METH_KEYWORDS, "Set callback for peers in YTP file"},
    {"channel_callback", (PyCFunction)YTPSequence_channel_callback,
     METH_VARARGS | METH_KEYWORDS, "Set callback for channels in YTP file"},
    {"data_callback", (PyCFunction)YTPSequence_data_callback,
     METH_VARARGS | METH_KEYWORDS, "Set callback for data by channel pattern"},
    {"peer", (PyCFunction)YTPSequence_peer, METH_VARARGS | METH_KEYWORDS,
     "Obtain desired peer by name"},
    {"poll", (PyCFunction)YTPSequence_poll, METH_NOARGS,
     "Poll for messages in sequence file"},
    {"remove_callbacks", (PyCFunction)YTPSequence_remove_callbacks, METH_NOARGS,
     "Remove all the registered callbacks"},
    {NULL, NULL, 0, NULL} /* Sentinel */
};

static PyMethodDef YTPPeer_methods[] = {
    {"name", (PyCFunction)YTPPeer_name, METH_NOARGS, "Peer name"},
    {"id", (PyCFunction)YTPPeer_id, METH_NOARGS, "Peer id"},
    {"stream", (PyCFunction)YTPPeer_stream, METH_VARARGS | METH_KEYWORDS,
     "Obtain stream for desired channel"},
    {"channel", (PyCFunction)YTPPeer_channel, METH_VARARGS | METH_KEYWORDS,
     "Obtain desired channel by name"},
    {NULL, NULL, 0, NULL} /* Sentinel */
};

static PyMethodDef YTPChannel_methods[] = {
    {"name", (PyCFunction)YTPChannel_name, METH_NOARGS, "Channel name"},
    {"id", (PyCFunction)YTPChannel_id, METH_NOARGS, "Channel id"},
    {"data_callback", (PyCFunction)YTPChannel_data_callback,
     METH_VARARGS | METH_KEYWORDS, "Set callback for data in channel"},
    {NULL, NULL, 0, NULL} /* Sentinel */
};

static PyMethodDef YTPStream_methods[] = {
    {"write", (PyCFunction)YTPStream_write, METH_VARARGS | METH_KEYWORDS,
     "Write message to YTP"},
    {"channel", (PyCFunction)YTPStream_channel, METH_NOARGS,
     "Obtain channel related to stream"},
    {"peer", (PyCFunction)YTPStream_peer, METH_NOARGS,
     "Obtain peer related to stream"},
    {NULL, NULL, 0, NULL} /* Sentinel */
};

static PyMethodDef YTPTransactions_methods[] = {
    {"subscribe", (PyCFunction)YTPTransactions_subscribe,
     METH_VARARGS | METH_KEYWORDS, "Subscribe to desired pattern"},
    {NULL, NULL, 0, NULL} /* Sentinel */
};

static PyTypeObject YTPSequenceType = {
    PyVarObject_HEAD_INIT(NULL, 0) "ytp.sequence", /* tp_name */
    sizeof(YTPSequence),                           /* tp_basicsize */
    0,                                             /* tp_itemsize */
    (destructor)YTPSequence_dealloc,               /* tp_dealloc */
    0,                                             /* tp_print */
    0,                                             /* tp_getattr */
    0,                                             /* tp_setattr */
    0,                                             /* tp_reserved */
    0,                                             /* tp_repr */
    0,                                             /* tp_as_number */
    0,                                             /* tp_as_sequence */
    0,                                             /* tp_as_mapping */
    0,                                             /* tp_hash  */
    0,                                             /* tp_call */
    0,                                             /* tp_str */
    0,                                             /* tp_getattro */
    0,                                             /* tp_setattro */
    0,                                             /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,      /* tp_flags */
    "YTP sequence",                                /* tp_doc */
    0,                                             /* tp_traverse */
    0,                                             /* tp_clear */
    0,                                             /* tp_richcompare */
    0,                                             /* tp_weaklistoffset */
    0,                                             /* tp_iter */
    0,                                             /* tp_iternext */
    YTPSequence_methods,                           /* tp_methods */
    0,                                             /* tp_members */
    0,                                             /* tp_getset */
    0,                                             /* tp_base */
    0,                                             /* tp_dict */
    0,                                             /* tp_descr_get */
    0,                                             /* tp_descr_set */
    0,                                             /* tp_dictoffset */
    (initproc)YTPSequence_init,                    /* tp_init */
    0,                                             /* tp_alloc */
    YTPSequence_new,                               /* tp_new */
};

static PyTypeObject YTPPeerType = {
    PyVarObject_HEAD_INIT(NULL, 0) "ytp.peer", /* tp_name */
    sizeof(YTPPeer),                           /* tp_basicsize */
    0,                                         /* tp_itemsize */
    (destructor)YTPPeer_dealloc,               /* tp_dealloc */
    0,                                         /* tp_print */
    0,                                         /* tp_getattr */
    0,                                         /* tp_setattr */
    0,                                         /* tp_reserved */
    0,                                         /* tp_repr */
    0,                                         /* tp_as_number */
    0,                                         /* tp_as_sequence */
    0,                                         /* tp_as_mapping */
    0,                                         /* tp_hash  */
    0,                                         /* tp_call */
    0,                                         /* tp_str */
    0,                                         /* tp_getattro */
    0,                                         /* tp_setattro */
    0,                                         /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,  /* tp_flags */
    "YTP peer",                                /* tp_doc */
    0,                                         /* tp_traverse */
    0,                                         /* tp_clear */
    0,                                         /* tp_richcompare */
    0,                                         /* tp_weaklistoffset */
    0,                                         /* tp_iter */
    0,                                         /* tp_iternext */
    YTPPeer_methods,                           /* tp_methods */
    0,                                         /* tp_members */
    0,                                         /* tp_getset */
    0,                                         /* tp_base */
    0,                                         /* tp_dict */
    0,                                         /* tp_descr_get */
    0,                                         /* tp_descr_set */
    0,                                         /* tp_dictoffset */
    (initproc)YTPPeer_init,                    /* tp_init */
    0,                                         /* tp_alloc */
    YTPPeer_new,                               /* tp_new */
};

static PyTypeObject YTPChannelType = {
    PyVarObject_HEAD_INIT(NULL, 0) "ytp.channel", /* tp_name */
    sizeof(YTPChannel),                           /* tp_basicsize */
    0,                                            /* tp_itemsize */
    (destructor)YTPChannel_dealloc,               /* tp_dealloc */
    0,                                            /* tp_print */
    0,                                            /* tp_getattr */
    0,                                            /* tp_setattr */
    0,                                            /* tp_reserved */
    0,                                            /* tp_repr */
    0,                                            /* tp_as_number */
    0,                                            /* tp_as_sequence */
    0,                                            /* tp_as_mapping */
    0,                                            /* tp_hash  */
    0,                                            /* tp_call */
    0,                                            /* tp_str */
    0,                                            /* tp_getattro */
    0,                                            /* tp_setattro */
    0,                                            /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,     /* tp_flags */
    "YTP channel",                                /* tp_doc */
    0,                                            /* tp_traverse */
    0,                                            /* tp_clear */
    0,                                            /* tp_richcompare */
    0,                                            /* tp_weaklistoffset */
    0,                                            /* tp_iter */
    0,                                            /* tp_iternext */
    YTPChannel_methods,                           /* tp_methods */
    0,                                            /* tp_members */
    0,                                            /* tp_getset */
    0,                                            /* tp_base */
    0,                                            /* tp_dict */
    0,                                            /* tp_descr_get */
    0,                                            /* tp_descr_set */
    0,                                            /* tp_dictoffset */
    (initproc)YTPChannel_init,                    /* tp_init */
    0,                                            /* tp_alloc */
    YTPChannel_new,                               /* tp_new */
};

static PyTypeObject YTPStreamType = {
    PyVarObject_HEAD_INIT(NULL, 0) "ytp.stream", /* tp_name */
    sizeof(YTPStream),                           /* tp_basicsize */
    0,                                           /* tp_itemsize */
    (destructor)YTPStream_dealloc,               /* tp_dealloc */
    0,                                           /* tp_print */
    0,                                           /* tp_getattr */
    0,                                           /* tp_setattr */
    0,                                           /* tp_reserved */
    0,                                           /* tp_repr */
    0,                                           /* tp_as_number */
    0,                                           /* tp_as_sequence */
    0,                                           /* tp_as_mapping */
    0,                                           /* tp_hash  */
    0,                                           /* tp_call */
    0,                                           /* tp_str */
    0,                                           /* tp_getattro */
    0,                                           /* tp_setattro */
    0,                                           /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,    /* tp_flags */
    "YTP stream",                                /* tp_doc */
    0,                                           /* tp_traverse */
    0,                                           /* tp_clear */
    0,                                           /* tp_richcompare */
    0,                                           /* tp_weaklistoffset */
    0,                                           /* tp_iter */
    0,                                           /* tp_iternext */
    YTPStream_methods,                           /* tp_methods */
    0,                                           /* tp_members */
    0,                                           /* tp_getset */
    0,                                           /* tp_base */
    0,                                           /* tp_dict */
    0,                                           /* tp_descr_get */
    0,                                           /* tp_descr_set */
    0,                                           /* tp_dictoffset */
    (initproc)YTPStream_init,                    /* tp_init */
    0,                                           /* tp_alloc */
    YTPStream_new,                               /* tp_new */
};

static PyTypeObject YTPTransactionsType = {
    PyVarObject_HEAD_INIT(NULL, 0) "ytp.transactions", /* tp_name */
    sizeof(YTPTransactions),                           /* tp_basicsize */
    0,                                                 /* tp_itemsize */
    (destructor)YTPTransactions_dealloc,               /* tp_dealloc */
    0,                                                 /* tp_print */
    0,                                                 /* tp_getattr */
    0,                                                 /* tp_setattr */
    0,                                                 /* tp_reserved */
    0,                                                 /* tp_repr */
    0,                                                 /* tp_as_number */
    0,                                                 /* tp_as_sequence */
    0,                                                 /* tp_as_mapping */
    0,                                                 /* tp_hash  */
    0,                                                 /* tp_call */
    0,                                                 /* tp_str */
    0,                                                 /* tp_getattro */
    0,                                                 /* tp_setattro */
    0,                                                 /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,          /* tp_flags */
    "YTP stream",                                      /* tp_doc */
    0,                                                 /* tp_traverse */
    0,                                                 /* tp_clear */
    0,                                                 /* tp_richcompare */
    0,                                                 /* tp_weaklistoffset */
    (getiterfunc)YTPTransactions_iter,                 /* tp_iter */
    (iternextfunc)YTPTransactions_next,                /* tp_iternext */
    YTPTransactions_methods,                           /* tp_methods */
    0,                                                 /* tp_members */
    0,                                                 /* tp_getset */
    0,                                                 /* tp_base */
    0,                                                 /* tp_dict */
    0,                                                 /* tp_descr_get */
    0,                                                 /* tp_descr_set */
    0,                                                 /* tp_dictoffset */
    (initproc)YTPTransactions_init,                    /* tp_init */
    0,                                                 /* tp_alloc */
    YTPTransactions_new,                               /* tp_new */
};

static PyTypeObject PyAPIWrapperType = {
    PyVarObject_HEAD_INIT(NULL, 0) "ytp.APIWrapper", /* tp_name */
    sizeof(PyAPIWrapper),                            /* tp_basicsize */
    0,                                               /* tp_itemsize */
    (destructor)PyAPIWrapper_dealloc,                /* tp_dealloc */
    0,                                               /* tp_print */
    0,                                               /* tp_getattr */
    0,                                               /* tp_setattr */
    0,                                               /* tp_reserved */
    0,                                               /* tp_repr */
    0,                                               /* tp_as_number */
    0,                                               /* tp_as_sequence */
    0,                                               /* tp_as_mapping */
    0,                                               /* tp_hash  */
    0,                                               /* tp_call */
    0,                                               /* tp_str */
    0,                                               /* tp_getattro */
    0,                                               /* tp_setattro */
    0,                                               /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,        /* tp_flags */
    "YTP API Wrapper",                               /* tp_doc */
    0,                                               /* tp_traverse */
    0,                                               /* tp_clear */
    0,                                               /* tp_richcompare */
    0,                                               /* tp_weaklistoffset */
    0,                                               /* tp_iter */
    0,                                               /* tp_iternext */
    0,                                               /* tp_methods */
    0,                                               /* tp_members */
    0,                                               /* tp_getset */
    0,                                               /* tp_base */
    0,                                               /* tp_dict */
    0,                                               /* tp_descr_get */
    0,                                               /* tp_descr_set */
    0,                                               /* tp_dictoffset */
    (initproc)PyAPIWrapper_init,                     /* tp_init */
    0,                                               /* tp_alloc */
    PyAPIWrapper_new,                                /* tp_new */
};

bool PyYTPSequence_Check(PyObject *obj) {
  return PyObject_TypeCheck(obj, &YTPSequenceType);
}

ytp_sequence_shared_t *PyYTPSequence_Shared(PyObject *obj) {
  return reinterpret_cast<YTPSequence *>(obj)->seq->shared_seq;
}

bool PyYTPPeer_Check(PyObject *obj) {
  return PyObject_TypeCheck(obj, &YTPPeerType);
}

ytp_sequence_shared_t *PyYTPPeer_Shared(PyObject *obj) {
  return reinterpret_cast<YTPPeer *>(obj)->seq->shared_seq;
}

ytp_peer_t PyYTPPeer_Id(PyObject *obj) {
  return reinterpret_cast<YTPPeer *>(obj)->id;
}

bool PyYTPChannel_Check(PyObject *obj) {
  return PyObject_TypeCheck(obj, &YTPChannelType);
}

ytp_sequence_shared_t *PyYTPChannel_Shared(PyObject *obj) {
  return reinterpret_cast<YTPChannel *>(obj)->seq->shared_seq;
}

ytp_channel_t PyYTPChannel_Id(PyObject *obj) {
  return reinterpret_cast<YTPChannel *>(obj)->id;
}

bool PyYTPStream_Check(PyObject *obj) {
  return PyObject_TypeCheck(obj, &YTPStreamType);
}

ytp_sequence_shared_t *PyYTPStream_Shared(PyObject *obj) {
  return reinterpret_cast<YTPStream *>(obj)->seq->shared_seq;
}

ytp_peer_t PyYTPStream_PeerId(PyObject *obj) {
  return reinterpret_cast<YTPStream *>(obj)->peer_id;
}

ytp_channel_t PyYTPStream_ChannelId(PyObject *obj) {
  return reinterpret_cast<YTPStream *>(obj)->channel_id;
}

static void ytp_sequence_peer_cb_wrapper(void *closure_, ytp_peer_t peer_id,
                                         size_t sz, const char *name) {
  if (PyErr_Occurred()) {
    return;
  }

  auto &closure = *(decltype(YTPSequenceBase::peer_cb)::value_type *)closure_;
  auto *seq = closure.first;
  auto *callback = closure.second;

  auto *py_peer = PyObject_CallObject(&YTPPeerType.ob_base.ob_base, NULL);
  if (!py_peer || PyErr_Occurred()) {
    return;
  }
  auto &peer = *((YTPPeer *)py_peer);
  peer.seq = seq->shared_from_this();
  peer.id = peer_id;

  Py_XDECREF(PyObject_CallFunction(callback, "Os#", py_peer, name, Py_ssize_t(sz)));

  Py_XDECREF(py_peer);
}

static void ytp_sequence_channel_cb_wrapper(void *closure_, ytp_peer_t peer_id,
                                            ytp_channel_t channel_id,
                                            uint64_t time, size_t sz,
                                            const char *name) {
  if (PyErr_Occurred()) {
    return;
  }

  auto &closure =
      *(decltype(YTPSequenceBase::channel_cb)::value_type *)closure_;
  auto *seq = closure.first;
  auto *callback = closure.second;

  auto py_peer = PyObject_CallObject(&YTPPeerType.ob_base.ob_base, NULL);
  if (!py_peer || PyErr_Occurred()) {
    return;
  }

  auto &peer = *((YTPPeer *)py_peer);
  peer.seq = seq->shared_from_this();
  peer.id = peer_id;

  auto py_channel = PyObject_CallObject(&YTPChannelType.ob_base.ob_base, NULL);
  if (!py_channel || PyErr_Occurred()) {
    return;
  }

  auto &channel = *((YTPChannel *)py_channel);
  channel.seq = seq->shared_from_this();
  channel.id = channel_id;

  Py_XDECREF(PyObject_CallFunction(callback, "OOKs#", py_channel, py_peer, time, name,
                        Py_ssize_t(sz)));

  Py_XDECREF(py_peer);
  Py_XDECREF(py_channel);
}

static void ytp_sequence_data_cb_wrapper(void *closure_, ytp_peer_t peer_id,
                                         ytp_channel_t channel_id,
                                         uint64_t time, size_t sz,
                                         const char *data) {
  if (PyErr_Occurred()) {
    return;
  }

  auto &closure = *(decltype(YTPSequenceBase::data_cb)::value_type *)closure_;
  auto *seq = std::get<YTPSequenceBase *>(closure);
  auto *callback = std::get<PyObject *>(closure);

  auto *py_peer = PyObject_CallObject(&YTPPeerType.ob_base.ob_base, NULL);
  if (!py_peer || PyErr_Occurred()) {
    return;
  }

  auto &peer = *((YTPPeer *)py_peer);
  peer.seq = seq->shared_from_this();
  peer.id = peer_id;

  auto *py_channel = PyObject_CallObject(&YTPChannelType.ob_base.ob_base, NULL);
  if (!py_channel || PyErr_Occurred()) {
    return;
  }

  auto &channel = *((YTPChannel *)py_channel);
  channel.seq = seq->shared_from_this();
  channel.id = channel_id;

  Py_XDECREF(PyObject_CallFunction(callback, "OOKy#", py_peer, py_channel, time, data,
                        Py_ssize_t(sz)));

  Py_XDECREF(py_peer);
  Py_XDECREF(py_channel);
}

static void ytp_sequence_data_cb_transactions_wrapper(void *closure_,
                                                      ytp_peer_t peer_id,
                                                      ytp_channel_t channel_id,
                                                      uint64_t time, size_t sz,
                                                      const char *data) {
  auto &closure =
      *(decltype(YTPSequenceBase::transactions_cb)::value_type *)closure_;
  auto &self = *std::get<YTPTransactions *>(closure);

  self.transaction.emplace(
      YTPTransaction{peer_id, channel_id, time, std::string_view(data, sz)});
}

static void ytp_sequence_prfx_cb_wrapper(void *closure_, ytp_peer_t peer_id,
                                         ytp_channel_t channel_id,
                                         uint64_t time, size_t sz,
                                         const char *data) {
  auto &closure = *(decltype(YTPSequenceBase::prfx_cb)::value_type *)closure_;
  auto *seq = std::get<YTPSequenceBase *>(closure);
  auto *callback = std::get<PyObject *>(closure);

  auto *py_peer = PyObject_CallObject(&YTPPeerType.ob_base.ob_base, NULL);
  if (!py_peer || PyErr_Occurred()) {
    return;
  }

  auto &peer = *((YTPPeer *)py_peer);
  peer.seq = seq->shared_from_this();
  peer.id = peer_id;

  auto *py_channel = PyObject_CallObject(&YTPChannelType.ob_base.ob_base, NULL);
  if (!py_channel || PyErr_Occurred()) {
    return;
  }

  auto &channel = *((YTPChannel *)py_channel);
  channel.seq = seq->shared_from_this();
  channel.id = channel_id;

  Py_XDECREF(PyObject_CallFunction(callback, "OOKy#", py_peer, py_channel, time, data,
                        Py_ssize_t(sz)));

  Py_XDECREF(py_peer);
  Py_XDECREF(py_channel);
}

static string gen_error(string prefix, fmc_error_t *error) {
  if (error) {
    return prefix + " with error: " + fmc_error_msg(error);
  }
  return prefix;
}

YTPSequenceBase::~YTPSequenceBase() {
  if (shared_seq) {
    fmc_error_t *error;
    clear();

    ytp_sequence_shared_dec(shared_seq, &error);
    if (error) {
      PyErr_SetString(
          PyExc_RuntimeError,
          gen_error("unable to delete YTP sequence", error).c_str());
    }
  }
}

void YTPSequenceBase::clear() {
  fmc_error_t *error;
  auto *seq = ytp_sequence_shared_get(shared_seq);
  for (auto &cl : peer_cb) {
    auto py_callback = cl.second;
    ytp_sequence_peer_cb_rm(seq, &ytp_sequence_peer_cb_wrapper, py_callback,
                            &error);
    Py_XDECREF(py_callback);
  }
  for (auto &cl : channel_cb) {
    auto py_callback = cl.second;
    ytp_sequence_ch_cb_rm(seq, &ytp_sequence_channel_cb_wrapper, py_callback,
                          &error);
    Py_XDECREF(py_callback);
  }
  for (auto &cl : data_cb) {
    auto py_callback = std::get<PyObject *>(cl);
    auto channel = std::get<ytp_channel_t>(cl);
    ytp_sequence_indx_cb_rm(seq, channel, &ytp_sequence_data_cb_wrapper, &cl,
                            &error);
    Py_XDECREF(py_callback);
  }
  for (auto &cl : prfx_cb) {
    auto py_callback = std::get<PyObject *>(cl);
    auto &pattern = std::get<std::string>(cl);
    ytp_sequence_prfx_cb_rm(seq, pattern.size(), pattern.data(),
                            &ytp_sequence_prfx_cb_wrapper, &cl, &error);
    Py_XDECREF(py_callback);
  }
  for (auto &cl : transactions_cb) {
    auto closure = std::get<YTPTransactions *>(cl);
    auto &pattern = std::get<std::string>(cl);
    ytp_sequence_prfx_cb_rm(seq, pattern.size(), pattern.data(),
                            ytp_sequence_data_cb_transactions_wrapper, closure,
                            &error);
  }
  peer_cb.clear();
  channel_cb.clear();
  data_cb.clear();
  prfx_cb.clear();
  transactions_cb.clear();
}

static PyObject *PyAPIWrapper_new(PyTypeObject *subtype, PyObject *args,
                                  PyObject *kwds) {
  auto *self = (PyAPIWrapper *)subtype->tp_alloc(subtype, 0);
  if (!self) {
    return nullptr;
  }
  return (PyObject *)self;
}

static int PyAPIWrapper_init(PyAPIWrapper *self, PyObject *args,
                             PyObject *kwds) {
  self->api = nullptr;
  self->py_api = nullptr;
  return 0;
}

static void PyAPIWrapper_dealloc(PyAPIWrapper *self) {
  Py_TYPE(self)->tp_free((PyObject *)self);
}

static PyObject *YTPSequence_new(PyTypeObject *subtype, PyObject *args,
                                 PyObject *kwds) {
  auto *self = (YTPSequence *)subtype->tp_alloc(subtype, 0);
  if (!self) {
    return nullptr;
  }
  return (PyObject *)self;
}

static int YTPSequence_init(YTPSequence *self, PyObject *args, PyObject *kwds) {
  new (self) YTPSequence;

  static char *kwlist[] = {(char *)"file_path", (char *)"readonly",
                           NULL /* Sentinel */};
  char *filename = nullptr;
  int readonly = 0;
  if (!PyArg_ParseTupleAndKeywords(args, kwds, "s|p", kwlist, &filename,
                                   &readonly)) {
    return -1;
  }
  fmc_fmode mode = readonly == 1 ? fmc_fmode::READ : fmc_fmode::READWRITE;
  fmc_error_t *error;
  self->seq = std::make_shared<YTPSequenceBase>();
  self->seq->shared_seq = ytp_sequence_shared_new(filename, mode, &error);
  if (error) {
    PyErr_SetString(PyExc_RuntimeError,
                    gen_error("unable to create new sequence", error).c_str());
    return -1;
  }

  return 0;
}

static void YTPSequence_dealloc(YTPSequence *self) {
  self->~YTPSequence();
  Py_TYPE(self)->tp_free((PyObject *)self);
}

static PyObject *YTPSequence_peer_callback(YTPSequence *self, PyObject *args,
                                           PyObject *kwds) {
  static char *kwlist[] = {(char *)"clbl", NULL /* Sentinel */};
  PyObject *py_callback = nullptr;
  if (!PyArg_ParseTupleAndKeywords(args, kwds, "O", kwlist, &py_callback)) {
    return nullptr;
  }

  fmc_error_t *error;
  auto *seq = ytp_sequence_shared_get(self->seq->shared_seq);
  auto &closure = self->seq->peer_cb.emplace_back(self->seq.get(), py_callback);
  ytp_sequence_peer_cb(seq, &ytp_sequence_peer_cb_wrapper, &closure, &error);
  if (error) {
    self->seq->peer_cb.pop_back();
    PyErr_SetString(PyExc_RuntimeError,
                    gen_error("unable to set peer callback", error).c_str());
    return nullptr;
  }

  Py_XINCREF(py_callback);
  Py_RETURN_NONE;
}

static PyObject *YTPSequence_channel_callback(YTPSequence *self, PyObject *args,
                                              PyObject *kwds) {
  static char *kwlist[] = {(char *)"clbl", NULL /* Sentinel */};
  PyObject *py_callback = nullptr;
  if (!PyArg_ParseTupleAndKeywords(args, kwds, "O", kwlist, &py_callback)) {
    return nullptr;
  }

  fmc_error_t *error;
  auto *seq = ytp_sequence_shared_get(self->seq->shared_seq);
  auto &closure =
      self->seq->channel_cb.emplace_back(self->seq.get(), py_callback);
  ytp_sequence_ch_cb(seq, &ytp_sequence_channel_cb_wrapper, &closure, &error);
  if (error) {
    self->seq->channel_cb.pop_back();
    PyErr_SetString(PyExc_RuntimeError,
                    gen_error("unable to set channel callback", error).c_str());
    return nullptr;
  }

  Py_XINCREF(py_callback);
  Py_RETURN_NONE;
}

static PyObject *YTPSequence_data_callback(YTPSequence *self, PyObject *args,
                                           PyObject *kwds) {
  static char *kwlist[] = {(char *)"pattern", (char *)"clbl",
                           NULL /* Sentinel */};
  char *pattern_ptr = nullptr;
  PyObject *py_callback = nullptr;
  if (!PyArg_ParseTupleAndKeywords(args, kwds, "sO", kwlist, &pattern_ptr,
                                   &py_callback)) {
    return nullptr;
  }

  std::string_view pattern = pattern_ptr;

  fmc_error_t *error;
  auto *seq = ytp_sequence_shared_get(self->seq->shared_seq);
  auto &closure = self->seq->prfx_cb.emplace_back(self->seq.get(), py_callback,
                                                  std::string(pattern_ptr));
  ytp_sequence_prfx_cb(seq, pattern.size(), pattern.data(),
                       &ytp_sequence_prfx_cb_wrapper, &closure, &error);
  if (error) {
    self->seq->prfx_cb.pop_back();
    PyErr_SetString(PyExc_RuntimeError,
                    gen_error("unable to set prefix callback", error).c_str());
    return nullptr;
  }

  Py_XINCREF(py_callback);
  Py_RETURN_NONE;
}

static PyObject *YTPSequence_peer(YTPSequence *self, PyObject *args,
                                  PyObject *kwds) {
  static char *kwlist[] = {(char *)"peer_name", NULL /* Sentinel */};
  const char *name = nullptr;
  if (!PyArg_ParseTupleAndKeywords(args, kwds, "s", kwlist, &name)) {
    return nullptr;
  }

  std::string_view peer_name = name;

  fmc_error_t *error;
  auto *seq = ytp_sequence_shared_get(self->seq->shared_seq);
  auto peer_id =
      ytp_sequence_peer_decl(seq, peer_name.size(), peer_name.data(), &error);
  if (error) {
    PyErr_SetString(
        PyExc_RuntimeError,
        gen_error(string("unable to obtain peer for name ") + name, error)
            .c_str());
    return nullptr;
  }

  auto py_peer = PyObject_CallObject(&YTPPeerType.ob_base.ob_base, NULL);
  if (!py_peer || PyErr_Occurred()) {
    return nullptr;
  }

  auto &peer = *((YTPPeer *)py_peer);
  peer.seq = self->seq;
  peer.id = peer_id;
  return py_peer;
}

static PyObject *YTPSequence_poll(YTPSequence *self) {
  fmc_error_t *error;
  auto *seq = ytp_sequence_shared_get(self->seq->shared_seq);
  auto res = ytp_sequence_poll(seq, &error);
  if (error) {
    PyErr_SetString(PyExc_RuntimeError,
                    gen_error("unable to poll from sequence", error).c_str());
  }
  if (PyErr_Occurred()) {
    return nullptr;
  }
  return PyBool_FromLong(res);
}

static PyObject *YTPSequence_remove_callbacks(YTPSequence *self) {
  self->seq->clear();
  Py_RETURN_NONE;
}

static PyObject *YTPPeer_new(PyTypeObject *subtype, PyObject *args,
                             PyObject *kwds) {
  auto *self = (YTPPeer *)subtype->tp_alloc(subtype, 0);
  if (!self) {
    return nullptr;
  }
  return (PyObject *)self;
}

static int YTPPeer_init(YTPPeer *self, PyObject *args, PyObject *kwds) {
  new (self) YTPPeer;

  static char *kwlist[] = {NULL /* Sentinel */};
  if (!PyArg_ParseTupleAndKeywords(args, kwds, "", kwlist)) {
    return -1;
  }
  return 0;
}

static void YTPPeer_dealloc(YTPPeer *self) {
  self->~YTPPeer();
  Py_TYPE(self)->tp_free((PyObject *)self);
}

static PyObject *YTPPeer_name(YTPPeer *self) {
  auto *seq = ytp_sequence_shared_get(self->seq->shared_seq);

  size_t sz;
  const char *name;

  fmc_error_t *error;
  ytp_sequence_peer_name(seq, self->id, &sz, &name, &error);
  if (error) {
    PyErr_SetString(PyExc_RuntimeError,
                    gen_error(string("unable to obtain peer name for peer ") +
                                  to_string(self->id),
                              error)
                        .c_str());
  }
  if (PyErr_Occurred()) {
    return nullptr;
  }

  return PyUnicode_FromStringAndSize(name, sz);
}

static PyObject *YTPPeer_id(YTPPeer *self) {
  return PyLong_FromUnsignedLongLong(self->id);
}

static PyObject *YTPPeer_stream(YTPPeer *self, PyObject *args, PyObject *kwds) {
  static char *kwlist[] = {(char *)"ch", NULL /* Sentinel */};
  PyObject *channel_obj = nullptr;
  if (!PyArg_ParseTupleAndKeywords(args, kwds, "O", kwlist, &channel_obj)) {
    return nullptr;
  }

  if (!PyObject_TypeCheck(channel_obj, &YTPChannelType)) {
    PyErr_SetString(PyExc_TypeError,
                    "channel provided must be of ytp.channel type");
    return nullptr;
  }

  auto &channel = *(YTPChannel *)channel_obj;

  auto py_stream = PyObject_CallObject(&YTPStreamType.ob_base.ob_base, NULL);
  if (!py_stream || PyErr_Occurred()) {
    return nullptr;
  }

  auto &stream = *((YTPStream *)py_stream);
  stream.seq = self->seq;
  stream.channel_id = channel.id;
  stream.peer_id = self->id;
  return py_stream;
}

static PyObject *YTPPeer_channel(YTPPeer *self, PyObject *args,
                                 PyObject *kwds) {
  static char *kwlist[] = {(char *)"time", (char *)"channel_name",
                           NULL /* Sentinel */};
  uint64_t time;
  const char *name = nullptr;
  if (!PyArg_ParseTupleAndKeywords(args, kwds, "Ks", kwlist, &time, &name)) {
    return nullptr;
  }

  std::string_view channel_name = name;

  fmc_error_t *error;
  auto *seq = ytp_sequence_shared_get(self->seq->shared_seq);
  auto channel_id = ytp_sequence_ch_decl(
      seq, self->id, time, channel_name.size(), channel_name.data(), &error);
  if (error) {
    PyErr_SetString(
        PyExc_RuntimeError,
        gen_error(string("unable to obtain channel for name ") + name, error)
            .c_str());
    return nullptr;
  }

  auto *py_channel = PyObject_CallObject(&YTPChannelType.ob_base.ob_base, NULL);
  if (!py_channel || PyErr_Occurred()) {
    return nullptr;
  }

  auto &channel = *((YTPChannel *)py_channel);
  channel.seq = self->seq;
  channel.id = channel_id;
  return py_channel;
}

static PyObject *YTPChannel_new(PyTypeObject *subtype, PyObject *args,
                                PyObject *kwds) {
  auto *self = (YTPChannel *)subtype->tp_alloc(subtype, 0);
  if (!self) {
    return nullptr;
  }
  return (PyObject *)self;
}

static int YTPChannel_init(YTPChannel *self, PyObject *args, PyObject *kwds) {
  new (self) YTPChannel;

  static char *kwlist[] = {NULL /* Sentinel */};
  if (!PyArg_ParseTupleAndKeywords(args, kwds, "", kwlist)) {
    return -1;
  }
  return 0;
}

static void YTPChannel_dealloc(YTPChannel *self) {
  self->~YTPChannel();
  Py_TYPE(self)->tp_free((PyObject *)self);
}

static PyObject *YTPChannel_name(YTPChannel *self) {
  auto *seq = ytp_sequence_shared_get(self->seq->shared_seq);

  size_t sz;
  const char *name;

  fmc_error_t *error;
  ytp_sequence_ch_name(seq, self->id, &sz, &name, &error);
  if (error) {
    PyErr_SetString(
        PyExc_RuntimeError,
        gen_error(string("unable to obtain channel name for channel ") +
                      to_string(self->id),
                  error)
            .c_str());
    return nullptr;
  }

  return PyUnicode_FromStringAndSize(name, sz);
}

static PyObject *YTPChannel_id(YTPChannel *self) {
  return PyLong_FromUnsignedLongLong(self->id);
}

static PyObject *YTPChannel_data_callback(YTPChannel *self, PyObject *args,
                                          PyObject *kwds) {
  static char *kwlist[] = {(char *)"clbl", NULL /* Sentinel */};
  PyObject *py_callback = nullptr;
  if (!PyArg_ParseTupleAndKeywords(args, kwds, "O", kwlist, &py_callback)) {
    return nullptr;
  }

  fmc_error_t *error;
  auto *seq = ytp_sequence_shared_get(self->seq->shared_seq);

  auto &closure =
      self->seq->data_cb.emplace_back(self->seq.get(), py_callback, self->id);

  ytp_sequence_indx_cb(seq, self->id, &ytp_sequence_data_cb_wrapper, &closure,
                       &error);
  if (error) {
    self->seq->data_cb.pop_back();
    PyErr_SetString(PyExc_RuntimeError,
                    gen_error("unable to set data callback", error).c_str());
    return nullptr;
  }

  Py_XINCREF(py_callback);
  Py_RETURN_NONE;
}

static PyObject *YTPStream_new(PyTypeObject *subtype, PyObject *args,
                               PyObject *kwds) {
  auto *self = (YTPStream *)subtype->tp_alloc(subtype, 0);
  if (!self) {
    return nullptr;
  }
  return (PyObject *)self;
}

static int YTPStream_init(YTPStream *self, PyObject *args, PyObject *kwds) {
  new (self) YTPStream;

  static char *kwlist[] = {NULL /* Sentinel */};
  if (!PyArg_ParseTupleAndKeywords(args, kwds, "", kwlist)) {
    return -1;
  }
  return 0;
}

static void YTPStream_dealloc(YTPStream *self) {
  self->~YTPStream();
  Py_TYPE(self)->tp_free((PyObject *)self);
}

static PyObject *YTPStream_write(YTPStream *self, PyObject *args,
                                 PyObject *kwds) {
  static char *kwlist[] = {(char *)"time", (char *)"data", NULL /* Sentinel */};
  uint64_t time;
  const char *data = nullptr;
  Py_ssize_t sz;
  if (!PyArg_ParseTupleAndKeywords(args, kwds, "Ky#", kwlist, &time, &data,
                                   &sz)) {
    return nullptr;
  }
  auto *seq = ytp_sequence_shared_get(self->seq->shared_seq);

  fmc_error_t *error;
  char *dst = ytp_sequence_reserve(seq, sz, &error);
  if (error) {
    PyErr_SetString(
        PyExc_RuntimeError,
        gen_error("unable to reserve data in sequence", error).c_str());
    return nullptr;
  }

  memcpy(dst, data, sz);

  ytp_sequence_commit(seq, self->peer_id, self->channel_id, time, dst, &error);
  if (error) {
    PyErr_SetString(
        PyExc_RuntimeError,
        gen_error("unable to commit data in sequence", error).c_str());
    return nullptr;
  }

  Py_RETURN_NONE;
}

static PyObject *YTPStream_peer(YTPStream *self) {
  auto py_peer = PyObject_CallObject(&YTPPeerType.ob_base.ob_base, NULL);
  if (!py_peer || PyErr_Occurred()) {
    return nullptr;
  }

  auto &peer = *((YTPPeer *)py_peer);
  peer.seq = self->seq;
  peer.id = self->peer_id;
  return py_peer;
}

static PyObject *YTPStream_channel(YTPStream *self) {
  auto *py_channel = PyObject_CallObject(&YTPChannelType.ob_base.ob_base, NULL);
  if (!py_channel || PyErr_Occurred()) {
    return nullptr;
  }

  auto &channel = *((YTPChannel *)py_channel);
  channel.seq = self->seq;
  channel.id = self->channel_id;
  return py_channel;
}

static PyObject *YTPTransactions_new(PyTypeObject *subtype, PyObject *args,
                                     PyObject *kwds) {
  auto *self = (YTPTransactions *)subtype->tp_alloc(subtype, 0);
  if (!self) {
    return nullptr;
  }
  return (PyObject *)self;
}

static int YTPTransactions_init(YTPTransactions *self, PyObject *args,
                                PyObject *kwds) {
  new (self) YTPTransactions;

  static char *kwlist[] = {(char *)"file_path", (char *)"readonly",
                           NULL /* Sentinel */};
  char *filename = nullptr;
  int readonly = 0;
  if (!PyArg_ParseTupleAndKeywords(args, kwds, "s|p", kwlist, &filename,
                                   &readonly)) {
    return -1;
  }

  fmc_fmode mode = readonly == 1 ? fmc_fmode::READ : fmc_fmode::READWRITE;
  fmc_error_t *error;
  self->seq = std::make_shared<YTPSequenceBase>();
  self->seq->shared_seq = ytp_sequence_shared_new(filename, mode, &error);
  if (error) {
    PyErr_SetString(PyExc_RuntimeError,
                    gen_error("unable to create new sequence", error).c_str());
    return -1;
  }
  return 0;
}

static void YTPTransactions_dealloc(YTPTransactions *self) {
  self->~YTPTransactions();
  Py_TYPE(self)->tp_free((PyObject *)self);
}

static PyObject *YTPTransactions_subscribe(YTPTransactions *self,
                                           PyObject *args, PyObject *kwds) {
  static char *kwlist[] = {(char *)"pattern", NULL /* Sentinel */};
  const char *pattern_cstr = nullptr;
  if (!PyArg_ParseTupleAndKeywords(args, kwds, "s", kwlist, &pattern_cstr)) {
    return nullptr;
  }

  std::string_view pattern = pattern_cstr;

  auto *seq = ytp_sequence_shared_get(self->seq->shared_seq);
  fmc_error_t *error;
  auto &closure =
      self->seq->transactions_cb.emplace_back(self, std::string(pattern));
  ytp_sequence_prfx_cb(seq, pattern.size(), pattern.data(),
                       ytp_sequence_data_cb_transactions_wrapper, &closure,
                       &error);
  if (error) {
    self->seq->transactions_cb.pop_back();
    PyErr_SetString(PyExc_RuntimeError,
                    gen_error("unable to set data callback", error).c_str());
    return nullptr;
  }

  Py_RETURN_NONE;
}

static PyObject *YTPTransactions_iter(YTPTransactions *self) {
  Py_INCREF(self);
  return (PyObject *)self;
}

static PyObject *YTPTransactions_next(YTPTransactions *self) {
  auto *seq = ytp_sequence_shared_get(self->seq->shared_seq);

  while (!self->transaction.has_value()) {
    fmc_error_t *error;
    auto poll = ytp_sequence_poll(seq, &error);
    if (error) {
      PyErr_SetString(PyExc_RuntimeError,
                      gen_error("unable to poll", error).c_str());
      return nullptr;
    }
    if (PyErr_Occurred()) {
      return nullptr;
    }
    if (!poll) {
      Py_RETURN_NONE;
    }
  }

  auto &transaction = *self->transaction;

  auto *ret = PyTuple_New(4);

  auto py_peer = PyObject_CallObject(&YTPPeerType.ob_base.ob_base, NULL);
  if (!py_peer || PyErr_Occurred()) {
    return nullptr;
  }

  auto &peer = *((YTPPeer *)py_peer);
  peer.seq = self->seq->shared_from_this();
  peer.id = transaction.peer;

  auto py_channel = PyObject_CallObject(&YTPChannelType.ob_base.ob_base, NULL);
  if (!py_channel || PyErr_Occurred()) {
    return nullptr;
  }

  auto &channel = *((YTPChannel *)py_channel);
  channel.seq = self->seq->shared_from_this();
  channel.id = transaction.channel;

  PyTuple_SET_ITEM(ret, 0, py_peer);
  PyTuple_SET_ITEM(ret, 1, py_channel);
  PyTuple_SET_ITEM(ret, 2, PyLong_FromUnsignedLongLong(transaction.time));
  PyTuple_SET_ITEM(ret, 3,
                   PyByteArray_FromStringAndSize(transaction.data.data(),
                                                 transaction.data.size()));

  self->transaction.reset();
  return ret;
}

static py_ytp_sequence_api_v1 py_api_inst{
    &PyYTPChannel_Check,                        // channel_check
    &PyYTPPeer_Check,                           // peer_check
    &PyYTPSequence_Check,                       // sequence_check
    &PyYTPStream_Check,                         // stream_check
    (pysharedseqfunc_get)&PyYTPSequence_Shared, // sequence_shared
    (pysharedseqfunc_get)&PyYTPStream_Shared,   // stream_shared
    &PyYTPStream_PeerId,                        // stream_peer_id
    &PyYTPStream_ChannelId,                     // stream_channel_id
    (pysharedseqfunc_get)&PyYTPPeer_Shared,     // peer_shared
    &PyYTPPeer_Id,                              // peer_id
    (pysharedseqfunc_get)&PyYTPChannel_Shared,  // channel_shared
    &PyYTPChannel_Id                            // channel_id
};

PyObject *YtpModule_api_v1(PyObject *self) {
  PyAPIWrapper *api = (PyAPIWrapper *)PyAPIWrapper_new(
      (PyTypeObject *)&PyAPIWrapperType, nullptr, nullptr);
  api->api = ytp_sequence_api_v1_get();
  api->py_api = &py_api_inst;
  return (PyObject *)api;
}

static PyMethodDef YtpMethods[] = {{"api_v1", (PyCFunction)YtpModule_api_v1,
                                    METH_NOARGS, "Obtain Python YTP V1 API"},
                                   {NULL, NULL, 0, NULL}};

static PyModuleDef YtpModule = {PyModuleDef_HEAD_INIT, "ytp", "ytp module", -1,
                                YtpMethods};

PyMODINIT_FUNC fm_ytp_py_init(void) FMMODFUNC FMPYMODPUB;

PyMODINIT_FUNC fm_ytp_py_init(void) {
  if (auto *module = PyModule_Create(&YtpModule); module) {
    ADD_PY_CLASS(YTPSequenceType, "sequence", module);
    ADD_PY_CLASS(YTPPeerType, "peer", module);
    ADD_PY_CLASS(YTPChannelType, "channel", module);
    ADD_PY_CLASS(YTPStreamType, "stream", module);
    ADD_PY_CLASS(YTPTransactionsType, "transactions", module);
    ADD_PY_CLASS(PyAPIWrapperType, "APIWrapper", module);
    if (PyModule_AddStringConstant(module, "__version__", YTP_VERSION) == -1)
      return NULL;
    return module;
  }

  return NULL;
}
