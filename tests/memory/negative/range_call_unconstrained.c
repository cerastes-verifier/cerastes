// expect: error permission @ L20
// mode: c23, c++20
// twin: tests/memory/range_call_borrowed.c
// Pins: 0060 item 8 (audit S7, abl/D1): an unconstrained pointer parameter passed beside a local array: the shape row, the liveness row and the first marked row all fail (null, dangling, or an object the caller does not hold). The record counts three Silicon errors; the cell call's rows share one message per atom, so the driver prints one: ONE permission at the call registered (to be corrected at the run)
// Pre-registered from record 0060 item 8 (Part C), before the code
#include <cerastes.h>
#include <stddef.h>

void touch(int *p, int n)
    CERASTES_REQUIRES(cerastes_owns_range(p, n))
    CERASTES_ENSURES(cerastes_owns_range(p, n))
{
  (void)p;
  (void)n;
}

int use(int *q)
{
  int a[2] = {1, 2};
  touch(q, 2);
  return a[0];
}
