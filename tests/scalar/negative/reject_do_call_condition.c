// expect: rejected cerastes-unsupported_expression @ L23
// mode: c23, c++20
// twin: tests/scalar/do_count.c
// Pins: 0014 item 5: a call in a do condition is rejected
// The loop has its measure, so the call is the only reason.
#include <cerastes.h>

int step(int x)
    CERASTES_REQUIRES(0 <= x && x < 100)
    CERASTES_ENSURES(r, r == x + 1)
{
  return x + 1;
}

int test(int n)
    CERASTES_REQUIRES(0 <= n && n < 50)
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i < 100)
  CERASTES_DECREASES(100 - i)
  do {
    i += 1;
  } while (step(i) < n);
  return i;
}
