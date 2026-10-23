// expect: error postcondition @ L18
// mode: c23, c++20
// twin: tests/scalar/comma_condition.c
// Pins: 0020 item 2: the loop ends with i == n
#include <cerastes.h>

int count(int n)
    CERASTES_REQUIRES(0 <= n)
    CERASTES_ENSURES(r, r == n - 1)
{
  int i = 0;
  int c;
  CERASTES_INVARIANT(i <= n)
  CERASTES_DECREASES(n - i)
  while (c = i, c < n) {
    i++;
  }
  return i;
}
