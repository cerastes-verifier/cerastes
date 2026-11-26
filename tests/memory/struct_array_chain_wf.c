// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0064 item 7 and handoff section 3 (f__wf's ghosts, one per leaf path, slot and loaded pointer) for a place reached through ONE heap read: the control of negative/struct_array_chain_wf_division.c. A contract that states p->q->a[1] != 0 in the clause that divides by it is well formed. The two-pointer form (a second loaded pointer, the other_root row) is held for the owner's decision on a lint rule (question 357)
// Pre-registered from record 0064 items 7 and 15 and the advisor's note (the guard stands in the clause that divides), before any run: verifies (unmeasured)
#include <cerastes.h>
#include <stddef.h>
#include <stdlib.h>

struct S {
  int a[4];
  int n;
};

struct R {
  struct S *q;
  int z;
};

int ok(struct R *p)
    CERASTES_REQUIRES(cerastes_reads(*p) && cerastes_reads(*p->q) && p->q->a[1] != 0)
    CERASTES_ENSURES(r, cerastes_reads(*p) && cerastes_reads(*p->q) && p->q->a[1] != 0 && r == 10 / p->q->a[1])
{
  return 10 / p->q->a[1];
}
