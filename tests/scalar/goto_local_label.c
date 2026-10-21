// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0015 item 2: a GNU local label declaration leaves no trace in the IL; the goto names its label entity
// __label__ out; only scopes the name, so it changes nothing that is verified.
#include <cerastes.h>

int bounded(int x)
    CERASTES_REQUIRES(0 <= x && x <= 100)
    CERASTES_ENSURES(r, (x > 50 && r == 50) || (x <= 50 && r == x))
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
