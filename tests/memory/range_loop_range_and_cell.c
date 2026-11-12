// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0060 item 11 (S44 (g) range_cell, with callers): a reads_range parameter and a reads(*q) cell parameter in one loop that reads both: the cell place carries the guard 'not inside the range', so the one function verifies for a cell inside the range (inside) and outside it (outside)
// Pre-registered from record 0060 item 11 (Part D), before the code: verifies
#include <cerastes.h>
#include <stddef.h>

int count_eq(const int *p, int n, const int *q)
    CERASTES_REQUIRES(cerastes_reads_range(p, n) && cerastes_reads(*q))
    CERASTES_ENSURES(r, cerastes_reads_range(p, n) && cerastes_reads(*q) && 0 <= r && r <= n)
{
  int i = 0;
  int c = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && 0 <= c && c <= i) CERASTES_DECREASES(n - i) while (i < n) {
    if (p[i] == *q) {
      c++;
    }
    i++;
  }
  return c;
}

int inside(void)
{
  int a[3] = {1, 2, 3};
  return count_eq(a, 3, &a[1]);
}

int outside(void)
{
  int a[3] = {1, 2, 3};
  int x = 2;
  return count_eq(a, 3, &x);
}
