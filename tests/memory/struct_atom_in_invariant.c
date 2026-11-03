// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0045 item 12 (flip of reject_struct_atom_in_invariant.c, question 99): a written atom on a struct place in a loop invariant
#include <cerastes.h>

struct S {
  int x;
  int y;
};

void walk(struct S *p, int n)
    CERASTES_REQUIRES(cerastes_owns(*p) && 0 <= n && n <= 10)
    CERASTES_ENSURES(cerastes_owns(*p))
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && cerastes_owns(*p))
  CERASTES_DECREASES(n - i)
  while (i < n) {
    p->x = i;
    i += 1;
  }
}
