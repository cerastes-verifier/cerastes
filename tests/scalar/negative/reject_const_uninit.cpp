// expect: rejected missing_initializer_on_const @ L9
// mode: c++20
// twin: tests/scalar/uninit_assign_read.c
// Pins: 0019 item 1: a const local without an initializer is an error of the C++ front end (#257)
#include <cerastes.h>

int f(void)
{
  const int k;
  return k;
}
