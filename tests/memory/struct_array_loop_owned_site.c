// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0064 item 15 (Part D; audit round D2, item 10): a block site read through a load, owns_block(r->q) in requires and ensures, carried by a loop that writes nothing: the element written before the loop is still known after it
// Pre-registered from the auditor's probe audit-D2/y04b_derived_v1.c function h (its first check, a[1] == 0 after the loop, passes; the second, check(0), fails: left out here), before any run of this file as a test: verifies in both modes
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

int h(struct R *r)
    CERASTES_REQUIRES(cerastes_owns(*r) && r->q != NULL && cerastes_owns_block(r->q))
    CERASTES_ENSURES(cerastes_owns(*r) && r->q != NULL && cerastes_owns_block(r->q))
{
  r->q->a[1] = 0;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 4)
  CERASTES_DECREASES(4 - i)
  while (i < 4) {
    i = i + 1;
  }
  CERASTES_CHECK(r->q->a[1] == 0);
  return 0;
}
