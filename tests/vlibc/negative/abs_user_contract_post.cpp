// expect: error postcondition @ L16
// twin: tests/vlibc/abs_user_contract.cpp
// Pins: 0046 item 7: the user's contract on abs is what a caller relies on, no more (abs(0) is 0, so r >= 1 does not follow from the contract)
#include <stdlib.h>
#include <cerastes.h>
#include "../headers/glibc_noexcept.h"

int abs(int x) VLIBC_TEST_NOEXCEPT
    CERASTES_REQUIRES(x > -1000)
    CERASTES_ENSURES(r, r >= 0 && (r == x || r == -x));

int magnitude(int v)
    CERASTES_REQUIRES(-100 <= v && v <= 100)
    CERASTES_ENSURES(r, r >= 1)
{
  return abs(v);
}
