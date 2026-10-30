// expect: error postcondition @ L18
// mode: c23, c++20
// twin: tests/memory/loop_atom_automatic.c
// Pins: 0040 item 5: the atom does not say what the loop wrote, so *p == n is not known after it
#include <cerastes.h>

void fill(int *p, int n)
    CERASTES_REQUIRES(cerastes_owns(*p) && 0 <= n && n <= 100)
    CERASTES_ENSURES(cerastes_owns(*p) && *p == n)
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && cerastes_owns(*p))
  CERASTES_DECREASES(n - i)
  while (i < n) {
    *p = i;
    i += 1;
  }
}
