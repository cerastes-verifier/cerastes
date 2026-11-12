// expect: verified
// expect: native-ok
// mode: c23, c++20
// tag::source[]
#include <cerastes.h>

int larger(const int *p, int n)
    CERASTES_REQUIRES(cerastes_reads_range(p, n) && n >= 2)
    CERASTES_ENSURES(cerastes_reads_range(p, n))
    CERASTES_ENSURES(r, (r == p[0] || r == p[1]) && r >= p[0] && r >= p[1])
{
  (void)n;
  return p[0] > p[1] ? p[0] : p[1];
}

int caller(void)
    CERASTES_ENSURES(r, r == 9)
{
  int a[3] = {4, 9, 2};
  int m = larger(a, 3);
  CERASTES_CHECK(a[0] == 4 && a[2] == 2);
  return m;
}
// end::source[]
