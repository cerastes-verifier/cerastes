// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0047 item 10's follow-up (re-audit B1, M4.5d2v): a parameter that owns its block (owns_block), a pointer local holding the address of one of its members, a loop writing through it, and the block freed after the loop
#include <cerastes.h>
#include <stdlib.h>

struct S {
  int a;
  int b;
};

void g(struct S *s, int n)
    CERASTES_REQUIRES(cerastes_owns_block(s) && 0 <= n && n <= 100)
{
  int *x = &s->b;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    *x = i;
    i += 1;
  }
  free(s);
}
