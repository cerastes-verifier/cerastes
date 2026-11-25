// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0062 items 3, 7 and 8 (S46 (h), cut 2 not taken): the int form: the quantified clause cerastes_forall(small_at, a, n) in the function's requires bounds every element, so the sum fits an int; the loop applies sum_to at i and the clause is one more obligation at the application (a conjunct of its guard)
// Pre-registered from record 0062 (Part D), before the code: verifies
#include <cerastes.h>

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

int sum(const int *a, int n)
    CERASTES_REQUIRES(0 <= n && n <= 1000 && cerastes_reads_range(a, n) && cerastes_forall(small_at, a, n))
    CERASTES_ENSURES(r, cerastes_reads_range(a, n) && r == sum_to(a, n))
{
  int s = 0;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && s == sum_to(a, i))
  CERASTES_DECREASES(n - i)
  while (i < n) {
    s = s + a[i];
    i = i + 1;
  }
  return s;
}
