// expect: error permission @ L25
// expect: error invariant-preserved @ L25
// mode: c23, c++20
// twin: tests/memory/range_loop_element_clauses.c
// Pins: 0060 item 11: the invariant reads p[i] where i may be n: the loop does not hold that element (the range place ends at n): permission at the invariant, which is the loop's line. The clause (i == 0 || p[i] >= 0) is not inductive (nothing says p[i + 1] >= 0 after an iteration), so invariant-preserved fails at the same line too (ruling in report 16: a clause that reads an unframed element cannot be made inductive from the loop's own facts)
// Pre-registered from record 0060 item 11 (Part D), before the code
#include <cerastes.h>
#include <stddef.h>

void countdown(int *p, int n)
    CERASTES_REQUIRES(cerastes_owns_range(p, n) && n > 0 && 0 <= p[0] && p[0] <= 100)
    CERASTES_ENSURES(cerastes_owns_range(p, n) && p[0] == 0)
{
  (void)n;
  CERASTES_INVARIANT(0 <= p[0] && p[0] <= 100) CERASTES_DECREASES(p[0]) while (p[0] > 0) {
    p[0] -= 1;
  }
}

int first_negative(const int *p, int n)
    CERASTES_REQUIRES(cerastes_reads_range(p, n))
    CERASTES_ENSURES(r, cerastes_reads_range(p, n) && (r == -1 || (0 <= r && r < n && p[r] < 0)))
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && (i == 0 || p[i] >= 0)) CERASTES_DECREASES(n - i) while (i < n) {
    if (p[i] < 0) {
      return i;
    }
    i++;
  }
  return -1;
}
