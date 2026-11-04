// A helper of tests/vlibc: glibc declares abs and abort `noexcept(true)` (`__THROW`) in C++, and a redeclaration without it
// draws the front end's #541 there (and an error from clang).  Include this AFTER <stdlib.h>: __GLIBC__ is defined by it.
#ifndef VLIBC_TEST_GLIBC_NOEXCEPT_H
#define VLIBC_TEST_GLIBC_NOEXCEPT_H
#ifdef __GLIBC__
#define VLIBC_TEST_NOEXCEPT noexcept
#else
#define VLIBC_TEST_NOEXCEPT
#endif
#endif
