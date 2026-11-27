// expect: error postcondition @ L20
// expect: error postcondition @ L28
// expect: error postcondition @ L43
// mode: c23, c++20
// twin: tests/memory/matrix_reads_pair.c
// Pins: 0064 items 4 and 12 (the encoder owner's U2_rrc_wrong.c, measured on edg49: postcondition @ f and @ use; the claim here is `r != p->c` instead of the owner's `... + 1`, which adds an `overflow` row of its own): the postcondition of rr_bad says the result differs from the member beside the matrix, which the body returns unchanged, so it fails at the return, and its caller, which relies on that contract, fails its own postcondition (the result is 5) at its return; and a caller that reads one object twice through rr2 (f(&a, &a), which verifies) and claims 6 where the member is 5 fails its postcondition at its return; the right-value siblings are rr2, use2_distinct and use2_alias in tests/memory/matrix_reads_pair.c
// Pre-registered from the encoder owner's measured table (build/m4.5e1e2d/edg-backend-report-19.md, "READS-READS AT A CALL (il231)": error postcondition @ f and @ use) before any run (test-mutator-11, 2026-11-27): the two postconditions of the pair and the one of the alias caller
#include <cerastes.h>

struct U2 {
  int m[2][2];
  int c;
};

int rr_bad(const struct U2 *p, const struct U2 *q)
    CERASTES_REQUIRES(cerastes_reads(*p) && cerastes_reads(*q))
    CERASTES_ENSURES(r, cerastes_reads(*p) && cerastes_reads(*q) && r != p->c)
{
  (void)q;
  return p->c;  // <- here
}

int use_bad(void)
    CERASTES_ENSURES(r, r == 5)
{
  struct U2 a = {{{1, 2}, {3, 4}}, 5};
  struct U2 b = {{{6, 7}, {8, 9}}, 5};
  return rr_bad(&a, &b);  // <- here
}

int rr2(const struct U2 *p, const struct U2 *q)
    CERASTES_REQUIRES(cerastes_reads(*p) && cerastes_reads(*q))
    CERASTES_ENSURES(r, cerastes_reads(*p) && cerastes_reads(*q) && r == p->c)
{
  (void)q;
  return p->c;
}

int use2_alias_wrong(void)
    CERASTES_ENSURES(r, r == 6)
{
  struct U2 a = {{{1, 2}, {3, 4}}, 5};
  return rr2(&a, &a);  // <- here
}
