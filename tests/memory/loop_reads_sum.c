// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0038 items 3 and 4: a reads(*p) parameter summed in a loop (its place keeps its value)
#include <cerastes.h>

int sum_reads(const int *p, int n)
    CERASTES_REQUIRES(cerastes_reads(*p) && 0 <= *p && *p <= 10 && 0 <= n && n <= 100)
    CERASTES_ENSURES(r, cerastes_reads(*p) && 0 <= r && r <= 1000)
{
  int s = 0;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && 0 <= s && s <= 10 * i)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    s += *p;
    i += 1;
  }
  return s;
}
