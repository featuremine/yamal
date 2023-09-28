/******************************************************************************
        COPYRIGHT (c) 2019-2023 by Featuremine Corporation.

        This Source Code Form is subject to the terms of the Mozilla Public
        License, v. 2.0. If a copy of the MPL was not distributed with this
        file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *****************************************************************************/

/**
 * @file wrapper.hpp
 * @author Maxim Trokhimtchouk
 * @date 12 Oct 2017
 * @brief File contains C++ python C API wrapper
 *
 * This file provides useful wrappers for python object
 */

#pragma once

#include "fmc++/misc.hpp"
#include <fmc++/mpl.hpp> // fmc_runtime_error_unless()
#include <fmc++/time.hpp>

#include <Python.h>
#include <chrono>
#include <datetime.h>
#include <map>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace fmc {
namespace python {

void raise_python_error();
inline bool is_python_error() { return PyErr_Occurred() != nullptr; }

template <class T, class Enable = void> struct _py_object_t;
template <class T> using py_object_t = typename _py_object_t<T>::type;

template <class T> auto make_py_object(T &&x) {
  return py_object_t<std::decay_t<T>>(std::forward<T>(x));
}

class interpreter {
public:
  interpreter(struct _inittab *newtab = nullptr) {
    if (newtab)
      PyImport_ExtendInittab(newtab);
  }
  void init() {
    if (!Py_IsInitialized()) {
      Py_Initialize();
    } else {
      external = false;
    }

    if (!PyDateTimeAPI)
      PyDateTime_IMPORT;
  }
  ~interpreter() {
    if (external)
      Py_FinalizeEx();
  }
  bool external = true;
};

class string;

class object {
public:
  object() = default;

  object(object &&m) : obj_(m.obj_) { m.obj_ = nullptr; }
  object(const object &c) : obj_(c.obj_) { Py_XINCREF(obj_); }
  ~object() { Py_XDECREF(obj_); }
  object &operator=(object &&o) {
    Py_XDECREF(obj_);
    obj_ = o.obj_;
    o.obj_ = nullptr;
    return *this;
  }
  object &operator=(const object &o) {
    Py_XDECREF(obj_);
    obj_ = o.obj_;
    Py_XINCREF(obj_);
    return *this;
  }
  operator bool() const { return obj_ != nullptr; }
  bool is_None() const { return obj_ == Py_None; }
  bool is_True() const { return PyObject_IsTrue(obj_); }
  object get_opt_attr(const char *attr_name) const {
    return PyObject_HasAttrString(obj_, attr_name)
               ? object::from_new(PyObject_GetAttrString(obj_, attr_name))
               : object();
  }
  object get_attr(const char *attr_name) const {
    auto *attr = PyObject_GetAttrString(obj_, attr_name);
    if (!attr)
      raise_python_error();
    return object::from_new(attr);
  }
  object operator[](const char *attr_name) const { return get_attr(attr_name); }
  object operator[](const std::string &name) const {
    return get_attr(name.c_str());
  }
  object operator[](string &name);
  object operator[](int pos) const {
    if (obj_ == nullptr)
      return object::from_new(nullptr);
    auto obj = PyTuple_GetItem(obj_, pos);
    return object::from_borrowed(obj);
  }
  template <class... Args> object operator()(Args &&...args) const {
    const size_t n = std::tuple_size<std::tuple<Args...>>::value;
    auto *obj = PyTuple_New(n);
    object arg_tuple = object::from_new(obj);
    if constexpr (n > 0) {
      PyObject *argv[n];
      size_t i = 0;
      for_each([&](auto &&a) { argv[i++] = make_py_object(a).steal_ref(); },
               args...);
      for (size_t i = 0; i < n; ++i) {
        PyTuple_SET_ITEM(obj, i, argv[i]);
      }
    }
    auto *retobj = PyObject_CallObject(get_ref(), arg_tuple.get_ref());
    if (!retobj)
      raise_python_error();
    return object::from_new(retobj);
  }

  template <class T> object get_item(T &&idx) {
    auto *obj = PyObject_GetItem(get_ref(), py_object_t<T>(idx).get_ref());
    if (!obj)
      raise_python_error();
    return object::from_new(obj);
  }
  template <class T> object get_opt_item(T &&idx) {
    auto *obj = PyObject_GetItem(get_ref(), py_object_t<T>(idx).get_ref());
    if (!obj) {
      PyErr_Clear();
      return object();
    }
    return object::from_new(obj);
  }
  object iter() { return object::from_new(PyObject_GetIter(obj_)); }
  object next() { return object::from_new(PyIter_Next(obj_)); }
  PyObject *get_ref() const { return obj_; }
  PyObject *steal_ref() {
    auto *ret = obj_;
    obj_ = nullptr;
    return ret;
  }
  std::string str() const {
    if (!obj_)
      return "";
    auto tmp = object(PyObject_Str(obj_));
    if (!tmp)
      return "";
    return PyUnicode_AsUTF8(tmp.get_ref());
  }
  static object from_borrowed(PyObject *obj) {
    Py_XINCREF(obj);
    return object(obj);
  }
  static object from_new(PyObject *obj) { return object(obj); }
  object type() { return object::from_new(PyObject_Type(obj_)); }

private:
  object(PyObject *obj) : obj_(obj) {}
  PyObject *obj_ = nullptr;
};

template <class T> T from_py(const object &obj) {
  auto wrap = py_object_t<T>(object(obj));
  T tmp = wrap.operator T();
  return tmp;
}

template <> struct _py_object_t<object> { using type = object; };

inline std::ostream &operator<<(std::ostream &s, const object &o) {
  s << o.str();
  return s;
}

class string : public object {
public:
  string(object &&obj) : object(obj) {
    fmc_runtime_error_unless(PyUnicode_Check(get_ref()))
        << "object is not of string type";
  }
  string(const char *str)
      : string(object::from_new(PyUnicode_FromString(str))) {
    if (!*this)
      raise_python_error();
  }
  string(std::string_view v)
      : string(
            object::from_new(PyUnicode_FromStringAndSize(v.data(), v.size()))) {
    if (!*this)
      raise_python_error();
  }
  operator std::string() {
    const char *ret = *this;
    raise_python_error();
    return std::string(ret);
  }
  operator const char *() { return PyUnicode_AsUTF8(get_ref()); }
};
template <> struct _py_object_t<const char *> { using type = string; };
template <> struct _py_object_t<std::string> { using type = string; };
template <> struct _py_object_t<std::string_view> { using type = string; };

inline object object::operator[](string &name) {
  return (*this)[(std::string)name];
}

class py_int : public object {
public:
  py_int(object &&obj) : object(obj) {
    fmc_range_error_unless(PyLong_Check(get_ref())) << str()
                                                    << " is not of int "
                                                       "type";
  }
  py_int(long l) : object(object::from_new(PyLong_FromLong(l))) {
    if (!this->operator bool())
      raise_python_error();
  }
  operator long() {
    long l = PyLong_AsLong(get_ref());
    raise_python_error();
    return l;
  }
  operator int() {
    long l = PyLong_AsLong(get_ref());
    fmc_range_error_unless(l <= numeric_limits<int>::max())
        << str() << " is higher than maximum int value";
    fmc_range_error_unless(l >= numeric_limits<int>::min())
        << str() << " is lower than minimum int value";
    return l;
  }
};
template <> struct _py_object_t<long> { using type = py_int; };
template <> struct _py_object_t<int> { using type = py_int; };

class py_unsigned : public object {
public:
  py_unsigned(object &&obj) : object(obj) {
    fmc_range_error_unless(PyLong_Check(get_ref()))
        << str() << " is not of int type";
    fmc_range_error_unless(PyLong_AsLong(get_ref()) >= 0)
        << str() << " is not unsigned";
  }
  py_unsigned(unsigned l) : object(object::from_new(PyLong_FromLong((long)l))) {
    if ((!this->operator bool()))
      raise_python_error();
  }
  operator unsigned() {
    long l = PyLong_AsLong(get_ref());
    raise_python_error();
    return (unsigned)l;
  }
  operator unsigned long long() {
    unsigned long long l = PyLong_AsUnsignedLongLong(get_ref());
    raise_python_error();
    return l;
  }
};
template <> struct _py_object_t<unsigned> { using type = py_unsigned; };
template <> struct _py_object_t<unsigned long> { using type = py_unsigned; };
template <> struct _py_object_t<unsigned long long> {
  using type = py_unsigned;
};

class py_long : public object {
public:
  py_long(object &&obj) : object(obj) {
    fmc_range_error_unless(PyLong_Check(get_ref())) << str()
                                                    << " is not of long "
                                                       "type";
  }
  py_long(long long l) : object(object::from_new(PyLong_FromLongLong(l))) {
    if (!*this)
      raise_python_error();
  }
  operator long long() {
    long long l = PyLong_AsLongLong(get_ref());
    raise_python_error();
    return l;
  }
};
template <> struct _py_object_t<long long> { using type = py_long; };

class py_bool : public object {
public:
  py_bool(object &&obj) : object(obj) {
    fmc_range_error_unless(PyBool_Check(get_ref())) << str()
                                                    << " is not of bool "
                                                       "type";
  } // namespace python
  py_bool(bool d) : object(object::from_borrowed(d ? Py_True : Py_False)) {}
  operator bool() { return get_ref() == Py_True ? true : false; }
};
template <> struct _py_object_t<bool> { using type = py_bool; };

class py_float : public object {
public:
  py_float(object &&obj) : object(obj) {
    fmc_range_error_unless(PyFloat_Check(get_ref())) << str()
                                                     << " is not of float "
                                                        "type";
  } // namespace python
  py_float(double d) : object(object::from_new(PyFloat_FromDouble(d))) {
    if (!*this)
      raise_python_error();
  }
  operator double() {
    double res = PyFloat_AsDouble(get_ref());
    raise_python_error();
    return res;
  }
};
template <> struct _py_object_t<double> { using type = py_float; };

class module : public object {
public:
  module() :object() {}
  explicit module(const string &s)
      : object(object::from_new(PyImport_Import(s.get_ref()))) {
    if (get_ref() == nullptr)
      raise_python_error();
  }
  explicit module(const char *name)
      : object(object::from_new(PyImport_ImportModule(name))) {
    if (get_ref() == nullptr)
      raise_python_error();
  }
};

class timedelta : public object {
public:
  timedelta(object &&obj) : object(obj) {
    PyDateTime_IMPORT;

    fmc_range_error_unless(
        PyDelta_Check(get_ref()) || PyFloat_Check(get_ref()) ||
        PyLong_Check(get_ref()) || is_pandas_timestamp_type(get_ref()))
        << str() << ". Got type "
        << std::string(Py_TYPE(get_ref()) ? Py_TYPE(get_ref())->tp_name : "")
        << ". It should be of type timedelta, numeric or pandas.Timestamp";
  }
  timedelta(time t)
      : object(object::from_new([=]() {
          using namespace std::chrono;
          auto us = duration_cast<microseconds>(t);
          auto sec = duration_cast<seconds>(us);
          auto tmp = duration_cast<microseconds>(sec);
          auto rem = us - tmp;
          return PyDelta_FromDSU(0, sec.count(), rem.count());
        }())) {
    if (!*this)
      raise_python_error();
  }
  operator time() {
    if (!PyDateTimeAPI)
      PyDateTime_IMPORT;
    using namespace std::chrono;
    if (PyDelta_Check(get_ref())) {
      auto h = hours(24 * long(py_int((*this)["days"])));
      auto sec = seconds(long(py_int((*this)["seconds"])));
      auto us = microseconds(long(py_int((*this)["microseconds"])));
      return duration_cast<time>(h) + duration_cast<time>(sec) +
             duration_cast<time>(us);
    } else if (PyFloat_Check(get_ref())) {
      auto fdur = duration<double>(PyFloat_AsDouble(get_ref()));
      auto nanos = duration_cast<nanoseconds>(fdur);
      return duration_cast<time>(nanos);
    } else if (PyLong_Check(get_ref())) {
      long long ns = PyLong_AsLongLong(get_ref());
      raise_python_error();
      return duration_cast<time>(nanoseconds(ns));
    } else if (is_pandas_timestamp_type(get_ref())) {
      auto ns = PyLong_AsLongLong((*this)["value"].get_ref());
      return duration_cast<time>(nanoseconds(ns));
    }
    fmc_range_error_unless(false)
        << str() << " must be timedelta, numeric or pandas.Timestamp";
    return seconds(0);
  }

  static bool is_pandas_timestamp_type(PyObject *obj) {
    return strcmp(Py_TYPE(obj)->tp_name, "Timestamp") == 0;
  }
};
template <> struct _py_object_t<time> { using type = timedelta; };

class datetime : public object {
public:
  datetime(object &&obj) : object(obj) {}
  operator fmc_time64_t() {
    using namespace std::chrono;
    if (fmc::python::datetime::is_timedelta_type(get_ref())) {
      auto h = 24 * PyLong_AsLong(PyObject_GetAttrString(get_ref(), "days"));
      auto sec = PyLong_AsLong(PyObject_GetAttrString(get_ref(), "seconds"));
      auto us =
          PyLong_AsLong(PyObject_GetAttrString(get_ref(), "microseconds"));
      return fmc_time64_from_nanos(us * 1000) +
             fmc_time64_from_seconds(h * 3600 + sec);
    } else if (PyFloat_Check(get_ref())) {
      auto fdur = duration<double>(PyFloat_AsDouble(get_ref()));
      auto nanos = duration_cast<nanoseconds>(fdur);
      return fmc_time64_from_nanos(nanos.count());
    } else if (PyLong_Check(get_ref()))
      return fmc_time64_from_nanos(PyLong_AsLongLong(get_ref()));
    else if (is_pandas_timestamp_type(get_ref())) {
      return fmc_time64_from_nanos(
          PyLong_AsLongLong((*this)["value"].get_ref()));
    }
    PyErr_SetString(PyExc_RuntimeError, "unsupported datetime type");
    return fmc_time64_from_nanos(0);
  }

  static bool is_pandas_timestamp_type(PyObject *obj) {
    return strcmp(Py_TYPE(obj)->tp_name, "Timestamp") == 0;
  }

  static object get_pandas_dttz_type() {
    static auto datetime =
        module("pandas")["core"]["dtypes"]["dtypes"]["DatetimeTZDtype"];
    return datetime;
  }
  static object get_timedelta_type() {
    static auto datetime = module("datetime")["timedelta"];
    return datetime;
  }
  static bool is_timedelta_type(PyObject *obj) {
    return PyObject_IsInstance(obj, get_timedelta_type().get_ref());
  }
  static object timedelta(int64_t days, int64_t seconds, int64_t microseconds) {
    auto args = fmc::python::object::from_new(PyTuple_New(0));
    auto kwargs = fmc::python::object::from_new(PyDict_New());
    auto d = fmc::python::object::from_new(PyLong_FromLongLong(days));
    PyDict_SetItemString(kwargs.get_ref(), "days", d.get_ref());
    auto s = fmc::python::object::from_new(PyLong_FromLongLong(seconds));
    PyDict_SetItemString(kwargs.get_ref(), "seconds", s.get_ref());
    auto m = fmc::python::object::from_new(PyLong_FromLongLong(microseconds));
    PyDict_SetItemString(kwargs.get_ref(), "microseconds", m.get_ref());
    return object::from_new(
        PyObject_Call(fmc::python::datetime::get_timedelta_type().get_ref(),
                      args.get_ref(), kwargs.get_ref()));
  }
};

class tuple : public object {
public:
  tuple(object &&obj) : object(obj) {
    fmc_runtime_error_unless(PyTuple_Check(get_ref()))
        << "object is not of tuple type";
  }
  tuple(int sz = 0) : object(object::from_new(PyTuple_New(sz))) {}
  template <class... Ts>
  tuple(const std::tuple<Ts...> &tup)
      : object(object::from_new(apply(from_args, tup))) {}

  template <class... Args> static PyObject *from_args(Args &&...args) {
    auto *obj = PyTuple_New(tuple_size_v<std::tuple<Args...>>);
    fmc_runtime_error_unless(obj) << "could not create Python tuple";

    if constexpr (sizeof...(Args) > 0) {
      unsigned i = 0;
      auto add_item = [&](auto &&arg) {
        PyTuple_SET_ITEM(obj, i++, make_py_object(arg).steal_ref());
      };
      (add_item(std::forward<Args>(args)), ...);
    }
    return obj;
  }

  template <class InputIt> tuple(InputIt first, InputIt last) {
    std::vector<PyObject *> argv;
    for (auto it = first; it != last; ++it) {
      auto value = *it;
      argv.push_back(py_object_t<decltype(value)>(value).steal_ref());
    }
    auto *obj = PyTuple_New(argv.size());
    for (size_t i = 0; i < argv.size(); ++i) {
      PyTuple_SET_ITEM(obj, i, argv[i]);
    }
    *this = object::from_new(obj);
  }

  template <class... Args> operator std::tuple<Args...>() {
    return get_tuple_items<std::tuple<Args...>>();
  }

private:
  template <typename Tup, std::size_t Idx>
  tuple_element_t<Idx, Tup> get_tuple_item() {
    using type = tuple_element_t<Idx, Tup>;
    auto py_wrap = py_object_t<type>(
        object::from_borrowed(PyTuple_GetItem(get_ref(), Idx)));
    type tmp = py_wrap;
    return tmp;
  }

  template <typename Tup, std::size_t... I>
  Tup get_tuple_items(std::index_sequence<I...>) {
    return Tup(get_tuple_item<Tup, I>()...);
  }

  template <typename Tup> auto get_tuple_items() {
    constexpr auto tup_sz = tuple_size_v<Tup>;
    fmc_runtime_error_unless(tup_sz == PyTuple_Size(get_ref()))
        << "tying to convert Python tuple to C++ tuple of incorrect type";

    return get_tuple_items<Tup>(make_index_sequence<tup_sz>{});
  }
};
template <class... Ts> struct _py_object_t<std::tuple<Ts...>> {
  using type = tuple;
};

template <class T> struct iterable : object, abstract_container<T> {
public:
  iterable(object &&obj) : object(obj) {
    fmc_runtime_error_unless(obj) << "trying to create an iterable from NULL";
  }
  using next_function = typename abstract_container<T>::next_function;
  next_function iterator_generator() override {
    auto iterator = object::from_new(PyObject_GetIter(get_ref()));
    if (!iterator)
      raise_python_error();
    return [iterator]() {
      auto next = object::from_new(PyIter_Next(iterator.get_ref()));
      return bool(next) ? optional<T>(T(py_object_t<T>(std::move(next))))
                        : optional<T>();
    };
  }
};

class py_dict : public object {
public:
  py_dict(object &&obj) : object(obj) {
    if (!PyDict_Check(obj.get_ref()))
      raise_python_error();
  }
  py_dict() : object(object::from_new(PyDict_New())) {}
  template <class Key, class Value>
  py_dict(const std::unordered_map<Key, Value> &map)
      : object(object::from_new([&]() {
          auto *obj = PyDict_New();
          fmc_runtime_error_unless(obj) << "could not create Python dict";
          for (auto &&[key, val] : map) {
            auto key_obj = make_py_object(key);
            auto val_obj = make_py_object(val);
            if (PyDict_SetItem(obj, key_obj.get_ref(), val_obj.get_ref()) != 0)
              raise_python_error();
          }
          return obj;
        }())) {}
  template <class Key, class Value>
  py_dict(const std::map<Key, Value> &map)
      : object(object::from_new([&]() {
          auto *obj = PyDict_New();
          fmc_runtime_error_unless(obj) << "could not create Python dict";
          for (auto &&[key, val] : map) {
            auto key_obj = make_py_object(key);
            auto val_obj = make_py_object(val);
            if (PyDict_SetItem(obj, key_obj.get_ref(), val_obj.get_ref()) != 0)
              raise_python_error();
          }
          return obj;
        }())) {}
  template <class Key> object operator[](const Key &key) {
    auto *key_obj = make_py_object(key).steal_ref();
    auto *obj = PyDict_GetItemWithError(get_ref(), key_obj);
    if (!obj && PyErr_Occurred()) {
      raise_python_error();
    }
    fmc_runtime_error_unless(obj) << key << " key is not found";
    return object::from_borrowed(obj);
  }
  template <class Key, class Value> operator std::unordered_map<Key, Value>() {
    std::unordered_map<Key, Value> map;
    iterable<std::tuple<Key, Value>> iter(
        object::from_new(PyDict_Items(get_ref())));
    for (auto &&[key, val] : iter) {
      map.emplace(key, val);
    }
    return map;
  }
  template <class Key, class Value> operator std::map<Key, Value>() {
    std::map<Key, Value> map;
    iterable<std::tuple<Key, Value>> iter(
        object::from_new(PyDict_Items(get_ref())));
    for (auto &&[key, val] : iter) {
      map.emplace(key, val);
    }
    return map;
  }
};
template <class Key, class Value>
struct _py_object_t<unordered_map<Key, Value>> {
  using type = py_dict;
};

class class_object : public object {
public:
  class_object(object &&obj) : object(obj) {
    if (!PyType_Check(obj.get_ref()))
      raise_python_error();
  }
  object create(const tuple &arg) {
    auto *retobj = PyObject_CallObject(get_ref(), arg.get_ref());
    if (!retobj)
      raise_python_error();
    return object::from_new(retobj);
  }
};

inline void raise_python_error() {
  if (PyErr_Occurred() != nullptr) {
    PyObject *ptype;
    PyObject *pvalue;
    PyObject *ptraceback;
    PyErr_Fetch(&ptype, &pvalue, &ptraceback);
    object type = object::from_new(ptype);
    object value = object::from_new(pvalue);
    object traceback = object::from_new(ptraceback);
    std::string errstr;

    static bool first = true;
    if (first && bool(traceback)) {
      first = false;
      module mod(string("traceback"));
      auto iter = iterable<const char *>(mod["format_tb"](traceback));
      for (auto &&s : iter) {
        errstr += s;
      }
      first = true;
    }
    fmc_runtime_error_unless(false)
        << "\n============== Python error ==============\n"
        << value.str() << "\n"
        << (errstr.empty() ? "could not obtain the stack\n" : errstr)
        << "==========================================";
  }
}
} // namespace python
} // namespace fmc
