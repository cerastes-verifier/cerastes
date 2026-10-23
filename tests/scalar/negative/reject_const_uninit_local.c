// expect: rejected cerastes-unsupported_declaration @ L9
// c-only: C++ refuses a const without an initializer (error #257): see reject_const_uninit.cpp
// twin: tests/scalar/uninit_assign_read.c
// Pins: 0019 item 1: a const local without an initializer can never be given a value (C; the C++ front end refuses it, error #257)
#include <cerastes.h>

int f(void)
{
  const int k;
  return k;
}
