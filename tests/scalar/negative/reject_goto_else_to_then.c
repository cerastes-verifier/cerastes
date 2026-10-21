// expect: rejected cerastes-unsupported_statement @ L16
// mode: c23, c++20
// twin: tests/scalar/goto_error_exit.c
// Pins: 0015 item 2: a goto from the else-branch back into the then-branch (backward, and into a branch)
// Both rules reject it; one diagnostic, at the goto.
#include <cerastes.h>

int branches(int x)
    CERASTES_REQUIRES(0 <= x && x <= 10)
{
  int y = 0;
  if (x > 5) {
back:
    y = 1;
  } else {
    goto back;
  }
  return y;
}
