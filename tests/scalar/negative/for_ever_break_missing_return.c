// expect: error missing-return @ L19
// mode: c23, c++20
// twin: tests/scalar/for_ever_break.c
// Pins: 0012 item 9 and 0014 item 4: the break out of for (;;) reaches the end of a non-void function
#include <cerastes.h>

int count_up(int n)
    CERASTES_REQUIRES(0 <= n && n <= 1000)
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_DECREASES(n - i)
  for (;;) {
    if (i >= n) {
      break;
    }
    i += 1;
  }
}
