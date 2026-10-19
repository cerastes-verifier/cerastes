// expect: error overflow @ tests/scalar/headers/bad_overflow.h:L7
// mode: c23, c++20
// twin: tests/scalar/header_use.c
// Pins: 0011 item 4: an overflow in the body of a user header is reported in the header
// The header function is the one with the bug; this file only includes it.
#include <cerastes.h>

#include "../headers/bad_overflow.h"

int use(int x)
    CERASTES_REQUIRES(x >= 0 && x <= 100)
{
  return twice_h(x);
}
