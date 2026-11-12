// expect: error permission @ L19
// mode: c23, c++20
// twin: tests/memory/range_return_sum_term.c
// Pins: 0060 item 9 (audit round 2, S4, s6): keepr(q) keeps a read fraction of the shared slot: the return's sum row (the cell's fraction plus the range's at that slot) fails: one permission at the return (the closing brace of the void function)
// Pre-registered from record 0060 item 9 (audit round 2), before the code
#include <cerastes.h>
#include <stddef.h>

void keepr(const int *x)
    CERASTES_REQUIRES(cerastes_reads(*x));

void same_slot(const int *p, int n, const int *q)
    CERASTES_REQUIRES(cerastes_reads_range(p, n) && cerastes_reads(*q) && p == q && n >= 1)
    CERASTES_ENSURES(cerastes_reads_range(p, n) && cerastes_reads(*q))
{
  (void)p;
  (void)n;
  keepr(q);
}
