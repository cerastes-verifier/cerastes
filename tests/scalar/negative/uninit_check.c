// expect: error uninitialized @ L11
// mode: c23, c++20
// twin: tests/scalar/uninit_assign_read.c
// Pins: 0019 item 2: a mention in check is a read
#include <cerastes.h>

int use(int a)
    CERASTES_REQUIRES(0 <= a && a <= 10)
{
  int x;
  CERASTES_CHECK(x <= 2147483647);
  x = a;
  return x;
}
