// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0064 item 10 (audit E3b finding 3; the auditor's q3, q5, q4's and q6/q7's controls): through a pointer to a structure whose member is an array of structures with an array member, a loop invariant may read p->v[a].a[b] with SYMBOLIC indexes that the requires bounds (the body writes other cells: p->k and p->v[a].n), and with the loop variable in the index (p->v[i % 2].a[0]); the same invariant over an entry value that holds and over indexes in range verifies
// Pre-registered (test-mutator-11, Task 1, before any run): verifies, both modes (keeps_cell is the auditor's q3_ok and loop_variable_index its q5 as measured on j4, both modes; entry_holds and in_range are the right-value controls of q4, q6 and q7 of negative/array_rows_struct_elem_member_invariant_wrong.c: the same programs with the entry value made right and the first and second index bounded by the requires); time it first
#include <cerastes.h>

struct P {
  int n;
  int a[3];
};

struct T {
  struct P v[2];
  int k;
};

int keeps_cell(struct T *p, int n, int a, int b)
    CERASTES_REQUIRES(cerastes_owns(*p) && n >= 0 && 0 <= a && a < 2 && 0 <= b && b < 3 && p->v[a].a[b] == 1)
    CERASTES_ENSURES(r, cerastes_owns(*p) && r == 1)
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && p->v[a].a[b] == 1)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    p->k = 5;
    p->v[a].n = 6;
    i++;
  }
  return p->v[a].a[b];
}

int loop_variable_index(struct T *p, int n)
    CERASTES_REQUIRES(cerastes_owns(*p) && n >= 0)
    CERASTES_ENSURES(r, cerastes_owns(*p))
{
  (void)p;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && p->v[i % 2].a[0] == p->v[i % 2].a[0])
  CERASTES_DECREASES(n - i)
  while (i < n) {
    i++;
  }
  return 0;
}

int entry_holds(struct T *p, int n, int a, int b)
    CERASTES_REQUIRES(cerastes_owns(*p) && n >= 0 && 0 <= a && a < 2 && 0 <= b && b < 3 && p->v[a].a[b] == 1)
    CERASTES_ENSURES(r, cerastes_owns(*p))
{
  (void)p;
  (void)a;
  (void)b;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && p->v[a].a[b] == 1)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    i++;
  }
  return 0;
}

int in_range(struct T *p, int n, int a, int b)
    CERASTES_REQUIRES(cerastes_owns(*p) && n >= 0 && 0 <= a && a < 2 && 0 <= b && b < 3)
    CERASTES_ENSURES(r, cerastes_owns(*p))
{
  (void)p;
  (void)a;
  (void)b;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && p->v[a].a[b] <= 2147483647)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    i++;
  }
  return 0;
}
