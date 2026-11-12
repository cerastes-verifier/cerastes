// expect: error permission @ L29
// mode: c23, c++20
// twin: tests/memory/range_call_reads.c
// Pins: PIN of question 238, the two-atom form (audit round 3, B1, abl/x1_sumform): f borrows reads_range(p, n) and promises nothing back; keep_slot(&p[1]) keeps a fraction of slot 1 and gives the liveness back; peek2(p, n, p, n) then takes two read ranges that meet at every slot: the call's marked rows are in the sum form (each amount plus the other's where the ranges overlap), so the first one fails: one permission at the call (before the fix this was an exit 3, Silicon's own precondition check; the amounts are as in the single-atom pin of question 238, the sum is what the false alarm is now)
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

void f(const int *p, int n)
    CERASTES_REQUIRES(cerastes_reads_range(p, n) && n > 1)
{
  keep_slot(&p[1]);
  peek2(p, n, p, n);
}
