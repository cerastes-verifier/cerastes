// expect: error termination @ L14
// mode: c23, c++20
// twin: tests/scalar/comma_condition.c
// Pins: 0020 item 2: the measure n does not decrease
#include <cerastes.h>

int count(int n)
    CERASTES_REQUIRES(0 <= n)
    CERASTES_ENSURES(r, r == n)
{
  int i = 0;
  int c;
  CERASTES_INVARIANT(i <= n)
  CERASTES_DECREASES(n)
  while (c = i, c < n) {
    i++;
  }
  return i;
}
