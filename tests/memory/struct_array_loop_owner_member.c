// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0064 item 15 (Part D's loop column; owning members, question 293) and 0055 items 3 and 7 (loops over the closure of an owning field): a loop in a function with owns(*r) in both lists writes the array of the block the owning member r->q holds
// Pre-registered from the plan's Part D table (section 7: `owner_loop_sites`, `owner_invariants`, `owner_rows` with an array leaf's lines) and record 0064 item 15 (owning members, question 293), read with records 0054 item 5 and 0055 item 7 (loops over owners), and the precedent owning_field_loop_local.c, before any run of this file: verifies in both modes; the rule is record 0064 item 15's clause "A heap block's array part in a loop" (written 2026-11-26 with il-analyst's code), and these expectations are predictions from it, not measurements
#include <cerastes.h>
#include <stddef.h>
#include <stdlib.h>

struct S {
  int a[4];
  int n;
};

struct R {
  CERASTES_OWNING struct S *q;
  int z;
};

int fill_q(struct R *r)
    CERASTES_REQUIRES(cerastes_owns(*r))
    CERASTES_ENSURES(cerastes_owns(*r))
{
  if (r->q == NULL) {
    return 0;
  }
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 4)
  CERASTES_DECREASES(4 - i)
  while (i < 4) {
    r->q->a[i] = i;
    i = i + 1;
  }
  return r->z;
}
