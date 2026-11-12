// expect: error invariant-entry @ L15
// mode: c23, c++20
// twin: tests/memory/range_loop_range_and_cell.c
// Pins: 0060 item 11: the invariant claims c < i, false on entry (c == i == 0): invariant-entry at the invariant, one error (c < i is preserved from a true start)
// Pre-registered from record 0060 item 11 (Part D), before the code
#include <cerastes.h>
#include <stddef.h>

int count_eq(const int *p, int n, const int *q)
    CERASTES_REQUIRES(cerastes_reads_range(p, n) && cerastes_reads(*q))
    CERASTES_ENSURES(r, cerastes_reads_range(p, n) && cerastes_reads(*q) && 0 <= r && r <= n)
{
  int i = 0;
  int c = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && 0 <= c && c < i) CERASTES_DECREASES(n - i) while (i < n) {
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
