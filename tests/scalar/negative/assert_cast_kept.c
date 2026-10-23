// expect: error check @ L12
// mode: c23, c++20
// twin: tests/scalar/assert_holds.c
// Pins: 0015 item 6: the assert's condition is the value of (unsigned char)x, so x == 0 fails the check
// With 0 <= x <= 255 the cast no longer changes the value, so it is not pinned here: assert_cast_conversion.c pins that the cast is kept.
#include <assert.h>
#include <cerastes.h>

int low_byte_set(int x)
    CERASTES_REQUIRES(0 <= x && x <= 255)
{
  assert((unsigned char)x);
  return x;
}
