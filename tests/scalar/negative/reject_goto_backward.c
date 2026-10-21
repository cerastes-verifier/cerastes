// expect: rejected cerastes-unsupported_statement @ L16
// mode: c23, c++20
// twin: tests/scalar/goto_error_exit.c
// Pins: 0015 item 2: a goto is never backward (it would be a loop that Silicon does not check)
// Written as a loop with a goto; the twin is a forward goto.
#include <cerastes.h>

int count_up(int n)
    CERASTES_REQUIRES(0 <= n && n <= 10)
    CERASTES_ENSURES(r, r == n)
{
  int i = 0;
again:
  if (i < n) {
    i += 1;
    goto again;
  }
  return i;
}
