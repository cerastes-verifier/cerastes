// expect: error postcondition @ L26
// expect: error check @ L31
// mode: c23, c++20
// twin: tests/memory/array_rows_struct_elem_member_atom.c
// Pins: 0064 items 5 and 9 (audit E3a finding 8 (h); new programs): the cell written through p->v[i].a[j] under owns(*p) is 7, so a result of 8 fails at the return, and the listed value of gt.v[1].a[3] is 9, so a check of 8 fails in main
// Pre-registered (test-mutator-10, Task 1/2, before any run): one `postcondition` and one `check`; right-value siblings: put's r == 7 and main's gt.v[1].a[3] == 9 in tests/memory/array_rows_struct_elem_member_atom.c
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
    CERASTES_ENSURES(r, cerastes_owns(*p) && r == 8 && p->v[i].a[j] == 7)
{
  p->v[i].a[j] = 7;
  return p->v[i].a[j];  // <- here
}

int main(void)
{
  CERASTES_CHECK(gt.v[1].a[3] == 8);  // <- here
  return gt.k - 11;
}
