/******************************************************************************
        COPYRIGHT (c) 2019-2023 by Featuremine Corporation.
        
        This Source Code Form is subject to the terms of the Mozilla Public
        License, v. 2.0. If a copy of the MPL was not distributed with this
        file, You can obtain one at https://mozilla.org/MPL/2.0/.
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

template <class Func, class... Args> void for_each(Func &&f, Args &&...args) {
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
struct _uniquely_callable<F, typename enable_if<_member_function_type<
                                 decltype(&F::operator())>::value>::type> {
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
template <class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

template <class... Args> struct type_list {};

template <class T> struct typify { using type = T; };

template <class Op> struct scope_end_call {
  scope_end_call(Op &&op) : op_(std::forward<Op>(op)) {}
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
  int i = snprintf(number, sizeof(number) / sizeof(char), "%i", num);
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
T *derived_by_name(string_view name, type_list<Ts...>, Args &&...args) {
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
                                       Args &&...args) {
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

template <class> struct typify_tuple_;
template <class... Args> struct typify_tuple_<tuple<Args...>> {
  using type = tuple<typify<Args>...>;
};

template <class T> using typify_tuple = typename typify_tuple_<T>::type;

template <typename Tuple, typename F, std::size_t... Indices>
void for_each_impl(Tuple &&tuple, F &&f, std::index_sequence<Indices...>) {
  (f(std::get<Indices>(std::forward<Tuple>(tuple))), ...);
}

template <typename Tuple, typename F>
void for_each_in_tuple(F &&f, Tuple &&tuple) {
  constexpr std::size_t N =
      std::tuple_size<std::remove_reference_t<Tuple>>::value;
  for_each_impl(std::forward<Tuple>(tuple), std::forward<F>(f),
                std::make_index_sequence<N>{});
}

template <class Func, class... Ts, class... Args>
void for_each_type(Func &&f, type_list<Ts...>, Args &&...args) {
  for_each([&](auto t) { f(t, forward<Args>(args)...); }, typify<Ts>()...);
}

template <class Tup> struct tuple_to_type_list;

template <class... Args> struct tuple_to_type_list<tuple<Args...>> {
  using type = type_list<Args...>;
};

template <class T>
using tuple_to_type_list_t = typename tuple_to_type_list<T>::type;

template <template <class> class Obj, class Tup> struct tuple_unpack;
template <template <class> class Obj, class... Ts>
struct tuple_unpack<Obj, tuple<Ts...>> {
  using type = Obj<Ts...>;
};

template <template <class> class Obj, class Tup>
using tuple_unpack_t = typename tuple_unpack<Obj, Tup>::type;

template <class Op, class Func, class... Args>
decltype(auto) apply_for_each(Op &&op, Func &&f, Args &&...args) {
  tuple<decltype(f(std::forward<Args>(args)))...> tup = {
      f(std::forward<Args>(args))...};
  return apply(forward<Op>(op), move(tup));
}

template <class Op, class Func, class... Ts, class... Args>
decltype(auto) apply_for_each_type(Op &&op, Func &&f, type_list<Ts...>,
                                   Args &&...args) {
  return apply_for_each(
      forward<Op>(op), [&](auto t) { return f(t, forward<Args>(args)...); },
      typify<Ts>()...);
}

template <class... Args> std::string type_names() {
  string res;
  bool first = true;
  for_each(
      [&](string s) {
        if (!first)
          res += " ";
        first = false;
        res += s;
      },
      type_name<Args>()...);
  return res;
}

template <class T> struct repeated_tuple;
template <class... TElements> struct repeated_tuple<std::tuple<TElements...>> {

  template <class Obj> repeated_tuple(Obj &obj) : tuple(TElements{obj}...) {}
  std::tuple<TElements...> tuple;
};

template <template <class> class M, class T> struct tuple_wrap;
template <template <class> class M, class... Ts>
struct tuple_wrap<M, tuple<Ts...>> {
  using type = tuple<M<Ts>...>;
};
template <template <class> class M, class... Ts>
struct tuple_wrap<M, type_list<Ts...>> {
  using type = tuple<M<Ts>...>;
};
template <template <class> class M, class T>
using tuple_wrap_t = typename tuple_wrap<M, T>::type;

template <template <class> class M, class T> struct type_list_wrap;
template <template <class> class M, class... Ts>
struct type_list_wrap<M, tuple<Ts...>> {
  using type = type_list<M<Ts>...>;
};
template <template <class> class M, class... Ts>
struct type_list_wrap<M, type_list<Ts...>> {
  using type = type_list<M<Ts>...>;
};
template <template <class> class M, class T>
using type_list_wrap_t = typename type_list_wrap<M, T>::type;

template <class M, class K, class T, class = void> struct is_map {
  static constexpr bool value = false;
};

template <class M, class K>
using _map_return = decltype(declval<M>()[declval<decay_t<K>>()]);

template <class M, class K, class T>
struct is_map<M, K, T, enable_if_t<is_convertible_v<_map_return<M, K>, T>>> {
  static constexpr bool value = true;
};

template <class M, class K, class T>
inline constexpr bool is_map_v = is_map<M, K, T>::value;

template <class Tup, class... Ts>
tuple<Ts...> _subtuple(Tup from, typify<tuple<Ts...>>) {
  return tuple<Ts...>(get<Ts>(from)...);
}

template <class Ret, class Tup> Ret subtuple(Tup from) {
  return _subtuple(from, typify<Ret>());
}

template <class Tuple, class T, int curridx = tuple_size_v<Tuple> - 1>
constexpr int tuple_index() {
  static_assert(curridx > -1, "Unable to find specified type in tuple.");
  if constexpr (is_same_v<tuple_element_t<curridx, Tuple>, T>)
    return curridx;
  else
    return tuple_index<Tuple, T, curridx - 1>();
}

template <typename T, typename Tuple> struct has_type;

template <typename T, typename... Us>
struct has_type<T, std::tuple<Us...>>
    : std::disjunction<std::is_same<T, Us>...> {};

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
