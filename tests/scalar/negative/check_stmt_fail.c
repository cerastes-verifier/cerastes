// expect: error check @ L11
// mode: c23, c++20
// twin: tests/scalar/check_stmt.c
// Pins: 0012 item 3: a check that may not hold, reported at the check
#include <cerastes.h>

int bump(int x)
    CERASTES_REQUIRES(x > 0 && x < 100)
{
  int y = x + 1;
  CERASTES_CHECK(y < 100);
  return y;
}
