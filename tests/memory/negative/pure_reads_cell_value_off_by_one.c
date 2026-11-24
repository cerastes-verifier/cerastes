// expect: error check @ L18
// mode: c23, c++20
// twin: tests/memory/pure_reads_cell.c
// Pins: 0062 item 7 (audit round 2 R2-S3): the application over a cell reads the cell's value: get(q) is 5, so == 6 is the check error
// Pre-registered from record 0062 (Part B), before the code: check. the value-observing twin of the positive: one check error, at the check
#include <cerastes.h>

CERASTES_PURE int get(const int *p)
    CERASTES_REQUIRES(cerastes_reads(*p))
{
  return *p;
}

int use(void)
{
  int x = 5;
  int *q = &x;
  CERASTES_CHECK(get(q) == 6);
  return *q;
}
