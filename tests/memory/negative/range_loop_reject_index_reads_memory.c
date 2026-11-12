// expect: rejected cerastes-unsupported_expression @ L14
// mode: c23, c++20
// twin: tests/memory/range_loop_element_clauses.c
// Pins: 0060 item 11 (audit round 4, S4): an element of a pointer whose index reads memory (p[*q]) in a loop invariant is rejected: the index must be an expression over the loop's locals and parameters (the audit's g1); the tag is the encoder's unsupported_expression, at the invariant (the loop's line)
// Pre-registered from record 0060 item 11 (audit round 4), before the code
#include <cerastes.h>
#include <stddef.h>

int f(const int *p, int n, const int *q)
    CERASTES_REQUIRES(cerastes_reads_range(p, n) && cerastes_reads(*q) && n > 0 && *q == 0)
    CERASTES_ENSURES(r, cerastes_reads_range(p, n) && cerastes_reads(*q))
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && p[*q] == p[0]) CERASTES_DECREASES(n - i) while (i < n) {
    i = i + 1;
  }
  return 0;
}
