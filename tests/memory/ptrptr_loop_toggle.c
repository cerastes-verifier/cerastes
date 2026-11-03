// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0045 item 10 (question 105): a loop that moves *pp between two owned objects and writes through **pp in each iteration
#include <cerastes.h>
#include <stddef.h>

void toggle(int **pp, int *a, int *b, int n)
    CERASTES_REQUIRES(cerastes_owns(*pp) && cerastes_owns(*a) && cerastes_owns(*b) && (*pp == a || *pp == b) && 0 <= n && n <= 100)
    CERASTES_ENSURES(cerastes_owns(*pp) && cerastes_owns(*a) && cerastes_owns(*b) && (*pp == a || *pp == b))
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && (*pp == a || *pp == b))
  CERASTES_DECREASES(n - i)
  while (i < n) {
    **pp = i;
    *pp = (*pp == a) ? b : a;
    i += 1;
  }
}
