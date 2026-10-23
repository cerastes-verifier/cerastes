// expect: rejected cerastes-self_assignment @ L13
// mode: c23, c++20
// twin: tests/scalar/self_assign_not_warned.c
// Pins: 0021 item 8 (clang -Wself-assign): a for step x = x is rejected
#include <cerastes.h>

int f(int n, int x)
    CERASTES_REQUIRES(0 <= n && n < 100)
{
  int i;
  CERASTES_INVARIANT(i <= n)
  CERASTES_DECREASES(n - i)
  for (i = 0; i < n; x = x) {
    i++;
  }
  return x + i;
}
