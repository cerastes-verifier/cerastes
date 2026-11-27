// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0064 item 15 (Part D, "Owners in loops", step 1; audit round D2, item 10): a loop that frees an owning global's block and takes another each iteration (the ghost block pointer is taken again at the back edge), then writes every cell and element of the new block: the must-verify beside negative/struct_array_loop_owner_global_reassign.c (which drops the free)
// Pre-registered from il-analyst's report 5 (v1c_reassign_fill.c verified in 32 to 42 s on the joint build), before any run of this file as a test: verifies in both modes
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
}
