// expect: error termination @ L14
// mode: c23, c++20
// twin: tests/scalar/for_ever_break.c
// Pins: 0013 and 0014 item 4: every iteration followed by another must decrease the measure, in a loop left by break
// count_up no longer makes progress; count_down is unchanged.
#include <cerastes.h>

int count_up(int n)
    CERASTES_REQUIRES(0 <= n)
    CERASTES_ENSURES(r, r == n)
{
  int i = 0;
  CERASTES_INVARIANT(i <= n)
  CERASTES_DECREASES(n - i)
  for (;;) {
    if (i >= n) {
      break;
    }
    i += 0;
  }
  return i;
}

int count_down(int n)
    CERASTES_REQUIRES(0 <= n)
    CERASTES_ENSURES(r, r == 0)
{
  int i = n;
  CERASTES_INVARIANT(0 <= i)
  CERASTES_DECREASES(i)
  while (1) {
    if (i <= 0) {
      break;
    }
    i -= 1;
  }
  return i;
}
