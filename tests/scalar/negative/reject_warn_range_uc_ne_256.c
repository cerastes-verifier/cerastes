// expect: rejected cerastes-constant_out_of_range_compare @ L9
// mode: c23, c++20
// twin: tests/scalar/warn_accepted_neighbours.c
// Pins: 0025 item 6 R10: `uc != 256`: the constant does not fit the type of the other operand (clang -Wtautological-constant-out-of-range-compare, by default; gcc with -Wextra)
#include <cerastes.h>

int test(unsigned char uc, signed char sc, unsigned short us, int x, unsigned u)
{
  return uc != 256;
}
