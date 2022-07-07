/******************************************************************************

        COPYRIGHT (c) 2017 by Featuremine Corporation.
        This software has been provided pursuant to a License Agreement
        containing restrictions on its use.  This software contains
        valuable trade secrets and proprietary information of
        Featuremine Corporation and is protected by law.  It may not be
        copied or distributed in any form or medium, disclosed to third
        parties, reverse engineered or used in any manner not provided
        for in said License Agreement except with the prior written
        authorization from Featuremine Corporation.

 *****************************************************************************/

/**
 * @file mpl.hpp
 * @author Maxim Trokhimtchouk
 * @date 22 Nov 2017
 * @brief C++ utilities
 *
 * This file defines some C++ utilities
 */

#pragma once

#include <fmc/platform.h>

#ifdef __GNUC__
#include <cxxabi.h>
#if defined(FMC_SYS_ARM)
#include <arm_neon.h>
#elif defined(__IWMMXT__)
#include <mmintrin.h>
#endif

#if defined(FMC_AMD64)
#include <x86intrin.h>
#endif

#elif _MSC_VER
#include <intrin.h>
#endif

#include <cerrno>
#include <chrono>
#include <cstring>
#include <functional>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>
#include <tuple>
#include <type_traits>
#include <typeinfo>

namespace fmc {

struct empty {};

inline std::string demangle(const char *name) {
#ifdef __GNUC__
  int status;
  char *realname = abi::__cxa_demangle(name, 0, 0, &status);
  std::string result = realname;
  free(realname);
  return result;
#elif _MSC_VER
  return std::string(name);
#endif
}

template <class T> std::string type_name() {
  return demangle(typeid(T).name());
}

using namespace std;

template <class Func, class... Args> void for_each(Func &&f, Args &&... args) {
  (f(std::forward<Args>(args)), ...);
}

namespace hidden {
template <class M> struct _member_function_type {
  enum { value = false };
};

template <class R, class G, class... Args>
struct _member_function_type<R (G::*)(Args...)> {
  enum { value = true };
  using result = R;
  using type = R(Args...);
  using args = tuple<Args...>;
};

template <class R, class G, class... Args>
struct _member_function_type<R (G::*)(Args...) const> {
  enum { value = true };
  using result = R;
  using type = R(Args...);
  using args = tuple<Args...>;
};

template <class T, class Enable = void> struct _uniquely_callable {
  enum { value = false };
};

template <class R, class... Args> struct _uniquely_callable<R(Args...)> {
  enum { value = true };
  using result = R;
  using type = R(Args...);
  using function = std::function<type>;
  using args = tuple<Args...>;
  enum { argc = tuple_size<args>::value };
  static function convert(R (*ptr)(Args...)) { return function(ptr); }
};

template <class F>
struct _uniquely_callable<F, typename enable_if<_member_function_type<decltype(
                                 &F::operator())>::value>::type> {
  using details = _member_function_type<decltype(&F::operator())>;
  enum { value = true };
  using result = typename details::result;
  using type = typename details::type;
  using function = std::function<type>;
  using args = typename details::args;
  enum { argc = tuple_size<args>::value };
  static function convert(F &&op) { return function(std::forward<F>(op)); }
};
} // namespace hidden

template <class F>
using uniquely_callable =
    hidden::_uniquely_callable<std::remove_cv_t<std::remove_reference_t<F>>>;

template <class E> struct exception_builder {
  int operator,(const std::ostream &s) {
    throw E(static_cast<const std::ostringstream &>(s).str());
    return 0;
  }
};

template <class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template <class... Ts> overloaded(Ts...)->overloaded<Ts...>;

template <class... Args> struct type_list {};

template <class T> struct typify { using type = T; };

template <class Op> struct scope_end_call {
  scope_end_call(Op &&op) : op_(forward<Op>(op)) {}
  ~scope_end_call() { op_(); }
  Op op_;
};

template <class I, class Op> class iterator_mapper {
public:
  using difference_type = typename I::difference_type;
  using value_type = decltype(declval<Op>()(*declval<I>()));
  using pointer = value_type *;
  using reference = value_type &;
  using iterator_category = typename I::iterator_category;

  iterator_mapper(I iter) : iter_(iter), op_() {}

  iterator_mapper(I iter, Op &&op) : iter_(iter), op_(std::forward<Op>(op)) {}
  bool operator==(const iterator_mapper &a) const { return iter_ == a.iter_; }
  bool operator!=(const iterator_mapper &a) const { return iter_ != a.iter_; }
  iterator_mapper &operator++() {
    ++iter_;
    return *this;
  }
  value_type operator*() { return op_(*iter_); }

private:
  I iter_;
  Op op_;
};

struct dereference {
  template <class T> T &operator()(T *obj) { return *obj; }
};

struct dereference_second {
  template <class K, class T> pair<K, T &> operator()(pair<K, T *> &p) {
    return {p.first, *p.second};
  }
  template <class K, class T>
  const pair<K, T &> operator()(const pair<K, T *> &p) {
    return {p.first, *p.second};
  }
};

template <class U, class... Ts>
inline constexpr bool _tuple_has(typify<tuple<Ts...>>) {
  return (std::is_same_v<U, Ts> || ...);
}

template <class T, class U>
inline constexpr bool tuple_has = _tuple_has<U>(typify<T>());

inline std::string &append_int(std::string &str, int num) {
  using namespace std;
  char number[32];
  int i = sprintf(number, "%i", num);
  str.append(number, number + i);
  return str;
}

template <class T>
inline typename std::enable_if<std::is_floating_point<T>::value, bool>::type
almost_equal(const T x, const T y) {
  auto v = x - y;
  const auto eps = std::numeric_limits<T>::epsilon();
  return (v <= eps) & (-eps <= v);
}

template <typename T>
inline typename std::enable_if<!std::is_floating_point<T>::value, bool>::type
almost_equal(const T x, const T y) {
  return x == y;
}

template <class T, class... Ts, class... Args>
T *derived_by_name(string_view name, type_list<Ts...>, Args &&... args) {
  T *result = nullptr;
  for_each(
      [&](auto t) {
        using Tt = decltype(t);
        using Tn = typename Tt::type;
        if (!result && type_name<Tn>() == name) {
          result = new Tn(forward<Args>(args)...);
        }
      },
      typify<Ts>()...);
  return result;
}

template <class T, class... Ts, class... Args>
std::unique_ptr<T> make_unique_by_name(string_view name, type_list<Ts...>,
                                       Args &&... args) {
  T *result = nullptr;
  for_each(
      [&](auto t) {
        using Tt = decltype(t);
        using Tn = typename Tt::type;
        if (!result && type_name<Tn>() == name) {
          result = new Tn(forward<Args>(args)...);
        }
      },
      typify<Ts>()...);
  return std::unique_ptr<T>(result);
}

template <class... Ts>
bool name_in_typelist(string_view name, type_list<Ts...>) {
  bool result = false;
  for_each(
      [&](auto t) {
        using Tt = decltype(t);
        using Tn = typename Tt::type;
        if (!result && type_name<Tn>() == name) {
          result = true;
        }
      },
      typify<Ts>()...);
  return result;
}

} // namespace fmc
#if defined(FMC_SYS_WIN)
#define __builtin_expect(a, b) a
#endif

#define fmc_logic_error_unless(C)                                              \
  if (__builtin_expect(!(C), 0))                                               \
  fmc::exception_builder<std::logic_error>(),                                  \
      std::stringstream() << "(" << __FILE__ << ":" << __LINE__ << ") "

#define fmc_range_error_unless(C)                                              \
  if (__builtin_expect(!(C), 0))                                               \
  fmc::exception_builder<std::range_error>(),                                  \
      std::stringstream() << "(" << __FILE__ << ":" << __LINE__ << ") "

#define fmc_runtime_error_unless(C)                                            \
  if (__builtin_expect(!(C), 0))                                               \
  fmc::exception_builder<std::runtime_error>(),                                \
      std::stringstream() << "(" << __FILE__ << ":" << __LINE__ << ") "

#define fmc_system_error_unless(C)                                             \
  if (__builtin_expect(!(C), 0))                                               \
  fmc::exception_builder<std::runtime_error>(),                                \
      std::stringstream() << "(" << __FILE__ << ":" << __LINE__ << ") ["       \
                          << errno << "](" << strerror(errno) << ") "

#define fmc_logic_error_unless_(C)                                             \
  if (__builtin_expect(!(C), 0))                                               \
  exception_builder<std::logic_error>(),                                       \
      std::stringstream() << "(" << __FILE__ << ":" << __LINE__ << ") "

#define fmc_range_error_unless_(C)                                             \
  if (__builtin_expect(!(C), 0))                                               \
  exception_builder<std::range_error>(),                                       \
      std::stringstream() << "(" << __FILE__ << ":" << __LINE__ << ") "

#define fmc_runtime_error_unless_(C)                                           \
  if (__builtin_expect(!(C), 0))                                               \
  exception_builder<std::runtime_error>(),                                     \
      std::stringstream() << "(" << __FILE__ << ":" << __LINE__ << ") "

#define fmc_system_error_unless_(C)                                            \
  if (__builtin_expect(!(C), 0))                                               \
  exception_builder<std::runtime_error>(),                                     \
      std::stringstream() << "(" << __FILE__ << ":" << __LINE__ << ") ["       \
                          << errno << "](" << strerror(errno) << ") "

#define HAS_METHOD(METHOD, ...)                                                \
  template <typename T> struct has_##METHOD##_method {                         \
  private:                                                                     \
    template <typename U>                                                      \
    static auto call(int)                                                      \
        -> decltype(std::declval<U>().METHOD(__VA_ARGS__), std::true_type());  \
    template <typename> static std::false_type call(...);                      \
                                                                               \
  public:                                                                      \
    static constexpr bool value = decltype(call<T>(0))::value;                 \
  };                                                                           \
  template <typename T>                                                        \
  static constexpr bool has_##METHOD##_method_v =                              \
      has_##METHOD##_method<T>::value;
