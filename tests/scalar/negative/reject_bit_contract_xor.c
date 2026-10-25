// expect: rejected cerastes-xor_as_power @ L8
// mode: c23, c++20
// twin: tests/scalar/bit_contract.c
// Pins: 0023 item 5: 2 ^ 8 in an ensures
#include <cerastes.h>

int pick(int x)
    CERASTES_ENSURES(r, r == (x ^ (2 ^ 8)))
{
  return x;
}
