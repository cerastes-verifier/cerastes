// expect: error uninitialized @ L32
// mode: c23, c++20
// twin: tests/memory/init_loop_nested.c
// Pins: 0042 item 5: the same loops, but the function returns x, which the loops write only when i == 5: uninitialized at the read
#include <cerastes.h>

int nested(int *q, int n)
    CERASTES_REQUIRES(cerastes_reads(*q) && 0 <= n && n <= 10)
    CERASTES_ENSURES(cerastes_reads(*q))
{
  int x;
  int *px = &x;
  int s = 0;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && 0 <= s && s <= 1)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    if (i == 5) {
      *px = 1;
    }
    int j = 0;
    CERASTES_INVARIANT(0 <= j && j <= n && 0 <= s && s <= 1)
    CERASTES_DECREASES(n - j)
    while (j < n) {
      if (*q > 0) {
        s = 1;
      }
      j += 1;
    }
    i += 1;
  }
  return s > 5 ? 0 : x;  // <- here
}
