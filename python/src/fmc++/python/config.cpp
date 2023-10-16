/******************************************************************************
        COPYRIGHT (c) 2019-2023 by Featuremine Corporation.

        This Source Code Form is subject to the terms of the Mozilla Public
        License, v. 2.0. If a copy of the MPL was not distributed with this
        file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *****************************************************************************/

/**
 * @file config.cpp
 * @author Maxim Trokhimtchouk
 * @date 9 Oct 2017
 * @brief File contains C++ definition of the python config query implementation
 *
 * This file describes the python config query implementation
 */

#include <fmc++/config/config.hpp> // fmc::configs::interface::*
#include <fmc++/metatable.hpp>     // fmc::metatable
#include <fmc++/misc.hpp>          // next_function
#include <fmc++/mpl.hpp>           // fmc::typify, fmc_runtime_error_unless()
#include <fmc++/python/config.hpp>
#include <fmc++/time.hpp> // fmc::time

#include <fmc++/python/wrapper.hpp>

#include <Python.h> // PyGILState_*

#include <functional>  // std::function
#include <optional>    // std::optional; std::make_optional
#include <string>      // std::string
#include <string_view> // std::string_view
#include <utility>     // std::pair ; std::move

namespace fmc {
namespace python {
namespace configs {

// *** node ***

node::node(fmc::python::object &&obj) : obj_(obj) {}

node &node::operator[](std::string_view key) { return as<section>()[key]; }

unsigned node::get(typify<unsigned>) { return get<unsigned>(); }

int node::get(typify<int>) { return get<int>(); }

long node::get(typify<long>) { return get<long>(); }

bool node::get(typify<bool>) { return get<bool>(); }

double node::get(typify<double>) { return get<double>(); }

fmc::time node::get(typify<fmc::time>) { return get<fmc::time>(); }

std::string node::get(typify<std::string>) { return get<std::string>(); }

fmc::python::object node::get(typify<fmc::python::object>) { return obj_; }

section &node::get(typify<section &>) {
  if (!section_) {
    section_.reset(new section(fmc::python::object(obj_)));
  }
  return *section_;
}

section &node::get(typify<section>) { return get(typify<section &>()); }

fmc::configs::interface::section &
node::get(typify<fmc::configs::interface::section &>) {
  return get(typify<section &>());
}

array &node::get(typify<array &>) {
  if (!array_) {
    array_.reset(new array(fmc::python::object(obj_)));
  }
  return *array_;
}

array &node::get(typify<array>) { return get(typify<array &>()); }

fmc::configs::interface::array &
node::get(typify<fmc::configs::interface::array &>) {
  return get(typify<array &>());
}

section &node::to_d() { return get(typify<section &>()); }

array &node::to_a() { return get(typify<array &>()); }

template <class T> T node::get() {
  PyGILState_STATE gstate;
  gstate = PyGILState_Ensure();
  T temp = fmc::python::py_object_t<T>(fmc::python::object(obj_));
  PyGILState_Release(gstate);
  return temp;
}

std::string node::str() const { return obj_.str(); }

fmc::configs::interface::node::_type node::type() {
  if (PyBool_Check(obj_.get_ref())) {
    return _type::BOOL;
  } else if (PyLong_Check(obj_.get_ref())) {
    return _type::INTEGER;
  } else if (PyFloat_Check(obj_.get_ref())) {
    return _type::FLOATING_POINT;
  } else if (fmc::python::datetime::is_timedelta_type(obj_.get_ref())) {
    return _type::TIME;
  } else if (PyUnicode_Check(obj_.get_ref())) {
    return _type::STRING;
  } else if (PyDict_Check(obj_.get_ref())) {
    return _type::SECTION;
  } else if (PyList_Check(obj_.get_ref())) {
    return _type::ARRAY;
  }
  return _type::INVALID;
}

// *** section ***

section::section(fmc::python::object &&obj)
    : obj_(obj), table_([this](std::string_view key) {
        return new node(obj_.get_item(std::string(key)));
      }) {}

node &section::operator[](std::string_view key) {
  return static_cast<node &>(get(key));
}

node &section::get(std::string_view key) { return table_(std::string(key)); }

bool section::has(std::string_view key) {
  PyGILState_STATE gstate;
  gstate = PyGILState_Ensure();
  if (obj_.get_opt_item(std::string(key))) {
    PyGILState_Release(gstate);
    return true;
  }
  PyGILState_Release(gstate);
  return false;
}

section::section_next_function section::iterator_generator() {
  fmc::python::object keys = obj_.get_attr("keys");
  fmc::python::object py_iter = keys();
  auto iterator =
      fmc::python::object::from_new(PyObject_GetIter(py_iter.get_ref()));
  if (!iterator)
    fmc::python::raise_python_error();
  return [iterator, this]() mutable {
    auto next = fmc::python::object::from_new(PyIter_Next(iterator.get_ref()));
    if (!next)
      return std::optional<value_type>();
    std::string key = fmc::python::py_object_t<std::string>(std::move(next));
    return std::make_optional<value_type>(key, table_(key));
  };
}

std::pair<std::string, fmc::configs::interface::node &>
section::_mapping::operator()(
    std::pair<std::string, fmc::configs::interface::node &> in) {
  return std::pair<std::string, fmc::configs::interface::node &>(
      in.first, static_cast<fmc::configs::interface::node &>(in.second));
}

// *** array ***

array::array(fmc::python::object &&obj)
    : obj_(obj), table_([this](size_t key) {
        return new node(obj_.get_item(long(key)));
      }) {}

node &array::operator[](size_t key) { return static_cast<node &>(get(key)); }

node &array::get(size_t key) { return table_(key); }
array::array_next_function array::iterator_generator() {
  fmc_runtime_error_unless(PyList_Check(obj_.get_ref()))
      << "the object must be a Python List";
  return [this, i = 0, size = PyList_Size(obj_.get_ref())]() mutable {
    if (i >= size)
      return std::optional<value_type>();
    auto index = i++;
    return std::make_optional<value_type>(index, table_(index));
  };
}

std::pair<size_t, fmc::configs::interface::node &> array::_mapping::operator()(
    std::pair<size_t, fmc::configs::interface::node &> in) {
  return std::pair<size_t, fmc::configs::interface::node &>(
      in.first, static_cast<fmc::configs::interface::node &>(in.second));
}

} // namespace configs
} // namespace python
} // namespace fmc
