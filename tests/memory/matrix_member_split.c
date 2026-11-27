// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0064 items 4 and 12 (the third internal error of the family, exit 3 "the leaves of an array element that is not a structure were asked for", found by the audit of Commit 6 and fixed in Commit 7: the pointer split of a function that holds a structure with a matrix member beside a pointer to its cell type, lowering il246's `split_candidates`; the auditor's probes vk_k01, vk_k06, vk_k08 and vq_q05 and the lowering owner's cases e1e2e_split_local_pick_*; the kept Viper of local_u2, local_cells and local_u holds the case splits of the pick's result against the matrix's first cell, e.g. `t == loc(member(loc(member(v_u, 0), 0), 0), 0)`; this file was not run on a pre-fix build): a local `struct U2` (the matrix first) and a local `struct U` (the matrix between a scalar and a scalar array) beside `q = pick(&x)` with a write through `q`, the matrix cells and the scalar members read back; two pointers into the matrices of two locals chosen by `pick2` (the lowering owner's _local_pick_matrix_cell) and written; a function that owns `*p` and `*q` (`int *q`) and writes both, with a caller that passes a local structure and a separate local int and reads a matrix cell back; a function that reads `*p` (const) and owns `*q`; and a parameter `p` under owns(*p) beside a local pick; the twins are negative/matrix_member_split_wrong.c
// Pre-registered from record 0064 items 4 and 12 and the lowering owner's measured outcomes (build/m4.5e1e2d/il-analyst-report-10.md, the `e1e2e_split_*` cases: _local_pick_scalar and _local_pick_matrix_cell verify, 9 to 14 s each) and the auditor's `_ok` probes (build/m4.5e1e2d/audit-S4/p/), before the first run (test-mutator-12, 2026-11-27; the bodies changed after the first three probes, which found the `pick` contract and the unspecified members of the owns_both caller, the expect line is `verified` throughout), with `pick` returning its argument (`r == a`): the auditor's `pick`, which ties `r` to nothing, ends in a `lifetime` error at the end of the function (edg50, build/m4.5e1e2d/edg-backend-report-21.md: vk_k01, vk_k06, vk_k08), so those probes are not positives; verifies (time it first: ten verifications, about 10 s each at the lowering level)
#include <cerastes.h>

struct U2 {
  int m[2][2];
  int c;
};

struct U {
  int a;
  int m[2][2];
  int b[2];
  int c;
};

int *pick(int *a)
    CERASTES_REQUIRES(cerastes_owns(*a))
    CERASTES_ENSURES(r, r == a && cerastes_owns(*a));

int local_u2(void)
    CERASTES_ENSURES(r, r == 5)
{
  struct U2 u = {{{1, 2}, {3, 4}}, 5};
  int x = 1;
  int *q = pick(&x);
  *q = 2;
  CERASTES_CHECK(u.m[1][1] == 4 && x == 2);
  return u.c;
}

int *pick2(int *a, int *b, int c)
    CERASTES_REQUIRES(cerastes_owns(*a) && cerastes_owns(*b))
    CERASTES_ENSURES(r, cerastes_owns(*a) && cerastes_owns(*b) && r == (c != 0 ? a : b));

int local_cells(int c)
    CERASTES_ENSURES(r, r == 10)
{
  struct U2 u = {{{1, 2}, {3, 4}}, 5};
  struct U2 w = {{{6, 7}, {8, 9}}, 5};
  int *q = pick2(&u.m[1][1], &w.m[1][1], c);
  *q = 2;
  CERASTES_CHECK(u.m[1][1] == 2 || w.m[1][1] == 2);
  return u.c + w.c;
}

int local_u(void)
    CERASTES_ENSURES(r, r == 24)
{
  struct U u = {1, {{2, 3}, {4, 5}}, {6, 7}, 8};
  int x = 1;
  u.m[0][1] = 9;
  int *q = pick(&x);
  *q = 2;
  CERASTES_CHECK(u.m[1][1] == 5 && u.a == 1);
  return u.m[0][1] + u.b[1] + u.c;
}

int owns_both(struct U2 *p, int *q)
    CERASTES_REQUIRES(cerastes_owns(*p) && cerastes_owns(*q))
    CERASTES_ENSURES(r, cerastes_owns(*p) && cerastes_owns(*q) && r == 9 && *q == 3 && p->m[1][1] == 6 && p->c == cerastes_old(p->c))
{
  p->m[1][1] = 6;
  *q = 3;
  return p->m[1][1] + *q;
}

int use_owns_both(void)
    CERASTES_ENSURES(r, r == 6539)
{
  struct U2 a = {{{1, 2}, {3, 4}}, 5};
  int x = 1;
  int t = owns_both(&a, &x);
  return a.m[1][1] * 1000 + a.c * 100 + x * 10 + t;
}

int reads_owns(const struct U2 *p, int *q)
    CERASTES_REQUIRES(cerastes_reads(*p) && cerastes_owns(*q))
    CERASTES_ENSURES(r, cerastes_reads(*p) && cerastes_owns(*q) && r == p->c)
{
  *q = p->m[1][0];
  return p->c;
}

int use_reads_owns(void)
    CERASTES_ENSURES(r, r == 543)
{
  struct U2 a = {{{1, 2}, {3, 4}}, 5};
  int x = 1;
  int t = reads_owns(&a, &x);
  return t * 100 + a.m[1][1] * 10 + a.m[1][0];
}

int owns_then_pick(struct U2 *p)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(r, cerastes_owns(*p) && r == cerastes_old(p->c))
{
  int x = 1;
  int *q = pick(&x);
  *q = 2;
  return p->c;
}
