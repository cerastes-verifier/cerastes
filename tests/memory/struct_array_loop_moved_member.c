// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0064 item 15 (Part D, step B; audit round D2 finding 6): a written owns(p->a) in a loop invariant with a MOVED pointer c (c = p at the back edge), so that the part's own conjuncts are in force (an unmoved parameter switches them off): the loop writes an element of the member array through c and keeps owning the array part
// Pre-registered from the auditor's probe audit-D2/w03_values.c function k (verifies but for its own value check, which this file leaves out), before any run of this file as a test: verifies in both modes
#include <cerastes.h>
#include <stddef.h>
#include <stdlib.h>

struct S {
  int a[4];
  int n;
};

void put(struct S *p)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p))
{
  struct S *c = p;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 4 && cerastes_owns(c->a))
  CERASTES_DECREASES(4 - i)
  while (i < 4) {
    c->a[1] = 5;
    c = p;
    i = i + 1;
  }
}
