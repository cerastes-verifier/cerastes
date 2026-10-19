// expect: rejected cerastes-unsupported_type @ L9
// mode: c23, c++20
// twin: tests/scalar/if_else.c
// Pins: 0010 item 2: a volatile type is rejected
#include <cerastes.h>

int read(int x)
{
  volatile int v = x;
  return v;
}
