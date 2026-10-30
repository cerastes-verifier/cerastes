// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0038 item 3 (S28 F2): a cell first initialized in the loop is read after it: the invariant states the value, so the read is allowed
#include <cerastes.h>

int init_in_loop(int n)
    CERASTES_REQUIRES(1 <= n && n <= 100)
    CERASTES_ENSURES(r, r == n)
{
  int x;
  int *p = &x;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && (i == 0 || x == i))
  CERASTES_DECREASES(n - i)
  while (i < n) {
    *p = i + 1;
    i += 1;
  }
  return *p;
}
