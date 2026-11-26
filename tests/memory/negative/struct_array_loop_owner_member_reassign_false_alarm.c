// expect: error invalid-free @ L36
// expect: error invalid-free @ L36
// expect: error leak @ L39
// expect: error permission @ L39
// expect: error permission @ L40
// expect: error leak @ L41
// mode: c23, c++20
// twin: tests/memory/struct_array_loop_owner_member.c
// Pins: 0064 item 15 "Owners in loops" (a listed false alarm, fail closed): the correct reassignment of an owning member in a loop, free(r->q) and then r->q = calloc(...), whose pointee has an array member, draws the rows below (two invalid-free at the free, a leak and a permission at the loop body's closing brace, a permission at the return, a leak at the function's end); the same false alarm exists for a structure without an array member (edg-backend report 9, (a), on the base). The wrong form (the store without the free) was a pin of its own until the Part D close: it drew no row that this correct form does not draw, so it tested nothing and was dropped (test-mutator report 9, step 3: the rows of both forms). Not lifted by Part D (record 0064 item 15 "Owners in loops" lists it)
// Pre-registered from the probe of this program (test-mutator report 9, step 3), measured on the final Part D build (c23 52 s, c++20 56 s on the box): the rows as the run shows them, each listed
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

int refill_q(struct R *r)
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
    free(r->q);
    r->q = (struct S *)calloc(1, sizeof(struct S));
    i = i + 1;
  }
  return r->z;
}
