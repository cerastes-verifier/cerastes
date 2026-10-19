// expect: error conversion @ L10
// mode: c23, c++20
// twin: tests/scalar/plain_char.c
// Pins: 0010 item 2 (P12): 200 does not fit a signed plain char
#include <cerastes.h>

char to_char(int x)
    CERASTES_REQUIRES(x >= -128 && x <= 200)
{
  return x;
}
