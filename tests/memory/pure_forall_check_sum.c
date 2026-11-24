// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0062 items 3, 7 and 8 (cut 2; audit round 2 R2-S1): an int sum with a quantified bound in its requires (cerastes_forall(small_at, a, n)): the function is checked once (its ensures by induction, the add's overflow row from the clause), and applied in a check on a local array whose elements meet the clause
// Pre-registered from record 0062 (Part B), before the code: verifies
#include <cerastes.h>
#include <stdbool.h>

CERASTES_PURE bool small_at(int i, const int *a, int n)
{
  return !(0 <= i && i < n) || (-1000 <= a[i] && a[i] <= 1000);
}
CERASTES_PURE int sum_to(const int *a, int n)
    CERASTES_REQUIRES(0 <= n && n <= 1000 && cerastes_reads_range(a, n) && cerastes_forall(small_at, a, n))
    CERASTES_ENSURES(r, -1000 * n <= r && r <= 1000 * n)
    CERASTES_DECREASES(n)
{
  return n == 0 ? 0 : sum_to(a, n - 1) + a[n - 1];
}
int main(void)
{
  int a[3] = {1, 2, 3};
  CERASTES_CHECK(sum_to(a, 3) <= 3000);
  return a[0];
}
