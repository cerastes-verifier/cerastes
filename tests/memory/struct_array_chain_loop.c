// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0064 item 15 (amends 0045 item 12, 0040 item 5; the pin struct_array_reject_annotations.c's loop_atom): owns(*p) written in a loop invariant on a structure with an array member, with an element read beside it: the loop zero-fills the array and the first element stays 0 (an array leaf's conjuncts are a range place's, its amount the first slot's)
// Pre-registered from record 0064 item 15 (S48 (k) verified the shape) and the precedent written_atom_member.c, before any run: verifies (unmeasured in the tree)
#include <cerastes.h>
#include <stddef.h>
#include <stdlib.h>

struct S {
  int a[4];
  int n;
};

void zero_fill(struct S *p)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p) && p->a[0] == 0)
{
  p->a[0] = 0;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 4 && cerastes_owns(*p) && p->a[0] == 0)
  CERASTES_DECREASES(4 - i)
  while (i < 4) {
    p->a[i] = 0;
    i = i + 1;
  }
}

int use(void)
    CERASTES_ENSURES(r, r == 0)
{
  struct S s = {{1, 2, 3, 4}, 5};
  zero_fill(&s);
  return s.a[0];
}
