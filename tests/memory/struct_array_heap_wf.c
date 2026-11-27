// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0064 item 7 and handoff section 3 (f__wf's ghosts: one per leaf path and slot) for a heap structure with an array member: the control of negative/struct_array_heap_wf_division.c. An owns_block contract that states p->a[1] != 0 in the clause that divides by it is well formed
// Pre-registered from record 0064 items 7 and 15 and the advisor's note (the guard stands in the clause that divides: no tie between the states of an element, question 345), before any run: verifies (unmeasured)
#include <cerastes.h>
#include <stddef.h>
#include <stdlib.h>

struct S {
  int a[4];
  int n;
};

int ok(struct S *p)
    CERASTES_REQUIRES(cerastes_owns_block(p) && p->a[1] != 0)
    CERASTES_ENSURES(r, cerastes_owns_block(p) && p->a[1] != 0 && (r == 0 || r == 10 / p->a[1]))
{
  (void)p;
  return 0;
}
