// expect: error postcondition @ L12
// mode: c23, c++20
// twin: tests/scalar/discarded_guarded_effects.c
// Pins: 0020 item 5: c ? x++ : 0 increments x when c is non-zero
#include <cerastes.h>

int guarded_incr(int c, int x)
    CERASTES_REQUIRES(x < 2147483647)
    CERASTES_ENSURES(r, r == (c != 0 ? x : x))
{
  c ? x++ : 0;
  return x;
}
