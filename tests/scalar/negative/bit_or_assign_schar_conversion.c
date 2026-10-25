// expect: error conversion @ L10
// mode: c23, c++20
// twin: tests/scalar/bit_promoted.c
// Pins: 0023 item 3: sc |= 0x80 on the non-negative signed char 1 gives 129, which does not fit
#include <cerastes.h>

signed char set_high(signed char sc)
    CERASTES_REQUIRES(sc == 1)
{
  sc |= 0x80;
  return sc;
}
