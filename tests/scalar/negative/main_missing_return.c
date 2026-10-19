// expect: error missing-return @ L15
// mode: c23, c++20
// twin: tests/scalar/main_implicit.c
// Pins: 0012 item 9: the same body in an ordinary function misses its return
// EDG's warning #940 is suppressed in verify mode.
#include <cerastes.h>

int helper(void)
{
  int x = 1;
  if (x > 5) {
    return 3;
  }
  CERASTES_CHECK(x == 1);
}
