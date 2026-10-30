// expect: error invariant-preserved @ L14
// mode: c23, c++20
// twin: tests/memory/loop_written_null_start.c
// Pins: 0040 item 5 (audit B3): the invariant claims *q == 1 where q is set to p, whose cell holds 0
#include <cerastes.h>
#include <stddef.h>

int null_start(int *p, int n)
    CERASTES_REQUIRES(cerastes_reads(*p) && *p == 0 && 0 <= n && n <= 10)
    CERASTES_ENSURES(r, cerastes_reads(*p) && r == 0)
{
  int *q = NULL;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && (q == NULL || (cerastes_reads(*q) && *q == 1)))
  CERASTES_DECREASES(n - i)
  while (i < n) {
    q = p;
    i += 1;
  }
  return q == NULL ? 0 : *q;
}
