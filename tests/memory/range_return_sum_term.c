// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0060 item 9 (audit round 2, S4/S6): a read range beside a read cell of one slot (p == q): a cell callee that takes the cell's fraction (q) or an element's (&p[1]) and gives it back leaves the return rows' sum term, which counts the range's fraction at the slot the cell shares, satisfied
// Pre-registered from record 0060 item 9 (audit round 2), before the code: verifies
#include <cerastes.h>
#include <stddef.h>

void peekr(const int *x)
    CERASTES_REQUIRES(cerastes_reads(*x))
    CERASTES_ENSURES(cerastes_reads(*x));

void same_slot(const int *p, int n, const int *q)
    CERASTES_REQUIRES(cerastes_reads_range(p, n) && cerastes_reads(*q) && p == q && n >= 1)
    CERASTES_ENSURES(cerastes_reads_range(p, n) && cerastes_reads(*q))
{
  (void)p;
  (void)n;
  peekr(q);
}

void other_slot(const int *p, int n, const int *q)
    CERASTES_REQUIRES(cerastes_reads_range(p, n) && cerastes_reads(*q) && p == q && n >= 2)
    CERASTES_ENSURES(cerastes_reads_range(p, n) && cerastes_reads(*q))
{
  (void)q;
  (void)n;
  peekr(&p[1]);
}
