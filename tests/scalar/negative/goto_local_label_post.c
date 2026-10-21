// expect: error postcondition @ L20
// mode: c23, c++20
// twin: tests/scalar/goto_local_label.c
// Pins: 0015 item 2: a goto to a local label is verified like any other
// The ensures forgets the clamp: for x > 50 the result is 50, not x.
#include <cerastes.h>

int bounded(int x)
    CERASTES_REQUIRES(0 <= x && x <= 100)
    CERASTES_ENSURES(r, r == x)
{
  __label__ out;
  int r = x;
  if (x > 50) {
    r = 50;
    goto out;
  }
  r = x;
out:
  return r;
}
