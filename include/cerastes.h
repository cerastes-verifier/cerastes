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
// `static`, `constexpr` and the return type.  CERASTES_OWNING marks a pointer parameter: the function owns its
// pointee unless the pointer is null (decision 0040 item 3).  Write it before the parameter's type or after its name.
// CERASTES_UNINIT marks an automatic local declared without an initializer (documentation, checked by --strict-init);
// CERASTES_REF_TO_UNINIT and CERASTES_MUST_INIT mark a pointer parameter whose pointee need not have a value on entry, and
// (must_init) has one on return (P4222's markers, decision 0042).  Each parameter marker needs an owns(*p) in the contract.
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

// Fails closed (decision 0008 item 2): under the cerastes front end the attributes must be available, or every
// macro below would expand to nothing and the contracts would silently vanish (for example after `-U__cplusplus`
// or `#undef __cplusplus` before this include).  Ordinary compilers never define __CERASTES__.
#if defined(__CERASTES__) && !defined(CERASTES_ATTRIBUTES_)
// A #error message is one line, over 120 columns here: the formatter would continue it after a backslash.
// clang-format off
#  error "cerastes.h: the cerastes front end is running (__CERASTES__ is defined) but the cerastes attributes are not available (check that __cplusplus, __STDC_VERSION__ and __has_cpp_attribute / __has_c_attribute were not changed); contracts would be dropped"
// clang-format on
#endif

#ifdef CERASTES_ATTRIBUTES_
#  define CERASTES_REQUIRES(...) [[cerastes::requires(__VA_ARGS__)]]
#  define CERASTES_ENSURES(...) [[cerastes::ensures(__VA_ARGS__)]]
#  define CERASTES_INVARIANT(...) [[cerastes::invariant(__VA_ARGS__)]]
#  define CERASTES_DECREASES(...) [[cerastes::decreases(__VA_ARGS__)]]
#  define CERASTES_CHECK(...) [[cerastes::check(__VA_ARGS__)]]
#  define CERASTES_FOLD(...) [[cerastes::fold(__VA_ARGS__)]]
#  define CERASTES_UNFOLD(...) [[cerastes::unfold(__VA_ARGS__)]]
#  define CERASTES_PREDICATE [[cerastes::predicate]] inline
#  define CERASTES_PURE [[cerastes::pure]] inline
#  define CERASTES_OWNING [[cerastes::owning]]
#  define CERASTES_UNINIT [[cerastes::uninit]]
#  define CERASTES_REF_TO_UNINIT [[cerastes::ref_to_uninit]]
#  define CERASTES_MUST_INIT [[cerastes::must_init]]
#else
#  define CERASTES_REQUIRES(...)
#  define CERASTES_ENSURES(...)
#  define CERASTES_INVARIANT(...)
#  define CERASTES_DECREASES(...)
#  define CERASTES_CHECK(...)
#  define CERASTES_FOLD(...)
#  define CERASTES_UNFOLD(...)
#  define CERASTES_PREDICATE inline
#  define CERASTES_PURE inline
#  define CERASTES_OWNING
#  define CERASTES_UNINIT
#  define CERASTES_REF_TO_UNINIT
#  define CERASTES_MUST_INIT
#endif

// The C spellings of the specification functions (decisions 0029 item 3, 0037 item 5), declared in C and in C++ alike so that a header shared
// by both writes one spelling.  Declared, never defined, like the C++ forms below: a call that reaches run time is a link error naming a
// cerastes_ function.  Each takes a place (`cerastes_owns(*p)`, `cerastes_reads(*p)`) or a pointer (`cerastes_live(p)`) through C23's
// variadic declaration without a named parameter.  `cerastes_owns_block(p)` takes a pointer: the whole heap block `p` starts, with write
// permission (decision 0047 item 4).  `cerastes_owns_block(p, size)` and `cerastes_zeroed(p)` are vlibc's own (the allocation
// contracts of `malloc` and `calloc`; the front end rejects them in a program's contracts).  `cerastes_old(e)` takes a place and is a macro over `__typeof__` (accepted by clang,
// gcc and the front end in both languages); it is only ever expanded inside a contract macro, which expands to nothing in another
// compiler.
#ifdef __cplusplus
extern "C" {
bool cerastes_owns(...) noexcept;
bool cerastes_reads(...) noexcept;
bool cerastes_live(...) noexcept;
bool cerastes_owns_block(...) noexcept;
bool cerastes_zeroed(...) noexcept;
const void* cerastes_old_(const void*) noexcept;
}
#else
_Bool cerastes_owns(...);
_Bool cerastes_reads(...);
_Bool cerastes_live(...);
_Bool cerastes_owns_block(...);
_Bool cerastes_zeroed(...);
const void* cerastes_old_(const void*);
#endif
#define cerastes_old(e) (*(__typeof__(e)*)cerastes_old_(&(e)))

#ifdef __cplusplus
// Specification functions: declared, never defined.  They may only be named in
// annotations and in the bodies of predicate and pure functions; a call that
// reaches run time is a link error naming a cerastes:: function.
namespace cerastes {
template <class T>
bool owns(const T& place) noexcept;
template <class T>
bool reads(const T& place) noexcept;
// live(p): `p` points into an object whose lifetime has not ended (decision 0028 item 3a); implies p != NULL.
template <class T>
bool live(const T* pointer) noexcept;
// owns_block(p): `p` is the start of a heap block and the function owns the whole block, with write permission (decision 0047 item 4).
// The two-argument form (a size in bytes) is vlibc's own and is rejected in a program's contracts.
template <class T>
bool owns_block(const T* pointer) noexcept;
template <class T>
bool owns_block(const T* pointer, decltype(sizeof 0) size) noexcept;
template <class T>
const T& old(const T& value) noexcept;
// forall(body, free...): body's first parameter is the quantified variable and
// its remaining parameters receive the trailing arguments, in order
// (decision 0006 item 9).
template <class F, class... Free>
bool forall(F body, const Free&... free) noexcept;
template <class T>
bool owns_range(const T* first, decltype(sizeof 0) count) noexcept;
} // namespace cerastes
#endif

#endif // CERASTES_H
