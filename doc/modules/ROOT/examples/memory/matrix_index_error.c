// expect: error out-of-bounds @ L11
// mode: c23, c++20
// The second index may be one past its row: a[i][j] with j equal to 3 is outside the three cells of row i, although the row i itself exists.
// Pre-registered from record 0064 items 7 and 13 (Part E), before any run: out-of-bounds at the read.
// tag::source[]
#include <cerastes.h>

int get(int a[][3], int n, int i, int j)
    CERASTES_REQUIRES(cerastes_reads_range(a, n) && 0 <= i && i < n && 0 <= j && j <= 3)
{
  return a[i][j];
}
// end::source[]
