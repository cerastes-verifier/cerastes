// expect: rejected cerastes-constant_out_of_range_compare @ L10
// expect: frontend-warning unsigned_compare_with_negative @ L10
// mode: c23, c++20
// twin: tests/scalar/warn_accepted_neighbours.c
// Pins: 0025 item 6 R10: `uc == -1` (clang by default); EDG prints its #514 (unsigned_compare_with_negative) for it too, which the test lists
#include <cerastes.h>

int test(unsigned char uc, signed char sc, unsigned short us, int x, unsigned u)
{
  return uc == -1;
}
