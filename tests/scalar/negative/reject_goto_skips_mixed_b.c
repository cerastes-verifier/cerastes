// expect: rejected cerastes-unsupported_statement @ L11
// c-only: C++ makes a jump past an initialized declaration a front-end error; C23 only warns (warning 546)
// twin: tests/scalar/goto_error_exit.c
// Pins: 0015 item 2 / 0019 item 6: the same, with the initialized declarator first (int y = 1, x;)
#include <cerastes.h>

int skip(int a)
    CERASTES_REQUIRES(0 <= a && a <= 10)
{
  if (a > 5) {
    goto out;
  }
  int y = 1, x;
  x = a;
  a = x + y;
out:
  return a;
}
