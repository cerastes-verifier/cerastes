// expect: error uninitialized @ L14
// mode: c23, c++20
// twin: tests/scalar/uninit_before_loop.c
// Pins: 0019 items 2 and 4: a mention in a decreases clause is a read, checked on entry
#include <cerastes.h>

int count(int n)
    CERASTES_REQUIRES(0 <= n && n <= 100)
    CERASTES_ENSURES(r, r == n)
{
  int step;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_DECREASES(n - i + step * 0)
  while (i < n) {
    step = 1;
    i += step;
  }
  return i;
}
