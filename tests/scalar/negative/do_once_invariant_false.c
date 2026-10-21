// expect: error invariant-entry @ L12
// mode: c23, c++20
// twin: tests/scalar/do_once_invariant.c
// Pins: 0014 item 5: a written invariant of do ... while (0) that is false on entry
#include <cerastes.h>

int add_one(int x)
    CERASTES_REQUIRES(x < 2147483647)
    CERASTES_ENSURES(r, r == x + 1)
{
  int y = x;
  CERASTES_INVARIANT(y != x)
  do {
    y += 1;
  } while (0);
  return y;
}
