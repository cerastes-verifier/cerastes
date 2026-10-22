// expect: error postcondition @ L24
// mode: c23, c++20
// twin: tests/scalar/goto_in_loop_body.c
// Pins: 0015 item 2: the exact ensures of the positive notices that the goto skips the count
// The ensures claims the count reaches n, which fails when limit < n (the goto skips the count from limit on).
#include <cerastes.h>

int count_below(int n, int limit)
    CERASTES_REQUIRES(0 <= n && n <= 1000 && 0 <= limit && limit <= 1000)
    CERASTES_ENSURES(r, r == n)
{
  int count = 0;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && count == (i < limit ? i : limit))
  CERASTES_DECREASES(n - i)
  while (i < n) {
    if (i >= limit) {
      goto next;
    }
    count += 1;
next:
    i += 1;
  }
  return count;
}
