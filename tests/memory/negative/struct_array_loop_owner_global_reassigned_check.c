// expect: error check @ L37
// mode: c23, c++20
// twin: tests/memory/struct_array_loop_owner_global_reassigned.c
// Pins: 0064 item 15 (Part D, step 1): the twin of the reassigned owning global: the same function with a check after the writes that the written values contradict (a[0] was written 0): one check error
// Pre-registered from the same probe, before any run of this file as a test: check at its line (the values written after the loop are known)
#include <cerastes.h>
#include <stddef.h>
#include <stdlib.h>

struct S {
  int a[4];
  int n;
};

CERASTES_OWNING struct S *g = NULL;

void refill(void)
    CERASTES_REQUIRES(cerastes_owns(g))
    CERASTES_ENSURES(cerastes_owns(g))
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 3)
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    free(g);
    g = (struct S *)calloc(1, sizeof(struct S));
    i += 1;
  }
  if (g == NULL) {
    return;
  }
  g->n = 0;
  g->a[0] = 0;
  g->a[1] = 0;
  g->a[2] = 0;
  g->a[3] = 0;
  CERASTES_CHECK(g->a[0] == 1);  // <- here
}
