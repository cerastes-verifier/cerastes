// expect: error check @ L12
// mode: c23, c++20
// twin: tests/scalar/assert_ndebug.c
// Pins: 0015 item 6: the same function without NDEBUG: the assertion is checked and fails
// x > 100 does not hold for every x.
#include <assert.h>
#include <cerastes.h>

int identity(int x)
    CERASTES_ENSURES(r, r == x)
{
  assert(x > 100);
  return x;
}
