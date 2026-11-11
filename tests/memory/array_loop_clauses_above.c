// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0059 item 8: a loop over an array whose clauses stand on lines of their own above the while: it verifies in both modes (the twin keeps an element before the loop)
// Pre-registered from record 0059 item 8 (Part C): verifies
#include <cerastes.h>

int run(int n)
    CERASTES_REQUIRES(0 <= n && n <= 3)
    CERASTES_ENSURES(r, r == n)
{
  int a[3] = {1, 2, 3};
  int i = 0;
  int s = 0;
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_INVARIANT(s == i)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    s = s + a[0] - a[0] + 1;
    i = i + 1;
  }
  return s;
}
