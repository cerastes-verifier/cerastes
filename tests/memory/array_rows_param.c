// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0064 items 7 and 13 (Part E; the program of the retired pin array_param_reject_array_of_arrays.c is unused_param): int a[][3] is a pointer to a row; owns_range(a, n) holds n rows (their lengths, the three cells of each by two bound variables, the flags, the type's range, one liveness fraction), a[i][j] is written and read in code and read in a contract, the matrix m is passed to it (its decay, bound to a local) with the count 2, and a reads_range reader gives back the value
// Pre-registered from record 0064 items 7 and 13 (Part E), before any run: verifies
#include <cerastes.h>

void put(int a[][3], int n, int i, int j, int v)
    CERASTES_REQUIRES(cerastes_owns_range(a, n) && 0 <= i && i < n && 0 <= j && j < 3)
    CERASTES_ENSURES(cerastes_owns_range(a, n) && a[i][j] == v)
{
  (void)n;
  a[i][j] = v;
}

int get(int a[][3], int n, int i, int j)
    CERASTES_REQUIRES(cerastes_reads_range(a, n) && 0 <= i && i < n && 0 <= j && j < 3)
    CERASTES_ENSURES(r, cerastes_reads_range(a, n) && r == a[i][j])
{
  (void)n;
  return a[i][j];
}

int use(int i, int j)
    CERASTES_REQUIRES(0 <= i && i < 2 && 0 <= j && j < 3)
    CERASTES_ENSURES(r, r == 9)
{
  int m[2][3] = {{0, 0, 0}, {0, 0, 0}};
  put(m, 2, i, j, 9);
  return get(m, 2, i, j);
}

int unused_param([[maybe_unused]] int a[][3])
{
  return 0;
}
