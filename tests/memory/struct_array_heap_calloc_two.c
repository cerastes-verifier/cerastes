// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0064 item 15 (question 354 (c); the view that no test held): calloc of a structure with TWO array members, `int a[4]; int b[3]; int n;`: the last element of each array and n are zero, freed
// Pre-registered from record 0064 item 15 (heap storage: a view of a structure with SEVERAL array leaves holds each leaf's rows and one zero clause per element, 7 + 1 clauses here) and the view saved by edg-backend report 6 (c) (build/m4.5e1e2c/views/two_calloc.c, verified in both modes there), before any run of this file: verifies in both modes (unmeasured here)
#include <cerastes.h>
#include <stddef.h>
#include <stdlib.h>

struct S {
  int a[4];
  int b[3];
  int n;
};

int two_arrays(void)
    CERASTES_ENSURES(r, r == 0 || r == -1)
{
  struct S *p = (struct S *)calloc(1, sizeof(struct S));
  if (p == NULL) {
    return -1;
  }
  CERASTES_CHECK(p->a[3] == 0 && p->b[2] == 0 && p->n == 0);
  free(p);
  return 0;
}
