// expect: error invariant-preserved @ L14
// mode: c23, c++20
// twin: tests/scalar/goto_in_loop_body.c
// Pins: 0015 item 2: the invariant is checked on the goto path to the end of the body
// count == i holds when the body counts, and fails when the goto skips the count (count stays, i grows).
#include <cerastes.h>

int count_below(int n, int limit)
    CERASTES_REQUIRES(0 <= n && n <= 1000 && 0 <= limit && limit <= 1000)
    CERASTES_ENSURES(r, 0 <= r && r <= n)
{
  int count = 0;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && count == i)
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
