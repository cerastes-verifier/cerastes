// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0064 item 15 (306; amends 0045 item 1; the pin struct_array_reject_annotations.c's through_load_atom is keep_both): owns(*p->q) of a structure with an array member behind a loaded pointer: a callee writes p->q->a[1] and states it, another keeps both atoms, and the caller's local array element has the value afterwards
// Pre-registered from record 0064 item 15 and the precedent struct_chain_whole.c, before any run: verifies (unmeasured: a load-rooted array leaf)
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

void zero_one(struct R *p)
    CERASTES_REQUIRES(cerastes_owns(*p) && cerastes_owns(*p->q))
    CERASTES_ENSURES(cerastes_owns(*p) && p->q == cerastes_old(p->q) && cerastes_owns(*p->q) && p->q->a[1] == 0)
{
  p->q->a[1] = 0;
}

int keep_both(struct R *p)
    CERASTES_REQUIRES(cerastes_owns(*p) && cerastes_owns(*p->q))
    CERASTES_ENSURES(r, cerastes_owns(*p) && p->q == cerastes_old(p->q) && cerastes_owns(*p->q) && r == 0)
{
  (void)p;
  return 0;
}

int use(void)
    CERASTES_ENSURES(r, r == 0)
{
  struct S t = {{1, 2, 3, 4}, 5};
  struct R s = {&t, 1};
  zero_one(&s);
  CERASTES_CHECK(t.a[1] == 0);
  int k = keep_both(&s);
  return k;
}
