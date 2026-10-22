// expect: error uninitialized @ L13
// mode: c23, c++20
// twin: tests/scalar/uninit_for_declaration.c
// Pins: 0019 item 1: the condition of the for loop reads the variable its init-statement declared
#include <cerastes.h>

int first(int n)
    CERASTES_REQUIRES(1 <= n && n <= 100)
{
  int total = 0;
  CERASTES_INVARIANT(0 <= total && total <= n)
  CERASTES_DECREASES(n - total)
  for (int i; total < n && i < n;) {
    i = total;
    total += 1;
  }
  return total;
}
