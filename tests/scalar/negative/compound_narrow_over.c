// expect: error conversion @ L11
// mode: c23, c++20
// twin: tests/scalar/compound_narrow.c
// Pins: 0010 item 5: 127 + 1 does not fit signed char again
#include <cerastes.h>

signed char bump(signed char c)
    CERASTES_REQUIRES(c >= -100 && c <= 127)
    CERASTES_ENSURES(r, r == c + 1)
{
  c += 1;
  return c;
}
