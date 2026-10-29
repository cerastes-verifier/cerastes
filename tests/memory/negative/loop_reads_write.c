// expect: error permission @ L15
// mode: c23, c++20
// twin: tests/memory/loop_owns_fill.c
// Pins: 0038 item 3: the place of a reads(*p) parameter is held at less than write, so the write in the loop has no permission
#include <cerastes.h>

void fill(int *p, int n)
    CERASTES_REQUIRES(cerastes_reads(*p) && *p == 0 && 0 <= n && n <= 100)
    CERASTES_ENSURES(cerastes_reads(*p))
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && *p == i)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    *p += 1;
    i += 1;
  }
}
