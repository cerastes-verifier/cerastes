// expect: error permission @ L25
// expect: error permission @ L39
// mode: c23, c++20
// twin: tests/memory/matrix_first_member_pair.c
// Pins: 0064 items 4 and 12 (the encoder owner's U2_orc_alias.c and U2_oon_alias.c, measured on edg48: permission at the call, "passes `a` to `f` twice, but one of the two places is to be owned and they may be the same object"): the same two callees called with one object twice, f(&a, &a): the contract needs one of the two places owned, so it is refused at the call; the right-value siblings are use_orc and use_oon in tests/memory/matrix_first_member_pair.c (two distinct locals)
// Pre-registered from the encoder owner's measured table (build/m4.5e1e2d/edg-backend-report-19.md, "U2: THE FIRST-MEMBER MATRIX": permission @ the call) before any run (test-mutator-11, 2026-11-27): one permission row per call
#include <cerastes.h>

struct U2 {
  int m[2][2];
  int c;
};

int orc(struct U2 *p, const struct U2 *q)
    CERASTES_REQUIRES(cerastes_owns(*p) && cerastes_reads(*q))
    CERASTES_ENSURES(r, cerastes_owns(*p) && cerastes_reads(*q) && r == cerastes_old(p->c))
{
  p->m[1][1] = q->m[0][0];
  return p->c;
}

int alias_orc(void)
{
  struct U2 a = {{{1, 2}, {3, 4}}, 5};
  return orc(&a, &a);  // <- here
}

int oon(struct U2 *p, struct U2 *q)
    CERASTES_REQUIRES(cerastes_owns(*p) && cerastes_owns(*q))
    CERASTES_ENSURES(r, cerastes_owns(*p) && cerastes_owns(*q) && r == cerastes_old(p->c))
{
  p->m[1][1] = q->m[0][0];
  return p->c;
}

int alias_oon(void)
{
  struct U2 a = {{{1, 2}, {3, 4}}, 5};
  return oon(&a, &a);  // <- here
}
