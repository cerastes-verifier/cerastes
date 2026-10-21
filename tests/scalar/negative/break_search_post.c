// expect: error postcondition @ L21
// mode: c23, c++20
// twin: tests/scalar/break_search.c
// Pins: 0014 item 4: the postcondition holds on the condition exit only, not on the break exit
// The strengthened ensures r == n fails on the break path, at the return.
#include <cerastes.h>

int count_until(int n, int limit)
    CERASTES_REQUIRES(0 <= n && 0 <= limit)
    CERASTES_ENSURES(r, r <= n && r <= limit && r == n)
{
  int i = 0;
  CERASTES_INVARIANT(i <= n && i <= limit)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    if (i == limit) {
      break;
    }
    i += 1;
  }
  return i;
}
