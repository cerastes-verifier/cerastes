// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0064 items 3, 5, 7 and 13 (audit E3a finding 8 (h); new programs, no auditor probe; the il-analyst's tsarr and gtsarr cases cover the lowering, edg-backend report 17's drafts the encoder): the member form p->v[i].a[j] under owns(*p) at symbolic indexes (read back as the written cell in the postcondition), a contract that states the guard in the clause that divides (question 345's workaround, which verifies), and a global structure with such a member: its program_start values listed, one cell written at symbolic indexes with the neighbours unchanged
// Pre-registered (test-mutator-10, Task 1/2, before any run): verifies (the right-value sibling of negative/array_rows_struct_elem_member_atom_wrong.c: put's r == 7 and main's gt.v[1].a[3] == 9, and the control of the pin negative/array_rows_struct_elem_member_atom_wf_tie_false_alarm.c: guarded); time it first
#include <cerastes.h>

struct W {
  int a[4];
  int n;
};

struct T {
  struct W v[2];
  int k;
};

static struct T gt = {{{{1, 2, 3, 4}, 5}, {{6, 7, 8, 9}, 10}}, 11};

int put(struct T *p, int i, int j)
    CERASTES_REQUIRES(cerastes_owns(*p) && 0 <= i && i < 2 && 0 <= j && j < 4)
    CERASTES_ENSURES(r, cerastes_owns(*p) && r == 7 && p->v[i].a[j] == 7)
{
  p->v[i].a[j] = 7;
  return p->v[i].a[j];
}

int guarded(struct T *p)
    CERASTES_REQUIRES(cerastes_reads(*p) && p->v[0].a[1] != 0)
    CERASTES_ENSURES(r, cerastes_reads(*p) && (r == 0 || (p->v[0].a[1] != 0 && r == 10 / p->v[0].a[1])))
{
  return p->v[0].a[1] * 0;
}

int main(void)
{
  CERASTES_CHECK(gt.v[1].a[3] == 9 && gt.k == 11 && gt.v[0].n == 5);
  int i = 1;
  int j = 2;
  gt.v[i].a[j] = 70;
  CERASTES_CHECK(gt.v[i].a[j] == 70 && gt.v[1].a[1] == 7 && gt.v[0].a[2] == 3);
  return gt.v[0].n - 5;
}
