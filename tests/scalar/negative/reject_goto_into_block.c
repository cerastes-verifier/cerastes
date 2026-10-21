// expect: rejected cerastes-unsupported_statement @ L13
// mode: c23, c++20
// twin: tests/scalar/goto_error_exit.c
// Pins: 0015 item 2: a goto never enters a block: the label is not a statement of a block that encloses it
// The label is in a plain nested block that comes after the goto.
#include <cerastes.h>

int enter(int x)
    CERASTES_REQUIRES(0 <= x && x <= 10)
{
  int y = 0;
  if (x > 5) {
    goto inside;
  }
  {
    y = 1;
inside:
    y += 1;
  }
  return y;
}
