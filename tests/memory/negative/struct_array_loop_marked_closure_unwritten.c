// expect: error uninitialized @ L32
// mode: c23, c++20
// twin: tests/memory/struct_array_loop_marked_closure.c
// Pins: 0064 item 15 (Part D, "Owners in loops"): a marked closure's element that was never written: the read after the loop is uninitialized, at the read
// Pre-registered from the auditor's probe audit-D2/o03_marked.c function h (uninitialized at the read of b->q->a[2]), before any run of this file as a test
#include <cerastes.h>
#include <stddef.h>
#include <stdlib.h>

struct S {
  int a[4];
  int n;
};

struct RM {
  struct S *q CERASTES_OWNING CERASTES_REF_TO_UNINIT;
  int z;
};

int unwritten(struct RM *b)
    CERASTES_REQUIRES(cerastes_owns(*b) && b->q != NULL)
    CERASTES_ENSURES(cerastes_owns(*b))
{
  b->q->a[0] = 1;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 4)
  CERASTES_DECREASES(4 - i)
  while (i < 4) {
    b->q->a[1] = 2;
    i = i + 1;
  }
  int v = b->q->a[2];  // <- here
  return v;
}
