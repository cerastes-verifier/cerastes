// expect: error postcondition @ L18
// mode: c23, c++20
// twin: tests/scalar/while_assign_condition.c
// Pins: 0020 item 3: the loop ends with i == n
#include <cerastes.h>

int reach(int n)
    CERASTES_REQUIRES(0 <= n && n < 2147483647)
    CERASTES_ENSURES(r, r == n - 1)
{
  int i = 0;
  int c;
  CERASTES_INVARIANT(i <= n)
  CERASTES_DECREASES(n - i)
  while ((c = i + 1) <= n) {
    i = c;
  }
  return i;
}
