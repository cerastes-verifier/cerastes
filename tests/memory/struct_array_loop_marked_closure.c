// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0064 item 15 (Part D, "Owners in loops"; audit round D2, item 10): a marked closure (an owning member that is ref_to_uninit) carried by a loop that writes nothing: every element written before the loop is known after it
// Pre-registered from the auditor's probe audit-D2/o03_marked.c function g (the empty loop verifies and its read is kept; the probe's own check(0) is replaced here by the kept value), before any run of this file as a test: verifies in both modes (the check v == 1 is the prediction that the read is kept)
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

int kept(struct RM *b)
    CERASTES_REQUIRES(cerastes_owns(*b) && b->q != NULL)
    CERASTES_ENSURES(cerastes_owns(*b))
{
  b->q->a[0] = 1;
  b->q->a[1] = 1;
  b->q->a[2] = 1;
  b->q->a[3] = 1;
  b->q->n = 1;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 4)
  CERASTES_DECREASES(4 - i)
  while (i < 4) {
    i = i + 1;
  }
  int v = b->q->a[0];
  CERASTES_CHECK(v == 1);
  return v;
}
