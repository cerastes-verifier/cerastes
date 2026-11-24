// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0062 item 7 (S46 (f)): an application in a check over an owns_range and a reads_range parameter (the rows hold under the parameters' atoms)
// Pre-registered from record 0062 (Part B), before the code: verifies. Edited before any run (audit round 2, R2-S3): the check observes the value, so a wrong slot or swapped arguments fails; its off-by-one twin is the negative of the same name with _value_off_by_one (the requires gives n >= 1 so that a[0] is framed)
#include <cerastes.h>

CERASTES_PURE long long sum_to(const int *a, int n)
    CERASTES_REQUIRES(0 <= n && cerastes_reads_range(a, n))
    CERASTES_ENSURES(r, -2147483648LL * n <= r && r <= 2147483647LL * n)
    CERASTES_DECREASES(n)
{
  return n == 0 ? 0 : sum_to(a, n - 1) + a[n - 1];
}

int over_owned(int *a, int n)
    CERASTES_REQUIRES(1 <= n && cerastes_owns_range(a, n))
    CERASTES_ENSURES(r, cerastes_owns_range(a, n))
{
  CERASTES_CHECK(sum_to(a, n) == sum_to(a, n));
  CERASTES_CHECK(sum_to(a, 1) == a[0]);
  (void)a;
  return n;
}

int over_read(const int *a, int n)
    CERASTES_REQUIRES(1 <= n && cerastes_reads_range(a, n))
    CERASTES_ENSURES(r, cerastes_reads_range(a, n))
{
  CERASTES_CHECK(sum_to(a, n) == sum_to(a, n));
  CERASTES_CHECK(sum_to(a, 1) == a[0]);
  (void)a;
  return n;
}
