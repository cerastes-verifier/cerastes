// expect: error permission @ L19
// mode: c23, c++20
// twin: tests/memory/range_return_sum_term.c
// Pins: 0060 item 9 (audit round 2, S4, s6b): keepr(&p[1]) keeps a read fraction of an element of the range: one permission at the return (the first row to fail may blame *q's liveness sum: the message is the audit's S4 note)
// Pre-registered from record 0060 item 9 (audit round 2), before the code
#include <cerastes.h>
#include <stddef.h>

void keepr(const int *x)
    CERASTES_REQUIRES(cerastes_reads(*x));

void other_slot(const int *p, int n, const int *q)
    CERASTES_REQUIRES(cerastes_reads_range(p, n) && cerastes_reads(*q) && p == q && n >= 2)
    CERASTES_ENSURES(cerastes_reads_range(p, n) && cerastes_reads(*q))
{
  (void)q;
  (void)n;
  keepr(&p[1]);
}
