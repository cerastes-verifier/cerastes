// expect: rejected cerastes-self_comparison @ L8
// mode: c23, c++20
// twin: tests/scalar/warn_accepted_neighbours.c
// Pins: 0025 item 6 R5: the result binder compared with itself in an ensures
#include <cerastes.h>

int test(int x)
    CERASTES_ENSURES(r, r == r)
{
  return x;
}
