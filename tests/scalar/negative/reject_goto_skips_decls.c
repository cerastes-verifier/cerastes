// expect: rejected cerastes-unsupported_statement @ L12
// c-only: C++ makes a jump past an initialized declaration a front-end error; C23 only warns (warning 546)
// twin: tests/scalar/goto_error_exit.c
// Pins: 0015 item 2: a goto never skips a declaration, however many declarators it has
// a and b are both in scope at the label.
#include <cerastes.h>

int skip(int x)
    CERASTES_REQUIRES(0 <= x && x <= 10)
{
  if (x > 5) {
    goto out;
  }
  int a = 1, b = 2;
  x += a + b;
out:
  return x;
}
