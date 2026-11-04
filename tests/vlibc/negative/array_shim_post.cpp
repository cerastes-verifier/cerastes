// expect: error postcondition @ L12
// twin: tests/vlibc/array_shim.cpp
// Pins: 0046 item 2: loading the shim through <array> does not weaken the checks of an unrelated function
// The postcondition claims one more than the body computes.
#include <array>
#include <cerastes.h>

int twice(int x)
    CERASTES_REQUIRES(0 <= x && x <= 100)
    CERASTES_ENSURES(r, r == 2 * x + 1)
{
  return x + x;
}
