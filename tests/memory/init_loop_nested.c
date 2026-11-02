// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0042 item 5 (audit round 2, question 89): nested loops, no marker: the outer loop reaches the cell x, the inner loop touches memory but not x; the outer row held && flag ==> range for x must survive the inner loop (it once exited 3)
#include <cerastes.h>

int nested(int *q, int n)
    CERASTES_REQUIRES(cerastes_reads(*q) && 0 <= n && n <= 10)
    CERASTES_ENSURES(cerastes_reads(*q))
{
  int x;
  int *px = &x;
  int s = 0;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && 0 <= s && s <= 1)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    if (i == 5) {
      *px = 1;
    }
    int j = 0;
    CERASTES_INVARIANT(0 <= j && j <= n && 0 <= s && s <= 1)
    CERASTES_DECREASES(n - j)
    while (j < n) {
      if (*q > 0) {
        s = 1;
      }
      j += 1;
    }
    i += 1;
  }
  return s;
}
