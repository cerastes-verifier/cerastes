// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0059 item 8 (0032 item 4): an array declared in the body of a loop is a fresh block each iteration, its scope exit at the body's end; the loop's range places are the arrays open at the loop (none here)
// Pre-registered from record 0059 item 8 (Part C), before the code: verifies
#include <cerastes.h>

int sum(int n)
    CERASTES_REQUIRES(0 <= n && n < 100)
    CERASTES_ENSURES(r, r == n)
{
  int s = 0;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_INVARIANT(s == i)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    int a[2] = {1, 2};
    s = s + a[0];
    i = i + 1;
  }
  return s;
}
