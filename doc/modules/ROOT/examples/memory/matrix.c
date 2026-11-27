// expect: verified
// expect: native-ok
// mode: c23, c++20
// A matrix passed to a function: a row parameter is a pointer to the first of n rows of three cells, `owns_range(a, n)` owns every cell of those rows, and a local matrix is passed to it whole.
// Pre-registered from record 0064 items 7, 8 and 13 (Part E), before any run: verifies.
// tag::source[]
#include <cerastes.h>

void put(int a[][3], [[maybe_unused]] int n, int i, int j, int v)
    CERASTES_REQUIRES(cerastes_owns_range(a, n) && 0 <= i && i < n && 0 <= j && j < 3)
    CERASTES_ENSURES(cerastes_owns_range(a, n) && a[i][j] == v)
{
  a[i][j] = v;
}

int get(int a[][3], [[maybe_unused]] int n, int i, int j)
    CERASTES_REQUIRES(cerastes_reads_range(a, n) && 0 <= i && i < n && 0 <= j && j < 3)
    CERASTES_ENSURES(r, cerastes_reads_range(a, n) && r == a[i][j])
{
  return a[i][j];
}

int demo(void)
    CERASTES_ENSURES(r, r == 9)
{
  int m[2][3] = {{0, 0, 0}, {0, 0, 0}};
  put(m, 2, 1, 2, 9);
  return get(m, 2, 1, 2);
}
// end::source[]
