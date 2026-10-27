// expect: rejected cerastes-chained_comparison @ L9
// mode: c23, c++20
// twin: tests/scalar/warn_accepted_neighbours.c
// Pins: 0025 item 6 R4: `a == b < c` (-Wparentheses: gcc only)
#include <cerastes.h>

int test(int a, int b, int c, int d)
{
  return a == b < c;
}
