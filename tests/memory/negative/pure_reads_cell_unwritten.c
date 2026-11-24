// expect: error uninitialized @ L18
// mode: c23, c++20
// twin: tests/memory/pure_reads_cell.c
// Pins: 0062 item 7: an application over the cell of a local that was never written in a check: the flag row, uninitialized, at the check
// Pre-registered from record 0062 (Part B), before the code: uninitialized. Rewritten before any run with its twin (the address is taken in code, 0038 item 6)
#include <cerastes.h>

CERASTES_PURE int get(const int *p)
    CERASTES_REQUIRES(cerastes_reads(*p))
{
  return *p;
}

int use(void)
{
  int x;
  int *q = &x;
  CERASTES_CHECK(get(q) == get(q));
  return q == 0;
}
