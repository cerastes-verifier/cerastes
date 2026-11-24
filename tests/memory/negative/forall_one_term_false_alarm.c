// expect: error postcondition @ L25
// mode: c23, c++20
// twin: tests/memory/forall_one_term_sibling.c
// Pins: PIN of question 253 (0061 item 6, S45 (b) oneterm; lifted by M4.5e2): a false alarm, fail closed: a multi-class quantifier is instantiated only where the program holds a term of every class, and the body reads only p[2] (not q[2]), so r >= 0 cannot be proved from `p[i] == q[i]` and `q[i] >= 0` although it is true; forall_one_term_sibling.c reads q[2] too and verifies
// Pre-registered from record 0061 item 7 (Part B), before the code
#include <cerastes.h>
#include <stddef.h>

CERASTES_PURE bool same_at(int i, const int *p, const int *q, int n)
{
  return !(0 <= i && i < n) || p[i] == q[i];
}

CERASTES_PURE bool nonneg(int i, const int *a, int n)
{
  return !(0 <= i && i < n) || a[i] >= 0;
}

int via_p(const int *p, const int *q, int n)
    CERASTES_REQUIRES(cerastes_reads_range(p, n) && cerastes_reads_range(q, n) && n > 2 && cerastes_forall(same_at, p, q, n) && cerastes_forall(nonneg, q, n))
    CERASTES_ENSURES(r, cerastes_reads_range(p, n) && cerastes_reads_range(q, n) && r >= 0)
{
  (void)n;
  (void)q;
  return p[2];
}
