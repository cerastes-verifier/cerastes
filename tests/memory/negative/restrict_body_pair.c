// expect: rejected cerastes-unsupported_annotation @ L16
// expect: rejected cerastes-unsupported_annotation @ L25
// expect: rejected cerastes-unsupported_annotation @ L34
// expect: rejected cerastes-unsupported_annotation @ L43
// expect: rejected cerastes-unsupported_annotation @ L52
// expect: rejected cerastes-unsupported_annotation @ L59
// mode: c23
// c-only: restrict does not exist in C++20 (EDG, clang++ and g++ refuse it, decision 0037 item 2)
// twin: tests/memory/restrict_dropped.c
// Pins: 0064 item 19 (question 326; audit round 2), the pair rule: an atom excludes only what it is sure to hold. A range atom whose count can be 0 holds nothing, so restrict is rejected at the function head when p's backing atom or the atom that must exclude it is such a range, unless both counts are structurally equal: owns(*p) beside owns_range(q, n) with q == p (probe a1c; the base verified it with n == 0, the body using the one permission there is through the other pointer), the same with reads_range (a2c), restrict p backed by owns_range(p, n) beside owns(*q) with p == q (a3c), two restrict ranges with counts 0 and 1 (a6), two ranges with counts of the same value written differently (n and n + 0: over-strict, listed), and owns_range(p, n) beside a held global owns(g) with p == &g and the body naming g (a7). The positive twin has the shapes with n > 0 [0064 item 19, question 326: every restrict parameter is now rejected by that one rule, whatever the contract; this pin's row keeps its tag and line and its condition no longer matters]
// Pre-registered from record 0064 item 19 as revised by audit round 2 (probes a1c, a2c, a3c, a6, a7), before any run: rejected (tag as range_restrict_reject_unbacked.c: unsupported_annotation at the function head)
#include <cerastes.h>

int g;

int cell_range_alias(int *restrict p, int *q, int n)  // <- here
    CERASTES_REQUIRES(cerastes_owns(*p) && cerastes_owns_range(q, n) && q == p)
    CERASTES_ENSURES(r, cerastes_owns(*p) && cerastes_owns_range(q, n) && r == 2)
{
  *p = 1;
  q[0] = 2;
  return *p;
}

int cell_reads_range_alias(int *restrict p, int *q, int n)  // <- here
    CERASTES_REQUIRES(cerastes_owns(*p) && cerastes_reads_range(q, n) && q == p)
    CERASTES_ENSURES(r, cerastes_owns(*p) && cerastes_reads_range(q, n) && r == 2)
{
  *p = 1;
  q[0] = 2;
  return *p;
}

int range_cell_alias(int *restrict p, int n, int *q)  // <- here
    CERASTES_REQUIRES(cerastes_owns_range(p, n) && cerastes_owns(*q) && p == q)
    CERASTES_ENSURES(r, cerastes_owns_range(p, n) && cerastes_owns(*q) && r == 2)
{
  p[0] = 1;
  *q = 2;
  return p[0];
}

int adjacent_ranges(int *restrict p, int *restrict q)  // <- here
    CERASTES_REQUIRES(cerastes_owns_range(p, 0) && cerastes_owns_range(q, 1) && p == q)
    CERASTES_ENSURES(r, cerastes_owns_range(p, 0) && cerastes_owns_range(q, 1) && r == 2)
{
  p[0] = 1;
  q[0] = 2;
  return p[0];
}

int counts_spelled_differently(int *restrict p, int *restrict q, int n)  // <- here
    CERASTES_REQUIRES(cerastes_owns_range(p, n) && cerastes_owns_range(q, n + 0))
    CERASTES_ENSURES(r, cerastes_owns_range(p, n) && cerastes_owns_range(q, n + 0) && r == 0)
{
  return 0;
}

int range_and_global(int *restrict p, int n)  // <- here
    CERASTES_REQUIRES(cerastes_owns_range(p, n) && cerastes_owns(g) && p == &g)
    CERASTES_ENSURES(r, cerastes_owns_range(p, n) && cerastes_owns(g) && r == 2)
{
  p[0] = 1;
  g = 2;
  return p[0];
}
