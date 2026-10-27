// expect: rejected cerastes-self_comparison @ L9
// mode: c23, c++20
// twin: tests/scalar/warn_accepted_neighbours.c
// Pins: 0025 item 6 R5: in a check annotation
#include <cerastes.h>

int test(int x)
{
  CERASTES_CHECK(x == x);
  return x;
}
