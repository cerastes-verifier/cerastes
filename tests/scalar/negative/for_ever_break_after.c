// expect: error check @ L21
// mode: c23, c++20
// twin: tests/scalar/for_ever_break.c
// Pins: 0014 item 4: after for (;;) only the break path's facts hold
// The break leaves with i == n, so i < n fails.
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
    i += 1;
  }
  CERASTES_CHECK(i < n);
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
