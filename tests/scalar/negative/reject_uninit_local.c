// expect: rejected cerastes-unsupported_declaration @ L9
// mode: c23, c++20
// twin: tests/scalar/if_else.c
// Pins: 0012 item 12: a local without an initializer is rejected
#include <cerastes.h>

int assign(int a)
{
  int x;
  x = a;
  return x;
}
