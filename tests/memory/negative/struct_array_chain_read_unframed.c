// expect: error permission @ L21
// mode: c23, c++20
// twin: tests/memory/struct_array_chain_read.c
// Pins: 0064 item 15 (306) and 0037 item 7 (the pin struct_array_reject_annotations.c's through_load_read as it stands): a contract that reads p->q->a[i] with an atom on *p and none on *p->q: permission, at the clause
// Pre-registered from record 0064 item 15, 0037 item 7 and the precedent negative/heap_guard_inner_unframed.c (a contract clause that reads a place no earlier atom frames: permission at the clause, one row), before any run: unmeasured for an array leaf behind a load
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

int get(struct R *p, int i)
    CERASTES_REQUIRES(cerastes_owns(*p) && 0 <= i && i < 4 && p->q->a[i] > 0)  // <- here
    CERASTES_ENSURES(r, cerastes_owns(*p) && r == 0)
{
  (void)i;
  return 0;
}
