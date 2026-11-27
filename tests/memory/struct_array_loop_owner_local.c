// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0064 item 15 (Part D, "Owners in loops"; audit round D2, item 10): an owning automatic local of a structure with an array member in a loop: one that the loop does not name (carried automatically while touch() works on another object), and one freed and reallocated in each iteration
// Pre-registered from the implementers' probes build d3probe/u1c_unmentioned_free.c (12 s) and u2c_written_free.c (12 s), both verified on the joint build, before any run of this file as a test: verifies in both modes
#include <cerastes.h>
#include <stddef.h>
#include <stdlib.h>

struct S {
  int a[4];
  int n;
};

void touch(int *x)
    CERASTES_REQUIRES(cerastes_owns(*x))
    CERASTES_ENSURES(cerastes_owns(*x))
{
  *x = 1;
}

void unmentioned(int *y)
    CERASTES_REQUIRES(cerastes_owns(*y))
    CERASTES_ENSURES(cerastes_owns(*y))
{
  CERASTES_OWNING struct S *q = (struct S *)calloc(1, sizeof(struct S));
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 3)
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    touch(y);
    i += 1;
  }
  free(q);
}

void reallocated(void)
{
  CERASTES_OWNING struct S *q = NULL;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 3)
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    free(q);
    q = (struct S *)calloc(1, sizeof(struct S));
    if (q != NULL) {
      q->a[1] = i;
    }
    i += 1;
  }
  free(q);
}
