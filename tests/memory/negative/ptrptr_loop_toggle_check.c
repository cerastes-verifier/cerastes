// expect: error check @ L20
// mode: c23, c++20
// twin: tests/memory/ptrptr_loop_toggle.c
// Pins: 0045 item 10: after the loop the entry object's value is not known (the loop wrote through **pp): check
#include <cerastes.h>
#include <stddef.h>

void toggle(int **pp, int *a, int *b, int n)
    CERASTES_REQUIRES(cerastes_owns(*pp) && cerastes_owns(*a) && cerastes_owns(*b) && (*pp == a || *pp == b) && *a == 0 && *b == 0 && 0 <= n && n <= 100)
    CERASTES_ENSURES(cerastes_owns(*pp) && cerastes_owns(*a) && cerastes_owns(*b))
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && (*pp == a || *pp == b))
  CERASTES_DECREASES(n - i)
  while (i < n) {
    **pp = i;
    *pp = (*pp == a) ? b : a;
    i += 1;
  }
  CERASTES_CHECK(*a == 0);  // <- here
}
