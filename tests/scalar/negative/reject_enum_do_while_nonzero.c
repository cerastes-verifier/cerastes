// expect: rejected cerastes-missing_decreases @ L14
// mode: c23, c++20
// twin: tests/scalar/enum_do_once.c
// Pins: 0018 item 9: do ... while (GREEN) with GREEN == 1 does not run once, so it needs a decreases (RED == 0 does not)
#include <cerastes.h>

enum color { RED, GREEN, BLUE };

int spin(int n)
    CERASTES_REQUIRES(0 <= n && n < 100)
{
  int i = n;
  CERASTES_INVARIANT(i >= n)
  do {
    i += 1;
    if (i > 50) {
      break;
    }
  } while (GREEN);
  return i;
}
