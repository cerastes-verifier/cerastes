// expect: error check @ L23
// mode: c23, c++20
// twin: tests/memory/struct_array_heap_calloc_two.c
// Pins: 0064 item 15 (question 354 (c)): the same structure with two array members: a check that an element of the SECOND array is 1 is false and is reported at the check (the zero clause of b's elements is as strong as a's)
// Pre-registered from record 0064 item 15 (heap storage: a view of a structure with SEVERAL array leaves holds each leaf's rows and one zero clause per element, 7 + 1 clauses here) and the view saved by edg-backend report 6 (c) (build/m4.5e1e2c/views/two_calloc.c, verified in both modes there), before any run: error check at the check (the precedent negative/struct_array_heap_calloc_wrong.c)
#include <cerastes.h>
#include <stddef.h>
#include <stdlib.h>

struct S {
  int a[4];
  int b[3];
  int n;
};

int two_arrays_wrong(void)
    CERASTES_ENSURES(r, r == 0 || r == -1)
{
  struct S *p = (struct S *)calloc(1, sizeof(struct S));
  if (p == NULL) {
    return -1;
  }
  CERASTES_CHECK(p->a[3] == 0 && p->b[2] == 1);  // <- here
  free(p);
  return 0;
}
