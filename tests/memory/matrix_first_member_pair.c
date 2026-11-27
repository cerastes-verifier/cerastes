// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0064 items 4 and 12 (the internal error of Commit 5's build, fixed in slice 4; the encoder owner's probes U2_orc_ok.c and U2_oon_ok.c, build/m4.5e1e2d/probeV/u2/): a structure whose FIRST member is a matrix (struct U2 { int m[2][2]; int c; }) under two pointer atoms, owns(*p) with reads(*q) (q const) and owns(*p) with owns(*q): the callee writes one cell of the matrix through p from a cell of q and returns the member beside the matrix, and each caller passes two distinct locals and gets the value back (a caller state of 5 for c); the two twins are negative/matrix_first_member_pair_wrong.c (the postcondition) and negative/matrix_first_member_pair_alias.c (the same call with one object twice)
// Pre-registered from record 0064 items 4 and 12 and the encoder owner's measured table (build/m4.5e1e2d/edg-backend-report-19.md, "U2: THE FIRST-MEMBER MATRIX": verified on edg48, 15.8 s and 18.3 s per function), before any run (test-mutator-11, 2026-11-27): verifies (time it first: two functions with two callers, the owner measured about 16 s for each pair)
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

int use_orc(void)
    CERASTES_ENSURES(r, r == 5)
{
  struct U2 a = {{{1, 2}, {3, 4}}, 5};
  struct U2 b = {{{6, 7}, {8, 9}}, 5};
  return orc(&a, &b);
}

int oon(struct U2 *p, struct U2 *q)
    CERASTES_REQUIRES(cerastes_owns(*p) && cerastes_owns(*q))
    CERASTES_ENSURES(r, cerastes_owns(*p) && cerastes_owns(*q) && r == cerastes_old(p->c))
{
  p->m[1][1] = q->m[0][0];
  return p->c;
}

int use_oon(void)
    CERASTES_ENSURES(r, r == 5)
{
  struct U2 a = {{{1, 2}, {3, 4}}, 5};
  struct U2 b = {{{6, 7}, {8, 9}}, 5};
  return oon(&a, &b);
}
