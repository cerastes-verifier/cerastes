// expect: error postcondition @ L21
// expect: error postcondition @ L29
// expect: error postcondition @ L37
// expect: error postcondition @ L45
// mode: c23, c++20
// twin: tests/memory/matrix_first_member_pair.c
// Pins: 0064 items 4 and 12 (the encoder owner's U2_orc_wrong.c and U2_oon_wrong.c, measured on edg48: postcondition @ f and @ use in both modes): the postcondition of each callee says the result differs from the old value of the member beside the matrix, which the body does not do (it returns that member unchanged), so it fails at the callee's return, and the caller, which relies on that contract, fails its own postcondition (the result is 5) at its return; the right-value siblings are orc, use_orc, oon and use_oon in tests/memory/matrix_first_member_pair.c
// Pre-registered from the encoder owner's measured table (build/m4.5e1e2d/edg-backend-report-19.md, "U2: THE FIRST-MEMBER MATRIX": error postcondition @ f and @ use) before any run (test-mutator-11, 2026-11-27); the claim is `r != cerastes_old(p->c)` instead of the owner's `... + 1`, which adds an `overflow` row of its own: the rows are the two postconditions of each pair
#include <cerastes.h>

struct U2 {
  int m[2][2];
  int c;
};

int orc(struct U2 *p, const struct U2 *q)
    CERASTES_REQUIRES(cerastes_owns(*p) && cerastes_reads(*q))
    CERASTES_ENSURES(r, cerastes_owns(*p) && cerastes_reads(*q) && r != cerastes_old(p->c))
{
  p->m[1][1] = q->m[0][0];
  return p->c;  // <- here
}

int use_orc(void)
    CERASTES_ENSURES(r, r == 5)
{
  struct U2 a = {{{1, 2}, {3, 4}}, 5};
  struct U2 b = {{{6, 7}, {8, 9}}, 5};
  return orc(&a, &b);  // <- here
}

int oon(struct U2 *p, struct U2 *q)
    CERASTES_REQUIRES(cerastes_owns(*p) && cerastes_owns(*q))
    CERASTES_ENSURES(r, cerastes_owns(*p) && cerastes_owns(*q) && r != cerastes_old(p->c))
{
  p->m[1][1] = q->m[0][0];
  return p->c;  // <- here
}

int use_oon(void)
    CERASTES_ENSURES(r, r == 5)
{
  struct U2 a = {{{1, 2}, {3, 4}}, 5};
  struct U2 b = {{{6, 7}, {8, 9}}, 5};
  return oon(&a, &b);  // <- here
}
