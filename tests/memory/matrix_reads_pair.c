// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0064 items 4 and 12 (the internal error of Commit 5's build, exit 3 at the CALL of a function whose contract has two reads atoms of a structure with a matrix member, fixed in slice 4: lowering il231; the encoder owner's probes U2_rrc_ok.c, U2_rrc_alias.c and U_rrc_ok.c, build/m4.5e1e2d/probeV/u2/): reads(*p) && reads(*q) of struct U2 { int m[2][2]; int c; } (matrix FIRST) and of struct U { int n; int m[2][2]; } (matrix second), the atoms returned in the ensures: a caller with two distinct locals gets the member back, and the call f(&a, &a) with ONE object read twice VERIFIES (two reads of one object are allowed, unlike the two owns atoms of matrix_first_member_pair.c); negative/matrix_reads_pair_wrong.c is the twin
// Pre-registered from the encoder owner's measured table (build/m4.5e1e2d/edg-backend-report-19.md, "READS-READS AT A CALL (il231)": verified, 23 s for two distinct locals and 14 s for f(&a, &a)), before any run (test-mutator-11, 2026-11-27): verifies (time it first: four callers in one file, the owner measured about 15 to 23 s for each)
#include <cerastes.h>

struct U2 {
  int m[2][2];
  int c;
};

struct U {
  int n;
  int m[2][2];
};

int rr2(const struct U2 *p, const struct U2 *q)
    CERASTES_REQUIRES(cerastes_reads(*p) && cerastes_reads(*q))
    CERASTES_ENSURES(r, cerastes_reads(*p) && cerastes_reads(*q) && r == p->c)
{
  (void)q;
  return p->c;
}

int use2_distinct(void)
    CERASTES_ENSURES(r, r == 5)
{
  struct U2 a = {{{1, 2}, {3, 4}}, 5};
  struct U2 b = {{{6, 7}, {8, 9}}, 5};
  return rr2(&a, &b);
}

int use2_alias(void)
    CERASTES_ENSURES(r, r == 5)
{
  struct U2 a = {{{1, 2}, {3, 4}}, 5};
  return rr2(&a, &a);
}

int rr1(const struct U *p, const struct U *q)
    CERASTES_REQUIRES(cerastes_reads(*p) && cerastes_reads(*q))
    CERASTES_ENSURES(r, cerastes_reads(*p) && cerastes_reads(*q) && r == p->n)
{
  (void)q;
  return p->n;
}

int use1_distinct(void)
    CERASTES_ENSURES(r, r == 5)
{
  struct U a = {5, {{1, 2}, {3, 4}}};
  struct U b = {5, {{6, 7}, {8, 9}}};
  return rr1(&a, &b);
}
