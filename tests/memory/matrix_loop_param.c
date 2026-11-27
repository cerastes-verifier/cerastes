// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0064 items 5, 10 and 13 (Part E; audit round E2a, finding 3): loops over an ARRAY-OF-ROWS PARAMETER's range: a fill whose invariant reads `a[0][1]` (the value the loop has written to row 0, kept for the postcondition), an invariant that reads `a[i][2]` of a range read only (guarded so that the read stays inside `a[0..n)`), a loop that only reads keeps every value (by name, through a callee that reads the rows, and through one that reads a row), and a loop over a local matrix beside the parameter leaves the parameter's value. Twin: negative/matrix_loop_param_wrong.c (a wrong invariant, a read outside the range, a read with no atom, a write under `reads_range`, and a loop whose store or callee loses the value); the auditor's l07, l07d2, l09b, l09e and l10
// Pre-registered from record 0064 items 5, 10 and 13 (Part E; audit round E2a, finding 3), before any run: verifies
#include <cerastes.h>

void wrow(int *p, int n)
    CERASTES_REQUIRES(cerastes_owns_range(p, n) && n > 0)
    CERASTES_ENSURES(cerastes_owns_range(p, n))
{
  p[0] = 0;
  (void)n;
}

int rrow(int *p, int n)
    CERASTES_REQUIRES(cerastes_reads_range(p, n) && n > 0)
    CERASTES_ENSURES(r, cerastes_reads_range(p, n) && r == 0)
{
  (void)p;
  (void)n;
  return 0;
}

void wmat(int a[][3], int n)
    CERASTES_REQUIRES(cerastes_owns_range(a, n) && n > 0)
    CERASTES_ENSURES(cerastes_owns_range(a, n))
{
  a[0][0] = 0;
  (void)n;
}

int rmat(int a[][3], int n)
    CERASTES_REQUIRES(cerastes_reads_range(a, n) && n > 0)
    CERASTES_ENSURES(r, cerastes_reads_range(a, n) && r == 0)
{
  (void)n;
  return a[0][0] * 0;
}

int fill_column(int a[][3], int n)
    CERASTES_REQUIRES(cerastes_owns_range(a, n) && 1 <= n && n <= 100)
    CERASTES_ENSURES(r, cerastes_owns_range(a, n) && a[0][1] == 5)
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && (i == 0 || a[0][1] == 5))
  CERASTES_DECREASES(n - i)
  while (i < n) {
    a[i][1] = 5;
    i = i + 1;
  }
  return 0;
}

int invariant_reads_a_row([[maybe_unused]] int a[][3], int n)
    CERASTES_REQUIRES(cerastes_reads_range(a, n) && 1 <= n && n <= 100)
    CERASTES_ENSURES(r, cerastes_reads_range(a, n))
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && (i == n || a[i][2] <= 2147483647))
  CERASTES_DECREASES(n - i)
  while (i < n) {
    i = i + 1;
  }
  return 0;
}

int loop_reads_keeps(int a[][3], int n)
    CERASTES_REQUIRES(cerastes_owns_range(a, n) && 1 <= n && n <= 100 && a[0][2] == 7)
    CERASTES_ENSURES(r, cerastes_owns_range(a, n) && a[0][2] == 7)
{
  int i = 0;
  int s = 0;
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    s = a[i][1] > 0 ? 1 : 0;
    i = i + 1;
  }
  return s;
}

int loop_reading_callees_keep(int a[][3], int n)
    CERASTES_REQUIRES(cerastes_owns_range(a, n) && 1 <= n && n <= 100 && a[0][2] == 7)
    CERASTES_ENSURES(r, cerastes_owns_range(a, n) && a[0][2] == 7)
{
  int i = 0;
  int s = 0;
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    s = rmat(a, n) + rrow(a[i], 3);
    i = i + 1;
  }
  return s;
}

int local_beside_the_parameter([[maybe_unused]] int a[][3])
    CERASTES_REQUIRES(cerastes_owns_range(a, 2) && a[0][2] == 7)
    CERASTES_ENSURES(r, cerastes_owns_range(a, 2) && a[0][2] == 7)
{
  int m[2][3] = {{1, 2, 3}, {4, 5, 6}};
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 2)
  CERASTES_DECREASES(2 - i)
  while (i < 2) {
    m[i][1] = 5;
    i = i + 1;
  }
  CERASTES_CHECK(m[0][0] == 1 || m[0][0] != 1);
  return m[0][0] * 0;
}
