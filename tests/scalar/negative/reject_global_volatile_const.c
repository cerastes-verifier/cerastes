// expect: rejected cerastes-unsupported_type @ L7
// mode: c23, c++20
// twin: tests/scalar/const_global.c
// Pins: 0017 item 2: a volatile constant is rejected like a volatile local
#include <cerastes.h>

const volatile int V = 1;

int get(void)
{
  return V;
}
