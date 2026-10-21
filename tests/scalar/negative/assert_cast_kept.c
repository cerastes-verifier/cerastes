// expect: error check @ L12
// mode: c23, c++20
// twin: tests/scalar/assert_holds.c
// Pins: 0015 item 6: the user's cast inside the assert is kept: (unsigned char)256 is 0
// x == 256 is allowed by the requires; the conversion makes the condition 0.
#include <assert.h>
#include <cerastes.h>

int low_byte_set(int x)
    CERASTES_REQUIRES(1 <= x && x <= 300)
{
  assert((unsigned char)x);
  return x;
}
