// expect: error unsequenced @ L34
// expect: error unsequenced @ L42
// expect: error unsequenced @ L50
// mode: c23, c++20
// twin: tests/memory/struct_array_chain_seq.c
// Pins: 0064 item 11 and handoff section 3: one function per row, each unsequenced at its expression, once. beside_read: wr(&s) owns *s.q and s.q->a[1] is read in the same expression; two_calls: wr(&s) twice; alias_leaf: the same element by the name of the local the pointer holds, t.a[1] (unmeasured: the row compares the call's loaded place with the object `t`, a pairing through a load)
// Pre-registered from record 0064 item 11, before any run: one unsequenced per function at the line of its expression; alias_leaf is the row most likely to need a correction
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

int wr(struct R *p)
    CERASTES_REQUIRES(cerastes_owns(*p) && cerastes_owns(*p->q))
    CERASTES_ENSURES(r, cerastes_owns(*p) && p->q == cerastes_old(p->q) && cerastes_owns(*p->q) && p->q->a[1] == 7 && r == 0)
{
  p->q->a[1] = 7;
  return 0;
}

int beside_read(void)
{
  struct S t = {{1, 2, 3, 4}, 5};
  struct R s = {&t, 1};
  int k = wr(&s) != s.q->a[1];  // <- here
  return k;
}

int two_calls(void)
{
  struct S t = {{1, 2, 3, 4}, 5};
  struct R s = {&t, 1};
  int k = wr(&s) + wr(&s);  // <- here
  return k;
}

int alias_leaf(void)
{
  struct S t = {{1, 2, 3, 4}, 5};
  struct R s = {&t, 1};
  int k = wr(&s) != t.a[1];  // <- here
  return k;
}
