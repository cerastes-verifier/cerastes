// expect: error uninitialized @ L12
// mode: c23, c++20
// twin: tests/scalar/uninit_assign_read.c
// Pins: 0019 item 2: a mention in assert is a read
#include <assert.h>
#include <cerastes.h>

int use(int a)
    CERASTES_REQUIRES(0 <= a && a <= 10)
{
  int x;
  assert(x <= 2147483647);
  x = a;
  return x;
}
