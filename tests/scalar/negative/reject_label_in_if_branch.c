// expect: rejected cerastes-unsupported_statement @ L16
// mode: c23, c++20
// twin: tests/scalar/goto_error_exit.c
// Pins: 0015 item 2: a label that is the whole body of an if without braces sits in a block the compiler made
// The label itself is rejected (a goto, there only so the label is referenced, adds no diagnostic of its own).
#include <cerastes.h>

int labeled(int x)
    CERASTES_REQUIRES(0 <= x && x <= 10)
{
  int y = 0;
  if (x > 8) {
    goto L;
  }
  if (x > 5)
    L: y = 1;
  return y;
}
