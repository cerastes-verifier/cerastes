// expect: verified
// expect: native-ok
// mode: c23, c++20
// tag::source[]
#include <cerastes.h>

CERASTES_PURE bool same_at(int i, const int *p, const int *q, int n)
{
  return !(0 <= i && i < n) || p[i] == q[i];
}

void copy(int *dst, const int *src, int n)
    CERASTES_REQUIRES(cerastes_owns_range(dst, n) && cerastes_reads_range(src, n))
    CERASTES_ENSURES(cerastes_owns_range(dst, n) && cerastes_reads_range(src, n) && cerastes_forall(same_at, dst, src, n))
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && cerastes_forall(same_at, dst, src, i))
  CERASTES_DECREASES(n - i)
  while (i < n) {
    dst[i] = src[i];
    i = i + 1;
  }
}

int use(void)
    CERASTES_ENSURES(r, r == 6)
{
  int a[2] = {0, 0};
  int b[2] = {5, 6};
  copy(a, b, 2);
  CERASTES_CHECK(a[1] == b[1]);
  return a[1];
}
// end::source[]
