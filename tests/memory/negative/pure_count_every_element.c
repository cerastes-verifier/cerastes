// expect: error invariant-preserved @ L22
// mode: c23, c++20
// twin: tests/memory/pure_count.c
// Pins: 0062 item 7: the loop counts every element, not only the true ones: the invariant c == count_to(a, i) is not preserved (invariant-preserved at the clause)
// Pre-registered from record 0062 (Part D), before the code: invariant-preserved
#include <cerastes.h>

CERASTES_PURE int count_to(const bool *a, int n)
    CERASTES_REQUIRES(0 <= n && cerastes_reads_range(a, n))
    CERASTES_ENSURES(r, 0 <= r && r <= n)
    CERASTES_DECREASES(n)
{
  return n == 0 ? 0 : count_to(a, n - 1) + (a[n - 1] ? 1 : 0);
}

int count_true(const bool *a, int n)
    CERASTES_REQUIRES(0 <= n && cerastes_reads_range(a, n))
    CERASTES_ENSURES(r, cerastes_reads_range(a, n) && r == count_to(a, n))
{
  int c = 0;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && c == count_to(a, i))
  CERASTES_DECREASES(n - i)
  while (i < n) {
    c = c + 1;
    i = i + 1;
  }
  return c;
}

int count_local(void)
{
  bool a[3] = {true, false, true};
  int c = 0;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 3 && c == count_to(a, i))
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    if (a[i]) {
      c = c + 1;
    }
    i = i + 1;
  }
  CERASTES_CHECK(c == count_to(a, 3));
  return c;
}
