// expect: error uninitialized @ L19
// mode: c23, c++20
// twin: tests/scalar/uninit_do_while.c
// Pins: 0019 items 2 and 4: the condition of a do loop reads x, which the continue path skipped assigning (an assertion, not an invariant)
#include <cerastes.h>

int f(int c)
{
  int x;
  int n = 0;
  CERASTES_INVARIANT(0 <= n && n < 3)
  CERASTES_DECREASES(3 - n)
  do {
    n += 1;
    if (c != 0) {
      continue;
    }
    x = 1;
  } while (n < 3 && x > 0);
  return n;
}
