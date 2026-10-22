// expect: error uninitialized @ L11
// mode: c23, c++20
// twin: tests/scalar/uninit_multi_declarator.c
// Pins: 0019 item 1: the first declarator has no initializer, the second has
#include <cerastes.h>

int second_only(void)
{
  int x, y = 3;
  y += 1;
  return x;
}
