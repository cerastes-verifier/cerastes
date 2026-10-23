// expect: error uninitialized @ L18
// mode: c23, c++20
// twin: tests/scalar/loop_condition_flag.c
// Pins: 0019 item 13: a condition without the assignment never gives c a value
#include <cerastes.h>

int drain(int n)
    CERASTES_REQUIRES(0 <= n)
    CERASTES_ENSURES(r, r == n)
{
  int c;
  int i = 0;
  CERASTES_INVARIANT(i <= n)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    i++;
  }
  return i - c;
}
