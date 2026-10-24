// expect: rejected cerastes-complement_of_truth_value @ L8
// mode: c23, c++20
// twin: tests/scalar/bit_contract.c
// Pins: 0023 item 5: ~ of a comparison in an ensures
#include <cerastes.h>

int pick(int x)
    CERASTES_ENSURES(r, ~(r > 0) != 0)
{
  return x;
}
