// expect: error uninitialized @ L12
// mode: c23, c++20
// twin: tests/scalar/uninit_assign_read.c
// Pins: 0019 item 2: a read before any assignment is reported at the read (the initializer of y)
#include <cerastes.h>

int copy_first(int a)
    CERASTES_ENSURES(r, r == a)
{
  int x;
  int y;
  y = x;
  x = a;
  return x + (y - y);
}
