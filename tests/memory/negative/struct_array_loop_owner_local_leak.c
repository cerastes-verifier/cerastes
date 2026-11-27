// expect: error leak @ L22
// mode: c23, c++20
// twin: tests/memory/struct_array_loop_owner_local.c
// Pins: 0064 item 15 (Part D, "Owners in loops"): the reallocation without the free: the owning local still holds the block it had when the loop stores the next one, so the store leaks it: one leak at the store
// Pre-registered from the implementers' probe d3probe/u2e_written_leak.c (leak at the store, il-analyst report 4), before any run of this file as a test
#include <cerastes.h>
#include <stddef.h>
#include <stdlib.h>

struct S {
  int a[4];
  int n;
};

void reallocated(void)
{
  CERASTES_OWNING struct S *q = NULL;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 3)
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    q = (struct S *)calloc(1, sizeof(struct S));  // <- here
    i += 1;
  }
  free(q);
}
