// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0063 item 1 (0061 item 10, 0062): a named array global as the argument of cerastes_forall and of a pure function in a check: the bound, the marked rows, the flag row and then the fact
// Pre-registered from record 0063 item 1 (Part B), before any run: verifies
#include <cerastes.h>
#include <stddef.h>

CERASTES_PURE bool nonneg(int i, const int *a, int n)
{
  return !(0 <= i && i < n) || a[i] >= 0;
}

CERASTES_PURE int count_to(const bool *a, int n)
    CERASTES_REQUIRES(0 <= n && cerastes_reads_range(a, n))
    CERASTES_ENSURES(r, 0 <= r && r <= n)
    CERASTES_DECREASES(n)
{
  return n == 0 ? 0 : count_to(a, n - 1) + (a[n - 1] ? 1 : 0);
}

static int g[4] = {1, 2, 3, 4};
static bool b[3] = {true, false, true};

int main(void)
{
  CERASTES_CHECK(cerastes_forall(nonneg, g, 4));
  CERASTES_CHECK(count_to(b, 3) <= 3);
  return g[0] + (b[0] ? 1 : 0) - 2;
}
