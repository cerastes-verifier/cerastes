// expect: rejected cerastes-unsupported_declaration @ L7
// mode: c23, c++20
// twin: tests/scalar/const_global.c
// Pins: 0017 item 2: a thread_local constant is rejected (a thread has its own copy)
#include <cerastes.h>

thread_local const int T = 5;

int get(void)
{
  return T;
}
