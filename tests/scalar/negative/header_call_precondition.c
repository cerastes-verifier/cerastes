// expect: error precondition @ L13
// mode: c23, c++20
// twin: tests/scalar/header_use.c
// Pins: 0011 item 5: a call of a header function is checked against its header contract, at the call in this file
#include <cerastes.h>

#include "../headers/clamp0.h"

int use(int x)
    CERASTES_REQUIRES(x >= -2000 && x <= 1000)
    CERASTES_ENSURES(r, r >= 1 && r <= 1001)
{
  return clamp0(x) + 1;
}
