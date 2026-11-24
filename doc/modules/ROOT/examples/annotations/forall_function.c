// expect: verified
// expect: native-ok
// mode: c23, c++20
// A pure function says what holds of one element; cerastes_forall(f, ...) says it holds for every i.
// tag::pure[]
#include <cerastes.h>

CERASTES_PURE bool nonneg(int i, const int *a, int n)
{
  return !(0 <= i && i < n) || a[i] >= 0;
}
// end::pure[]

// tag::contract[]
int first(const int *a, int n)
    CERASTES_REQUIRES(cerastes_reads_range(a, n) && n > 0 && cerastes_forall(nonneg, a, n))
    CERASTES_ENSURES(r, cerastes_reads_range(a, n) && r >= 0)
{
  (void)n;
  return a[0];
}
// end::contract[]

// tag::invariant[]
int fill_then_read(void)
    CERASTES_ENSURES(r, r >= 0)
{
  int a[3];
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 3 && cerastes_forall(nonneg, a, i))
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    a[i] = i;
    i = i + 1;
  }
  return a[1];
}
// end::invariant[]

// tag::check[]
int checked(void)
    CERASTES_ENSURES(r, r == 0)
{
  int a[4] = {1, 2, 3, 4};
  CERASTES_CHECK(cerastes_forall(nonneg, a, 4));
  (void)a[0];
  return 0;
}
// end::check[]
