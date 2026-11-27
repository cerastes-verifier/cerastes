// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0064 item 12 (question 323, the member form: a new program, no auditor probe; the il-analyst's copy cases cover the lowering): t.v[1] = t.v[0] of a MEMBER array of structures whose elements have an array member copies the element, local and through p->v[1] = p->v[0] under owns(*p); a structure with such a member costs about 45 s per function (rule 3, timed)
// Pre-registered (test-mutator-10, Task 1/2, before any run): verifies (the right-value sibling of negative/array_rows_struct_elem_member_copy_wrong.c); time it first
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
    CERASTES_ENSURES(r, r == 8)
{
  struct T t;
  t.k = 0;
  t.v[0].n = 5;
  t.v[0].a[0] = 1;
  t.v[0].a[1] = 2;
  t.v[0].a[2] = 3;
  t.v[0].a[3] = 4;
  t.v[1] = t.v[0];
  return t.v[1].a[2] + t.v[1].n;
}

int through_pointer(struct T *p)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(r, cerastes_owns(*p) && r == p->v[0].a[2])
{
  p->v[1] = p->v[0];
  return p->v[1].a[2];
}
