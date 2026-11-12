// expect: error permission @ L29
// mode: c23, c++20
// twin: tests/memory/range_call_reads.c
// Pins: Audit round 3, x3 (question 238's two-atom form on a local array): keep_slot(&a[1]) keeps a fraction of a[1] and pins the array, and peek2(a, 2, a, 2) then fails its first sum-form marked row: one permission at the call, which ends the path, so the lifetime error of the array's scope exit (the kept fraction, a real error) is not reached. UNCERTAIN: if the sum-form row passes here, the lifetime at `return 0;` is what the record predicts
// Pre-registered from record 0060 items 6 and 8 (audit round 3), before the code
#include <cerastes.h>
#include <stddef.h>

int peek2(const int *p, int n, const int *q, int m)
    CERASTES_REQUIRES(cerastes_reads_range(p, n) && cerastes_reads_range(q, m) && n > 0 && m > 0)
    CERASTES_ENSURES(r, cerastes_reads_range(p, n) && cerastes_reads_range(q, m))
{
  (void)n;
  (void)m;
  return 0;
}

void keep_slot(const int *x)
    CERASTES_REQUIRES(cerastes_reads(*x))
    CERASTES_ENSURES(cerastes_live(x))
{
  (void)x;
}

int main(void)
{
  int a[2] = {1, 2};
  keep_slot(&a[1]);
  peek2(a, 2, a, 2);
  return 0;
}
