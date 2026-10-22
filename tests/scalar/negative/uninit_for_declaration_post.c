// expect: error postcondition @ L18
// mode: c23, c++20
// twin: tests/scalar/uninit_for_declaration.c
// Pins: 0019 item 1: the loop body assigns and reads the variable, and the loop result is wrong
#include <cerastes.h>

int first(int n)
    CERASTES_REQUIRES(0 <= n && n <= 100)
    CERASTES_ENSURES(r, r == n)
{
  int total = 0;
  CERASTES_INVARIANT(0 <= total && total <= n)
  CERASTES_DECREASES(n - total)
  for (int i; total < n;) {
    i = total + 1;
    total = i;
  }
  return total + 1;
}
