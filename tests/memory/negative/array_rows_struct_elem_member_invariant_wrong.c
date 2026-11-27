// expect: error invariant-preserved @ L26
// expect: error invariant-entry @ L40
// expect: error permission @ L53
// expect: error out-of-bounds @ L66
// mode: c23, c++20
// twin: tests/memory/array_rows_struct_elem_member_invariant.c
// Pins: 0064 item 10 (audit E3b finding 3; the auditor's q1, q4, q7 and q6, measured on j4, both modes): a loop invariant that reads p->v[a].a[b] through a pointer is checked like any other: a body that writes that cell does not keep it (`invariant-preserved`), a value that does not hold at entry is `invariant-entry`, a FIRST index that may be outside p->v (a <= 2) is `permission` ("this loop invariant reads `a[b]`, which this loop does not hold": a true alarm in another category's words, not a false alarm) and a second index outside p->v[a].a (b <= 3) is `out-of-bounds`
// Pre-registered (test-mutator-11, Task 1, before any run): one row per function at its invariant line, taken from the auditor's measured outcome of exactly each function (build/m4.5e1e2d/audit-E3b/out-tq.txt: q1 and q4 and q7 and q6, both modes, one row each); controls: keeps_cell, entry_holds and in_range in tests/memory/array_rows_struct_elem_member_invariant.c (the programs with the body's write removed, the entry value in the requires and the indexes bounded)
#include <cerastes.h>

struct P {
  int n;
  int a[3];
};

struct T {
  struct P v[2];
  int k;
};

int body_writes_the_cell(struct T *p, int n, int a, int b)
    CERASTES_REQUIRES(cerastes_owns(*p) && n >= 0 && 0 <= a && a < 2 && 0 <= b && b < 3 && p->v[a].a[b] == 1)
    CERASTES_ENSURES(r, cerastes_owns(*p))
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && p->v[a].a[b] == 1)  // <- here
  CERASTES_DECREASES(n - i)
  while (i < n) {
    p->v[a].a[b] = 2;
    i++;
  }
  return 0;
}

int wrong_entry_value(struct T *p, int n, int a, int b)
    CERASTES_REQUIRES(cerastes_owns(*p) && n >= 0 && 0 <= a && a < 2 && 0 <= b && b < 3)
    CERASTES_ENSURES(r, cerastes_owns(*p))
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && p->v[a].a[b] == 1)  // <- here
  CERASTES_DECREASES(n - i)
  while (i < n) {
    i++;
  }
  return 0;
}

int first_index_outside(struct T *p, int n, int a, int b)
    CERASTES_REQUIRES(cerastes_owns(*p) && n >= 0 && 0 <= a && a <= 2 && 0 <= b && b < 3)
    CERASTES_ENSURES(r, cerastes_owns(*p))
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && p->v[a].a[b] <= 2147483647)  // <- here
  CERASTES_DECREASES(n - i)
  while (i < n) {
    i++;
  }
  return 0;
}

int second_index_outside(struct T *p, int n, int a, int b)
    CERASTES_REQUIRES(cerastes_owns(*p) && n >= 0 && 0 <= a && a < 2 && 0 <= b && b <= 3)
    CERASTES_ENSURES(r, cerastes_owns(*p))
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && p->v[a].a[b] <= 2147483647)  // <- here
  CERASTES_DECREASES(n - i)
  while (i < n) {
    i++;
  }
  return 0;
}
