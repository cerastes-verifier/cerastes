// expect: error postcondition @ L30
// expect: error postcondition @ L38
// mode: c23, c++20
// twin: tests/memory/array_rows_struct_elem_member_copy.c
// Pins: 0064 item 12 (the member form; a new program): after t.v[1] = t.v[0] the copy's cell a[2] is 3 and its scalar 5, so a result of 7 fails at the return, and through the pointer the copied cell is the source's a[2], not its a[1]
// Pre-registered (test-mutator-10, Task 1/2, before any run): two `postcondition` rows at the returns; right-value siblings: member_copy (r == 8) and through_pointer (r == p->v[0].a[2]) in tests/memory/array_rows_struct_elem_member_copy.c
#include <cerastes.h>

struct W {
  int a[4];
  int n;
};

struct T {
  struct W v[2];
  int k;
};

int member_copy(void)
    CERASTES_ENSURES(r, r == 7)
{
  struct T t;
  t.k = 0;
  t.v[0].n = 5;
  t.v[0].a[0] = 1;
  t.v[0].a[1] = 2;
  t.v[0].a[2] = 3;
  t.v[0].a[3] = 4;
  t.v[1] = t.v[0];
  return t.v[1].a[2] + t.v[1].n;  // <- here
}

int through_pointer(struct T *p)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(r, cerastes_owns(*p) && r == p->v[0].a[1])
{
  p->v[1] = p->v[0];
  return p->v[1].a[2];  // <- here
}
