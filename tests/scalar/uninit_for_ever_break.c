// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0019 item 4: for (;;) assigns before it breaks, so the value is known after the loop
#include <cerastes.h>

int find_ten(int n)
    CERASTES_REQUIRES(0 <= n && n <= 2147483637)
    CERASTES_ENSURES(r, r == n + 10)
{
  int x;
  int i = 0;
  CERASTES_INVARIANT(i <= n)
  CERASTES_DECREASES(n - i)
  for (;;) {
    x = i + 10;
    if (i >= n) {
      break;
    }
    i += 1;
  }
  return x;
}
