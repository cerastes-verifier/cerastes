// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0038 item 3: three reads parameters that may alias are three automatic places counted once (case splits at the loop head, record 0037's Consequences)
#include <cerastes.h>

int sum3(const int *a, const int *b, const int *c, int n)
    CERASTES_REQUIRES(cerastes_reads(*a) && cerastes_reads(*b) && cerastes_reads(*c) && 0 <= *a && *a <= 10 && 0 <= *b && *b <= 10 && 0 <= *c && *c <= 10 && 0 <= n && n <= 100)
    CERASTES_ENSURES(r, cerastes_reads(*a) && cerastes_reads(*b) && cerastes_reads(*c) && 0 <= r && r <= 3000)
{
  int s = 0;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && 0 <= s && s <= 30 * i)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    s += *a + *b + *c;
    i += 1;
  }
  return s;
}
