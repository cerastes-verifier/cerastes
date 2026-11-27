// expect: error permission @ L26
// mode: c23, c++20
// twin: tests/memory/struct_array_owner_partnerless.c
// Pins: 0064 item 15 (Part D's loop column; a listed false alarm, fail closed; il-analyst's report 3, route r7): a loop over the array part of a structure that a callee gave by a partnerless `ensures` atom `owns(*r)` has no place for it (the same for a structure of scalars: a loop over a given result in a function that is no heap function has none), so the store is a false `permission`; the straight-line stores through a guarded result verify (the twin)
// Pre-registered from il-analyst's report 3 (route r7: a false `permission: writes p->a[i] without owning it`), before any run of this file as a test: record 0064 item 15's clause "A heap block's array part in a loop" lists it as a false alarm; the expectation is a prediction from il-analyst's report, not a measurement
#include <cerastes.h>
#include <stddef.h>
#include <stdlib.h>

struct S {
  int a[4];
  int n;
};

struct S *give(void)
    CERASTES_ENSURES(r, r != NULL && cerastes_owns(*r));

int f(void)
    CERASTES_ENSURES(r, r == 0)
{
  struct S *p = give();
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 4)
  CERASTES_DECREASES(4 - i)
  while (i < 4) {
    p->a[i] = i;  // <- here
    i = i + 1;
  }
  return 0;
}
