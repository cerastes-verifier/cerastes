// expect: error check @ L30
// mode: c23, c++20
// twin: tests/memory/struct_array_loop_heap_values_kept.c
// Pins: 0064 item 15 (Part D's loop column; audit round D1, finding 3, probe e1): the loop stores through `int *x = &p->a[0]`, a store of the field's type that can reach the part, so its values are kept only below `write` and the part is held at `write`: after the loop nothing says `p->a[0]` is still 0, and the check fails (a stronger program than the twin's, not a false alarm: the loop does write the element)
// Pre-registered from the plan's Part D table (section 7: `loop_places` and `loop` with an array leaf's lines) and record 0064 item 15 (heap storage), read with record 0047 item 10 (a heap block is a place of a loop, with a back-edge row) and audit round D1's probe e1 (finding 3, seeds that behave correctly today), before any run of this file: check at the line of the check; the rule is record 0064 item 15's clause "A heap block's array part in a loop" (written 2026-11-26 with il-analyst's code), and these expectations are predictions from it, not measurements and from the loop's value conjunct (`kav > none`, and `kav < write` where a write can reach the field: lower.cpp `h.reach`, a store of the field's type anywhere in the loop)
#include <cerastes.h>
#include <stddef.h>
#include <stdlib.h>

struct S {
  int a[4];
  int n;
};

int values_lost(void)
    CERASTES_ENSURES(r, r == 0 || r == -1)
{
  struct S *p = (struct S *)calloc(1, sizeof(struct S));
  if (p == NULL) {
    return -1;
  }
  int *x = &p->a[0];
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 4)
  CERASTES_DECREASES(4 - i)
  while (i < 4) {
    *x = 5;
    i = i + 1;
  }
  CERASTES_CHECK(p->a[0] == 0);  // <- here
  free(p);
  return 0;
}
