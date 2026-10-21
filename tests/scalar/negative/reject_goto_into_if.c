// expect: rejected cerastes-unsupported_statement @ L13
// mode: c23, c++20
// twin: tests/scalar/goto_error_exit.c
// Pins: 0015 item 2: a goto never enters an if branch (the facts of the condition would be skipped)
// The label is inside the then-branch of a later if.
#include <cerastes.h>

int enter(int x)
    CERASTES_REQUIRES(0 <= x && x <= 10)
{
  int y = 0;
  if (x > 5) {
    goto inside;
  }
  if (x < 3) {
    y = 1;
inside:
    y += 1;
  }
  return y;
}
