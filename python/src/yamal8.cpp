#include <fmc/platform.h>
#include <fmc/python/common.h>

#include <ytp/version.h>

#include <Python.h>

struct Yamal {
  PyObject_HEAD;
};

static int Yamal_init(Yamal *self, PyObject *args, PyObject *kwds) {

}

static void Yamal_dealloc(Yamal *self) {

}

static PyMethodDef Yamal_methods[] = {
    {NULL, NULL, 0, NULL} /* Sentinel */
};

static PyTypeObject YamalType = {
    PyVarObject_HEAD_INIT(NULL, 0) "yamal.yamal8.yamal",  /* tp_name */
    sizeof(Yamal),                                 /* tp_basicsize */
    0,                                             /* tp_itemsize */
    (destructor)Yamal_dealloc,                     /* tp_dealloc */
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
    "Yamal object",                                /* tp_doc */
    0,                                             /* tp_traverse */
    0,                                             /* tp_clear */
    0,                                             /* tp_richcompare */
    0,                                             /* tp_weaklistoffset */
    0,                                             /* tp_iter */
    0,                                             /* tp_iternext */
    Yamal_methods,                                 /* tp_methods */
    0,                                             /* tp_members */
    0,                                             /* tp_getset */
    0,                                             /* tp_base */
    0,                                             /* tp_dict */
    0,                                             /* tp_descr_get */
    0,                                             /* tp_descr_set */
    0,                                             /* tp_dictoffset */
    (initproc)Yamal_init,                          /* tp_init */
    0,                                             /* tp_alloc */
    0,                                             /* tp_new */
};

PyMODINIT_FUNC PyInit_yamal8(void) FMMODFUNC FMPYMODPUB;

static PyModuleDef Yamal8Module = {PyModuleDef_HEAD_INIT, "yamal8", "yamal8 module", -1, NULL};

PyMODINIT_FUNC PyInit_yamal8(void) {
  if (auto *module = PyModule_Create(&Yamal8Module); module) {
    ADD_PY_CLASS(YamalType, "yamal", module);
    if (PyModule_AddStringConstant(module, "__version__", YTP_VERSION) == -1)
      return NULL;
    return module;
  }

  return NULL;
}
