// expect: error postcondition @ L21
// mode: c23, c++20
// twin: tests/scalar/do_break.c
// Pins: 0014 item 4: the postcondition holds on the condition exit only, not on the break exit
// The break exit has i == limit, which is below n when limit < n.
#include <cerastes.h>

int count_until(int n, int limit)
    CERASTES_REQUIRES(1 <= n && 1 <= limit)
    CERASTES_ENSURES(r, r == n)
{
  int i = 0;
  CERASTES_INVARIANT(i < n && i < limit)
  CERASTES_DECREASES(n - i)
  do {
    i += 1;
    if (i == limit) {
      break;
    }
  } while (i < n);
  return i;
}
