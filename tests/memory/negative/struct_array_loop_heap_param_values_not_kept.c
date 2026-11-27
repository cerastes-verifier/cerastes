// expect: error check @ L26
// mode: c23, c++20
// twin: tests/memory/struct_array_loop_heap_param_values_kept.c
// Pins: 0064 item 15 (Part D's loop column; audit round D1, finding 3, probe e10): the callee's loop stores `p->a[i] = 5`, so the values of the part are kept only below `write` and the part is held at `write`: `p->a[0] == 0` from the precondition no longer holds after the loop, and the check fails
// Pre-registered from the plan's Part D table (section 7: `loop_places` and `loop` with an array leaf's lines) and record 0064 item 15 (heap storage), read with record 0047 item 10 (a heap block is a place of a loop, with a back-edge row) and audit round D1's probe e10 (finding 3, seeds that behave correctly today), before any run of this file: check at the line of the check; the rule is record 0064 item 15's clause "A heap block's array part in a loop" (written 2026-11-26 with il-analyst's code), and these expectations are predictions from it, not measurements and from the loop's value conjunct (`kav > none`, and `kav < write` where a write can reach the field: lower.cpp `h.reach`, a store of the field's type anywhere in the loop)
#include <cerastes.h>
#include <stddef.h>
#include <stdlib.h>

struct S {
  int a[4];
  int n;
};

void overwrite(struct S *p)
    CERASTES_REQUIRES(cerastes_owns_block(p) && p->a[0] == 0)
    CERASTES_ENSURES(cerastes_owns_block(p))
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 4)
  CERASTES_DECREASES(4 - i)
  while (i < 4) {
    p->a[i] = 5;
    i = i + 1;
  }
  CERASTES_CHECK(p->a[0] == 0);  // <- here
}
