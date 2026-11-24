// expect: verified
// expect: native-ok
// mode: c23, c++20
// Functions over a range of elements, and a caller that passes a whole local array.
// tag::source[]
#include <cerastes.h>

int middle(const int *a, int n)
    CERASTES_REQUIRES(cerastes_reads_range(a, n) && n >= 3)
    CERASTES_ENSURES(r, cerastes_reads_range(a, n) && r == a[1])
{
  (void)n;
  return a[1];
}

void set_middle(int *a, int n, int v)
    CERASTES_REQUIRES(cerastes_owns_range(a, n) && n >= 3)
    CERASTES_ENSURES(cerastes_owns_range(a, n) && a[1] == v)
{
  (void)n;
  a[1] = v;
}

int caller(void)
    CERASTES_ENSURES(r, r == 9)
{
  int a[4] = {1, 2, 3, 4};
  set_middle(a, 4, 9);
  return middle(a, 4);
}
// end::source[]
