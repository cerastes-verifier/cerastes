// expect: error permission @ L33
// mode: c23, c++20
// twin: tests/memory/struct_array_loop_owner_member.c
// Pins: 0064 item 15 (Part D's loop column; owning members, question 293, "Owners in loops") and 0055 items 5 and 7: the loop frees the block the owning member r->q holds and does not replace it, so an iteration ends without the permission to r->q->a that it began with (the loop carries the array part of the block r->q holds): one permission error at the loop body's closing brace, naming r->q->a
// Pre-registered from the implementer's probe t2_r5_free (the same loop shape without the early return, measured on the final Part D build, test-mutator report 9 step 3: one `permission: an iteration of this loop ends without the permission to r->q->a that it began with` at the loop's closing brace and nothing else), before any run of this file: permission at the closing brace of the loop
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
    free(r->q);
    i = i + 1;
  }  // <- here
  return r->z;
}
