// expect: error missing-return @ L15
// mode: c23, c++20
// twin: tests/scalar/all_paths_return.c
// Pins: 0012 item 9: a path that reaches the closing brace of a non-void function
// EDG's warning #940 is suppressed in verify mode.
#include <cerastes.h>

int pick(int x) CERASTES_ENSURES(r, (x > 0 && r == 1) || (x <= 0 && r == 2))
{
  if (x > 0) {
    return 1;
  } else if (x < 0) {
    return 2;
  }
}
