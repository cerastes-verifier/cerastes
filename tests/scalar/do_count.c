// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0014 item 5: the invariant of a do loop holds at the start of every iteration
// The invariant is not about the state after the last iteration (i == n): it is about each iteration's start.
#include <cerastes.h>

int count_up(int n)
    CERASTES_REQUIRES(1 <= n)
    CERASTES_ENSURES(r, r == n)
{
  int i = 0;
  CERASTES_INVARIANT(i < n)
  CERASTES_DECREASES(n - i)
  do {
    i += 1;
  } while (i < n);
  return i;
}
