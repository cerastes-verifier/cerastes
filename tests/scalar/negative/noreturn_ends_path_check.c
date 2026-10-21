// expect: error check @ L13
// mode: c23, c++20
// twin: tests/scalar/noreturn_ends_path.c
// Pins: 0015 item 4: a check before the call of abort() is still reachable
// The false check is on the path where x < 0, before the program ends.
#include <stdlib.h>
#include <cerastes.h>

int non_negative(int x)
    CERASTES_ENSURES(r, r == x && x >= 0)
{
  if (x < 0) {
    CERASTES_CHECK(false);
    abort();
  }
  return x;
}
