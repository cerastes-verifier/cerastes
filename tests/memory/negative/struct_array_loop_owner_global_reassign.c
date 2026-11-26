// expect: error leak @ L28
// mode: c23, c++20
// twin: tests/memory/struct_array_loop_owner_global.c
// Pins: 0064 item 15 (Part D's loop column) and 0054 item 5 ("Stores"): the loop replaces the block the owning global holds without freeing it: leak at the store (the precedent negative/owning_global_loop_cycle_no_free.c, a loop that reallocates the owning global)
// Pre-registered from the plan's Part D table (section 7: `owner_loop_sites`, `owner_invariants`, `owner_rows` with an array leaf's lines) and record 0064 item 15 (owning members, question 293), read with records 0054 item 5 and 0055 item 7 (loops over owners), and the precedent owning_global_loop_cycle_no_free.c (leak at the store in the loop), before any run; the rule is record 0064 item 15's clause "A heap block's array part in a loop" (written 2026-11-26 with il-analyst's code), and these expectations are predictions from it, not measurements
#include <cerastes.h>
#include <stddef.h>
#include <stdlib.h>

struct S {
  int a[4];
  int n;
};

CERASTES_OWNING static struct S *g = NULL;

void refill(void)
    CERASTES_REQUIRES(cerastes_owns(g))
    CERASTES_ENSURES(cerastes_owns(g))
{
  if (g == NULL) {
    return;
  }
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 4)
  CERASTES_DECREASES(4 - i)
  while (i < 4) {
    g = (struct S *)calloc(1, sizeof(struct S));  // <- here
    i = i + 1;
  }
}

int main(void)
{
  refill();
  return 0;
}
