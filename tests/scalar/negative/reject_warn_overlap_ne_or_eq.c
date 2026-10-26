// expect: rejected cerastes-overlapping_comparisons @ L9
// mode: c23, c++20
// twin: tests/scalar/warn_accepted_neighbours.c
// Pins: 0025 item 6 R11: `x != 1 || x == 1` is always false or always true (clang -Wtautological-overlap-compare, -Wall; code only, not in annotations)
#include <cerastes.h>

int test(int x, int y, int z, unsigned u)
{
  return x != 1 || x == 1;
}
