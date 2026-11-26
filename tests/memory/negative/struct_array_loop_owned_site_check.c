// expect: error check @ L32
// mode: c23, c++20
// twin: tests/memory/struct_array_loop_owned_site.c
// Pins: 0064 item 15 (Part D): the twin of struct_array_loop_owned_site.c: the same function with a check(0) after the loop, which must fail at its own line (the carried site does not hide it)
// Pre-registered from the auditor's probe audit-D2/y04b_derived_v1.c function h (check(0) fails at its line and nothing else is reported), before any run of this file as a test
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
  CERASTES_CHECK(0);  // <- here
  return 0;
}
