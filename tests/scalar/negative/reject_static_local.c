// expect: rejected cerastes-unsupported_declaration @ L9
// mode: c23, c++20
// twin: tests/scalar/if_else.c
// Pins: 0012 item 12: a static local is rejected
#include <cerastes.h>

int counter(int a)
{
  [[maybe_unused]] static int count = 0;
  return a;
}
