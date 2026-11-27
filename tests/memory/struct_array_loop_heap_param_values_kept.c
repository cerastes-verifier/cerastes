// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0064 item 15 (Part D's loop column): a callee that holds the block of its parameter, a structure with an array member, and stores nothing in its loop: `p->a[0] == 0` from the precondition (read under owns_block) holds after the loop, so the function returns 0 (the value conjunct of the part carries `kav > none` alone where no store of the field's type reaches it)
// Pre-registered from the plan's Part D table (section 7: `loop_places` and `loop` with an array leaf's lines) and record 0064 item 15 (heap storage), read with record 0047 item 10 (a heap block is a place of a loop, with a back-edge row) and audit round D1's probe e10 (finding 3, seeds that behave correctly today), before any run of this file: verifies in both modes; the rule is record 0064 item 15's clause "A heap block's array part in a loop" (written 2026-11-26 with il-analyst's code), and these expectations are predictions from it, not measurements and from the loop's value conjunct (`kav > none`, and `kav < write` where a write can reach the field: lower.cpp `h.reach`, a store of the field's type anywhere in the loop)
#include <cerastes.h>
#include <stddef.h>
#include <stdlib.h>

struct S {
  int a[4];
  int n;
};

int keep(struct S *p)
    CERASTES_REQUIRES(cerastes_owns_block(p) && p->a[0] == 0)
    CERASTES_ENSURES(r, cerastes_owns_block(p) && r == 0)
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 4)
  CERASTES_DECREASES(4 - i)
  while (i < 4) {
    i = i + 1;
  }
  return p->a[0];
}
