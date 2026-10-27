// expect: rejected cerastes-logical_not_comparison @ L10
// mode: c23, c++20
// twin: tests/scalar/warn_accepted_neighbours.c
// Pins: 0025 item 6 R3: the right operand is a comma whose value is a truth value, which the compilers do not take as one
#include <cerastes.h>

int test(int x, int a, int b, int c)
{
  int z = 0;
  int t = !x == (z = 1, a > b);
  return t + z;
}
