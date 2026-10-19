// expect: error conversion @ L11
// mode: c23, c++20
// twin: tests/scalar/compound_shift.c
// Pins: 0010 item 5: 64 << 1 fits int but not signed char
#include <cerastes.h>

signed char double_c(signed char c)
    CERASTES_REQUIRES(c >= 0 && c <= 64)
    CERASTES_ENSURES(r, r == c + c)
{
  c <<= 1;
  return c;
}
