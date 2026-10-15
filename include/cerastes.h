// Part of cerastes, under the Apache License v2.0 with LLVM Exceptions.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// <cerastes.h>: the annotation macros and the specification functions
// (decision 0008).  It includes no other header.
//
// With the cerastes front end each CERASTES_* macro expands to the attribute it
// stands for (decision 0006).  The front end is recognised by the attributes
// themselves (__has_cpp_attribute / __has_c_attribute of cerastes::requires is 1
// there and 0 in any other compiler), not by the macro __CERASTES__ that the front
// end also defines, so that undefining that macro cannot silently drop the
// contracts.  (The front end rejects a source-level #undef or #define of
// __CERASTES__.)  With any other compiler the contract and statement macros expand
// to nothing, and CERASTES_PREDICATE and CERASTES_PURE expand to `inline`.
// Tampering with the macros of this header is not supported.
//
// Write CERASTES_PREDICATE and CERASTES_PURE first in the declaration, before
// `static`, `constexpr` and the return type.
#ifndef CERASTES_H
#define CERASTES_H

#if defined(__cplusplus) && defined(__has_cpp_attribute)
#  if __has_cpp_attribute(cerastes::requires)
#    define CERASTES_ATTRIBUTES_ 1
#  endif
#elif !defined(__cplusplus) && defined(__has_c_attribute)
#  if __has_c_attribute(cerastes::requires)
#    define CERASTES_ATTRIBUTES_ 1
#  endif
#endif

#ifdef CERASTES_ATTRIBUTES_
#  define CERASTES_REQUIRES(...)  [[cerastes::requires(__VA_ARGS__)]]
#  define CERASTES_ENSURES(...)   [[cerastes::ensures(__VA_ARGS__)]]
#  define CERASTES_INVARIANT(...) [[cerastes::invariant(__VA_ARGS__)]]
#  define CERASTES_CHECK(...)     [[cerastes::check(__VA_ARGS__)]]
#  define CERASTES_FOLD(...)      [[cerastes::fold(__VA_ARGS__)]]
#  define CERASTES_UNFOLD(...)    [[cerastes::unfold(__VA_ARGS__)]]
#  define CERASTES_PREDICATE      [[cerastes::predicate]] inline
#  define CERASTES_PURE           [[cerastes::pure]] inline
#else
#  define CERASTES_REQUIRES(...)
#  define CERASTES_ENSURES(...)
#  define CERASTES_INVARIANT(...)
#  define CERASTES_CHECK(...)
#  define CERASTES_FOLD(...)
#  define CERASTES_UNFOLD(...)
#  define CERASTES_PREDICATE      inline
#  define CERASTES_PURE           inline
#endif

#ifdef __cplusplus
// Specification functions: declared, never defined.  They may only be named in
// annotations and in the bodies of predicate and pure functions; a call that
// reaches run time is a link error naming a cerastes:: function.
namespace cerastes {
template <class T> bool owns(const T& place) noexcept;
template <class T> bool reads(const T& place) noexcept;
template <class T> const T& old(const T& value) noexcept;
// forall(body, free...): body's first parameter is the quantified variable and
// its remaining parameters receive the trailing arguments, in order
// (decision 0006 item 9).
template <class F, class... Free> bool forall(F body, const Free&... free) noexcept;
template <class T> bool owns_range(const T* first, decltype(sizeof 0) count) noexcept;
}  // namespace cerastes
#endif

#endif  // CERASTES_H
