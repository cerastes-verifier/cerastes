// expect: error lifetime @ L35
// expect: error null-dereference @ L42
// expect: error permission @ L42
// expect: error check @ L53
// expect: error permission @ L61
// expect: error permission @ L78
// expect: error postcondition @ L96
// mode: c23, c++20
// twin: tests/memory/matrix_member_split.c
// Pins: 0064 items 4 and 12 (the third internal error of the family, fixed in Commit 7: on Commit 6's build the auditor's shapes of `cmp_unatomed` (q04, vr_min_q04.c) and of a local structure with a matrix beside `q = pick(&x)` (vk_k01, vk_k06, vk_k08) ended in exit 3 at the pointer split, audit-S4/out-vk.txt and out-vr.txt; this file itself was NOT run on a pre-fix build), one row per function: (1) a `reads(*p)` function of a structure with a matrix member compares a pointer `q` that no atom covers, which is a `lifetime` error at the use of `q` (as for a structure without a matrix member; the lowering owner's case e1e2e_split_reads_param_unatomed_q_compared_fail); (2) a store through an unatomed `int *q` beside `owns(*p)` is a `null-dereference` and a `permission` error at the store (e1e2e_split_owns_param_unatomed_q_write_fail); (3) a local `struct U` (the matrix between a scalar and a scalar array) beside `q = pick(&x)`, a write through `q` and a `check` that states a wrong value for a matrix cell; (4) a callee under `reads(*p)` that writes a matrix cell: `permission` at the store; (5) a caller that passes the address of a matrix cell of `*p` as the `int *q` beside `owns(*p) && owns(*q)`: the two atoms overlap, `permission` at the call; (6) a caller that reads a wrong value back after the call: `postcondition` at its return; the right-value siblings are in tests/memory/matrix_member_split.c, and each wrong part made right (the comparison removed, owns(*q) written, the check right, no write, a separate local passed, the right value) verifies (probed apart, build/m4.5e1e2d/tm12/right/)
// Pre-registered from record 0064 items 4 and 12, the lowering owner's measured outcomes (build/m4.5e1e2d/il-analyst-report-10.md, the `e1e2e_split_*` cases) and the briefing of this task, before any run (test-mutator-12, 2026-11-27), with `pick` returning its argument (`r == a`: the auditor's `pick` fails `lifetime` at the end of the function, report 21); the first probe of row 6 (a caller that read members the callee's contract leaves unspecified) gave two `overflow` rows besides the postcondition, so the contract now states what the CALLER reads (`p->c == cerastes_old(p->c)`, `*q == 3`, `p->m[1][1] == 6`, the result), the expect lines unchanged (a new test's own construction, question 160); row 5 is `permission` first (the overlap of two owns atoms at a call), its measured category is reported under question 160 before any change
#include <cerastes.h>
#include <stddef.h>

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

int cmp_unatomed(const struct U2 *p, int *q)
    CERASTES_REQUIRES(cerastes_reads(*p))
    CERASTES_ENSURES(r, cerastes_reads(*p))
{
  return q == NULL ? p->c : 0;  // <- here
}

int write_unatomed(struct U2 *p, int *q)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(r, cerastes_owns(*p))
{
  *q = 1;  // <- here
  return p->c;
}

int local_pick_wrong(void)
    CERASTES_ENSURES(r, r == 8)
{
  struct U u = {1, {{2, 3}, {4, 5}}, {6, 7}, 8};
  int x = 1;
  int *q = pick(&x);
  *q = 2;
  CERASTES_CHECK(u.m[1][1] == 6);  // <- here
  return u.c;
}

int rd_write(struct U2 *p)
    CERASTES_REQUIRES(cerastes_reads(*p))
    CERASTES_ENSURES(r, cerastes_reads(*p))
{
  p->m[1][1] = 7;  // <- here
  return p->c;
}

int both(struct U2 *p, int *q)
    CERASTES_REQUIRES(cerastes_owns(*p) && cerastes_owns(*q))
    CERASTES_ENSURES(r, cerastes_owns(*p) && cerastes_owns(*q) && r == 7)
{
  (void)p;
  *q = 3;
  return 7;
}

int alias_call(void)
    CERASTES_ENSURES(r, r == 7)
{
  struct U2 u = {{{1, 2}, {3, 4}}, 5};
  return both(&u, &u.m[1][1]);  // <- here
}

int owns_both(struct U2 *p, int *q)
    CERASTES_REQUIRES(cerastes_owns(*p) && cerastes_owns(*q))
    CERASTES_ENSURES(r, cerastes_owns(*p) && cerastes_owns(*q) && r == 9 && *q == 3 && p->m[1][1] == 6 && p->c == cerastes_old(p->c))
{
  p->m[1][1] = 6;
  *q = 3;
  return p->m[1][1] + *q;
}

int after_wrong(void)
    CERASTES_ENSURES(r, r == 6540)
{
  struct U2 a = {{{1, 2}, {3, 4}}, 5};
  int x = 1;
  int t = owns_both(&a, &x);
  return a.m[1][1] * 1000 + a.c * 100 + x * 10 + t;  // <- here
}
